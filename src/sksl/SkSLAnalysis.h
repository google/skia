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

namespace SkSL {

struct Expression;
struct Program;
struct ProgramElement;
struct Statement;
struct Variable;

/**
 * Provides utilities for analyzing SkSL statically before it's composed into a full program.
 */
struct Analysis {
    static SampleUsage GetSampleUsage(const Program& program, const Variable& fp);

    static bool ReferencesSampleCoords(const Program& program);
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

class ProgramVisitor {
public:
    virtual ~ProgramVisitor() { SkASSERT(!fProgram); }

    bool visit(const Program&);

protected:
    const Program& program() const {
        SkASSERT(fProgram);
        return *fProgram;
    }

    virtual bool visitExpression(const Expression&);
    virtual bool visitStatement(const Statement&);
    virtual bool visitProgramElement(const ProgramElement&);

private:
    const Program* fProgram;
};

}

#endif
