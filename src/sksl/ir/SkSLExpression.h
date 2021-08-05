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
        kBoolLiteral,
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
        kIntLiteral,
        kFieldAccess,
        kFloatLiteral,
        kFunctionReference,
        kFunctionCall,
        kIndex,
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

    Expression(int offset, Kind kind, const Type* type)
        : INHERITED(offset, (int) kind)
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
     *  e.g. replace `e.kind() == Expression::Kind::kIntLiteral` with `e.is<IntLiteral>()`.
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

    /**
     *  Use as<T> to downcast expressions: e.g. replace `(IntLiteral&) i` with `i.as<IntLiteral>()`.
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
     * Returns the n'th compile-time constant expression within a literal or constructor.
     * Use Type::slotCount to determine the number of subexpressions within an expression.
     * Subexpressions which are not compile-time constants will return null.
     * `vec4(1, vec2(2), 3)` contains four subexpressions: (1, 2, 2, 3)
     * `mat2(f)` contains four subexpressions: (null, 0,
     *                                          0, null)
     */
    virtual const Expression* getConstantSubexpression(int n) const {
        return nullptr;
    }

    virtual std::unique_ptr<Expression> clone() const = 0;

private:
    const Type* fType;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
