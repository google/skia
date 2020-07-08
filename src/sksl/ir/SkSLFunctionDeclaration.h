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

#include <atomic>

namespace SkSL {

struct FunctionDefinition;

/**
 * A function declaration (not a definition -- does not contain a body).
 */
struct FunctionDeclaration : public Symbol {
    FunctionDeclaration(int offset, Modifiers modifiers, StringFragment name,
                        std::vector<const Variable*> parameters, const Type& returnType,
                        bool builtin)
    : INHERITED(offset, kFunctionDeclaration_Kind, std::move(name))
    , fDefinition(nullptr)
    , fBuiltin(builtin)
    , fModifiers(modifiers)
    , fParameters(std::move(parameters))
    , fReturnType(returnType) {}

    String declaration() const {
        String result = fReturnType.displayName() + " " + fName + "(";
        String separator;
        for (auto p : fParameters) {
            result += separator;
            separator = ", ";
            result += p->fType.displayName();
        }
        result += ")";
        return result;
    }

    String description() const override {
        return this->declaration();
    }

#ifdef SKSL_STANDALONE
    static bool is_simple(const Variable* p) {
        return p->fModifiers == Modifiers() &&
               !strstr(SymbolWriter::symbolCode(p->fType).c_str(), "new ");
    }

    String constructionCode() const override {
        String parameters;
        const char* separator = "";
        for (const auto& p : fParameters) {
            parameters += separator;
            separator = ", ";
            if (is_simple(p)) {
                parameters += String::printf("simple_parameter(\"%s\", \"%s\", symbols)",
                                             String(p->fName).c_str(), p->fType.name().c_str());
            } else {
                parameters += String::printf("(Variable*) symbols->takeOwnership("
                                             "std::unique_ptr<Symbol>(%s))",
                                             p->constructionCode().c_str());
            }
        }
        return String::printf("new FunctionDeclaration(-1, %s, \"%s\", { %s }, *%s, %d)",
                              fModifiers.constructionCode().c_str(),
                              String(fName).c_str(), parameters.c_str(),
                              SymbolWriter::symbolCode(fReturnType).c_str(),
                              fBuiltin);
    }

    String addSymbolCode() const override {
        String parameters;
        for (const auto& p : fParameters) {
            if (is_simple(p)) {
                parameters += String::printf(", \"%s\", \"%s\"",
                                             String(p->fName).c_str(),
                                             p->fType.name().c_str());
            } else {
                return INHERITED::addSymbolCode();
            }
        }
        return String::printf("create_simple_function(symbols, %s, \"%s\", \"%s\", %d%s);\n",
                              fModifiers.constructionCode().c_str(), String(fName).c_str(),
                              String(fReturnType.fName).c_str(), (int) fParameters.size(),
                              parameters.c_str());
    }
#endif

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
        SkASSERT(arguments.size() == fParameters.size());
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
            if (genericIndex == -1) {
                return false;
            }
            *outReturnType = fReturnType.coercibleTypes()[genericIndex];
        } else {
            *outReturnType = &fReturnType;
        }
        return true;
    }

    mutable FunctionDefinition* fDefinition;
    bool fBuiltin;
    Modifiers fModifiers;
    const std::vector<const Variable*> fParameters;
    const Type& fReturnType;
    mutable std::atomic<int> fCallCount = 0;

    typedef Symbol INHERITED;
};

} // namespace

#endif
