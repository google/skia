/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLModifiers.h"

namespace SkSL {

class Variable;

namespace dsl {

class DSLType;

class DSLVar {
public:
    /**
     * Constructs a new variable with the specified type and name. The name is used (in mangled
     * form) in the resulting shader code; it is not otherwise important. Since mangling prevents
     * name conflicts and the variable's name is only important when debugging shaders, the name
     * parameter is optional.
     */
    DSLVar(DSLType type, const char* name = "var");

    DSLVar(DSLModifiers modifiers, DSLType type, const char* name = "var");

    DSLVar(DSLVar&&) = delete;

    DSLExpression x() {
        return DSLExpression(*this).x();
    }

    DSLExpression y() {
        return DSLExpression(*this).y();
    }

    DSLExpression z() {
        return DSLExpression(*this).z();
    }

    DSLExpression w() {
        return DSLExpression(*this).w();
    }

    DSLExpression r() {
        return DSLExpression(*this).r();
    }

    DSLExpression g() {
        return DSLExpression(*this).g();
    }

    DSLExpression b() {
        return DSLExpression(*this).b();
    }

    DSLExpression a() {
        return DSLExpression(*this).a();
    }

    DSLExpression field(const char* name) {
        return DSLExpression(*this).field(name);
    }

    DSLExpression operator=(const DSLVar& var) {
        return this->operator=(DSLExpression(var));
    }

    DSLExpression operator=(DSLExpression expr);

    DSLExpression operator=(int expr) {
        return this->operator=(DSLExpression(expr));
    }

    DSLExpression operator=(float expr) {
        return this->operator=(DSLExpression(expr));
    }

    DSLExpression operator[](DSLExpression&& index);

    DSLExpression operator++() {
        return ++DSLExpression(*this);
    }

    DSLExpression operator++(int) {
        return DSLExpression(*this)++;
    }

private:
    /**
     * Constructs a reference to a variable that already exists in the symbol table. This is used
     * internally to reference built-in vars.
     */
    DSLVar(const char* name);

    const SkSL::Variable* var() const {
        return fVar;
    }

    const char* name() const {
        return fName;
    }

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    GrGLSLUniformHandler::UniformHandle fUniformHandle;

    GrGLSLUniformHandler::UniformHandle uniformHandle() const;
#endif

    std::unique_ptr<SkSL::Statement> fDeclaration;
    const SkSL::Variable* fVar = nullptr;
    const char* fName;

    friend class DSLCore;
    friend class DSLExpression;
    friend class DSLWriter;
};

} // namespace dsl

} // namespace SkSL


#endif
