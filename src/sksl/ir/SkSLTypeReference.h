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

    TypeReference(const Context& context, int line, const Type* value)
        : TypeReference(line, value, context.fTypes.fInvalid.get()) {}

    // Creates a reference to an SkSL type; uses the ErrorReporter to report errors.
    static std::unique_ptr<TypeReference> Convert(const Context& context,
                                                  int line,
                                                  const Type* type);

    // Creates a reference to an SkSL type; reports errors via ASSERT.
    static std::unique_ptr<TypeReference> Make(const Context& context, int line, const Type* type);

    const Type& value() const {
        return fValue;
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    String description() const override {
        return String(this->value().name());
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TypeReference(fLine, &this->value(), &this->type()));
    }

private:
    TypeReference(int line, const Type* value, const Type* type)
        : INHERITED(line, kExpressionKind, type)
        , fValue(*value) {}

    const Type& fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
