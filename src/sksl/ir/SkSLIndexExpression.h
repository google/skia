/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INDEX
#define SKSL_INDEX

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Given a type, returns the type that will result from extracting an array value from it.
 */
static const Type& index_type(const Context& context, const Type& type) {
    if (type.typeKind() == Type::TypeKind::kMatrix) {
        if (type.componentType() == *context.fFloat_Type) {
            switch (type.rows()) {
                case 2: return *context.fFloat2_Type;
                case 3: return *context.fFloat3_Type;
                case 4: return *context.fFloat4_Type;
                default: SkASSERT(false);
            }
        } else if (type.componentType() == *context.fHalf_Type) {
            switch (type.rows()) {
                case 2: return *context.fHalf2_Type;
                case 3: return *context.fHalf3_Type;
                case 4: return *context.fHalf4_Type;
                default: SkASSERT(false);
            }
        }
    }
    return type.componentType();
}

/**
 * An expression which extracts a value from an array or matrix, as in 'm[2]'.
 */
struct IndexExpression : public Expression {
    static constexpr Kind kExpressionKind = Kind::kIndex;

    IndexExpression(const Context& context, std::unique_ptr<Expression> base,
                    std::unique_ptr<Expression> index)
    : INHERITED(base->fOffset, kExpressionKind, &index_type(context, base->type())) {
        SkASSERT(index->type() == *context.fInt_Type || index->type() == *context.fUInt_Type);
        fExpressionChildren.reserve_back(2);
        fExpressionChildren.push_back(std::move(base));
        fExpressionChildren.push_back(std::move(index));
    }

    std::unique_ptr<Expression>& base() {
        return fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& base() const {
        return fExpressionChildren[0];
    }

    std::unique_ptr<Expression>& index() {
        return fExpressionChildren[1];
    }

    const std::unique_ptr<Expression>& index() const {
        return fExpressionChildren[1];
    }

    bool hasProperty(Property property) const override {
        return this->base()->hasProperty(property) || this->index()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new IndexExpression(this->base()->clone(),
                                                               this->index()->clone(),
                                                               &this->type()));
    }

    String description() const override {
        return this->base()->description() + "[" + this->index()->description() + "]";
    }

    using INHERITED = Expression;

private:
    IndexExpression(std::unique_ptr<Expression> base, std::unique_ptr<Expression> index,
                    const Type* type)
    : INHERITED(base->fOffset, Kind::kIndex, type) {
        fExpressionChildren.reserve_back(2);
        fExpressionChildren.push_back(std::move(base));
        fExpressionChildren.push_back(std::move(index));
    }
};

}  // namespace SkSL

#endif
