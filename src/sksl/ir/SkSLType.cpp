/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLType.h"
#include "SkSLContext.h"

namespace SkSL {

int Type::coercionCost(const Type& other) const {
    if (*this == other) {
        return 0;
    }
    if (this->kind() == kVector_Kind && other.kind() == kVector_Kind) {
        if (this->columns() == other.columns()) {
            return this->componentType().coercionCost(other.componentType());
        }
        return INT_MAX;
    }
    if (this->kind() == kMatrix_Kind) {
        if (this->columns() == other.columns() && this->rows() == other.rows()) {
            return this->componentType().coercionCost(other.componentType());
        }
        return INT_MAX;
    }
    if (this->isNumber() && other.isNumber() && other.priority() > this->priority()) {
        return other.priority() - this->priority();
    }
    for (size_t i = 0; i < fCoercibleTypes.size(); i++) {
        if (*fCoercibleTypes[i] == other) {
            return (int) i + 1;
        }
    }
    return INT_MAX;
}

const Type& Type::toCompound(const Context& context, int columns, int rows) const {
    SkASSERT(this->kind() == Type::kScalar_Kind);
    if (columns == 1 && rows == 1) {
        return *this;
    }
    if (*this == *context.fFloat_Type || *this == *context.fFloatLiteral_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
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
    } else if (*this == *context.fDouble_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return *context.fDouble2_Type;
                    case 3: return *context.fDouble3_Type;
                    case 4: return *context.fDouble4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fDouble2x2_Type;
                    case 3: return *context.fDouble3x2_Type;
                    case 4: return *context.fDouble4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fDouble2x3_Type;
                    case 3: return *context.fDouble3x3_Type;
                    case 4: return *context.fDouble4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fDouble2x4_Type;
                    case 3: return *context.fDouble3x4_Type;
                    case 4: return *context.fDouble4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fInt_Type || *this == *context.fIntLiteral_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
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
                    case 2: return *context.fBool2_Type;
                    case 3: return *context.fBool3_Type;
                    case 4: return *context.fBool4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    }
    ABORT("unsupported scalar_to_compound type %s", this->description().c_str());
}

} // namespace
