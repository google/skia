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
class Variable;
class VariableReference;
enum class VariableRefKind : int8_t;

/**
 * Provides utilities for analyzing SkSL statically before it's composed into a full program.
 */
struct Analysis {
    /**
     * Determines how `program` samples `fp`. By default, assumes that the sample coords
     * (SK_MAIN_COORDS_BUILTIN) might be modified, so `sample(fp, sampleCoords)` is treated as
     * Explicit. If writesToSampleCoords is false, treats that as PassThrough, instead.
     * If elidedSampleCoordCount is provided, the pointed to value will be incremented by the
     * number of sample calls where the above rewrite was performed.
     */
    static SampleUsage GetSampleUsage(const Program& program,
                                      const Variable& fp,
                                      bool writesToSampleCoords = true,
                                      int* elidedSampleCoordCount = nullptr);

    static bool ReferencesBuiltin(const Program& program, int builtin);

    static bool ReferencesSampleCoords(const Program& program);
    static bool ReferencesFragCoords(const Program& program);

    static bool CallsSampleOutsideMain(const Program& program);

    /*
     * Does the function call graph of the program include any cycles? If so, emits an error.
     */
    static bool DetectStaticRecursion(SkSpan<std::unique_ptr<ProgramElement>> programElements,
                                      ErrorReporter& errors);

    static int NodeCountUpToLimit(const FunctionDefinition& function, int limit);

    /**
     * Finds unconditional exits from a switch-case. Returns true if this statement unconditionally
     * causes an exit from this switch (via continue, break or return).
     */
    static bool SwitchCaseContainsUnconditionalExit(Statement& stmt);

    /**
     * A switch-case "falls through" when it doesn't have an unconditional exit.
     */
    static bool SwitchCaseFallsThrough(Statement& stmt) {
        return !SwitchCaseContainsUnconditionalExit(stmt);
    }

    /**
     * Finds conditional exits from a switch-case. Returns true if this statement contains a
     * conditional that wraps a potential exit from the switch (via continue, break or return).
     */
    static bool SwitchCaseContainsConditionalExit(Statement& stmt);

    static std::unique_ptr<ProgramUsage> GetUsage(const Program& program);
    static std::unique_ptr<ProgramUsage> GetUsage(const LoadedModule& module);

    static bool StatementWritesToVariable(const Statement& stmt, const Variable& var);

    struct AssignmentInfo {
        VariableReference* fAssignedVar = nullptr;
    };
    static bool IsAssignable(Expression& expr, AssignmentInfo* info = nullptr,
                             ErrorReporter* errors = nullptr);

    // Updates the `refKind` field of exactly one VariableReference inside `expr`.
    // `expr` must be `IsAssignable`; returns an error otherwise.
    static bool MakeAssignmentExpr(Expression* expr, VariableRefKind kind, ErrorReporter* errors);

    // Updates the `refKind` field of every VariableReference found within `expr`.
    // `expr` is allowed to have zero, one, or multiple VariableReferences.
    static void UpdateRefKind(Expression* expr, VariableRefKind refKind);

    // A "trivial" expression is one where we'd feel comfortable cloning it multiple times in
    // the code, without worrying about incurring a performance penalty. Examples:
    // - true
    // - 3.14159265
    // - myIntVariable
    // - myColor.rgb
    // - myArray[123]
    // - myStruct.myField
    // - half4(0)
    //
    // Trivial-ness is stackable. Somewhat large expressions can occasionally make the cut:
    // - half4(myColor.a)
    // - myStruct.myArrayField[7].xyz
    static bool IsTrivialExpression(const Expression& expr);

    // Returns true if both expression trees are the same. Used by the optimizer to look for self-
    // assignment or self-comparison; won't necessarily catch complex cases. Rejects expressions
    // that may cause side effects.
    static bool IsSameExpressionTree(const Expression& left, const Expression& right);

    // Is 'expr' a constant-expression, as defined by GLSL 1.0, section 5.10? A constant expression
    // is one of:
    //
    // - A literal value
    // - A global or local variable qualified as 'const', excluding function parameters
    // - An expression formed by an operator on operands that are constant expressions, including
    //   getting an element of a constant vector or a constant matrix, or a field of a constant
    //   structure
    // - A constructor whose arguments are all constant expressions
    //
    // GLSL (but not SkSL, yet - skbug.com/10835) also provides:
    // - A built-in function call whose arguments are all constant expressions, with the exception
    //   of the texture lookup functions
    static bool IsConstantExpression(const Expression& expr);

