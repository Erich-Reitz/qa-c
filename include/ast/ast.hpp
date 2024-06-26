#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../parser/st.hpp"
#include "AstNode.hpp"
#include "DataType.hpp"
#include "Stmt.hpp"

namespace ast {

[[nodiscard]] auto bin_op_to_string(BinOpKind kind) -> std::string;
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

    explicit ConstIntAstNode(int p_value) : value(p_value) {}

    [[nodiscard]] auto toString() const -> std::string override { return std::to_string(value); }
};

struct ConstFloatNode : public AstNode {
   public:
    float value;

    explicit ConstFloatNode(float p_value) : value(p_value) {}

    [[nodiscard]] auto toString() const -> std::string override { return std::to_string(value); }
};

struct ReturnAstNode : public AstNode {
   public:
    ExprNode expr;

    explicit ReturnAstNode(ExprNode p_expr) : expr(std::move(p_expr)) {}

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
    ExprNode lhs;
    std::optional<ExprNode> rhs;

    MoveAstNode(ExprNode p_lhs, ExprNode p_rhs) : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)) {}
    MoveAstNode(ExprNode p_lhs, std::optional<ExprNode> p_rhs)
        : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)) {}

    [[nodiscard]] auto toString() const -> std::string override;
};

struct BinaryOpAstNode : public AstNode {
   public:
    ExprNode lhs;
    ExprNode rhs;
    BinOpKind kind;

    BinaryOpAstNode(ExprNode p_lhs, ExprNode p_rhs, BinOpKind p_kind)
        : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)), kind(p_kind) {}

    const BinOpKind* get_bin_op() const override { return &kind; }

    [[nodiscard]] auto toString() const -> std::string override {
        return lhs.toString() + " " + bin_op_to_string(kind) + " " + rhs.toString();
    }
};

struct DerefReadAstNode : public AstNode {
   public:
    ExprNode expr;

    explicit DerefReadAstNode(ExprNode p_expr) : expr(std::move(p_expr)) {}

    [[nodiscard]] auto deref_depth() const -> int {
        if (std::holds_alternative<std::shared_ptr<DerefReadAstNode>>(expr.node)) {
            return std::get<std::shared_ptr<DerefReadAstNode>>(expr.node)->deref_depth() + 1;
        }
        return 1;
    }

    [[nodiscard]] auto toString() const -> std::string override { return "*" + expr.toString(); }
};

struct DerefWriteAstNode : public AstNode {
   public:
    ExprNode expr;

    explicit DerefWriteAstNode(ExprNode p_expr) : expr(std::move(p_expr)) {}

    [[nodiscard]] auto toString() const -> std::string override { return "*" + expr.toString(); }
};

struct AddrAstNode : public AstNode {
   public:
    ExprNode expr;

    explicit AddrAstNode(ExprNode p_expr) : expr(std::move(p_expr)) {}

    [[nodiscard]] auto toString() const -> std::string override { return "&" + expr.toString(); }
};

struct JumpAstNode : public AstNode {
   public:
    std::string jumpToLabelValue;

    explicit JumpAstNode(std::string p_jump_to_label_value)
        : jumpToLabelValue(std::move(p_jump_to_label_value)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        return "jump " + jumpToLabelValue;
    }
};

struct VariableAstNode : public AstNode {
   public:
    std::string name;
    DataType type;

    explicit VariableAstNode(const std::string& p_name, DataType p_type)
        : name(p_name), type(p_type) {}
    [[nodiscard]] auto toString() const -> std::string override {
        return name + " : " + dt_to_string(type);
    }
};

struct IfNode : public AstNode {
   public:
    std::shared_ptr<BinaryOpAstNode> condition;
    std::vector<BodyNode> then;
    std::optional<std::vector<BodyNode>> else_;

    IfNode(std::shared_ptr<BinaryOpAstNode> p_condition, std::vector<BodyNode> p_then,
           std::vector<BodyNode> p_else)
        : condition(std::move(p_condition)), then(std::move(p_then)), else_(std::move(p_else)) {}

    IfNode(std::shared_ptr<BinaryOpAstNode> p_condition, std::vector<BodyNode> p_then,
           std::optional<std::vector<BodyNode>> p_else)
        : condition(std::move(p_condition)), then(std::move(p_then)), else_(std::move(p_else)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        std::string result = "if (" + condition->toString() + ") {\n";
        for (const auto& node : then) {
            result += node.toString() + "\n";
        }
        result += "}\n";
        if (else_.has_value()) {
            result += "else {\n";
            for (const auto& node : else_.value()) {
                result += node.toString() + "\n";
            }
            result += "}";
        }
        return result;
    }
};

struct FunctionCallAstNode : public AstNode {
   public:
    std::string callName;
    std::vector<ExprNode> callArgs;
    ast::DataType returnType;

    FunctionCallAstNode(std::string p_call_name, std::vector<ExprNode> p_call_args,
                        ast::DataType p_return_type)
        : callName(std::move(p_call_name)),
          callArgs(std::move(p_call_args)),
          returnType(p_return_type) {}

    [[nodiscard]] auto toString() const -> std::string override { return callName; }
};

struct ForLoopAstNode : public AstNode {
   public:
    std::shared_ptr<MoveAstNode> forInit;
    std::optional<std::shared_ptr<BinaryOpAstNode>> forCondition;
    std::optional<ExprNode> forUpdate;
    std::vector<BodyNode> forBody;

    ForLoopAstNode(std::shared_ptr<MoveAstNode> p_for_init,
                   std::optional<std::shared_ptr<BinaryOpAstNode>> p_for_condition,
                   std::optional<ExprNode> p_for_update, std::vector<BodyNode> p_for_body)
        : forInit(std::move(p_for_init)),
          forCondition(std::move(p_for_condition)),
          forUpdate(std::move(p_for_update)),
          forBody(std::move(p_for_body)) {}

    [[nodiscard]] auto toString() const -> std::string override {
        std::string result = "for (";
        if (forInit) {
            result += forInit->toString();
        }
        result += "; ";
        if (forCondition.has_value()) {
            result += forCondition.value()->toString();
        }
        result += "; ";
        if (forUpdate.has_value()) {
            result += forUpdate.value().toString();
        }
        result += ") {\n";
        for (const auto& node : forBody) {
            result += node.toString() + "\n";
        }
        result += "}";
        return result;
    }
};

struct TopLevelNode : public AstNode {
    std::variant<std::shared_ptr<FrameAstNode>, std::shared_ptr<MoveAstNode>> node;

    TopLevelNode(std::shared_ptr<FrameAstNode> p_node) : node(std::move(p_node)) {}

    TopLevelNode(std::shared_ptr<MoveAstNode> p_node) : node(std::move(p_node)) {}

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
        return std::visit([](const auto& v_node) { return v_node->toString(); }, node);
    }
};

}  // namespace ast
