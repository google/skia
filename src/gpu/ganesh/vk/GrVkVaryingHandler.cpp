/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkVaryingHandler.h"

#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/GrShaderVar.h"

/** Returns the number of locations take up by a given SkSLType. We assume that all
    scalar values are 32 bits. */
static inline int sksltype_to_location_size(SkSLType type) {
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
        case SkSLType::kSampler:
        case SkSLType::kTexture2D:
        case SkSLType::kInput:
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
    }
    SK_ABORT("Unexpected type");
}

static void finalize_helper(GrVkVaryingHandler::VarArray& vars) {
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
    // Vulkan requires at least 64 locations to be supported for both vertex output and fragment
    // input. If we ever hit this assert, then we'll need to add a cap to actually check the
    // supported input and output values and adjust our supported shaders based on those values.
    SkASSERT(locationIndex <= 64);
}

void GrVkVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
