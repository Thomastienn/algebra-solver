#pragma once
#include "Token.h"
#include <memory>

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

    virtual std::string toString() = 0;
    virtual std::unique_ptr<ASTNode> clone() const = 0;
};

class AtomNode : public ASTNode {
public:
    AtomNode(Token token) : ASTNode(token, NodeType::Atom) {}

    std::string toString() override {
        return getToken().getValue();
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<AtomNode>(getToken());
    }
};

class BinaryOpNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

public:
    BinaryOpNode(Token op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right) 
        : ASTNode(op, NodeType::BinaryOp), left(std::move(left)), right(std::move(right)) {
        if (!Token::isOperation(op.getType())) {
            throw std::runtime_error("BinaryOpNode requires an operation token");
        }
    }

    ASTNode *getLeft() const { return left.get(); }
    ASTNode *getRight() const { return right.get(); }

    void setLeft(std::unique_ptr<ASTNode> newLeft) {
        left = std::move(newLeft);
    }
    void setRight(std::unique_ptr<ASTNode> newRight) {
        right = std::move(newRight);
    }

    std::unique_ptr<ASTNode>& getLeftRef() { return left; }
    std::unique_ptr<ASTNode>& getRightRef() { return right; }

    std::string toString() override {
        return "(" + left->toString() + 
                " " + getToken().getValue() + " " + 
                right->toString() + ")";
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<BinaryOpNode>(
            getToken(), 
            left->clone(), 
            right->clone()
        );
    }
};

class UnaryOpNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> operand;

public:
    UnaryOpNode(Token op, std::unique_ptr<ASTNode> operand) 
        : ASTNode(op, NodeType::UnaryOp), operand(std::move(operand)) {
        if (!Token::isUnaryOperation(op.getType())) {
            throw std::runtime_error("UnaryOpNode requires a unary operation token");
        }
    }

    ASTNode *getOperand() const { return operand.get(); }
    std::unique_ptr<ASTNode>& getOperandRef() { return operand; }
    
    void setOperand(std::unique_ptr<ASTNode> newOperand) {
        operand = std::move(newOperand);
    }

    std::string toString() override {
        return getToken().getValue() + operand->toString();
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<UnaryOpNode>(
            getToken(), 
            operand->clone()
        );
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
