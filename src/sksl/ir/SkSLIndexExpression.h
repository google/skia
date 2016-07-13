/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_INDEX
#define SKSL_INDEX

#include "SkSLExpression.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * Given a type, returns the type that will result from extracting an array value from it.
 */
static std::shared_ptr<Type> index_type(const Type& type) {
    if (type.kind() == Type::kMatrix_Kind) {
        if (type.componentType() == kFloat_Type) {
            switch (type.columns()) {
                case 2: return kVec2_Type;
                case 3: return kVec3_Type;
                case 4: return kVec4_Type;
                default: ASSERT(false);
            }
        } else {
            ASSERT(type.componentType() == kDouble_Type);
            switch (type.columns()) {
                case 2: return kDVec2_Type;
                case 3: return kDVec3_Type;
                case 4: return kDVec4_Type;
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
    IndexExpression(std::unique_ptr<Expression> base, std::unique_ptr<Expression> index)
    : INHERITED(base->fPosition, kIndex_Kind, index_type(*base->fType))
    , fBase(std::move(base))
    , fIndex(std::move(index)) {
        ASSERT(fIndex->fType == kInt_Type);
    }

    std::string description() const override {
        return fBase->description() + "[" + fIndex->description() + "]";
    }

    const std::unique_ptr<Expression> fBase;
    const std::unique_ptr<Expression> fIndex;

    typedef Expression INHERITED;
};

} // namespace

#endif
