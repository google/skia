/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FLOATLITERAL
#define SKSL_FLOATLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <cinttypes>

namespace SkSL {

/**
 * A literal value. These can contain ints, floats, or booleans.
 */

class Literal : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kLiteral;

    Literal(int line, double value, const Type* type)
        : INHERITED(line, kExpressionKind, type)
        , fValue(value) {}

    // Makes a literal of $floatLiteral type.
    static std::unique_ptr<Literal> MakeFloat(const Context& context, int line, float value) {
        return std::make_unique<Literal>(line, value, context.fTypes.fFloatLiteral.get());
    }

    // Makes a float literal of the specified type.
    static std::unique_ptr<Literal> MakeFloat(int line, float value, const Type* type) {
        SkASSERT(type->isFloat());
        return std::make_unique<Literal>(line, value, type);
    }

    // Makes a literal of $intLiteral type.
    static std::unique_ptr<Literal> MakeInt(const Context& context, int line, SKSL_INT value) {
        return std::make_unique<Literal>(line, value, context.fTypes.fIntLiteral.get());
    }

    // Makes an int literal of the specified type.
    static std::unique_ptr<Literal> MakeInt(int line, SKSL_INT value, const Type* type) {
        SkASSERT(type->isInteger());
        SkASSERTF(value >= type->minimumValue(), "Value %" PRId64 " does not fit in type %s",
                                                 value, type->description().c_str());
        SkASSERTF(value <= type->maximumValue(), "Value %" PRId64 " does not fit in type %s",
                                                 value, type->description().c_str());
        return std::make_unique<Literal>(line, value, type);
    }

    // Makes a literal of boolean type.
    static std::unique_ptr<Literal> MakeBool(const Context& context, int line, bool value) {
        return std::make_unique<Literal>(line, value, context.fTypes.fBool.get());
    }

    // Makes a literal of boolean type. (Functionally identical to the above, but useful if you
    // don't have access to the Context.)
    static std::unique_ptr<Literal> MakeBool(int line, bool value, const Type* type) {
        SkASSERT(type->isBoolean());
        return std::make_unique<Literal>(line, value, type);
    }

    // Makes a literal of the specified type, rounding as needed.
    static std::unique_ptr<Literal> Make(int line, double value, const Type* type) {
        if (type->isFloat()) {
            return MakeFloat(line, value, type);
        }
        if (type->isInteger()) {
            return MakeInt(line, value, type);
        }
        SkASSERT(type->isBoolean());
        return MakeBool(line, value, type);
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

    String description() const override {
        if (this->type().isFloat()) {
            return to_string(this->floatValue());
        }
        if (this->type().isInteger()) {
            return to_string(this->intValue());
        }
        SkASSERT(this->type().isBoolean());
        return fValue ? "true" : "false";
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isCompileTimeConstant() const override {
        return true;
    }

    ComparisonResult compareConstant(const Expression& other) const override {
        if (!other.is<Literal>() || this->type().numberKind() != other.type().numberKind()) {
            return ComparisonResult::kUnknown;
        }
        return this->value() == other.as<Literal>().value()
                       ? ComparisonResult::kEqual
                       : ComparisonResult::kNotEqual;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Literal>(fLine, this->value(), &this->type());
    }

    bool allowsConstantSubexpressions() const override {
        return true;
    }

    const Expression* getConstantSubexpression(int n) const override {
        SkASSERT(n == 0);
        return this;
    }

private:
    double fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
