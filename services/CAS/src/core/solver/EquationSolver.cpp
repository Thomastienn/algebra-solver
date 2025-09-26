#include "EquationSolver.h"
#include <iostream>
#include <queue>
#include "../../utils/ASTUtils.h"
#include "../../utils/Debug.h"

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

std::unordered_set<Token> EquationSolver::dependencies(const std::string &variable, std::unique_ptr<ASTNode> equation) {
    std::unordered_set<Token> deps;
    if (equation->getNodeType() == NodeType::Atom) {
        if (equation->getToken() == TokenType::VARIABLE &&
            equation->getToken().getValue() != variable) {
            deps.insert(equation->getToken());
        }
    } else if (equation->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(equation.get());
        std::unordered_set<Token> leftDeps = EquationSolver::dependencies(
            variable, 
            std::move(binaryNode->getLeftRef())
        );
        std::unordered_set<Token> rightDeps = EquationSolver::dependencies(
            variable, 
            std::move(binaryNode->getRightRef())
        );
        deps.merge(leftDeps);
        deps.merge(rightDeps);
    } else if (equation->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(equation.get());
        std::unordered_set<Token> operandDeps = EquationSolver::dependencies(
            variable, 
            std::move(unaryNode->getOperandRef())
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
    std::vector<EquationEntry> entries;
    std::queue<int> queue;
    std::unordered_map<std::string, std::vector<int>> varToEquation;
    int i = 0;
    for (auto &eq : equations) {
        std::unique_ptr<ASTNode> normalized = EquationSolver::normalizeEquation(std::move(eq));
        this->simplifier.simplify(normalized);
        entries.emplace_back(
            std::move(normalized), 
            i
        );
        if (ASTUtils::containsVariable(entries.back().equation, variable)) {
            queue.push(i);
        }

        // Build graph connections
        // Eq 1: a = b + c (variables: a,b,c)
        // Eq 2: b = 2 * d (variables: b,d)
        // Eq 1 and 2 are connected because they share variable b
        std::unordered_set<std::string> vars = EquationSolver::extractVariables(entries.back().equation);
        for (const std::string &var : vars) {
            if (varToEquation.find(var) != varToEquation.end()) {
                varToEquation[var].push_back(i);
            } else {
                varToEquation[var] = {i};
            }
        }

        i++;
    }

    for (const auto& [var, eqs] : varToEquation) {
        dbg(var, eqs);
        for (const auto& eq : eqs) {
            // dbg(entries[eq].equation->toString());
            std::unique_ptr<ASTNode> clonedEq = entries[eq].equation->clone();
            this->isolator.isolateVariable(clonedEq, var);
            this->simplifier.simplify(clonedEq);
            dbg(clonedEq->toString());
        }
    }
    
    // Start with equation contains the variable
    // while (!queue.empty()){
    //     EquationEntry* entry = queue.front();
    //     queue.pop();
    //
    //     // WARNING: There could be a loop of dependencies
    //
    //     // Try to subsitute other variables in the equation
    // }


    return nullptr;
}
