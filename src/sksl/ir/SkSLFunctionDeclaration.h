/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

#include "SkSLExpression.h"
#include "SkSLModifiers.h"
#include "SkSLSymbol.h"
#include "SkSLSymbolTable.h"
#include "SkSLType.h"
#include "SkSLVariable.h"

namespace SkSL {

/**
 * A function declaration (not a definition -- does not contain a body).
 */
struct FunctionDeclaration : public Symbol {
    FunctionDeclaration(Position position, String name,
                        std::vector<const Variable*> parameters, const Type& returnType)
    : INHERITED(position, kFunctionDeclaration_Kind, std::move(name))
    , fDefined(false)
    , fBuiltin(false)
    , fParameters(std::move(parameters))
    , fReturnType(returnType) {}

    String description() const override {
        String result = fReturnType.description() + " " + fName + "(";
        String separator;
        for (auto p : fParameters) {
            result += separator;
            separator = ", ";
            result += p->description();
        }
        result += ")";
        return result;
    }

    bool matches(const FunctionDeclaration& f) const {
        if (fName != f.fName) {
            return false;
        }
        if (fParameters.size() != f.fParameters.size()) {
            return false;
        }
        for (size_t i = 0; i < fParameters.size(); i++) {
            if (fParameters[i]->fType != f.fParameters[i]->fType) {
                return false;
            }
        }
        return true;
    }

    /**
     * Determine the effective types of this function's parameters and return value when called with
     * the given arguments. This is relevant for functions with generic parameter types, where this
     * will collapse the generic types down into specific concrete types.
     *
     * Returns true if it was able to select a concrete set of types for the generic function, false
     * if there is no possible way this can match the argument types. Note that even a true return
     * does not guarantee that the function can be successfully called with those arguments, merely
     * indicates that an attempt should be made. If false is returned, the state of
     * outParameterTypes and outReturnType are undefined.
     */
    bool determineFinalTypes(const std::vector<std::unique_ptr<Expression>>& arguments,
                             std::vector<const Type*>* outParameterTypes,
                             const Type** outReturnType) const {
        assert(arguments.size() == fParameters.size());
        int genericIndex = -1;
        for (size_t i = 0; i < arguments.size(); i++) {
            if (fParameters[i]->fType.kind() == Type::kGeneric_Kind) {
                std::vector<const Type*> types = fParameters[i]->fType.coercibleTypes();
                if (genericIndex == -1) {
                    for (size_t j = 0; j < types.size(); j++) {
                        if (arguments[i]->fType.canCoerceTo(*types[j])) {
                            genericIndex = j;
                            break;
                        }
                    }
                    if (genericIndex == -1) {
                        return false;
                    }
                }
                outParameterTypes->push_back(types[genericIndex]);
            } else {
                outParameterTypes->push_back(&fParameters[i]->fType);
            }
        }
        if (fReturnType.kind() == Type::kGeneric_Kind) {
            assert(genericIndex != -1);
            *outReturnType = fReturnType.coercibleTypes()[genericIndex];
        } else {
            *outReturnType = &fReturnType;
        }
        return true;
    }

    mutable bool fDefined;
    bool fBuiltin;
    const std::vector<const Variable*> fParameters;
    const Type& fReturnType;

    typedef Symbol INHERITED;
};

} // namespace

#endif
