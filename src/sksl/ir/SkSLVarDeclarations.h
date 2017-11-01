/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "SkSLExpression.h"
#include "SkSLProgramElement.h"
#include "SkSLStatement.h"
#include "SkSLVariable.h"

namespace SkSL {

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct VarDeclarations : public ProgramElement {
    VarDeclarations(int offset, const Type* baseType, std::vector<Variable*> vars)
    : INHERITED(offset, kVar_Kind)
    , fBaseType(*baseType)
    , fVars(std::move(vars)) {}

    String description() const override {
        if (!fVars.size()) {
            return String();
        }
        String result = fVars[0]->fModifiers.description() +
                fBaseType.description() + " ";
        String separator;
        for (const auto& var : fVars) {
            result += separator;
            separator = ", ";
            result += var->fName;
            for (const auto& size : var->fSizes) {
                if (size) {
                    result += "[" + size->description() + "]";
                } else {
                    result += "[]";
                }
            }
            if (var->fInitialValue) {
                result += " = " + var->fInitialValue->description();
            }
        }
        return result;
    }

    const Type& fBaseType;
    std::vector<Variable*> fVars;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
