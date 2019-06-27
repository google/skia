/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

/**
 * A function declaration (not a definition -- does not contain a body).
 */
struct FunctionDeclaration : public Symbol {
    FunctionDeclaration(IRGenerator* irGenerator, int offset, Modifiers modifiers,
                        StringFragment name, std::vector<IRNode::ID> parameters,
                        IRNode::ID returnType)
    : INHERITED(irGenerator, offset, kFunctionDeclaration_Kind, std::move(name))
    , fDefined(false)
    , fBuiltin(false)
    , fModifiers(modifiers)
    , fParameters(std::move(parameters))
    , fReturnType(returnType) {}

    String description() const override {
        String result = fReturnType.node().description() + " " + fName + "(";
        String separator;
        for (auto p : fParameters) {
            result += separator;
            separator = ", ";
            result += p.node().description();
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
            if (fParameters[i].expressionNode().fType != f.fParameters[i].expressionNode().fType) {
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
    bool determineFinalTypes(const std::vector<IRNode::ID>& arguments,
                             std::vector<IRNode::ID>* outParameterTypes,
                             IRNode::ID* outReturnType) const {
        SkASSERT(arguments.size() == fParameters.size());
        int genericIndex = -1;
        for (size_t i = 0; i < arguments.size(); i++) {
            Expression& arg = (Expression&) arguments[i].expressionNode();
            const Type& type = (Type&) arg.fType.typeNode();
            const Variable& parameter = (const Variable&) fParameters[i].node();
            if (parameter.fType.typeNode().kind() == Type::kGeneric_Kind) {
                std::vector<IRNode::ID> types = parameter.fType.typeNode().coercibleTypes();
                if (genericIndex == -1) {
                    for (size_t j = 0; j < types.size(); j++) {
                        if (type.canCoerceTo(types[j].typeNode())) {
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
                outParameterTypes->push_back(parameter.fType);
            }
        }
        if (fReturnType.typeNode().kind() == Type::kGeneric_Kind) {
            SkASSERT(genericIndex != -1);
            *outReturnType = fReturnType.typeNode().coercibleTypes()[genericIndex];
        } else {
            *outReturnType = fReturnType;
        }
        return true;
    }

    mutable bool fDefined;
    bool fBuiltin;
    Modifiers fModifiers;
    const std::vector<IRNode::ID> fParameters;
    IRNode::ID fReturnType;

    typedef Symbol INHERITED;
};

} // namespace

#endif
