/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrSPIRVVaryingHandler.h"

#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/GrShaderVar.h"

/** Returns the number of locations take up by a given SkSLType. We assume that all
    scalar values are 32 bits. */
static inline int sksltype_to_location_size(SkSLType type) {
    // If a new GrSL type is added, this function will need to be updated.
    static_assert(kSkSLTypeCount == 41);

    switch(type) {
        case SkSLType::kVoid:
            return 0;
        case SkSLType::kFloat: // fall through
        case SkSLType::kHalf:
            return 1;
        case SkSLType::kFloat2: // fall through
        case SkSLType::kHalf2:
            return 1;
        case SkSLType::kFloat3:
        case SkSLType::kHalf3:
            return 1;
        case SkSLType::kFloat4:
        case SkSLType::kHalf4:
            return 1;
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
        case SkSLType::kShort2:
        case SkSLType::kUShort2:
            return 1;
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
        case SkSLType::kShort3:
        case SkSLType::kUShort3:
            return 1;
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
        case SkSLType::kShort4:
        case SkSLType::kUShort4:
            return 1;
        case SkSLType::kFloat2x2:
        case SkSLType::kHalf2x2:
            return 2;
        case SkSLType::kFloat3x3:
        case SkSLType::kHalf3x3:
            return 3;
        case SkSLType::kFloat4x4:
        case SkSLType::kHalf4x4:
            return 4;
        case SkSLType::kTexture2DSampler:
            return 0;
        case SkSLType::kTextureExternalSampler:
             return 0;
        case SkSLType::kTexture2DRectSampler:
             return 0;
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
             return 1;
        case SkSLType::kInt: // fall through
        case SkSLType::kShort:
             return 1;
        case SkSLType::kUInt: // fall through
        case SkSLType::kUShort:
             return 1;
        case SkSLType::kTexture2D:
             return 0;
        case SkSLType::kSampler:
             return 0;
        case SkSLType::kInput:
            return 0;
    }
    SK_ABORT("Unexpected type");
}

static void finalize_helper(GrSPIRVVaryingHandler::VarArray& vars) {
    int locationIndex = 0;
    for (GrShaderVar& var : vars.items()) {
        SkString location;
        location.appendf("location = %d", locationIndex);
        var.addLayoutQualifier(location.c_str());

        int elementSize = sksltype_to_location_size(var.getType());
        SkASSERT(elementSize > 0);
        int numElements = var.isArray() ? var.getArrayCount() : 1;
        SkASSERT(numElements > 0);
        locationIndex += elementSize * numElements;
    }
    // TODO: determine the layout limits for SPIR-V, and enforce them via asserts here.
}

void GrSPIRVVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
