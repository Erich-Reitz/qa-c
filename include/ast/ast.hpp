#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "AstNode.hpp"
#include "DataType.hpp"
#include "Stmt.hpp"
#include "../parser/st.hpp"

namespace ast {

[[nodiscard]] auto is_arithmetic(BinOpKind kind) -> bool;
[[nodiscard]] auto is_comparison(BinOpKind kind) -> bool;

enum class SelectionKind { If };

struct FrameParam {
    std::string name;
    DataType type;
};

struct ConstIntAstNode : public AstNode {
   public:
    int value;

    explicit ConstIntAstNode(int value) : value(value) {}

    [[nodiscard]] auto toString() const -> std::string override { return std::to_string(value); }
};

struct ReturnAstNode : public AstNode {
   public:
    Stmt expr;

    explicit ReturnAstNode(Stmt expr) : expr(std::move(expr)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        return "return " + expr.toString();
    }
};

struct FrameAstNode : public AstNode {
   public:
    std::string name;
    std::vector<BodyNode> body;
    std::vector<FrameParam> params;

    FrameAstNode(const std::string& p_name, std::vector<BodyNode> p_body,
                 std::vector<FrameParam> p_params)
        : name(p_name), body(std::move(p_body)), params(std::move(p_params)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        auto result = "fn " + name + "(";
        for (const auto& param : params) {
            result += param.name + ", ";
        }
        result += ") {\n";
        for (const auto& node : body) {
            result += node.toString() + "\n";
        }
        result += "}";
        return result;
    }
};

struct MoveAstNode : public AstNode {
   public:
    Stmt lhs;
    std::optional<Stmt> rhs;

    MoveAstNode(Stmt lhs, Stmt rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    MoveAstNode(Stmt lhs, std::optional<Stmt>) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    [[nodiscard]] auto toString() const -> std::string override;
};

struct BinaryOpAstNode : public AstNode {
   public:
    Stmt lhs;
    Stmt rhs;
    BinOpKind kind;

    BinaryOpAstNode(Stmt lhs, Stmt rhs, BinOpKind kind)
        : lhs(std::move(lhs)), rhs(std::move(rhs)), kind(kind) {}

    const BinOpKind* get_bin_op() const override { return &kind; }

    [[nodiscard]] auto toString() const -> std::string override {
        return lhs.toString() + " " + bin_op_to_string(kind) + " " + rhs.toString();
    }
};

struct DerefReadAstNode : public AstNode {
   public:
    Stmt base_expr;
    std::optional<Stmt> offset_expr;

    explicit DerefReadAstNode(Stmt expr, Stmt offset_expr)
        : base_expr(std::move(expr)), offset_expr(std::move(offset_expr)) {}
    
    explicit DerefReadAstNode(Stmt expr, std::optional<Stmt> offset_expr)
        : base_expr(std::move(expr)), offset_expr(std::move(offset_expr)) {}

    [[nodiscard]] auto deref_depth() const -> int {
        if (std::holds_alternative<std::shared_ptr<DerefReadAstNode>>(base_expr.node)) {
            return std::get<std::shared_ptr<DerefReadAstNode>>(base_expr.node)->deref_depth() + 1;
        }
        return 1;
    }

    [[nodiscard]] auto toString() const -> std::string override { 
        if (offset_expr.has_value()) {
            return "*" + base_expr.toString() + "[" + offset_expr.value().toString() + "]";
        }
        
        return "*" + base_expr.toString();
    }
};

struct DerefWriteAstNode : public AstNode {
   public:
    Stmt base_expr;
    std::optional<Stmt> offset_expr;

    explicit DerefWriteAstNode(Stmt expr, Stmt offset_expr)
        : base_expr(std::move(expr)), offset_expr(std::move(offset_expr)) {}

    explicit DerefWriteAstNode(Stmt expr, std::optional<Stmt> offset_expr)
        : base_expr(std::move(expr)), offset_expr(std::move(offset_expr)) {}

