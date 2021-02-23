/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLAnalysis_DEFINED
#define SkSLAnalysis_DEFINED

#include "include/private/SkSLSampleUsage.h"
#include "src/sksl/SkSLDefines.h"

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
    static SampleUsage GetSampleUsage(const Program& program, const Variable& fp);

    static bool ReferencesBuiltin(const Program& program, int builtin);

    static bool ReferencesSampleCoords(const Program& program);
    static bool ReferencesFragCoords(const Program& program);

    static int NodeCountUpToLimit(const FunctionDefinition& function, int limit);

    /**
     * Finds unconditional exits from a switch-case. Returns true if this statement unconditionally
     * causes an exit from this switch (via continue, break or return).
     */
    static bool SwitchCaseContainsUnconditionalExit(Statement& stmt);

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
    static bool IsAssignable(Expression& expr, AssignmentInfo* info,
                             ErrorReporter* errors = nullptr);

    // Updates the `refKind` field of every VariableReference found within `expr`.
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

    struct UnrollableLoopInfo {
        const Variable* fIndex;
        double fStart;
        double fDelta;
        int fCount;
    };

    // Ensures that 'loop' meets the strict requirements of The OpenGL ES Shading Language 1.00,
    // Appendix A, Section 4.
    // Information about the loop's structure are placed in outLoopInfo (if not nullptr).
    // If the function returns false, specific reasons are reported via errors (if not nullptr).
    static bool ForLoopIsValidForES2(const ForStatement& loop,
                                     UnrollableLoopInfo* outLoopInfo,
                                     ErrorReporter* errors);

    static void ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors);
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

template <typename PROG, typename EXPR, typename STMT, typename ELEM>
class TProgramVisitor {
public:
    virtual ~TProgramVisitor() = default;

protected:
    virtual bool visitExpression(EXPR expression);
    virtual bool visitStatement(STMT statement);
    virtual bool visitProgramElement(ELEM programElement);
};

// Squelch bogus Clang warning about template vtables: https://bugs.llvm.org/show_bug.cgi?id=18733
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-template-vtables"
#endif
extern template class TProgramVisitor<const Program&, const Expression&,
                                      const Statement&, const ProgramElement&>;
extern template class TProgramVisitor<Program&, Expression&, Statement&, ProgramElement&>;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

class ProgramVisitor : public TProgramVisitor<const Program&,
                                              const Expression&,
                                              const Statement&,
                                              const ProgramElement&> {
public:
    bool visit(const Program& program);
};

using ProgramWriter = TProgramVisitor<Program&, Expression&, Statement&, ProgramElement&>;

}  // namespace SkSL

#endif
