#include "EquationSolver.h"
#include <iostream>
#include <queue>
#include "../../utils/ASTUtils.h"
#include "../../utils/Debug.h"

void EquationSolver::subsituteVariable(
    std::unique_ptr<ASTNode>& equation,
    const std::string &variable,
    std::unique_ptr<ASTNode> substitution
) {
    if (equation->getNodeType() == NodeType::Atom) {
        if (equation->getToken() == TokenType::VARIABLE &&
            equation->getToken().getValue() == variable) {
            equation = substitution->clone();
        }
    } else if (equation->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(equation.get());
        EquationSolver::subsituteVariable(binaryNode->getLeftRef(), variable, substitution->clone());
        EquationSolver::subsituteVariable(binaryNode->getRightRef(), variable, std::move(substitution));
    } else if (equation->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(equation.get());
        EquationSolver::subsituteVariable(unaryNode->getOperandRef(), variable, std::move(substitution));
    }
}

std::unordered_set<std::string> EquationSolver::extractVariables(std::unique_ptr<ASTNode>& node) {
    std::unordered_set<std::string> vars;
    if (node->getNodeType() == NodeType::Atom) {
        if (node->getToken() == TokenType::VARIABLE) {
            vars.insert(node->getToken().getValue());
        }
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        std::unordered_set<std::string> leftVars = EquationSolver::extractVariables(binaryNode->getLeftRef());
        std::unordered_set<std::string> rightVars = EquationSolver::extractVariables(binaryNode->getRightRef());
        vars.merge(leftVars);
        vars.merge(rightVars);
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        std::unordered_set<std::string> operandVars = EquationSolver::extractVariables(unaryNode->getOperandRef());
        vars.merge(operandVars);
    }

    return vars;
}

void EquationSolver::reorderConstants(std::unique_ptr<ASTNode>& node) {
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        TokenType opType = binaryNode->getToken().getType();

        if (opType == TokenType::MULTIPLY) {
            ASTNode *left = binaryNode->getLeft();
            ASTNode *right = binaryNode->getRight();
            if (!left || !right) {
                std::cerr << "Corrupted child detected!" << std::endl;
                return;
            }

            bool leftIsConst = left->getNodeType() == NodeType::Atom && 
                static_cast<AtomNode *>(left)->getToken().getType() == TokenType::NUMBER;
            bool rightIsConst = right->getNodeType() == NodeType::Atom && 
                static_cast<AtomNode *>(right)->getToken().getType() == TokenType::NUMBER;

            if (!leftIsConst && rightIsConst) {
                std::swap(binaryNode->getLeftRef(), binaryNode->getRightRef());
            }
        }
        reorderConstants(binaryNode->getLeftRef());
        reorderConstants(binaryNode->getRightRef());
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        reorderConstants(unaryNode->getOperandRef());
    }
}

std::unique_ptr<ASTNode> EquationSolver::normalizeEquation(std::unique_ptr<ASTNode> equation) {
    if (equation->getNodeType() != NodeType::BinaryOp || equation->getToken() != TokenType::ASSIGN) {
        throw std::runtime_error("Equation must be an assignment (LHS = RHS)");
    }

    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation.get());

    auto lhs = std::move(assignNode->getLeftRef());
    auto rhs = std::move(assignNode->getRightRef());

    auto minusToken = Token(TokenType::MINUS, "-");
    auto zeroNode = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));

    auto newLHS = std::make_unique<BinaryOpNode>(minusToken, std::move(lhs), std::move(rhs));
    auto newEquation = std::make_unique<BinaryOpNode>(
        Token(TokenType::ASSIGN, "="), 
        std::move(newLHS), 
        std::move(zeroNode)
    );

    std::unique_ptr<ASTNode> baseEquation = std::move(newEquation);
    EquationSolver::reorderConstants(baseEquation);
    return baseEquation;
}

