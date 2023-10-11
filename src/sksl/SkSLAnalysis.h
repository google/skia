/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLAnalysis_DEFINED
#define SkSLAnalysis_DEFINED

#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkTArray.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace SkSL {

class Context;
class ErrorReporter;
class Expression;
class FunctionDeclaration;
class FunctionDefinition;
class Position;
class ProgramElement;
class ProgramUsage;
class Statement;
class SymbolTable;
class Variable;
class VariableReference;
enum class VariableRefKind : int8_t;
struct ForLoopPositions;
struct LoopUnrollInfo;
struct Module;
struct Program;

/**
 * Provides utilities for analyzing SkSL statically before it's composed into a full program.
 */
namespace Analysis {

/**
 * Determines how `program` samples `child`. By default, assumes that the sample coords might be
 * modified, so `child.eval(sampleCoords)` is treated as Explicit. If writesToSampleCoords is false,
 * treats that as PassThrough, instead. If elidedSampleCoordCount is provided, the pointed to value
 * will be incremented by the number of sample calls where the above rewrite was performed.
 */
SampleUsage GetSampleUsage(const Program& program,
                           const Variable& child,
                           bool writesToSampleCoords = true,
                           int* elidedSampleCoordCount = nullptr);

bool ReferencesBuiltin(const Program& program, int builtin);

bool ReferencesSampleCoords(const Program& program);
bool ReferencesFragCoords(const Program& program);

bool CallsSampleOutsideMain(const Program& program);

bool CallsColorTransformIntrinsics(const Program& program);

/**
 * Determines if `function` always returns an opaque color (a vec4 where the last component is known
 * to be 1). This is conservative, and based on constant expression analysis.
 */
bool ReturnsOpaqueColor(const FunctionDefinition& function);

/**
 * Determines if `function` is a color filter which returns the alpha component of the input color
 * unchanged. This is a very conservative analysis, and only supports returning a swizzle of the
 * input color, or returning a constructor that ends with `input.a`.
 */
bool ReturnsInputAlpha(const FunctionDefinition& function, const ProgramUsage& usage);

/**
 * Checks for recursion or overly-deep function-call chains, and rejects programs which have them.
 * Also, computes the size of the program in a completely flattened state--loops fully unrolled,
 * function calls inlined--and rejects programs that exceed an arbitrary upper bound.
 */
bool CheckProgramStructure(const Program& program, bool enforceSizeLimit);

/** Determines if `expr` contains a reference to the variable sk_RTAdjust. */
bool ContainsRTAdjust(const Expression& expr);

/** Determines if `expr` contains a reference to variable `var`. */
bool ContainsVariable(const Expression& expr, const Variable& var);

/** Determines if `expr` has any side effects. (Is the expression state-altering or pure?) */
bool HasSideEffects(const Expression& expr);

/** Determines if `expr` is a compile-time constant (composed of just constructors and literals). */
bool IsCompileTimeConstant(const Expression& expr);

/**
 * Determines if `expr` is a dynamically-uniform expression; this returns true if the expression
 * could be evaluated at compile time if uniform values were known.
 */
bool IsDynamicallyUniformExpression(const Expression& expr);

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
bool SwitchCaseContainsUnconditionalExit(const Statement& stmt);

/**
 * Finds conditional exits from a switch-case. Returns true if this statement contains a
 * conditional that wraps a potential exit from the switch (via continue, break or return).
 */
bool SwitchCaseContainsConditionalExit(const Statement& stmt);

std::unique_ptr<ProgramUsage> GetUsage(const Program& program);
std::unique_ptr<ProgramUsage> GetUsage(const Module& module);

/** Returns true if the passed-in statement might alter `var`. */
bool StatementWritesToVariable(const Statement& stmt, const Variable& var);

/**
 * Detects if the passed-in block contains a `continue`, `break` or `return` that could directly
 * affect its control flow. (A `continue` or `break` nested inside an inner loop/switch will not
 * affect the loop, but a `return` will.)
 */
struct LoopControlFlowInfo {
    bool fHasContinue = false;
    bool fHasBreak = false;
    bool fHasReturn = false;
};
LoopControlFlowInfo GetLoopControlFlowInfo(const Statement& stmt);

/**
 * Returns true if the expression can be assigned-into. Pass `info` if you want to know the
 * VariableReference that will be written to. Pass `errors` to report an error for expressions that
 * are not actually writable.
 */
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
 * - myStruct.myArrayField[7].xzy
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
 * Ensures that any index-expressions inside of for-loops qualify as 'constant-index-expressions' as
 * defined by GLSL 1.0, Appendix A, Section 5. A constant-index-expression is:
 * - A constant-expression
 * - Loop indices (as defined in Appendix A, Section 4)
 * - Expressions composed of both of the above
 */
void ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors);

/**
 * Ensures that a for-loop meets the strict requirements of The OpenGL ES Shading Language 1.00,
 * Appendix A, Section 4.
 * If the requirements are met, information about the loop's structure is returned.
 * If the requirements are not met, the problem is reported via `errors` (if not nullptr), and
 * null is returned.
 * The loop test-expression may be altered by this check. For example, a loop like this:
 *     for (float x = 1.0; x != 0.0; x -= 0.01) {...}
 * appears to be ES2-safe, but due to floating-point rounding error, it may not actually terminate.
 * We rewrite the test condition to `x > 0.0` in order to ensure loop termination.
 */
std::unique_ptr<LoopUnrollInfo> GetLoopUnrollInfo(const Context& context,
                                                  Position pos,
                                                  const ForLoopPositions& positions,
                                                  const Statement* loopInitializer,
                                                  std::unique_ptr<Expression>* loopTestPtr,
                                                  const Expression* loopNext,
                                                  const Statement* loopStatement,
                                                  ErrorReporter* errors);

/** Detects functions that fail to return a value on at least one path. */
bool CanExitWithoutReturningValue(const FunctionDeclaration& funcDecl, const Statement& body);

/** Determines if a given function has multiple and/or early returns. */
enum class ReturnComplexity {
    kSingleSafeReturn,
    kScopedReturns,
    kEarlyReturns,
};
ReturnComplexity GetReturnComplexity(const FunctionDefinition& funcDef);

/**
 * Runs at finalization time to perform any last-minute correctness checks:
 * - Reports dangling FunctionReference or TypeReference expressions
 * - Reports function `out` params which are never written to (structs are currently exempt)
 */
void DoFinalizationChecks(const Program& program);

/**
 * Error checks compute shader in/outs and returns a vector containing them ordered by location.
 */
skia_private::TArray<const SkSL::Variable*> GetComputeShaderMainParams(const Context& context,
                                                                       const Program& program);

/**
 * Tracks the symbol table stack, in conjunction with a ProgramVisitor. Inside `visitStatement`,
 * pass the current statement and a symbol-table vector to a SymbolTableStackBuilder and the symbol
 * table stack will be maintained automatically.
 */
class SymbolTableStackBuilder {
public:
    // If the passed-in statement holds a symbol table, adds it to the stack.
    SymbolTableStackBuilder(const Statement* stmt,
                            std::vector<std::shared_ptr<SymbolTable>>* stack);

    // If a symbol table was added to the stack earlier, removes it from the stack.
    ~SymbolTableStackBuilder();

    // Returns true if an entry was added to the symbol-table stack.
    bool foundSymbolTable() {
        return fStackToPop != nullptr;
    }

private:
    std::vector<std::shared_ptr<SymbolTable>>* fStackToPop = nullptr;
};

}  // namespace Analysis
}  // namespace SkSL

#endif
