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
    if (*this == *context.fTypes.fFloat || *this == *context.fTypes.fFloatLiteral) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fFloat;
                    case 2: return *context.fTypes.fFloat2;
                    case 3: return *context.fTypes.fFloat3;
                    case 4: return *context.fTypes.fFloat4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fTypes.fFloat2x2;
                    case 3: return *context.fTypes.fFloat3x2;
                    case 4: return *context.fTypes.fFloat4x2;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fTypes.fFloat2x3;
                    case 3: return *context.fTypes.fFloat3x3;
                    case 4: return *context.fTypes.fFloat4x3;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fTypes.fFloat2x4;
                    case 3: return *context.fTypes.fFloat3x4;
                    case 4: return *context.fTypes.fFloat4x4;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fHalf) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fHalf;
                    case 2: return *context.fTypes.fHalf2;
                    case 3: return *context.fTypes.fHalf3;
                    case 4: return *context.fTypes.fHalf4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fTypes.fHalf2x2;
                    case 3: return *context.fTypes.fHalf3x2;
                    case 4: return *context.fTypes.fHalf4x2;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fTypes.fHalf2x3;
                    case 3: return *context.fTypes.fHalf3x3;
                    case 4: return *context.fTypes.fHalf4x3;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fTypes.fHalf2x4;
                    case 3: return *context.fTypes.fHalf3x4;
                    case 4: return *context.fTypes.fHalf4x4;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fInt || *this == *context.fTypes.fIntLiteral) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fInt;
                    case 2: return *context.fTypes.fInt2;
                    case 3: return *context.fTypes.fInt3;
                    case 4: return *context.fTypes.fInt4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fShort) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fShort;
                    case 2: return *context.fTypes.fShort2;
                    case 3: return *context.fTypes.fShort3;
                    case 4: return *context.fTypes.fShort4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fByte) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fByte;
                    case 2: return *context.fTypes.fByte2;
                    case 3: return *context.fTypes.fByte3;
                    case 4: return *context.fTypes.fByte4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fUInt) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fUInt;
                    case 2: return *context.fTypes.fUInt2;
                    case 3: return *context.fTypes.fUInt3;
                    case 4: return *context.fTypes.fUInt4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fUShort) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fUShort;
                    case 2: return *context.fTypes.fUShort2;
                    case 3: return *context.fTypes.fUShort3;
                    case 4: return *context.fTypes.fUShort4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fUByte) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fUByte;
                    case 2: return *context.fTypes.fUByte2;
                    case 3: return *context.fTypes.fUByte3;
                    case 4: return *context.fTypes.fUByte4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fTypes.fBool) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fBool;
                    case 2: return *context.fTypes.fBool2;
                    case 3: return *context.fTypes.fBool3;
                    case 4: return *context.fTypes.fBool4;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    }
#ifdef SK_DEBUG
    ABORT("unsupported toCompound type %s", this->description().c_str());
#endif
    return *context.fTypes.fVoid;
}

}  // namespace SkSL
