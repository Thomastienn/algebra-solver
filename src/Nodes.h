#pragma once
#include "Token.h"

enum class NodeType {
    Atom,
    BinaryOp,
    UnaryOp,
};

class ASTNode {
private:
    Token token;
    NodeType type;

public:
    ASTNode() : token(Token(UNKNOWN, "")) {}
    ASTNode(Token token, NodeType type) : token(token), type(type) {}
    virtual ~ASTNode() = default;

    Token getToken() const { return token; }
    NodeType getNodeType() const { return type; }

    void setToken(const Token& newToken) { token = newToken; }
    void setType(const NodeType& newType) { type = newType; }
};

class AtomNode : public ASTNode {
public:
    AtomNode(Token token) : ASTNode(token, NodeType::Atom) {}
};

class BinaryOpNode : public ASTNode {
private:
    ASTNode *left;
    ASTNode *right;

public:
    BinaryOpNode(Token op, ASTNode *left, ASTNode *right) : ASTNode(op, NodeType::BinaryOp), left(left), right(right) {
        if (!Token::isOperation(op.getType())) {
            throw std::runtime_error("BinaryOpNode requires an operation token");
        }
    }

    ~BinaryOpNode() {
        delete left;
        delete right;
    }

    ASTNode *getLeft() const { return left; }
    ASTNode *getRight() const { return right; }
};

class UnaryOpNode : public ASTNode {
private:
    ASTNode *operand;

public:
    UnaryOpNode(Token op, ASTNode *operand) : ASTNode(op, NodeType::UnaryOp), operand(operand) {
        if (!Token::isUnaryOperation(op.getType())) {
            throw std::runtime_error("UnaryOpNode requires a unary operation token");
        }
    }

    ~UnaryOpNode() { delete operand; }

    ASTNode *getOperand() const { return operand; }
    void setOperand(ASTNode* newOperand) {
        delete operand;
        operand = newOperand;
    }
};

inline std::ostream &operator<<(std::ostream &os, const ASTNode &node) {
    switch (node.getNodeType()) {
    case NodeType::Atom:
        os << node.getToken().getValue();
        break;
    case NodeType::BinaryOp: {
        const BinaryOpNode &n = static_cast<const BinaryOpNode &>(node);
        os << "(";
        os << n.getToken().getValue() << " " << *(n.getLeft()) << " " << *(n.getRight());
        os << ")";
        break;
    }

    case NodeType::UnaryOp: {
        const UnaryOpNode &n = static_cast<const UnaryOpNode &>(node);
        os << "(" << n.getToken().getValue() << " " << *n.getOperand() << ")";
        break;
    }

    default:
        throw std::runtime_error("Unknown node type");
    }
    return os;
}
