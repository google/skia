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
    if (type.kind() == Type::kMatrix_Kind) {
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
        } else {
           SkASSERT(type.componentType() == *context.fDouble_Type);
            switch (type.rows()) {
                case 2: return *context.fDouble2_Type;
                case 3: return *context.fDouble3_Type;
                case 4: return *context.fDouble4_Type;
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
    IndexExpression(const Context& context, std::unique_ptr<Expression> base,
                    std::unique_ptr<Expression> index)
    : INHERITED(base->fOffset, kIndex_Kind, index_type(context, base->fType))
    , fBase(std::move(base))
    , fIndex(std::move(index)) {
        SkASSERT(fIndex->fType == *context.fInt_Type || fIndex->fType == *context.fUInt_Type);
    }

    bool hasSideEffects() const override {
        return fBase->hasSideEffects() || fIndex->hasSideEffects();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new IndexExpression(fBase->clone(), fIndex->clone(),
                                                               &fType));
    }

    String description() const override {
        return fBase->description() + "[" + fIndex->description() + "]";
    }

    std::unique_ptr<Expression> fBase;
    std::unique_ptr<Expression> fIndex;

    typedef Expression INHERITED;

private:
    IndexExpression(std::unique_ptr<Expression> base, std::unique_ptr<Expression> index,
                    const Type* type)
    : INHERITED(base->fOffset, kIndex_Kind, *type)
    , fBase(std::move(base))
    , fIndex(std::move(index)) {}
};

} // namespace

#endif
