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
#include <set>
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

bool CallsColorTransformIntrinsics(const Program& program);

/**
 * Determines if `function` always returns an opaque color (a vec4 where the last component is known
 * to be 1). This is conservative, and based on constant expression analysis.
 */
bool ReturnsOpaqueColor(const FunctionDefinition& function);

/**
 * Checks for recursion or overly-deep function-call chains, and rejects programs which have them.
 * Also, computes the size of the program in a completely flattened state--loops fully unrolled,
 * function calls inlined--and rejects programs that exceed an arbitrary upper bound. This is
 * intended to prevent absurdly large programs from overwhemling SkVM. Only strict-ES2 mode is
 * supported; complex control flow is not SkVM-compatible (and this becomes the halting problem)
 */
bool CheckProgramStructure(const Program& program, bool enforceSizeLimit);

/** Determines if `expr` contains a reference to the variable sk_RTAdjust. */
bool ContainsRTAdjust(const Expression& expr);

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
bool SwitchCaseContainsUnconditionalExit(Statement& stmt);

/**
 * Finds conditional exits from a switch-case. Returns true if this statement contains a
 * conditional that wraps a potential exit from the switch (via continue, break or return).
 */
bool SwitchCaseContainsConditionalExit(Statement& stmt);

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
std::unique_ptr<LoopUnrollInfo> GetLoopUnrollInfo(Position pos,
                                                  const ForLoopPositions& positions,
                                                  const Statement* loopInitializer,
                                                  const Expression* loopTest,
                                                  const Expression* loopNext,
                                                  const Statement* loopStatement,
                                                  ErrorReporter* errors);

void ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors);

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
SkTArray<const SkSL::Variable*> GetComputeShaderMainParams(const Context& context,
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

private:
    std::vector<std::shared_ptr<SymbolTable>>* fStackToPop = nullptr;
};

}  // namespace Analysis
}  // namespace SkSL

#endif
