/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONCALL
#define SKSL_FUNCTIONCALL

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * A function invocation.
 */
struct FunctionCall : public Expression {
    FunctionCall(IRGenerator* irGenerator, int offset, IRNode::ID type,
                 IRNode::ID function, std::vector<IRNode::ID> arguments)
    : INHERITED(irGenerator, offset, kFunctionCall_Kind, type)
    , fFunction(std::move(function))
    , fArguments(std::move(arguments)) {}

    bool hasSideEffects() const override {
        for (const auto& arg : fArguments) {
            if (arg.expression().hasSideEffects()) {
                return true;
            }
        }
        return ((FunctionDeclaration&) fFunction.node()).fModifiers.fFlags &
               Modifiers::kHasSideEffects_Flag;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new FunctionCall(fIRGenerator, fOffset, fType, fFunction,
                                                         fArguments));
    }

    String description() const override {
        String result = String(((FunctionDeclaration&) fFunction.node()).fName) + "(";
        String separator;
        for (auto arg : fArguments) {
            result += separator;
            result += arg.node().description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    IRNode::ID fFunction;
    std::vector<IRNode::ID> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
