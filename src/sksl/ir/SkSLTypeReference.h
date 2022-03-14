/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPEREFERENCE
#define SKSL_TYPEREFERENCE

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to a type. This is an intermediate value: TypeReferences are
 * always eventually replaced by Constructors in valid programs.
 */
class TypeReference final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kTypeReference;

    TypeReference(const Context& context, Position pos, const Type* value)
        : TypeReference(pos, value, context.fTypes.fInvalid.get()) {}

    // Creates a reference to an SkSL type; uses the ErrorReporter to report errors.
    static std::unique_ptr<TypeReference> Convert(const Context& context,
                                                  Position pos,
                                                  const Type* type);

    // Creates a reference to an SkSL type; reports errors via ASSERT.
    static std::unique_ptr<TypeReference> Make(const Context& context, Position pos,
            const Type* type);

    const Type& value() const {
        return fValue;
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    std::string description() const override {
        return std::string(this->value().name());
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TypeReference(fPosition, &this->value(),
                &this->type()));
    }

private:
    TypeReference(Position pos, const Type* value, const Type* type)
        : INHERITED(pos, kExpressionKind, type)
        , fValue(*value) {}

    const Type& fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
