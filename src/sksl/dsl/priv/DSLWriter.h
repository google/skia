/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "include/core/SkStringView.h"
#include "include/core/SkTypes.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#include <memory>

namespace SkSL {

class Variable;
class Statement;

namespace dsl {

class DSLGlobalVar;
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
     * Returns whether name mangling is enabled. Mangling is important for the DSL because its
     * variables normally all go into the same symbol table; for instance if you were to translate
     * this legal (albeit silly) GLSL code:
     *     int x;
     *     {
     *         int x;
     *     }
     *
     * into DSL, you'd end up with:
     *     DSLVar x1(kInt_Type, "x");
     *     DSLVar x2(kInt_Type, "x");
     *     Declare(x1);
     *     Block(Declare(x2));
     *
     * with x1 and x2 ending up in the same symbol table. This is fine as long as their effective
     * names are different, so mangling prevents this situation from causing problems.
     */
    static bool ManglingEnabled();

    static skstd::string_view Name(skstd::string_view name);

    /**
     * Returns the SkSL variable corresponding to a DSL var.
     */
    static const SkSL::Variable* Var(DSLVarBase& var);

    /**
     * Creates an SkSL variable corresponding to a DSLParameter.
     */
    static std::unique_ptr<SkSL::Variable> CreateParameterVar(DSLParameter& var);

    /**
     * Returns the SkSL declaration corresponding to a DSLVar.
     */
    static std::unique_ptr<SkSL::Statement> Declaration(DSLVarBase& var);

    /**
     * For use in testing only: marks the variable as having been declared, so that it can be
     * destroyed without generating errors.
     */
    static void MarkDeclared(DSLVarBase& var);

    /**
     * Returns whether DSLVars should automatically be marked declared upon creation. This is used
     * to simplify testing.
     */
    static bool MarkVarsDeclared();

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

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    static GrGLSLUniformHandler::UniformHandle VarUniformHandle(const DSLGlobalVar& var);
#endif

    friend class DSLCore;
    friend class DSLVar;
};

} // namespace dsl

} // namespace SkSL

#endif
