#include <gtest/gtest.h>

#include <cstdlib>
#include <expected>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

constexpr std::string compiler_path = "./build/bin/qac";
constexpr std::string temp_dir = "./tmp/";
constexpr std::string test_dir = "./tests/sources";
constexpr std::string asm_path = "test.asm";
const std::string compiler_gen_asm_path = temp_dir + asm_path;
const std::string compiler_gen_object_path = temp_dir + "test.o";
const std::string compiler_gen_binary_path = temp_dir + "test.out";

[[nodiscard]] auto invoke_qac(const std::string& sourcePath) -> std::expected<int, std::string> {
    const auto command =
        compiler_path.data() + std::string(" ") + sourcePath + " -o " + compiler_gen_asm_path;
    const auto result = system(command.c_str());
    if (result != 0) {
        return std::unexpected("Failed to compile the source file");
    }
    return result;
}

[[nodiscard]] auto nasm_assemble_elf64(const std::string& asmPath, const std::string& objectPath)
    -> std::expected<int, std::string> {
    const auto command = "nasm -f  elf64 -o " + objectPath + " " + asmPath;
    const auto result = system(command.c_str());
    if (result != 0) {
        return std::unexpected("Failed to compile the asm file");
    }
    return result;
}

[[nodiscard]] auto gcc_link_standalone_binary(const std::string& objectPath,
                                              const std::string& binaryPath)
    -> std::expected<int, std::string> {
    const auto command = "gcc -o " + binaryPath + " " + objectPath + " -nostartfiles -lc -fPIE";
    const auto result = system(command.c_str());
    if (result != 0) {
        return std::unexpected("Failed to link the object file");
    }
    return result;
}

[[nodiscard]] auto compile(const std::string& sourcePath) -> std::expected<int, std::string> {
    const auto compileResult = invoke_qac(sourcePath);
    if (!compileResult) {
        return std::unexpected(compileResult.error());
    }

    const auto assembleResult =
        nasm_assemble_elf64(compiler_gen_asm_path, compiler_gen_object_path);
    if (!assembleResult) {
        return std::unexpected(assembleResult.error());
    }

    const auto linkResult =
        gcc_link_standalone_binary(compiler_gen_object_path, compiler_gen_binary_path);
    if (!linkResult) {
        return std::unexpected(linkResult.error());
    }

    return 0;
}

[[nodiscard]] auto parse_expected_return_from_source(const std::string& sourcePath)
    -> std::expected<int, std::string> {
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        return std::unexpected("Failed to open the source file");
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("// EXPECTED_RETURN: ")) {
            return std::stoi(line.substr(19));
        }
    }
    return std::unexpected("Expected return value not found");
}

[[nodiscard]] auto run_test_for_status_code(const std::string& sourcePath)
    -> std::expected<bool, std::string> {
    const auto expectedReturn = parse_expected_return_from_source(sourcePath);

    if (!expectedReturn) {
        return std::unexpected(expectedReturn.error());
    }
    const auto expected_return_code = expectedReturn.value();

    const auto compileResult = compile(sourcePath);
    if (!compileResult) {
        return std::unexpected(compileResult.error());
    }
    const auto result = system(compiler_gen_binary_path.data());
    if (WIFEXITED(result)) {
        int normal_exit_status = WEXITSTATUS(result);
        if (normal_exit_status != expected_return_code) {
            const auto message = "Expected return value: " + std::to_string(expected_return_code) +
                                 " but got: " + std::to_string(normal_exit_status);
            return std::unexpected(message);
        }
    } else {
        return std::unexpected("Compiled binary crashed during execution");
    }

    return true;
}

#define RUN_TEST_CASE(NAME, PATH)                                                     \
    TEST(CompilerIntegrationTest, NAME) {                                             \
        auto result = run_test_for_status_code("tests/sources/" + std::string(PATH)); \
        if (!result) {                                                                \
            std::cerr << result.error() << std::endl;                                 \
            FAIL();                                                                   \
        }                                                                             \
        SUCCEED();                                                                    \
    }

