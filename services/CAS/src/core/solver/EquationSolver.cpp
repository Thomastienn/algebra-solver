#include "EquationSolver.h"
#include <iostream>
#include <queue>
#include <climits>
#include "../../utils/ASTUtils.h"
#include "../../utils/Debug.h"
#include "../../utils/Config.h"

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


// TODO: This current approach is brute forcing with heuristic paths
// We will implement further optimizations like Gauss elimination later
std::unique_ptr<ASTNode> EquationSolver::solve(
    std::vector<std::unique_ptr<ASTNode>>& equations,
    const std::string &variable
) {
    // Normalize and simplify equations
    std::priority_queue<EquationEntry> queue;
    std::unordered_map<std::string, std::vector<EquationEntry>> varToEquation;
    int i = 0;
    for (auto &eq : equations) {
        std::unique_ptr<ASTNode> normalized = EquationSolver::normalizeEquation(std::move(eq));
        this->simplifier.simplify(normalized);
        
        std::unordered_set<std::string> vars = EquationSolver::extractVariables(normalized);
        int numVars = ASTUtils::countVariableOccurrences(normalized);
        int distinctVars = ASTUtils::countDistinctVariables(normalized);

        bool containsVar = ASTUtils::containsVariable(normalized, variable);
        EquationEntry entry(std::move(normalized), vars, numVars, distinctVars);

        // Build graph connections
        // Eq 1: a = b + c (variables: a,b,c)
        // Eq 2: b = 2 * d (variables: b,d)
        // Eq 1 and 2 are connected because they share variable b
        for (const std::string &var : vars) {
            varToEquation[var].push_back(entry.clone());
        }
        if (containsVar) {
            queue.push(std::move(entry));
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
    // For now, the strategy is prioritize reducing the number of dependencies

    int iterations = 0;
    std::unordered_set<std::string> visited;
    int bestDistinctVars = INT_MAX;
    int iterationsSinceImprovement = 0;
    
    while (!queue.empty()){
        iterations++;
        if (iterations > Config::MAX_ITERATIONS_CONVERGE_SOLVE) {
            std::cerr << "Max iterations reached in EquationSolver::solve" << std::endl;
            return nullptr;
        }
        EquationEntry entry = queue.top().clone();
        queue.pop();

        // Already did this one
        std::string eqStr = entry.equation->toString();
        if (visited.count(eqStr) > 0) {
            continue;
        }
        visited.insert(eqStr);

        // dbg(entry.equation->toString(), entry.vars, entry.numVariables, entry.distinctVariables);

        // Did improve distinct variable count
        if (entry.distinctVariables < bestDistinctVars) {
            bestDistinctVars = entry.distinctVariables;
            iterationsSinceImprovement = 0;
        } else {
            // Don't count as "no improvement" if we're processing a 2-variable equation
            // that contains the target variable, these are promising
            if (!(entry.distinctVariables == 2 && entry.vars.count(variable) > 0)) {
                iterationsSinceImprovement++;
                if (iterationsSinceImprovement > Config::MAX_ITERATIONS_WITHOUT_IMPROVEMENT) {
                    std::cerr << "Stuck at " << bestDistinctVars << " variables after " 
                              << Config::MAX_ITERATIONS_WITHOUT_IMPROVEMENT << " iterations without improvement" << std::endl;
                    return nullptr;
                }
            }
        }

        // No more dependencies, final result - but only if it's the variable we're solving for!
        if (entry.numVariables == 1 && entry.vars.count(variable) == 1) {
            std::unique_ptr<ASTNode> isolated = entry.equation->clone();
            // dbg("Initial", isolated->toString());
            this->isolator.isolateVariable(isolated, variable);
            // dbg("Isolated", isolated->toString());
            this->simplifier.simplify(isolated);
            // dbg("Simplify", isolated->toString());
            return isolated;
        }
        
        // If we have a 1-variable equation but it's not our target:
        // Still useful! Isolate it and use it to create new substituted equations
        if (entry.numVariables == 1 && entry.vars.count(variable) == 0) {
            // Get the variable name (the only one in the set)
            std::string solvedVar = *entry.vars.begin();
            
            // Isolate this variable to get its value
            std::unique_ptr<ASTNode> isolated = entry.equation->clone();
            this->isolator.isolateVariable(isolated, solvedVar);
            this->simplifier.simplify(isolated);
            
            // Extract the value (right side of assignment)
            if (isolated->getNodeType() == NodeType::BinaryOp) {
                BinaryOpNode* assignNode = static_cast<BinaryOpNode*>(isolated.get());
                std::unique_ptr<ASTNode> solvedValue = assignNode->getRightRef()->clone();
                
                // Now substitute this into ALL related equations that contain this variable
                if (varToEquation.find(solvedVar) != varToEquation.end()) {
                    std::vector<EquationEntry>& relatedEqs = varToEquation.at(solvedVar);
                    for (EquationEntry& relatedEq : relatedEqs) {
                        // Skip if it's the same equation or if it doesn't help us get to target
                        if (relatedEq.equation->toString() == entry.equation->toString()) {
                            continue;
                        }
                        if (relatedEq.vars.count(variable) == 0) {
                            // Doesn't contain target variable, not useful
                            continue;
                        }
                        
                        // Create new entry with substitution
                        EquationEntry newEntry = relatedEq.clone();
                        EquationSolver::subsituteVariable(newEntry.equation, solvedVar, solvedValue->clone());
                        this->simplifier.simplify(newEntry.equation);
                        
                        int newNumVars = ASTUtils::countVariableOccurrences(newEntry.equation);
                        int newDistinctVars = ASTUtils::countDistinctVariables(newEntry.equation);
                        
                        // Only add if it reduces complexity
                        if (newDistinctVars < relatedEq.distinctVariables) {
                            newEntry.numVariables = newNumVars;
                            newEntry.distinctVariables = newDistinctVars;
                            newEntry.vars = EquationSolver::extractVariables(newEntry.equation);
                            
                            // Add to varToEquation map
                            for (const std::string &v : newEntry.vars) {
                                varToEquation[v].push_back(newEntry.clone());
                            }
                            
                            queue.push(std::move(newEntry));
                        }
                    }
                }
            }
            
            // Continue searching for target variable
            continue;
        }

        std::unordered_set<std::string> varsToProcess = entry.vars;
        for (const std::string& var: varsToProcess){
            // dbg("Processing variable", var);
            // Do not replace the variable we want to solve
            if (var == variable) {
                continue;
            }

            // Already use this variable
            if (entry.varToIsolatedEquation.find(var) != entry.varToIsolatedEquation.end()) {
                // Replace this var with the already isolated equation
                std::unique_ptr<ASTNode> substitution = entry.varToIsolatedEquation.at(var)->clone();
                EquationSolver::subsituteVariable(entry.equation, var, std::move(substitution));
                this->simplifier.simplify(entry.equation);
                entry.numVariables = ASTUtils::countVariableOccurrences(entry.equation);
                entry.distinctVariables = ASTUtils::countDistinctVariables(entry.equation);
                entry.vars = EquationSolver::extractVariables(entry.equation);
                queue.push(std::move(entry));
                break;
            }
            // No equation to derive this variable
            if (varToEquation.find(var) == varToEquation.end()) {
                throw std::runtime_error("This variable cannot be derived: " + var);
            }
            std::vector<EquationEntry>& relatedEqs = varToEquation.at(var);
            for (EquationEntry &relatedEq : relatedEqs) {
                // Do not use the same equation to substitute
                if (relatedEq.equation->toString() == entry.equation->toString()) {
                    // dbg("Skipping same equation");
                    continue;
                }
                // dbg(var, relatedEq.equation->toString());
                
                EquationEntry newEntry = entry.clone();

                std::unique_ptr<ASTNode> isolated = relatedEq.equation->clone();
                this->isolator.isolateVariable(isolated, var);
                // dbg("Isolated:", isolated->toString());
                this->simplifier.simplify(isolated);
                // dbg("Simplified:", isolated->toString());

                if (isolated->getNodeType() != NodeType::BinaryOp) {
                    throw std::runtime_error("Isolated equation is not a binary operation");
                }
                BinaryOpNode* assignNode = static_cast<BinaryOpNode *>(isolated.get());

                EquationSolver::subsituteVariable(newEntry.equation, var, assignNode->getRightRef()->clone());
                // dbg("After substitution:", newEntry.equation->toString());
                this->simplifier.simplify(newEntry.equation);
                // dbg("After simplification:", newEntry.equation->toString());

                int newNumVariables = ASTUtils::countVariableOccurrences(newEntry.equation);
                int newDistinctVariables = ASTUtils::countDistinctVariables(newEntry.equation);

                if (((float)newDistinctVariables / entry.distinctVariables) > Config::LIMIT_RATIO_NEW_DISTINCT_VARS) {
                    // dbg("Skipping, more variables");
                    continue;
                }

                newEntry.numVariables = newNumVariables;
                newEntry.distinctVariables = newDistinctVariables;
                newEntry.vars = EquationSolver::extractVariables(newEntry.equation);

                // Add this derived equation to varToEquation so it can be used in future substitutions
                for (const std::string &v : newEntry.vars) {
                    varToEquation[v].push_back(newEntry.clone());
                }
                
                newEntry.varToIsolatedEquation[var] = isolated->clone();
                
                // dbg(newEntry.equation->toString(), newEntry.numVariables);
                queue.push(std::move(newEntry));
            }
        }

    }

    return nullptr;
}
