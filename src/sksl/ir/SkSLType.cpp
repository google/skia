/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

CoercionCost Type::coercionCost(const Type& other) const {
    if (*this == other) {
        return CoercionCost::Free();
    }
    if (this->typeKind() == TypeKind::kNullable && other.typeKind() != TypeKind::kNullable) {
        CoercionCost result = this->componentType().coercionCost(other);
        if (result.isPossible(/*allowNarrowing=*/true)) {
            ++result.fNormalCost;
        }
        return result;
    }
    if (this->name() == "null" && other.typeKind() == TypeKind::kNullable) {
        return CoercionCost::Free();
    }
    if (this->typeKind() == TypeKind::kVector && other.typeKind() == TypeKind::kVector) {
        if (this->columns() == other.columns()) {
            return this->componentType().coercionCost(other.componentType());
        }
        return CoercionCost::Impossible();
    }
    if (this->typeKind() == TypeKind::kMatrix) {
        if (this->columns() == other.columns() && this->rows() == other.rows()) {
            return this->componentType().coercionCost(other.componentType());
        }
        return CoercionCost::Impossible();
    }
    if (this->isNumber() && other.isNumber()) {
        if (other.priority() >= this->priority()) {
            return CoercionCost::Normal(other.priority() - this->priority());
        } else {
            return CoercionCost::Narrowing(this->priority() - other.priority());
        }
    }
    for (size_t i = 0; i < fCoercibleTypes.size(); i++) {
        if (*fCoercibleTypes[i] == other) {
            return CoercionCost::Normal((int) i + 1);
        }
    }
    return CoercionCost::Impossible();
}

const Type& Type::toCompound(const Context& context, int columns, int rows) const {
    SkASSERT(this->isScalar());
    if (columns == 1 && rows == 1) {
        return *this;
    }
    if (*this == *context.fFloat_Type || *this == *context.fFloatLiteral_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fFloat_Type;
                    case 2: return *context.fFloat2_Type;
                    case 3: return *context.fFloat3_Type;
                    case 4: return *context.fFloat4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fFloat2x2_Type;
                    case 3: return *context.fFloat3x2_Type;
                    case 4: return *context.fFloat4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fFloat2x3_Type;
                    case 3: return *context.fFloat3x3_Type;
                    case 4: return *context.fFloat4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fFloat2x4_Type;
                    case 3: return *context.fFloat3x4_Type;
                    case 4: return *context.fFloat4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fHalf_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fHalf_Type;
                    case 2: return *context.fHalf2_Type;
                    case 3: return *context.fHalf3_Type;
                    case 4: return *context.fHalf4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fHalf2x2_Type;
                    case 3: return *context.fHalf3x2_Type;
                    case 4: return *context.fHalf4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fHalf2x3_Type;
                    case 3: return *context.fHalf3x3_Type;
                    case 4: return *context.fHalf4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fHalf2x4_Type;
                    case 3: return *context.fHalf3x4_Type;
                    case 4: return *context.fHalf4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fInt_Type || *this == *context.fIntLiteral_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fInt_Type;
                    case 2: return *context.fInt2_Type;
                    case 3: return *context.fInt3_Type;
                    case 4: return *context.fInt4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fShort_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fShort_Type;
                    case 2: return *context.fShort2_Type;
                    case 3: return *context.fShort3_Type;
                    case 4: return *context.fShort4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fByte_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fByte_Type;
                    case 2: return *context.fByte2_Type;
                    case 3: return *context.fByte3_Type;
                    case 4: return *context.fByte4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fUInt_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fUInt_Type;
                    case 2: return *context.fUInt2_Type;
                    case 3: return *context.fUInt3_Type;
                    case 4: return *context.fUInt4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fUShort_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fUShort_Type;
                    case 2: return *context.fUShort2_Type;
                    case 3: return *context.fUShort3_Type;
                    case 4: return *context.fUShort4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fUByte_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fUByte_Type;
                    case 2: return *context.fUByte2_Type;
                    case 3: return *context.fUByte3_Type;
                    case 4: return *context.fUByte4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fBool_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fBool_Type;
                    case 2: return *context.fBool2_Type;
                    case 3: return *context.fBool3_Type;
                    case 4: return *context.fBool4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    }
#ifdef SK_DEBUG
    ABORT("unsupported toCompound type %s", this->description().c_str());
#endif
    return *context.fVoid_Type;
}

}  // namespace SkSL
