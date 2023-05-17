/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLSymbol.h"

#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Context;
class ExpressionArray;
class FunctionDefinition;
class Position;
class Type;
class Variable;

struct Modifiers;

/**
 * A function declaration (not a definition -- does not contain a body).
 */
class FunctionDeclaration final : public Symbol {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kFunctionDeclaration;

    FunctionDeclaration(Position pos,
                        const Modifiers* modifiers,
                        std::string_view name,
                        skia_private::TArray<Variable*> parameters,
                        const Type* returnType,
                        bool builtin);

    static FunctionDeclaration* Convert(const Context& context,
                                        Position pos,
                                        Position modifiersPos,
                                        const Modifiers* modifiers,
                                        std::string_view name,
                                        skia_private::TArray<std::unique_ptr<Variable>> parameters,
                                        Position returnTypePos,
                                        const Type* returnType);

    void addParametersToSymbolTable(const Context& context);

    const Modifiers& modifiers() const {
        return *fModifiers;
    }

    void setModifiers(const Modifiers* m) {
        fModifiers = m;
    }

    const FunctionDefinition* definition() const {
        return fDefinition;
    }

    void setDefinition(const FunctionDefinition* definition) {
        fDefinition = definition;
        fIntrinsicKind = kNotIntrinsic;
    }

    void setNextOverload(FunctionDeclaration* overload) {
        SkASSERT(!overload || overload->name() == this->name());
        fNextOverload = overload;
    }

    SkSpan<Variable* const> parameters() const {
        return fParameters;
    }

    const Type& returnType() const {
        return *fReturnType;
    }

    bool isBuiltin() const {
        return fBuiltin;
    }

    bool isMain() const {
        return fIsMain;
    }

    IntrinsicKind intrinsicKind() const {
        return fIntrinsicKind;
    }

    bool isIntrinsic() const {
        return this->intrinsicKind() != kNotIntrinsic;
    }

    const FunctionDeclaration* nextOverload() const {
        return fNextOverload;
    }

    FunctionDeclaration* mutableNextOverload() const {
        return fNextOverload;
    }

    std::string mangledName() const;

    std::string description() const override;

    bool matches(const FunctionDeclaration& f) const;

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
    using ParamTypes = skia_private::STArray<8, const Type*>;
    bool determineFinalTypes(const ExpressionArray& arguments,
                             ParamTypes* outParameterTypes,
                             const Type** outReturnType) const;

private:
    const FunctionDefinition* fDefinition;
    FunctionDeclaration* fNextOverload = nullptr;
    const Modifiers* fModifiers;
    skia_private::TArray<Variable*> fParameters;
    const Type* fReturnType;
    bool fBuiltin;
    bool fIsMain;
    mutable IntrinsicKind fIntrinsicKind = kNotIntrinsic;

    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
