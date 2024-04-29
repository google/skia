/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSION
#define SKSL_EXPRESSION

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace SkSL {

class AnyConstructor;
class Context;
enum class OperatorPrecedence : uint8_t;

/**
 * Abstract supertype of all expressions.
 */
class Expression : public IRNode {
public:
    using Kind = ExpressionKind;

    Expression(Position pos, Kind kind, const Type* type)
        : INHERITED(pos, (int) kind)
        , fType(type) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Kind kind() const {
        return (Kind)fKind;
    }

    const Type& type() const {
        return *fType;
    }

    bool isAnyConstructor() const {
        static_assert((int)Kind::kConstructorArray - 1 == (int)Kind::kChildCall);
        static_assert((int)Kind::kConstructorStruct + 1 == (int)Kind::kEmpty);
        return this->kind() >= Kind::kConstructorArray && this->kind() <= Kind::kConstructorStruct;
    }

    bool isIntLiteral() const {
        return this->kind() == Kind::kLiteral && this->type().isInteger();
    }

    bool isFloatLiteral() const {
        return this->kind() == Kind::kLiteral && this->type().isFloat();
    }

    bool isBoolLiteral() const {
        return this->kind() == Kind::kLiteral && this->type().isBoolean();
    }

    AnyConstructor& asAnyConstructor();
    const AnyConstructor& asAnyConstructor() const;

    /**
     * Returns true if this expression is incomplete. Specifically, dangling function/method-call
     * references that were never invoked, or type references that were never constructed, are
     * considered incomplete expressions and should result in an error.
     */
    bool isIncomplete(const Context& context) const;

    /**
     * Compares this constant expression against another constant expression. Returns kUnknown if
     * we aren't able to deduce a result (an expression isn't actually constant, the types are
     * mismatched, etc).
     */
    enum class ComparisonResult {
        kUnknown = -1,
        kNotEqual,
        kEqual
    };
    virtual ComparisonResult compareConstant(const Expression& other) const {
        return ComparisonResult::kUnknown;
    }

    CoercionCost coercionCost(const Type& target) const {
        return this->type().coercionCost(target);
    }

    /**
     * Returns true if this expression type supports `getConstantValue`. (This particular expression
     * may or may not actually contain a constant value.) It's harmless to call `getConstantValue`
     * on expressions which don't support constant values or don't contain any constant values, but
     * if `supportsConstantValues` returns false, you can assume that `getConstantValue` will return
     * nullopt for every slot of this expression. This allows for early-out opportunities in some
     * cases. (Some expressions have tons of slots but never hold a constant value; e.g. a variable
     * holding a very large array.)
     */
    virtual bool supportsConstantValues() const {
        return false;
    }

    /**
     * Returns the n'th compile-time constant value within a literal or constructor.
     * Use Type::slotCount to determine the number of slots within an expression.
     * Slots which do not contain compile-time constant values will return nullopt.
     * `vec4(1, vec2(2), 3)` contains four compile-time constants: (1, 2, 2, 3)
     * `mat2(f)` contains four slots, and two are constant: (nullopt, 0,
     *                                                       0, nullopt)
     * All classes which override this function must also implement `supportsConstantValues`.
     */
    virtual std::optional<double> getConstantValue(int n) const {
        SkASSERT(!this->supportsConstantValues());
        return std::nullopt;
    }

    virtual std::unique_ptr<Expression> clone(Position pos) const = 0;

    /**
     * Returns a clone at the same position.
     */
    std::unique_ptr<Expression> clone() const { return this->clone(fPosition); }

    /**
     * Returns a description of the expression.
     */
    std::string description() const final;
    virtual std::string description(OperatorPrecedence parentPrecedence) const = 0;


private:
    const Type* fType;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
