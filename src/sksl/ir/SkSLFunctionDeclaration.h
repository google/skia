/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDECLARATION
#define SKSL_FUNCTIONDECLARATION

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramKind.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkTArray.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class FunctionDefinition;

// This enum holds every intrinsic supported by SkSL.
#define SKSL_INTRINSIC(name) k_##name##_IntrinsicKind,
enum IntrinsicKind : int8_t {
    kNotIntrinsic = -1,
    SKSL_INTRINSIC_LIST
};
#undef SKSL_INTRINSIC

/**
 * A function declaration (not a definition -- does not contain a body).
 */
class FunctionDeclaration final : public Symbol {
public:
    inline static constexpr Kind kSymbolKind = Kind::kFunctionDeclaration;

    FunctionDeclaration(int line,
                        const Modifiers* modifiers,
                        skstd::string_view name,
                        std::vector<const Variable*> parameters,
                        const Type* returnType,
                        bool builtin);

    static const FunctionDeclaration* Convert(const Context& context,
                                              SymbolTable& symbols,
                                              int line,
                                              const Modifiers* modifiers,
                                              skstd::string_view name,
                                              std::vector<std::unique_ptr<Variable>> parameters,
                                              const Type* returnType);

    const Modifiers& modifiers() const {
        return *fModifiers;
    }

    const FunctionDefinition* definition() const {
        return fDefinition;
    }

    void setDefinition(const FunctionDefinition* definition) const {
        fDefinition = definition;
        fIntrinsicKind = kNotIntrinsic;
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

    bool isMain() const {
        return fIsMain;
    }

    IntrinsicKind intrinsicKind() const {
        return fIntrinsicKind;
    }

    bool isIntrinsic() const {
        return this->intrinsicKind() != kNotIntrinsic;
    }

    String mangledName() const;

    String description() const override;

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
    using ParamTypes = SkSTArray<8, const Type*>;
    bool determineFinalTypes(const ExpressionArray& arguments,
                             ParamTypes* outParameterTypes,
                             const Type** outReturnType) const;

private:
    mutable const FunctionDefinition* fDefinition;
    const Modifiers* fModifiers;
    std::vector<const Variable*> fParameters;
    const Type* fReturnType;
    bool fBuiltin;
    bool fIsMain;
    mutable IntrinsicKind fIntrinsicKind = kNotIntrinsic;

    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
