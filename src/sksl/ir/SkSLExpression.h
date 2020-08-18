/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSION
#define SKSL_EXPRESSION

#include "src/sksl/ir/SkSLType.h"

#include <unordered_map>

namespace SkSL {

struct Expression;
class IRGenerator;
struct Variable;

typedef std::unordered_map<const Variable*, std::unique_ptr<Expression>*> DefinitionMap;

/**
 * Abstract supertype of all expressions.
 */
struct Expression : public IRNode {
    enum Kind {
        kBinary_Kind,
        kBoolLiteral_Kind,
        kConstructor_Kind,
        kExternalFunctionCall_Kind,
        kExternalValue_Kind,
        kIntLiteral_Kind,
        kFieldAccess_Kind,
        kFloatLiteral_Kind,
        kFunctionReference_Kind,
        kFunctionCall_Kind,
        kIndex_Kind,
        kNullLiteral_Kind,
        kPrefix_Kind,
        kPostfix_Kind,
        kSetting_Kind,
        kSwizzle_Kind,
        kTernary_Kind,
        kTypeReference_Kind,
        kVariableReference_Kind,
        kDefined_Kind
    };

    enum class Property {
        kSideEffects,
        kContainsRTAdjust
    };

    Expression(int offset, Kind kind, const Type& type)
    : INHERITED(offset)
    , fKind(kind)
    , fType(std::move(type)) {}

    /**
     *  Use as<T> to downcast expressions: e.g. replace `(IntLiteral&) i` with `i.as<IntLiteral>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->fKind == T::kExpressionKind);
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->fKind == T::kExpressionKind);
        return static_cast<T&>(*this);
    }

    /**
     * Returns true if this expression is constant. compareConstant must be implemented for all
     * constants!
     */
    virtual bool isCompileTimeConstant() const {
        return false;
    }

    /**
     * Compares this constant expression against another constant expression of the same type. It is
     * an error to call this on non-constant expressions, or if the types of the expressions do not
     * match.
     */
    virtual bool compareConstant(const Context& context, const Expression& other) const {
        ABORT("cannot call compareConstant on this type");
    }

    /**
     * For an expression which evaluates to a constant int, returns the value. Otherwise calls
     * ABORT.
     */
    virtual int64_t getConstantInt() const {
        ABORT("not a constant int");
    }

    /**
     * For an expression which evaluates to a constant float, returns the value. Otherwise calls
     * ABORT.
     */
    virtual double getConstantFloat() const {
        ABORT("not a constant float");
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

    /**
     * Returns true if the expression is a constant numeric literal with the specified value, or a
     * constant vector with all elements equal to the specified value.
     */
    bool isEqualToConstant(double value) const {
        return this->isEqualToConstantScalar(value) ||
               this->isEqualToConstantVector(value);
    }

    /**
     * Returns true if the expression is a constant numeric literal with the specified value
     */
    bool isEqualToConstantScalar(double value) const {
        switch (fKind) {
            case kIntLiteral_Kind:
                return this->as<IntLiteral>().fValue == value;
            case Expression::kFloatLiteral_Kind:
                return this->as<FloatLiteral>().fValue == value;
        }
        return false;
    }

    /**
     * Returns true if the expression is a constant vector with all elements equal to the specified
     * value.
     */
    bool isEqualToConstantVector(double value) const {
        if (fKind == kConstructor_Kind) {
            const Constructor& c = this->as<Constructor>();
            if (c.fType.kind() == Type::kVector_Kind && c.isCompileTimeConstant()) {
                bool isFloat = c.fType.columns() > 1 ? c.fType.componentType().isFloat()
                                                     : c.fType.isFloat();

                // We've got a compile-time-constant vector constructor. Check each field to ensure
                // that its value is a match.
                for (int i = 0; i < c.fType.columns(); ++i) {
                    if (isFloat) {
                        if (c.getFVecComponent(i) != value) {
                            return false;
                        }
                    } else {
                        if (c.getIVecComponent(i) != value) {
                            return false;
                        }
                    }
                }

                // All fields match!
                return true;
            }
        }

        return false;
    }

    /**
     * Given a map of known constant variable values, substitute them in for references to those
     * variables occurring in this expression and its subexpressions.  Similar simplifications, such
     * as folding a constant binary expression down to a single value, may also be performed.
     * Returns a new expression which replaces this expression, or null if no replacements were
     * made. If a new expression is returned, this expression is no longer valid.
     */
    virtual std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                          const DefinitionMap& definitions) {
        return nullptr;
    }

    virtual int coercionCost(const Type& target) const {
        return fType.coercionCost(target);
    }

    /**
     * For a literal vector expression, return the floating point value of the n'th vector
     * component. It is an error to call this method on an expression which is not a literal vector.
     */
    virtual SKSL_FLOAT getFVecComponent(int n) const {
        SkASSERT(false);
        return 0;
    }

    /**
     * For a literal vector expression, return the integer value of the n'th vector component. It is
     * an error to call this method on an expression which is not a literal vector.
     */
    virtual SKSL_INT getIVecComponent(int n) const {
        SkASSERT(false);
        return 0;
    }

    /**
     * For a literal matrix expression, return the floating point value of the component at
     * [col][row]. It is an error to call this method on an expression which is not a literal
     * matrix.
     */
    virtual SKSL_FLOAT getMatComponent(int col, int row) const {
        SkASSERT(false);
        return 0;
    }

    virtual std::unique_ptr<Expression> clone() const = 0;

    const Kind fKind;
    const Type& fType;

    typedef IRNode INHERITED;
};

}  // namespace SkSL

#endif
