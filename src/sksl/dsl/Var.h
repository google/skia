/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/dsl/Expression.h"
#include "src/sksl/dsl/Modifiers.h"

namespace SkSL {

namespace dsl {

class Variable;

class Type;

class Var {
public:
    Var(const char* name);

    Var(Type type, Expression initialValue);

    Var(Modifiers modifiers, const SkSL::Type& type, Expression initialValue);

    Var(Type type, const char* name = "var");

    Var(Modifiers modifiers, Type type, const char* name = "var");

    Var(Type type, const char* name, Expression initialValue);

    Var(Modifiers modifiers, Type type, const char* name, Expression initialValue);

    Var(Var&&) = delete;

    Var& operator=(Var&&) = delete;

    Expression operator=(Expression&& expr);

    Expression operator=(int expr) {
        return this->operator=(Expression(expr));
    }

    Expression operator=(float expr) {
        return this->operator=(Expression(expr));
    }

    Expression operator[](Expression&& index);

    const SkSL::Variable* var() const;

    const SkSL::String& name() const {
        return fName;
    }

    GrGLSLUniformHandler::UniformHandle uniformHandle() {
        return fUniformHandle;
    }

private:
    SkSL::String fName;
    const SkSL::Variable* fVar;
    Expression fInitialValue;
    GrGLSLUniformHandler::UniformHandle fUniformHandle;

    friend class DSLWriter;
};

} // namespace dsl

} // namespace SkSL


#endif