/** Basic **/
RUN_TEST_CASE(IntReturnValue, "int_return.c");
RUN_TEST_CASE(IntAssignment, "int_assignment.c");

/** Addition **/
RUN_TEST_CASE(IntVariableAddition, "int_variable_addition.c");

// /** For loop test cases  **/
RUN_TEST_CASE(ForLoopIncrement, "for_loop_increment.c");
RUN_TEST_CASE(ForLoopDecrement, "for_loop_decrement.c");

// /** Int* pointers  **/
RUN_TEST_CASE(IntSwap, "int_swap.c");
RUN_TEST_CASE(IntDerefAddition, "int_deref_addition.c");
RUN_TEST_CASE(BasicDeref, "ptr_set.c");
RUN_TEST_CASE(ArrayGetterAlternativeSyntax, "array_getter_after_passed_ptr_arth.c");

RUN_TEST_CASE(PointerArthAddRhs, "ptr_arth_addition_rhs.c");
RUN_TEST_CASE(PointerArthRhs2, "ptr_arth_addition_subtraction_rhs2.c");
RUN_TEST_CASE(PointerArthLhs, "ptr_arth_lhs.c");
RUN_TEST_CASE(PointerArthLhs2, "ptr_arth_lhs2.c");

// /** Comparision operators  **/
RUN_TEST_CASE(IntEqualityEq_NegativeCase, "int_equality_eq_negative.c");
RUN_TEST_CASE(IntEqualityEq_PositiveCase, "int_equality_eq_positive.c");

RUN_TEST_CASE(IntEqualityNeq_NegativeCase, "int_equality_neq_negative.c");
RUN_TEST_CASE(IntEqualityNeq_PositiveCase, "int_equality_neq_positive.c");

RUN_TEST_CASE(IntLessThanNegativeCase, "int_less_than_negative.c");
RUN_TEST_CASE(IntLessThanPositiveCase, "int_less_than_positive.c");

RUN_TEST_CASE(IntGreaterThanNegativeCase, "int_greater_than_negative.c");
RUN_TEST_CASE(IntGreaterThanPositiveCase, "int_greater_than_positive.c");
RUN_TEST_CASE(IntEqVarImmediateNeg, "int_eq_var_immediate_neg.c");
RUN_TEST_CASE(IntEqVarImmediatePos, "int_eq_var_immediate_pos.c");
RUN_TEST_CASE(IntLtVarImmediateNeg, "int_lt_var_immediate_neg.c");
RUN_TEST_CASE(IntLtVarImmediatePos, "int_lt_var_immediate_pos.c");
RUN_TEST_CASE(IntLtImVarNeg, "int_lt_im_var_neg.c");
RUN_TEST_CASE(IntLtImVarPos, "int_lt_im_var_pos.c");
RUN_TEST_CASE(IntGtImVarNeg, "int_gt_im_var_neg.c");
RUN_TEST_CASE(IntGtImVarPos, "int_gt_im_var_pos.c");

/** If Statements */
RUN_TEST_CASE(BasicIfStatement, "basic_if_statement.c");
RUN_TEST_CASE(IfGtStatement, "basic_if_gt_statement.c");
RUN_TEST_CASE(NestedIfStatement, "nested_if_statement.c");
RUN_TEST_CASE(IfElseStatement, "IfElse.c");
RUN_TEST_CASE(ComplicatedIfStatement, "complicated_if.c");

/** Subtraction */
RUN_TEST_CASE(IntSubtractionInvolvingNegatives, "int_subtraction_involving_negative.c");
RUN_TEST_CASE(IntSubtractionResultReg, "int_subtraction_nq.c");
RUN_TEST_CASE(IntSubtractionResultReg2, "much_add_sub.c");

