/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

#include "include/private/SkTArray.h"
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
class FunctionDeclaration : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kFunctionDeclaration;

    FunctionDeclaration(int offset, ModifiersPool::Handle modifiers, StringFragment name,
                        std::vector<const Variable*> parameters, const Type* returnType,
                        bool builtin)
    : INHERITED(offset, FunctionDeclarationData{name, /*fDefiniition=*/nullptr, modifiers,
                                                std::move(parameters), returnType,
                                                /*fCallCount=*/0, builtin}) {}

    const Modifiers& modifiers() const {
        return *this->functionDeclarationData().fModifiersHandle;
    }

    StringFragment name() const override {
        return this->functionDeclarationData().fName;
    }

    const FunctionDefinition* definition() const {
        return this->functionDeclarationData().fDefinition;
    }

    void setDefinition(const FunctionDefinition* definition) const {
        this->functionDeclarationData().fDefinition = definition;
    }

    const std::vector<const Variable*>& parameters() const {
        return this->functionDeclarationData().fParameters;
    }

    const Type& returnType() const {
        return *this->functionDeclarationData().fReturnType;
    }

    bool isBuiltin() const {
        return this->functionDeclarationData().fBuiltin;
    }

    std::atomic<int>& callCount() const {
        return this->functionDeclarationData().fCallCount;
    }

    String description() const override {
        String result = this->returnType().displayName() + " " + this->name() + "(";
        String separator;
        for (auto p : this->parameters()) {
            result += separator;
            separator = ", ";
            result += p->type().displayName();
        }
        result += ")";
        return result;
    }

    bool matches(const FunctionDeclaration& f) const {
        if (this->name() != f.name()) {
            return false;
        }
        const std::vector<const Variable*>& parameters = this->parameters();
        const std::vector<const Variable*>& otherParameters = f.parameters();
        if (parameters.size() != otherParameters.size()) {
            return false;
        }
        for (size_t i = 0; i < parameters.size(); i++) {
            if (parameters[i]->type() != otherParameters[i]->type()) {
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
     *
     * This always assumes narrowing conversions are *allowed*. The calling code needs to verify
     * that each argument can actually be coerced to the final parameter type, respecting the
     * narrowing-conversions flag. This is handled in callCost(), or in convertCall() (via coerce).
     */
    using ParamTypes = SkSTArray<8, const Type*>;
    bool determineFinalTypes(const ExpressionArray& arguments,
                             ParamTypes* outParameterTypes, const Type** outReturnType) const {
        const std::vector<const Variable*>& parameters = this->parameters();
        SkASSERT(arguments.size() == parameters.size());

        outParameterTypes->reserve_back(arguments.size());
        int genericIndex = -1;
        for (size_t i = 0; i < arguments.size(); i++) {
            const Type& parameterType = parameters[i]->type();
            if (parameterType.typeKind() == Type::TypeKind::kGeneric) {
                const std::vector<const Type*>& types = parameterType.coercibleTypes();
                if (genericIndex == -1) {
                    for (size_t j = 0; j < types.size(); j++) {
                        if (arguments[i]->type().canCoerceTo(*types[j], /*allowNarrowing=*/true)) {
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
                outParameterTypes->push_back(&parameterType);
            }
        }
        const Type& returnType = this->returnType();
        if (returnType.typeKind() == Type::TypeKind::kGeneric) {
            if (genericIndex == -1) {
                return false;
            }
            *outReturnType = returnType.coercibleTypes()[genericIndex];
        } else {
            *outReturnType = &returnType;
        }
        return true;
    }

private:
    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
