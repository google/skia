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
#include "include/sksl/DSLType.h"

namespace SkSL {

class IRGenerator;
class Variable;
enum class VariableStorage : int8_t;

namespace dsl {

class DSLVar {
public:
    /**
     * Creates an empty, unpopulated DSLVar. Can be replaced with a real DSLVar later via `swap`.
     */
    DSLVar() : fType(kVoid_Type), fDeclared(true) {}

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

    DSLVar(DSLVar&&) = default;

    ~DSLVar();

    const char* name() const {
        return fName;
    }

    void swap(DSLVar& other);

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

    DSLPossibleExpression operator=(DSLVar& var) {
        return this->operator=(DSLExpression(var));
    }

    DSLPossibleExpression operator=(DSLExpression expr);

    DSLPossibleExpression operator=(int expr) {
        return this->operator=(DSLExpression(expr));
    }

    DSLPossibleExpression operator=(float expr) {
        return this->operator=(DSLExpression(expr));
    }

    DSLPossibleExpression operator=(double expr) {
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

    DSLModifiers fModifiers;
    // We only need to keep track of the type here so that we can create the SkSL::Variable. For
    // predefined variables this field is unnecessary, so we don't bother tracking it and just set
    // it to kVoid; in other words, you shouldn't generally be relying on this field to be correct.
    // If you need to determine the variable's type, look at DSLWriter::Var(...).type() instead.
    DSLType fType;
    int fUniformHandle = -1;
    std::unique_ptr<SkSL::Statement> fDeclaration;
    const SkSL::Variable* fVar = nullptr;
    const char* fRawName = nullptr; // for error reporting
    const char* fName = nullptr;
    DSLExpression fInitialValue;
    VariableStorage fStorage;
    bool fDeclared = false;

    friend DSLVar sk_SampleCoord();

    friend class DSLCore;
    friend class DSLExpression;
    friend class DSLFunction;
    friend class DSLWriter;
    friend class ::SkSL::IRGenerator;
};

} // namespace dsl

} // namespace SkSL


#endif
