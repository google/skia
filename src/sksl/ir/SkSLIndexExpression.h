/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INDEX
#define SKSL_INDEX

#include "SkSLContext.h"
#include "SkSLExpression.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * Given a type, returns the type that will result from extracting an array value from it.
 */
static const Type& index_type(const Context& context, const Type& type) {
    if (type.kind() == Type::kMatrix_Kind) {
        if (type.componentType() == *context.fFloat_Type) {
            switch (type.rows()) {
                case 2: return *context.fVec2_Type;
                case 3: return *context.fVec3_Type;
                case 4: return *context.fVec4_Type;
                default: ASSERT(false);
            }
        } else {
            ASSERT(type.componentType() == *context.fDouble_Type);
            switch (type.rows()) {
                case 2: return *context.fDVec2_Type;
                case 3: return *context.fDVec3_Type;
                case 4: return *context.fDVec4_Type;
                default: ASSERT(false);
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
    : INHERITED(base->fPosition, kIndex_Kind, index_type(context, base->fType))
    , fBase(std::move(base))
    , fIndex(std::move(index)) {
        ASSERT(fIndex->fType == *context.fInt_Type || fIndex->fType == *context.fUInt_Type);
    }

    String description() const override {
        return fBase->description() + "[" + fIndex->description() + "]";
    }

    std::unique_ptr<Expression> fBase;
    std::unique_ptr<Expression> fIndex;

    typedef Expression INHERITED;
};

} // namespace

#endif
