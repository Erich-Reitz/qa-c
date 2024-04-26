#pragma once

#include <concepts>
#include <ostream>
#include <string>
#include <variant>

#include "../ast/ast.hpp"
#include "qa_x86.hpp"

namespace qa_ir {
struct Temp; 
struct Variable;
struct ConstInt;
using Value = std::variant<Temp, target::HardcodedRegister, Variable, ConstInt>;

struct Variable {
    std::string name = "";
    ast::DataType type = ast::DataType{.name = "", .size = 0};
    Value *offset = nullptr;
};

struct Temp {
    int id;
    int size;
};

struct ConstInt {
    int numerical_value;
};



std::ostream& operator<<(std::ostream& os, const Value& v);

struct Label {
    std::string name;
};

std::ostream& operator<<(std::ostream& os, const Label& label);

struct Mov {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const Mov& mov);

struct Ret {
    Value value;
};

std::ostream& operator<<(std::ostream& os, const Ret& ret);

struct Add {
    Value dst;

    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const Add& add);

struct Sub {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const Sub& sub);

struct MovR {
    Value dst;
    target::HardcodedRegister src;
};

std::ostream& operator<<(std::ostream& os, const MovR& movr);

struct Addr {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const Addr& addr);

struct Deref {
    Value dst;
    Value src;
    int depth = 1;
};

std::ostream& operator<<(std::ostream& os, const Deref& deref);

struct StoreAddr {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const StoreAddr& storeaddr);

struct Compare {
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const Compare& cmp);

struct Equal {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const Equal& eq);

struct NotEqual {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const NotEqual& neq);

struct GreaterThan {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const GreaterThan& gt);

struct LessThan {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const LessThan& lt);

struct ConditionalJumpEqual {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpEqual& cje);

struct ConditionalJumpNotEqual {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpNotEqual& cjne);

struct ConditionalJumpGreater {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpGreater& cjg);

struct ConditionalJumpLess {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpLess& cjl);

struct Jump {
    Label label;
};

std::ostream& operator<<(std::ostream& os, const Jump& jump);

struct Call {
    std::string name;
    std::vector<Value> args;
    Value dst;
};
std::ostream& operator<<(std::ostream& os, const Call& call);

struct LabelDef {
    Label label;
};

std::ostream& operator<<(std::ostream& os, const LabelDef& labeldef);

struct DerefStore {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const DerefStore& derefstore);

struct DefineStackPushed {
    std::string name;
    int size;
};

std::ostream& operator<<(std::ostream& os, const DefineStackPushed& dsp);

using Operation =
    std::variant<Mov, Ret, Add, Sub, MovR, Addr, DefineStackPushed, Deref, Compare, Jump, Equal,
                 ConditionalJumpEqual, ConditionalJumpGreater, ConditionalJumpNotEqual, LabelDef,
                 Call, DerefStore, GreaterThan, ConditionalJumpLess, NotEqual, LessThan>;

using CondJ = std::variant<ConditionalJumpEqual, ConditionalJumpGreater, ConditionalJumpNotEqual,
                           ConditionalJumpLess>;

std::ostream& operator<<(std::ostream& os, const Operation& ins);

template <typename T>
concept IsRegister = std::is_same<T, target::HardcodedRegister>::value ||
                     std::is_same<T, target::VirtualRegister>::value;

template <typename T>
concept IsIRLocation =
    std::is_same<T, qa_ir::Temp>::value || std::is_same<T, qa_ir::Variable>::value;

bool operator<(const Temp& lhs, const Temp& rhs);
std::ostream& operator<<(std::ostream& os, const Temp& temp);

bool operator<(const target::HardcodedRegister& lhs, const target::HardcodedRegister& rhs);
std::ostream& operator<<(std::ostream& os, const target::HardcodedRegister& reg);

[[nodiscard]] int SizeOf(Value v);
[[nodiscard]] int SizeOfWhatItPointsTo(Value v);

}  // namespace qa_ir