/** multiplication */
RUN_TEST_CASE(IntMultiplication, "int_basic_mult.c");
RUN_TEST_CASE(IntVarValMult, "int_var_val_mult.c");
RUN_TEST_CASE(IntMultAgainstSelf, "int_mult_against_self.c");
RUN_TEST_CASE(FloatVarMult, "float_mult.c");

/** division */
RUN_TEST_CASE(IntDivision, "int_division.c");
RUN_TEST_CASE(IntDivisionByRhsImmediate, "int_division_rhs_im.c");
RUN_TEST_CASE(IntDivisionByLhsImmediate, "int_division_lhs_im.c");

// /** Parameter Passing  **/
RUN_TEST_CASE(IntMax, "max.c");
RUN_TEST_CASE(PtrSet2, "ptr_set_2.c");
RUN_TEST_CASE(PassVariablesOnStack, "pass_vars_on_stack.c");
RUN_TEST_CASE(PassVariablesOnStackMoreInvolved, "pass_vars_on_stack_more_involved.c");

// ** Variable without initialization **/
RUN_TEST_CASE(IntNoInit, "int_var_no_init.c");

// ** Arrays */
RUN_TEST_CASE(BasicIntArray, "int_arr.c");
RUN_TEST_CASE(TwoArraysInFunctionReferenceFirst, "int_arr2.c");
RUN_TEST_CASE(TwoArraysInFunctionReferenceSecond, "int_arr3.c");
RUN_TEST_CASE(ForLoopArray, "for_loop_arr.c");
RUN_TEST_CASE(FillArrayWithFive, "pass_arr.c");
RUN_TEST_CASE(FillArrayWithHundred, "pass_arr_2.c");
RUN_TEST_CASE(FillArrayWithHundredPlusHundred, "pass_arr3.c");
RUN_TEST_CASE(FromNegativeHundredToZero, "pass_arr4_neg_values.c");
RUN_TEST_CASE(ArrayPassedButNotMutated, "array_no_mutation.c");
RUN_TEST_CASE(ArrayGetterBasic, "array_getter_basic.c");
RUN_TEST_CASE(ArrayGetterAfterPassed, "array_getter_after_passed.c");
RUN_TEST_CASE(ArrayGetterAfterPassedSimple, "array_getter_after_passed_simple.c");
RUN_TEST_CASE(TrailingArrayGetter, "trailing_array_getter.c");
RUN_TEST_CASE(ArrayReturnLast, "array_return_last.c");
/** Floats */
RUN_TEST_CASE(FloatBasic, "float_basic.c");
RUN_TEST_CASE(FloatBasic2, "float_basic_2.c");

RUN_TEST_CASE(FloatLessThanNegativeCase, "float_less_than_negative.c");
RUN_TEST_CASE(FloatLessThanPositiveCase, "float_less_than_positive.c");
RUN_TEST_CASE(FloatGreaterThanNegativeCase, "float_greater_negative.c");
RUN_TEST_CASE(FloatGreaterThanPositiveCase, "float_greater_positive.c");

// testing constant < var
RUN_TEST_CASE(FloatLtValVarNeg, "float_lt_val_var_neg.c");
RUN_TEST_CASE(FloatLtValVarPos, "float_lt_val_var_pos.c");

// testing constant > var
RUN_TEST_CASE(FloatGtValVarNeg, "float_gt_val_var_neg.c");
RUN_TEST_CASE(FloatGtValVarPos, "float_gt_val_var_pos.c");

// testing var < constant
RUN_TEST_CASE(FloatLtVarValNeg, "float_lt_var_val_neg.c");
RUN_TEST_CASE(FloatLtVarValPos, "float_lt_var_val_pos.c");

// testing var > constant
RUN_TEST_CASE(FloatGtVarValNeg, "float_gt_var_val_neg.c");
RUN_TEST_CASE(FloatGtVarValPos, "float_gt_var_val_pos.c");

// array of floats
RUN_TEST_CASE(FloatArr, "float_arr.c");

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
