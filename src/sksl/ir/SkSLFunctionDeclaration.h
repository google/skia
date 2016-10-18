/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

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
    FunctionDeclaration(Position position, std::string name, 
                        std::vector<const Variable*> parameters, const Type& returnType)
    : INHERITED(position, kFunctionDeclaration_Kind, std::move(name))
    , fDefined(false)
    , fParameters(std::move(parameters))
    , fReturnType(returnType) {}

    std::string description() const override {
        std::string result = fReturnType.description() + " " + fName + "(";
        std::string separator = "";
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

    mutable bool fDefined;
    const std::vector<const Variable*> fParameters;
    const Type& fReturnType;

    typedef Symbol INHERITED;
};

} // namespace

#endif
