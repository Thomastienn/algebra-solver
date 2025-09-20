#include "EquationSolver.h"
#include <cassert>
#include <cmath>
#include <memory>

void EquationSolver::addEquation(std::unique_ptr<ASTNode> equation) { 
    equations.push_back(std::move(equation)); 
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
    Simplifier::simplify(newEquation->getLeftRef());
    return newEquation;
}

std::unordered_set<Token> EquationSolver::dependencies(const Token &variable, std::unique_ptr<ASTNode> equation) {
    std::unordered_set<Token> deps;
    if (equation->getNodeType() == NodeType::Atom) {
        if (equation->getToken() == TokenType::VARIABLE &&
            equation->getToken() != variable) {
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


bool EquationSolver::isIsolated(std::unique_ptr<ASTNode>& node, const std::string& variable){
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        bool varAtom = atomNode->getToken().getType() == TokenType::VARIABLE && 
                atomNode->getToken().getValue() == variable;
        return varAtom;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return EquationSolver::isIsolated(unaryNode->getOperandRef(), variable);
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        return EquationSolver::isIsolated(static_cast<BinaryOpNode *>(node.get())->getLeftRef(), variable) && 
               EquationSolver::isIsolated(static_cast<BinaryOpNode *>(node.get())->getRightRef(), variable);
    }
    return false;   
}

bool EquationSolver::containsVariable(std::unique_ptr<ASTNode>& node, const std::string& variable) {
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        return atomNode->getToken().getType() == TokenType::VARIABLE && 
               atomNode->getToken().getValue() == variable;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return EquationSolver::containsVariable(unaryNode->getOperandRef(), variable);
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        return EquationSolver::containsVariable(static_cast<BinaryOpNode *>(node.get())->getLeftRef(), variable) || 
               EquationSolver::containsVariable(static_cast<BinaryOpNode *>(node.get())->getRightRef(), variable);
    }
    return false;   
}

std::unique_ptr<ASTNode> EquationSolver::isolateVariable(std::unique_ptr<ASTNode> equation, const std::string &variable) {
    return nullptr;
}
