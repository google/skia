/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FLOATLITERAL
#define SKSL_FLOATLITERAL

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <cinttypes>
#include <memory>
#include <optional>
#include <string>

namespace SkSL {

enum class OperatorPrecedence : uint8_t;

/**
 * A literal value. These can contain ints, floats, or booleans.
 */

class Literal : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kLiteral;

    Literal(Position pos, double value, const Type* type)
        : INHERITED(pos, kIRNodeKind, type)
        , fValue(value) {}

    // Makes a literal of $floatLiteral type.
    static std::unique_ptr<Literal> MakeFloat(const Context& context, Position pos, float value) {
        return std::make_unique<Literal>(pos, value, context.fTypes.fFloatLiteral.get());
    }

    // Makes a float literal of the specified type.
    static std::unique_ptr<Literal> MakeFloat(Position pos, float value, const Type* type) {
        SkASSERT(type->isFloat());
        return std::make_unique<Literal>(pos, value, type);
    }

    // Makes a literal of $intLiteral type.
    static std::unique_ptr<Literal> MakeInt(const Context& context, Position pos, SKSL_INT value) {
        return std::make_unique<Literal>(pos, value, context.fTypes.fIntLiteral.get());
    }

    // Makes an int literal of the specified type.
    static std::unique_ptr<Literal> MakeInt(Position pos, SKSL_INT value, const Type* type) {
        SkASSERT(type->isInteger());
        SkASSERTF(value >= type->minimumValue(), "Value %" PRId64 " does not fit in type %s",
                                                 value, type->description().c_str());
        SkASSERTF(value <= type->maximumValue(), "Value %" PRId64 " does not fit in type %s",
                                                 value, type->description().c_str());
        return std::make_unique<Literal>(pos, value, type);
    }

    // Makes a literal of boolean type.
    static std::unique_ptr<Literal> MakeBool(const Context& context, Position pos, bool value) {
        return std::make_unique<Literal>(pos, value, context.fTypes.fBool.get());
    }

    // Makes a literal of boolean type. (Functionally identical to the above, but useful if you
    // don't have access to the Context.)
    static std::unique_ptr<Literal> MakeBool(Position pos, bool value, const Type* type) {
        SkASSERT(type->isBoolean());
        return std::make_unique<Literal>(pos, value, type);
    }

    // Makes a literal of the specified type, rounding as needed.
    static std::unique_ptr<Literal> Make(Position pos, double value, const Type* type) {
        if (type->isFloat()) {
            return MakeFloat(pos, value, type);
        }
        if (type->isInteger()) {
            return MakeInt(pos, value, type);
        }
        SkASSERT(type->isBoolean());
        return MakeBool(pos, value, type);
    }

    float floatValue() const {
        SkASSERT(this->type().isFloat());
        return (SKSL_FLOAT)fValue;
    }

    SKSL_INT intValue() const {
        SkASSERT(this->type().isInteger());
        return (SKSL_INT)fValue;
    }

    SKSL_INT boolValue() const {
        SkASSERT(this->type().isBoolean());
        return (bool)fValue;
    }

    double value() const {
        return fValue;
    }

    std::string description(OperatorPrecedence) const override;

    ComparisonResult compareConstant(const Expression& other) const override {
        if (!other.is<Literal>() || this->type().numberKind() != other.type().numberKind()) {
            return ComparisonResult::kUnknown;
        }
        return this->value() == other.as<Literal>().value()
                       ? ComparisonResult::kEqual
                       : ComparisonResult::kNotEqual;
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<Literal>(pos, this->value(), &this->type());
    }

    bool supportsConstantValues() const override {
        return true;
    }

    std::optional<double> getConstantValue(int n) const override {
        SkASSERT(n == 0);
        return fValue;
    }

private:
    double fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
