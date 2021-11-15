/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSION
#define SKSL_EXPRESSION

#include "include/private/SkSLStatement.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTOptional.h"
#include "src/sksl/ir/SkSLType.h"

#include <unordered_map>

namespace SkSL {

class AnyConstructor;
class Expression;
class IRGenerator;
class Variable;

/**
 * Abstract supertype of all expressions.
 */
class Expression : public IRNode {
public:
    enum class Kind {
        kBinary = (int) Statement::Kind::kLast + 1,
        kChildCall,
        kCodeString,
        kConstructorArray,
        kConstructorArrayCast,
        kConstructorCompound,
        kConstructorCompoundCast,
        kConstructorDiagonalMatrix,
        kConstructorMatrixResize,
        kConstructorScalarCast,
        kConstructorSplat,
        kConstructorStruct,
        kExternalFunctionCall,
        kExternalFunctionReference,
        kFieldAccess,
        kFunctionReference,
        kFunctionCall,
        kIndex,
        kLiteral,
        kMethodReference,
        kPoison,
        kPostfix,
        kPrefix,
        kSetting,
        kSwizzle,
        kTernary,
        kTypeReference,
        kVariableReference,

        kFirst = kBinary,
        kLast = kVariableReference
    };

    enum class Property {
        kSideEffects,
        kContainsRTAdjust
    };

    Expression(int line, Kind kind, const Type* type)
        : INHERITED(line, (int) kind)
        , fType(type) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Kind kind() const {
        return (Kind) fKind;
    }

    virtual const Type& type() const {
        return *fType;
    }

    /**
     *  Use is<T> to check the type of an expression.
     *  e.g. replace `e.kind() == Expression::Kind::kLiteral` with `e.is<Literal>()`.
     */
    template <typename T>
    bool is() const {
        return this->kind() == T::kExpressionKind;
    }

    bool isAnyConstructor() const {
        static_assert((int)Kind::kConstructorArray - 1 == (int)Kind::kCodeString);
        static_assert((int)Kind::kConstructorStruct + 1 == (int)Kind::kExternalFunctionCall);
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

    /**
     *  Use as<T> to downcast expressions: e.g. replace `(Literal&) i` with `i.as<Literal>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->is<T>());
        return static_cast<T&>(*this);
    }

    AnyConstructor& asAnyConstructor();
    const AnyConstructor& asAnyConstructor() const;

    /**
     * Returns true if this expression is constant. compareConstant must be implemented for all
     * constants!
     */
    virtual bool isCompileTimeConstant() const {
        return false;
    }

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

    /**
     * Returns true if, given fixed values for uniforms, this expression always evaluates to the
     * same result with no side effects.
     */
    virtual bool isConstantOrUniform() const {
        SkASSERT(!this->isCompileTimeConstant() || !this->hasSideEffects());
        return this->isCompileTimeConstant();
    }

    virtual bool hasProperty(Property property) const = 0;

    bool hasSideEffects() const {
        return this->hasProperty(Property::kSideEffects);
    }

    bool containsRTAdjust() const {
        return this->hasProperty(Property::kContainsRTAdjust);
    }

    virtual CoercionCost coercionCost(const Type& target) const {
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
    virtual skstd::optional<double> getConstantValue(int n) const {
        SkASSERT(!this->supportsConstantValues());
        return skstd::nullopt;
    }

    virtual std::unique_ptr<Expression> clone() const = 0;

private:
    const Type* fType;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
