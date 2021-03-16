/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLModifiers.h"

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
    DSLVar(DSLType type, const char* name = "var", DSLExpression initialValue = DSLExpression());

    DSLVar(DSLType type, DSLExpression initialValue);

    DSLVar(DSLModifiers modifiers, DSLType type, const char* name = "var",
           DSLExpression initialValue = DSLExpression());

    DSLVar(DSLModifiers modifiers, DSLType type, DSLExpression initialValue);

    DSLVar(DSLVar&&) = delete;

    ~DSLVar();

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

    DSLPossibleExpression operator=(const DSLVar& var) {
        return this->operator=(DSLExpression(var));
    }

    DSLPossibleExpression operator=(DSLExpression expr);

    DSLPossibleExpression operator=(int expr) {
        return this->operator=(DSLExpression(expr));
    }

    DSLPossibleExpression operator=(float expr) {
        return this->operator=(DSLExpression(expr));
    }

    DSLPossibleExpression operator[](DSLExpression&& index);

    DSLPossibleExpression operator++() {
        return ++DSLExpression(*this);
    }

    DSLPossibleExpression operator++(int) {
        return DSLExpression(*this)++;
    }

    DSLPossibleExpression operator--() {
        return --DSLExpression(*this);
    }

    DSLPossibleExpression operator--(int) {
        return DSLExpression(*this)--;
    }

private:
    /**
     * Constructs a reference to a variable that already exists in the symbol table. This is used
     * internally to reference built-in vars.
     */
    DSLVar(const char* name);

    const char* name() const {
        return fName;
    }

    int fUniformHandle;
    std::unique_ptr<SkSL::Statement> fDeclaration;
    bool fDeclared = false;
    const SkSL::Variable* fVar;
    const char* fRawName; // for error reporting
    const char* fName;

    friend DSLVar sk_SampleCoord();

    friend class DSLCore;
    friend class DSLExpression;
    friend class DSLFunction;
    friend class DSLWriter;
};

} // namespace dsl

} // namespace SkSL


#endif
