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
#include "src/sksl/dsl/DSLExpression.h"

namespace SkSL {

class Statement;
class Variable;

namespace dsl {

class DSLExpression;
class DSLModifiers;
class DSLType;

class DSLVar {
public:
    DSLVar(const char* name);

    DSLVar(DSLType type, const char* name = "var");

    DSLVar(DSLModifiers modifiers, DSLType type, const char* name = "var");

    DSLVar(DSLVar&&) = delete;

    DSLExpression operator=(const DSLVar& var) {
        return this->operator=(DSLExpression(var));
    }

    DSLExpression operator=(DSLExpression&& expr);

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
    const SkSL::Variable* var() const;

    const SkSL::String& name() const {
        return fName;
    }

    GrGLSLUniformHandler::UniformHandle uniformHandle();

    // this object owns the var until it is added to a symboltable
    std::unique_ptr<SkSL::Variable> fOwnedVar;
    const SkSL::Variable* fVar = nullptr;
    SkSL::String fName;
    DSLExpression fInitialValue;
    GrGLSLUniformHandler::UniformHandle fUniformHandle;

    friend class DSLExpression;
    friend class DSLFunction;
    friend class DSLWriter;
    friend DSLStatement Declare(DSLVar& var, DSLExpression initialValue);

    friend DSLExpression operator+=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator-=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator*=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator/=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator%=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator<<=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator>>=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator&=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator|=(DSLVar& left, DSLExpression right);
    friend DSLExpression operator^=(DSLVar& left, DSLExpression right);
};

} // namespace dsl

} // namespace SkSL


#endif
