/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "include/core/SkTypes.h"

#include <memory>

namespace SkSL {

class Variable;
class Statement;

namespace dsl {

class DSLParameter;
class DSLStatement;
class DSLVarBase;
class DSLVar;

/**
 * Various utility methods needed by DSL code.
 */
class DSLWriter {
public:
    /**
     * Returns the SkSL variable corresponding to a DSL var.
     */
    static SkSL::Variable* Var(DSLVarBase& var);

    /**
     * Creates an SkSL variable corresponding to a DSLParameter.
     */
    static std::unique_ptr<SkSL::Variable> CreateParameterVar(DSLParameter& var);

    /**
     * Returns the SkSL declaration corresponding to a DSLVar.
     */
    static std::unique_ptr<SkSL::Statement> Declaration(DSLVarBase& var);

    /**
     * Adds a new declaration into an existing declaration statement. This either turns the original
     * declaration into an unscoped block or, if it already was, appends a new statement to the end
     * of it.
     */
    static void AddVarDeclaration(DSLStatement& existing, DSLVar& additional);

    /**
     * Clears any elements or symbols which have been output.
     */
    static void Reset();
};

} // namespace dsl

} // namespace SkSL

#endif
