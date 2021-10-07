/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLAnalysis_DEFINED
#define SkSLAnalysis_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLSampleUsage.h"

#include <memory>
#include <set>

namespace SkSL {

class ErrorReporter;
class Expression;
class ForStatement;
class FunctionDeclaration;
class FunctionDefinition;
struct LoadedModule;
struct Program;
class ProgramElement;
class ProgramUsage;
class Statement;
struct LoopUnrollInfo;
class Variable;
class VariableReference;
enum class VariableRefKind : int8_t;

/**
 * Provides utilities for analyzing SkSL statically before it's composed into a full program.
 */
namespace Analysis {

/**
 * Determines how `program` samples `child`. By default, assumes that the sample coords
 * (SK_MAIN_COORDS_BUILTIN) might be modified, so `child.eval(sampleCoords)` is treated as
 * Explicit. If writesToSampleCoords is false, treats that as PassThrough, instead.
 * If elidedSampleCoordCount is provided, the pointed to value will be incremented by the
 * number of sample calls where the above rewrite was performed.
 */
SampleUsage GetSampleUsage(const Program& program,
                           const Variable& child,
                           bool writesToSampleCoords = true,
                           int* elidedSampleCoordCount = nullptr);

bool ReferencesBuiltin(const Program& program, int builtin);

bool ReferencesSampleCoords(const Program& program);
bool ReferencesFragCoords(const Program& program);

bool CallsSampleOutsideMain(const Program& program);

/**
 * Computes the size of the program in a completely flattened state--loops fully unrolled,
 * function calls inlined--and rejects programs that exceed an arbitrary upper bound. This is
 * intended to prevent absurdly large programs from overwhemling SkVM. Only strict-ES2 mode is
 * supported; complex control flow is not SkVM-compatible (and this becomes the halting problem)
 */
bool CheckProgramUnrolledSize(const Program& program);

/**
 * Detect an orphaned variable declaration outside of a scope, e.g. if (true) int a;. Returns
 * true if an error was reported.
 */
bool DetectVarDeclarationWithoutScope(const Statement& stmt, ErrorReporter* errors = nullptr);

int NodeCountUpToLimit(const FunctionDefinition& function, int limit);

/**
 * Finds unconditional exits from a switch-case. Returns true if this statement unconditionally
 * causes an exit from this switch (via continue, break or return).
 */
bool SwitchCaseContainsUnconditionalExit(Statement& stmt);

/**
 * Finds conditional exits from a switch-case. Returns true if this statement contains a
 * conditional that wraps a potential exit from the switch (via continue, break or return).
 */
bool SwitchCaseContainsConditionalExit(Statement& stmt);

std::unique_ptr<ProgramUsage> GetUsage(const Program& program);
std::unique_ptr<ProgramUsage> GetUsage(const LoadedModule& module);

bool StatementWritesToVariable(const Statement& stmt, const Variable& var);

struct AssignmentInfo {
    VariableReference* fAssignedVar = nullptr;
};
bool IsAssignable(Expression& expr, AssignmentInfo* info = nullptr,
                  ErrorReporter* errors = nullptr);

/**
 * Updates the `refKind` field of the VariableReference at the top level of `expr`.
 * If `expr` can be assigned to (`IsAssignable`), true is returned and no errors are reported.
 * If not, false is returned. and an error is reported if `errors` is non-null.
 */
bool UpdateVariableRefKind(Expression* expr, VariableRefKind kind, ErrorReporter* errors = nullptr);

/**
 * A "trivial" expression is one where we'd feel comfortable cloning it multiple times in
 * the code, without worrying about incurring a performance penalty. Examples:
 * - true
 * - 3.14159265
 * - myIntVariable
 * - myColor.rgb
 * - myArray[123]
 * - myStruct.myField
 * - half4(0)
 *
 * Trivial-ness is stackable. Somewhat large expressions can occasionally make the cut:
 * - half4(myColor.a)
 * - myStruct.myArrayField[7].xyz
 */
bool IsTrivialExpression(const Expression& expr);

/**
 * Returns true if both expression trees are the same. Used by the optimizer to look for self-
 * assignment or self-comparison; won't necessarily catch complex cases. Rejects expressions
 * that may cause side effects.
 */
bool IsSameExpressionTree(const Expression& left, const Expression& right);

/**
 * Returns true if expr is a constant-expression, as defined by GLSL 1.0, section 5.10.
 * A constant expression is one of:
 * - A literal value
 * - A global or local variable qualified as 'const', excluding function parameters
 * - An expression formed by an operator on operands that are constant expressions, including
 *   getting an element of a constant vector or a constant matrix, or a field of a constant
 *   structure
 * - A constructor whose arguments are all constant expressions
 * - A built-in function call whose arguments are all constant expressions, with the exception
 *   of the texture lookup functions
 */
bool IsConstantExpression(const Expression& expr);

/**
 * Returns true if expr is a valid constant-index-expression, as defined by GLSL 1.0, Appendix A,
 * Section 5. A constant-index-expression is:
 * - A constant-expression
 * - Loop indices (as defined in Appendix A, Section 4)
 * - Expressions composed of both of the above
 */
bool IsConstantIndexExpression(const Expression& expr,
                               const std::set<const Variable*>* loopIndices);

/**
 * Ensures that a for-loop meets the strict requirements of The OpenGL ES Shading Language 1.00,
 * Appendix A, Section 4.
 * If the requirements are met, information about the loop's structure is returned.
 * If the requirements are not met, the problem is reported via `errors` (if not nullptr), and
 * null is returned.
 */
std::unique_ptr<LoopUnrollInfo> GetLoopUnrollInfo(int line,
                                                  const Statement* loopInitializer,
                                                  const Expression* loopTest,
                                                  const Expression* loopNext,
                                                  const Statement* loopStatement,
                                                  ErrorReporter* errors);

void ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors);

/** Detects functions that fail to return a value on at least one path. */
bool CanExitWithoutReturningValue(const FunctionDeclaration& funcDecl, const Statement& body);

/**
 * Searches for @if/@switch statements that didn't optimize away, or dangling
 * FunctionReference or TypeReference expressions, and reports them as errors.
 */
void VerifyStaticTestsAndExpressions(const Program& program);

}  // namespace Analysis
}  // namespace SkSL

#endif
