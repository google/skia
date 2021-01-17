/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/sksl/dsl/DSLExpression.h"

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

    DSLVar(DSLVar&&) = delete;

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

    const SkSL::Variable* var() const;

    const char* name() const {
        return fName;
    }

    // this object owns the var until it is added to a symboltable
    std::unique_ptr<SkSL::Variable> fOwnedVar;
    // mutable to allow us to cache lookups of system vars
    mutable const SkSL::Variable* fVar = nullptr;
    const char* fName;

    friend class DSLExpression;
    friend class DSLWriter;
};

} // namespace dsl

} // namespace SkSL


#endif
