/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLType.h"
#include "SkSLContext.h"

namespace SkSL {

bool Type::determineCoercionCost(const Type& other, int* outCost) const {
    if (*this == other) {
        *outCost = 0;
        return true;
    }
    if (this->kind() == kVector_Kind && other.kind() == kVector_Kind) {
        if (this->columns() == other.columns()) {
            return this->componentType().determineCoercionCost(other.componentType(), outCost);
        }
        return false;
    }
    if (this->kind() == kMatrix_Kind) {
        if (this->columns() == other.columns() &&
            this->rows() == other.rows()) {
            return this->componentType().determineCoercionCost(other.componentType(), outCost);
        }
        return false;
    }
    for (size_t i = 0; i < fCoercibleTypes.size(); i++) {
        if (*fCoercibleTypes[i] == other) {
            *outCost = (int) i + 1;
            return true;
        }
    }
    return false;
}

const Type& Type::toCompound(const Context& context, int columns, int rows) const {
    ASSERT(this->kind() == Type::kScalar_Kind);
    if (columns == 1 && rows == 1) {
        return *this;
    }
    if (*this == *context.fFloat_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return *context.fVec2_Type;
                    case 3: return *context.fVec3_Type;
                    case 4: return *context.fVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fMat2x2_Type;
                    case 3: return *context.fMat3x2_Type;
                    case 4: return *context.fMat4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fMat2x3_Type;
                    case 3: return *context.fMat3x3_Type;
                    case 4: return *context.fMat4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fMat2x4_Type;
                    case 3: return *context.fMat3x4_Type;
                    case 4: return *context.fMat4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fDouble_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return *context.fDVec2_Type;
                    case 3: return *context.fDVec3_Type;
                    case 4: return *context.fDVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fDMat2x2_Type;
                    case 3: return *context.fDMat3x2_Type;
                    case 4: return *context.fDMat4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fDMat2x3_Type;
                    case 3: return *context.fDMat3x3_Type;
                    case 4: return *context.fDMat4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fDMat2x4_Type;
                    case 3: return *context.fDMat3x4_Type;
                    case 4: return *context.fDMat4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fInt_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return *context.fIVec2_Type;
                    case 3: return *context.fIVec3_Type;
                    case 4: return *context.fIVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fUInt_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return *context.fUVec2_Type;
                    case 3: return *context.fUVec3_Type;
                    case 4: return *context.fUVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *context.fBool_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return *context.fBVec2_Type;
                    case 3: return *context.fBVec3_Type;
                    case 4: return *context.fBVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    }
    ABORT("unsupported scalar_to_compound type %s", this->description().c_str());
}

} // namespace
