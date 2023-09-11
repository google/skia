/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPEREFERENCE
#define SKSL_TYPEREFERENCE

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <memory>
#include <string>

namespace SkSL {

enum class OperatorPrecedence : uint8_t;

/**
 * Represents an identifier referring to a type. This is an intermediate value: TypeReferences are
 * always eventually replaced by Constructors in valid programs.
 */
class TypeReference final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kTypeReference;

    TypeReference(const Context& context, Position pos, const Type* value)
            : TypeReference(pos, value, context.fTypes.fInvalid.get()) {}

    // Reports an error and returns false if the type is generic or, in a strict-ES2 program, if the
    // type is not allowed in ES2. Otherwise, returns true. (These are the same checks performed by
    // Convert.)
    static bool VerifyType(const Context& context, const SkSL::Type* type, Position pos);

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

    std::string description(OperatorPrecedence) const override {
        return std::string(this->value().name());
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::unique_ptr<Expression>(new TypeReference(pos, &this->value(), &this->type()));
    }

private:
    TypeReference(Position pos, const Type* value, const Type* type)
        : INHERITED(pos, kIRNodeKind, type)
        , fValue(*value) {}

    const Type& fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
