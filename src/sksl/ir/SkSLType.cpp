/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

int Type::coercionCost(const Type& other) const {
    if (*this == other) {
        return 0;
    }
    if (this->kind() == kNullable_Kind && other.kind() != kNullable_Kind) {
        int result = this->componentType().typeNode().coercionCost(other);
        if (result != INT_MAX) {
            ++result;
        }
        return result;
    }
    if (this->fName == "null" && other.kind() == kNullable_Kind) {
        return 0;
    }
    if (this->kind() == kVector_Kind && other.kind() == kVector_Kind) {
        if (this->columns() == other.columns()) {
            return this->componentType().typeNode().coercionCost(other.componentType().typeNode());
        }
        return INT_MAX;
    }
    if (this->kind() == kMatrix_Kind) {
        if (this->columns() == other.columns() && this->rows() == other.rows()) {
            return this->componentType().typeNode().coercionCost(other.componentType().typeNode());
        }
        return INT_MAX;
    }
    if (this->isNumber() && other.isNumber() && other.priority() > this->priority()) {
        return other.priority() - this->priority();
    }
    for (size_t i = 0; i < fCoercibleTypes.size(); i++) {
        if (fCoercibleTypes[i].typeNode() == other) {
            return (int) i + 1;
        }
    }
    return INT_MAX;
}

IRNode::ID Type::toCompound(int columns, int rows) const {
    SkASSERT(this->kind() == Type::kScalar_Kind);
    if (*this == fIRGenerator->fContext.fFloat_Type.typeNode() ||
        *this == fIRGenerator->fContext.fFloatLiteral_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fFloat_Type;
                    case 2: return fIRGenerator->fContext.fFloat2_Type;
                    case 3: return fIRGenerator->fContext.fFloat3_Type;
                    case 4: return fIRGenerator->fContext.fFloat4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return fIRGenerator->fContext.fFloat2x2_Type;
                    case 3: return fIRGenerator->fContext.fFloat3x2_Type;
                    case 4: return fIRGenerator->fContext.fFloat4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return fIRGenerator->fContext.fFloat2x3_Type;
                    case 3: return fIRGenerator->fContext.fFloat3x3_Type;
                    case 4: return fIRGenerator->fContext.fFloat4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return fIRGenerator->fContext.fFloat2x4_Type;
                    case 3: return fIRGenerator->fContext.fFloat3x4_Type;
                    case 4: return fIRGenerator->fContext.fFloat4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fHalf_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fHalf_Type;
                    case 2: return fIRGenerator->fContext.fHalf2_Type;
                    case 3: return fIRGenerator->fContext.fHalf3_Type;
                    case 4: return fIRGenerator->fContext.fHalf4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return fIRGenerator->fContext.fHalf2x2_Type;
                    case 3: return fIRGenerator->fContext.fHalf3x2_Type;
                    case 4: return fIRGenerator->fContext.fHalf4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return fIRGenerator->fContext.fHalf2x3_Type;
                    case 3: return fIRGenerator->fContext.fHalf3x3_Type;
                    case 4: return fIRGenerator->fContext.fHalf4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return fIRGenerator->fContext.fHalf2x4_Type;
                    case 3: return fIRGenerator->fContext.fHalf3x4_Type;
                    case 4: return fIRGenerator->fContext.fHalf4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fInt_Type.typeNode() ||
               *this == fIRGenerator->fContext.fIntLiteral_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fInt_Type;
                    case 2: return fIRGenerator->fContext.fInt2_Type;
                    case 3: return fIRGenerator->fContext.fInt3_Type;
                    case 4: return fIRGenerator->fContext.fInt4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fShort_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fShort_Type;
                    case 2: return fIRGenerator->fContext.fShort2_Type;
                    case 3: return fIRGenerator->fContext.fShort3_Type;
                    case 4: return fIRGenerator->fContext.fShort4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fByte_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fByte_Type;
                    case 2: return fIRGenerator->fContext.fByte2_Type;
                    case 3: return fIRGenerator->fContext.fByte3_Type;
                    case 4: return fIRGenerator->fContext.fByte4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fUInt_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fUInt_Type;
                    case 2: return fIRGenerator->fContext.fUInt2_Type;
                    case 3: return fIRGenerator->fContext.fUInt3_Type;
                    case 4: return fIRGenerator->fContext.fUInt4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fUShort_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fUShort_Type;
                    case 2: return fIRGenerator->fContext.fUShort2_Type;
                    case 3: return fIRGenerator->fContext.fUShort3_Type;
                    case 4: return fIRGenerator->fContext.fUShort4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fUByte_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fUByte_Type;
                    case 2: return fIRGenerator->fContext.fUByte2_Type;
                    case 3: return fIRGenerator->fContext.fUByte3_Type;
                    case 4: return fIRGenerator->fContext.fUByte4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == fIRGenerator->fContext.fBool_Type.typeNode()) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return fIRGenerator->fContext.fBool_Type;
                    case 2: return fIRGenerator->fContext.fBool2_Type;
                    case 3: return fIRGenerator->fContext.fBool3_Type;
                    case 4: return fIRGenerator->fContext.fBool4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    }
    ABORT("unsupported scalar_to_compound type %s", this->description().c_str());
}

} // namespace
