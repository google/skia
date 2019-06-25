/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALFUNCTIONCALL
#define SKSL_EXTERNALFUNCTIONCALL

#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An external function invocation.
 */
struct ExternalFunctionCall : public Expression {
    ExternalFunctionCall(IRGenerator* irGenerator, int offset, IRNode::ID type,
                         ExternalValue* function, std::vector<IRNode::ID> arguments)
    : INHERITED(irGenerator, offset, kExternalFunctionCall_Kind, type)
    , fFunction(function)
    , fArguments(std::move(arguments)) {}

    bool hasSideEffects() const override {
        return true;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new ExternalFunctionCall(fIRGenerator, fOffset, fType,
                                                                 fFunction, fArguments));
    }

    String description() const override {
        String result = String(fFunction->fName) + "(";
        String separator;
        for (auto arg : fArguments) {
            result += separator;
            result += arg.node().description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    ExternalValue* fFunction;
    std::vector<IRNode::ID> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
