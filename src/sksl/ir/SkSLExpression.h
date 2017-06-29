/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSION
#define SKSL_EXPRESSION

#include "SkSLType.h"
#include "SkSLVariable.h"

#include <unordered_map>

namespace SkSL {

struct Expression;
class IRGenerator;

typedef std::unordered_map<const Variable*, std::unique_ptr<Expression>*> DefinitionMap;

/**
 * Abstract supertype of all expressions.
 */
struct Expression : public IRNode {
    enum Kind {
        kBinary_Kind,
        kBoolLiteral_Kind,
        kConstructor_Kind,
        kIntLiteral_Kind,
        kFieldAccess_Kind,
        kFloatLiteral_Kind,
        kFunctionReference_Kind,
        kFunctionCall_Kind,
        kIndex_Kind,
        kPrefix_Kind,
        kPostfix_Kind,
        kSetting_Kind,
        kSwizzle_Kind,
        kVariableReference_Kind,
        kTernary_Kind,
        kTypeReference_Kind,
        kDefined_Kind
    };

    Expression(Position position, Kind kind, const Type& type)
    : INHERITED(position)
    , fKind(kind)
    , fType(std::move(type)) {}

    /**
     * Returns true if this expression is constant. compareConstant must be implemented for all
     * constants!
     */
    virtual bool isConstant() const {
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
     * Returns true if evaluating the expression potentially has side effects. Expressions may never
     * return false if they actually have side effects, but it is legal (though suboptimal) to
     * return true if there are not actually any side effects.
     */
    virtual bool hasSideEffects() const = 0;

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

    const Kind fKind;
    const Type& fType;

    typedef IRNode INHERITED;
};

} // namespace

#endif
