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
#include "SkSLType.h"
#include "SkSLVariable.h"

namespace SkSL {

/**
 * A function declaration (not a definition -- does not contain a body).
 */
struct FunctionDeclaration : public Symbol {
    FunctionDeclaration(Position position, std::string name, 
                        std::vector<std::shared_ptr<Variable>> parameters, 
                        std::shared_ptr<Type> returnType)
    : INHERITED(position, kFunctionDeclaration_Kind, std::move(name))
    , fDefined(false)
    , fParameters(parameters)
    , fReturnType(returnType) {}

    std::string description() const override {
        std::string result = fReturnType->description() + " " + fName + "(";
        std::string separator = "";
        for (auto p : fParameters) {
            result += separator;
            separator = ", ";
            result += p->description();
        }
        result += ")";
        return result;
    }

    bool matches(FunctionDeclaration& f) {
        return fName == f.fName && fParameters == f.fParameters;
    }

    mutable bool fDefined;
    const std::vector<std::shared_ptr<Variable>> fParameters;
    const std::shared_ptr<Type> fReturnType;

    typedef Symbol INHERITED;
};

} // namespace

#endif