std::unordered_set<std::string> EquationSolver::dependencies(const std::string &variable, std::unique_ptr<ASTNode>& equation) {
    std::unordered_set<std::string> deps;
    if (equation->getNodeType() == NodeType::Atom) {
        if (equation->getToken() == TokenType::VARIABLE &&
            equation->getToken().getValue() != variable) {
            deps.insert(equation->getToken().getValue());
        }
    } else if (equation->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(equation.get());
        std::unordered_set<std::string> leftDeps = EquationSolver::dependencies(
            variable, 
            binaryNode->getLeftRef()
        );
        std::unordered_set<std::string> rightDeps = EquationSolver::dependencies(
            variable, 
            binaryNode->getRightRef()
        );
        deps.merge(leftDeps);
        deps.merge(rightDeps);
    } else if (equation->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(equation.get());
        std::unordered_set<std::string> operandDeps = EquationSolver::dependencies(
            variable, 
            unaryNode->getOperandRef()
        );
        deps.merge(operandDeps);
    }

    return deps;
}


std::unique_ptr<ASTNode> EquationSolver::solve(
    std::vector<std::unique_ptr<ASTNode>>& equations,
    const std::string &variable
) {
    // Normalize and simplify equations
    std::queue<EquationEntry> queue;
    std::unordered_map<std::string, std::vector<EquationEntry>> varToEquation;
    int i = 0;
    for (auto &eq : equations) {
        std::unique_ptr<ASTNode> normalized = EquationSolver::normalizeEquation(std::move(eq));
        this->simplifier.simplify(normalized);
        
        std::unordered_set<std::string> vars = EquationSolver::extractVariables(normalized);
        std::unordered_set<std::string> needSolveVars = EquationSolver::dependencies(variable, normalized);

        EquationEntry entry(std::move(normalized), needSolveVars);
        if (ASTUtils::containsVariable(normalized, variable)) {
            queue.push(entry);
        }

        // Build graph connections
        // Eq 1: a = b + c (variables: a,b,c)
        // Eq 2: b = 2 * d (variables: b,d)
        // Eq 1 and 2 are connected because they share variable b
        for (const std::string &var : vars) {
            if (varToEquation.find(var) != varToEquation.end()) {
                varToEquation[var].push_back(entry.clone());
            } else {
                varToEquation[var] = {entry.clone()};
            }
        }

        i++;
    }

    // for (const auto& [var, eqs] : varToEquation) {
    //     dbg(var, eqs);
    //     for (const auto& eq : eqs) {
    //         // dbg(entries[eq].equation->toString());
    //         std::unique_ptr<ASTNode> clonedEq = entries[eq].equation->clone();
    //         this->isolator.isolateVariable(clonedEq, var);
    //         this->simplifier.simplify(clonedEq);
    //         dbg(clonedEq->toString());
    //     }
    // }
    
    // Start with equation contains the variable
    // TODO: Do heuristic search in the future
    // Instead of BSF, use A* or Dijkstra to find the shortest path

    while (!queue.empty()){
        EquationEntry entry = queue.front().clone();
        queue.pop();

        // No more dependencies, final result
        if (entry.dependencies.empty()) {
            // Isolate the variable
            std::unique_ptr<ASTNode> isolated = entry.equation->clone();
            this->isolator.isolateVariable(isolated, variable);
            this->simplifier.simplify(isolated);
            return isolated;
        }

        // WARNING: There could be a loop of dependencies
        for (std::string dep: entry.dependencies){
            if (varToEquation.find(dep) == varToEquation.end()) {
                continue;
            }
            std::vector<EquationEntry> relatedEqs = varToEquation[dep];
            for (EquationEntry &relatedEq : relatedEqs) {
                // TODO: Skip self
                std::unique_ptr<ASTNode> isolated = relatedEq.equation->clone();
                this->isolator.isolateVariable(isolated, dep);
                this->simplifier.simplify(isolated);

                EquationSolver::subsituteVariable(entry.equation, dep, std::move(isolated));
                this->simplifier.simplify(entry.equation);

                queue.push(entry.clone());
            }
        }

    }


    return nullptr;
}
