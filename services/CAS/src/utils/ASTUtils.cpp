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
