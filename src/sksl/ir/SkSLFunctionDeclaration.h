/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkTArray.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class FunctionDefinition;

/**
 * A function declaration (not a definition -- does not contain a body).
 */
class FunctionDeclaration final : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kFunctionDeclaration;

    FunctionDeclaration(int offset, const Modifiers* modifiers, StringFragment name,
                        std::vector<const Variable*> parameters, const Type* returnType,
                        bool builtin)
    : INHERITED(offset, kSymbolKind, name, /*type=*/nullptr)
    , fDefinition(nullptr)
    , fModifiers(modifiers)
    , fParameters(std::move(parameters))
    , fReturnType(returnType)
    , fBuiltin(builtin) {}

    const Modifiers& modifiers() const {
        return *fModifiers;
    }

    const FunctionDefinition* definition() const {
        return fDefinition;
    }

    void setDefinition(const FunctionDefinition* definition) const {
        fDefinition = definition;
    }

    const std::vector<const Variable*>& parameters() const {
        return fParameters;
    }

    const Type& returnType() const {
        return *fReturnType;
    }

    bool isBuiltin() const {
        return fBuiltin;
    }

    String description() const override {
        String result = this->returnType().displayName() + " " + this->name() + "(";
        String separator;
        for (auto p : this->parameters()) {
            result += separator;
            separator = ", ";
            result += p->type().displayName();
            result += " ";
            result += p->name();
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
            // Non-generic parameters are final as-is.
            const Type& parameterType = parameters[i]->type();
            if (parameterType.typeKind() != Type::TypeKind::kGeneric) {
                outParameterTypes->push_back(&parameterType);
                continue;
            }
            // We use the first generic parameter we find to lock in the generic index;
            // e.g. if we find `float3` here, all `$genType`s will be assumed to be `float3`.
            const std::vector<const Type*>& types = parameterType.coercibleTypes();
            if (genericIndex == -1) {
                for (size_t j = 0; j < types.size(); j++) {
                    if (arguments[i]->type().canCoerceTo(*types[j], /*allowNarrowing=*/true)) {
                        genericIndex = j;
                        break;
                    }
                }
                if (genericIndex == -1) {
                    // The passed-in type wasn't a match for ANY of the generic possibilities.
                    // This function isn't a match at all.
                    return false;
                }
            }
            outParameterTypes->push_back(types[genericIndex]);
        }
        // Apply the generic index to our return type.
        const Type& returnType = this->returnType();
        if (returnType.typeKind() == Type::TypeKind::kGeneric) {
            if (genericIndex == -1) {
                // We don't support functions with a generic return type and no other generics.
                return false;
            }
            *outReturnType = returnType.coercibleTypes()[genericIndex];
        } else {
            *outReturnType = &returnType;
        }
        return true;
    }

private:
    mutable const FunctionDefinition* fDefinition;
    const Modifiers* fModifiers;
    std::vector<const Variable*> fParameters;
    const Type* fReturnType;
    bool fBuiltin;

    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