    struct UnrollableLoopInfo {
        const Variable* fIndex;
        double fStart;
        double fDelta;
        int fCount;
    };

    // Ensures that a for-loop meets the strict requirements of The OpenGL ES Shading Language 1.00,
    // Appendix A, Section 4.
    // Information about the loop's structure are placed in outLoopInfo (if not nullptr).
    // If the function returns false, specific reasons are reported via errors (if not nullptr).
    static bool ForLoopIsValidForES2(int offset,
                                     const Statement* loopInitializer,
                                     const Expression* loopTest,
                                     const Expression* loopNext,
                                     const Statement* loopStatement,
                                     UnrollableLoopInfo* outLoopInfo,
                                     ErrorReporter* errors);

    static void ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors);

    // Detects functions that fail to return a value on at least one path.
    static bool CanExitWithoutReturningValue(const FunctionDeclaration& funcDecl,
                                             const Statement& body);
};

/**
 * Utility class to visit every element, statement, and expression in an SkSL program IR.
 * This is intended for simple analysis and accumulation, where custom visitation behavior is only
 * needed for a limited set of expression kinds.
 *
 * Subclasses should override visitExpression/visitStatement/visitProgramElement as needed and
 * intercept elements of interest. They can then invoke the base class's function to visit all
 * sub expressions. They can also choose not to call the base function to arrest recursion, or
 * implement custom recursion.
 *
 * The visit functions return a bool that determines how the default implementation recurses. Once
 * any visit call returns true, the default behavior stops recursing and propagates true up the
 * stack.
 */
template <typename T>
class TProgramVisitor {
public:
    virtual ~TProgramVisitor() = default;

protected:
    virtual bool visitExpression(typename T::Expression& expression);
    virtual bool visitStatement(typename T::Statement& statement);
    virtual bool visitProgramElement(typename T::ProgramElement& programElement);

    virtual bool visitExpressionPtr(typename T::UniquePtrExpression& expr) = 0;
    virtual bool visitStatementPtr(typename T::UniquePtrStatement& stmt) = 0;
};

// ProgramVisitors take const types; ProgramWriters do not.
struct ProgramVisitorTypes {
    using Program = const SkSL::Program;
    using Expression = const SkSL::Expression;
    using Statement = const SkSL::Statement;
    using ProgramElement = const SkSL::ProgramElement;
    using UniquePtrExpression = const std::unique_ptr<SkSL::Expression>;
    using UniquePtrStatement = const std::unique_ptr<SkSL::Statement>;
};

struct ProgramWriterTypes {
    using Program = SkSL::Program;
    using Expression = SkSL::Expression;
    using Statement = SkSL::Statement;
    using ProgramElement = SkSL::ProgramElement;
    using UniquePtrExpression = std::unique_ptr<SkSL::Expression>;
    using UniquePtrStatement = std::unique_ptr<SkSL::Statement>;
};

// Squelch bogus Clang warning about template vtables: https://bugs.llvm.org/show_bug.cgi?id=18733
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-template-vtables"
#endif
extern template class TProgramVisitor<ProgramVisitorTypes>;
extern template class TProgramVisitor<ProgramWriterTypes>;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

class ProgramVisitor : public TProgramVisitor<ProgramVisitorTypes> {
public:
    bool visit(const Program& program);

private:
    // ProgramVisitors shouldn't need access to unique_ptrs, and marking these as final should help
    // these accessors inline away. Use ProgramWriter if you need the unique_ptrs.
    bool visitExpressionPtr(const std::unique_ptr<Expression>& e) final {
        return this->visitExpression(*e);
    }
    bool visitStatementPtr(const std::unique_ptr<Statement>& s) final {
        return this->visitStatement(*s);
    }
};

class ProgramWriter : public TProgramVisitor<ProgramWriterTypes> {
public:
    // Subclass these methods if you want access to the unique_ptrs of IRNodes in a program.
    // This will allow statements or expressions to be replaced during a visit.
    bool visitExpressionPtr(std::unique_ptr<Expression>& e) override {
        return this->visitExpression(*e);
    }
    bool visitStatementPtr(std::unique_ptr<Statement>& s) override {
        return this->visitStatement(*s);
    }
};

}  // namespace SkSL

#endif