    [[nodiscard]] auto toString() const -> std::string override { 
        if (offset_expr.has_value()) {
            return "*" + base_expr.toString() + "[" + offset_expr.value().toString() + "]";
        }
        
        return "*" + base_expr.toString();
    
    }
};

struct AddrAstNode : public AstNode {
   public:
    Stmt expr;

    explicit AddrAstNode(Stmt expr) : expr(std::move(expr)) {}

    [[nodiscard]] auto toString() const -> std::string override { return "&" + expr.toString(); }
};

struct JumpAstNode : public AstNode {
   public:
    std::string jumpToLabelValue;

    explicit JumpAstNode(std::string jumpToLabelValue)
        : jumpToLabelValue(std::move(jumpToLabelValue)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        return "jump " + jumpToLabelValue;
    }
};

struct VariableAstNode : public AstNode {
   public:
    std::string name;
    DataType type;
    std::optional<Stmt> offset;

    explicit VariableAstNode(const std::string& p_name, DataType p_type, Stmt p_offset)
        : name(p_name), type(p_type), offset(std::move(p_offset)) {}
    explicit VariableAstNode(const std::string& p_name, DataType p_type, std::optional<Stmt> p_offset)
        : name(p_name), type(p_type), offset(std::move(p_offset)) {}
    [[nodiscard]] auto toString() const -> std::string override { 
        if (offset.has_value()) {
            return name + "[" + offset.value().toString() + "]";
        }
        
        return name;
    }
}; 

struct IfNode : public AstNode {
   public:
    Stmt condition;
    std::vector<BodyNode> then;
    std::optional<std::vector<BodyNode>> else_;

    IfNode(Stmt condition, std::vector<BodyNode> then, std::vector<BodyNode> else_)
        : condition(std::move(condition)), then(std::move(then)), else_(std::move(else_)) {}

    IfNode(Stmt condition, std::vector<BodyNode> then, std::optional<std::vector<BodyNode>> else_)
        : condition(std::move(condition)), then(std::move(then)), else_(std::move(else_)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        return "if " + condition.toString();
    }
};

struct FunctionCallAstNode : public AstNode {
   public:
    std::string callName;
    std::vector<Stmt> callArgs;
    ast::DataType returnType;

    FunctionCallAstNode(std::string callName, std::vector<Stmt> callArgs, ast::DataType returnType)
        : callName(std::move(callName)), callArgs(std::move(callArgs)), returnType(returnType) {}

    [[nodiscard]] auto toString() const -> std::string override { return callName; }
};

struct ForLoopAstNode : public AstNode {
   public:
    std::shared_ptr<MoveAstNode> forInit;
    std::optional<Stmt> forCondition;
    std::optional<Stmt> forUpdate;
    std::vector<BodyNode> forBody;

    ForLoopAstNode(std::shared_ptr<MoveAstNode> forInit, std::optional<Stmt> forCondition,
                   std::optional<Stmt> forUpdate, std::vector<BodyNode> forBody)
        : forInit(std::move(forInit)),
          forCondition(std::move(forCondition)),
          forUpdate(std::move(forUpdate)),
          forBody(std::move(forBody)) {}

    [[nodiscard]] auto toString() const -> std::string override { return "for"; }
};

struct TopLevelNode : public AstNode {
    std::variant<std::shared_ptr<FrameAstNode>, std::shared_ptr<MoveAstNode>> node;

    TopLevelNode(std::shared_ptr<FrameAstNode> node) : node(std::move(node)) {}

    TopLevelNode(std::shared_ptr<MoveAstNode> node) : node(std::move(node)) {}

    [[nodiscard]] auto is_function() const -> bool {
        return std::holds_alternative<std::shared_ptr<FrameAstNode>>(node);
    }

    [[nodiscard]] auto get_function() const -> FrameAstNode* {
        if (!is_function()) {
            return nullptr;
        }

        return std::get<std::shared_ptr<FrameAstNode>>(node).get();
    }

    [[nodiscard]] auto toString() const -> std::string override {
        return std::visit([](const auto& node) { return node->toString(); }, node);
    }
};

}  // namespace ast