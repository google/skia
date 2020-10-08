/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLAnalysis_DEFINED
#define SkSLAnalysis_DEFINED

#include <vector>

#include "include/private/SkSLSampleUsage.h"
#include "src/sksl/SkSLDefines.h"

namespace SkSL {

class ErrorReporter;
class Expression;
struct FunctionDefinition;
struct Program;
class ProgramElement;
class Statement;
class Variable;
class VariableReference;

/**
 * Provides utilities for analyzing SkSL statically before it's composed into a full program.
 */
struct Analysis {
    static SampleUsage GetSampleUsage(const Program& program, const Variable& fp);

    static bool ReferencesBuiltin(const Program& program, int builtin);

    static bool ReferencesSampleCoords(const Program& program);
    static bool ReferencesFragCoords(const Program& program);

    static bool NodeCountExceeds(const FunctionDefinition& function, int limit);

    static bool StatementWritesToVariable(const Statement& stmt, const Variable& var);
    static bool IsAssignable(Expression& expr, VariableReference** assignableVar,
                             ErrorReporter* errors = nullptr);
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

    bool visit(PROG program);

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

using ProgramVisitor = TProgramVisitor<const Program&, const Expression&,
                                       const Statement&, const ProgramElement&>;
using ProgramWriter = TProgramVisitor<Program&, Expression&, Statement&, ProgramElement&>;

}  // namespace SkSL

#endif
