#include "ASTUtils.h"

bool ASTUtils::containsVariable(std::unique_ptr<ASTNode>& node, const std::string& variable) {
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        return atomNode->getToken().getType() == TokenType::VARIABLE && 
               atomNode->getToken().getValue() == variable;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return ASTUtils::containsVariable(unaryNode->getOperandRef(), variable);
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        return ASTUtils::containsVariable(static_cast<BinaryOpNode *>(node.get())->getLeftRef(), variable) || 
               ASTUtils::containsVariable(static_cast<BinaryOpNode *>(node.get())->getRightRef(), variable);
    }
    return false;   
}

int ASTUtils::countVariableOccurrences(std::unique_ptr<ASTNode>& node) {
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        return atomNode->getToken().getType() == TokenType::VARIABLE ? 1 : 0;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return ASTUtils::countVariableOccurrences(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        return ASTUtils::countVariableOccurrences(binaryNode->getLeftRef()) + 
               ASTUtils::countVariableOccurrences(binaryNode->getRightRef());
    }
    return 0;   
}
