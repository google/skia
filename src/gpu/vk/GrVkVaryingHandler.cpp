/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkVaryingHandler.h"

/** Returns the number of locations take up by a given GrSLType. We assume that all
    scalar values are 32 bits. */
static inline int grsltype_to_location_size(GrSLType type) {
    switch(type) {
        case kVoid_GrSLType:
            return 0;
        case kFloat_GrSLType: [[fallthrough]];
        case kHalf_GrSLType:
            return 1;
        case kFloat2_GrSLType: [[fallthrough]];
        case kHalf2_GrSLType:
            return 1;
        case kFloat3_GrSLType: [[fallthrough]];
        case kHalf3_GrSLType:
            return 1;
        case kFloat4_GrSLType: [[fallthrough]];
        case kHalf4_GrSLType:
            return 1;
        case kUint2_GrSLType:
            return 1;
        case kInt2_GrSLType:    [[fallthrough]];
        case kShort2_GrSLType:  [[fallthrough]];
        case kUShort2_GrSLType: [[fallthrough]];
        case kByte2_GrSLType:   [[fallthrough]];
        case kUByte2_GrSLType:
            return 1;
        case kInt3_GrSLType:    [[fallthrough]];
        case kShort3_GrSLType:  [[fallthrough]];
        case kUShort3_GrSLType: [[fallthrough]];
        case kByte3_GrSLType:   [[fallthrough]];
        case kUByte3_GrSLType:
            return 1;
        case kInt4_GrSLType:    [[fallthrough]];
        case kShort4_GrSLType:  [[fallthrough]];
        case kUShort4_GrSLType: [[fallthrough]];
        case kByte4_GrSLType:   [[fallthrough]];
        case kUByte4_GrSLType:
            return 1;
        case kFloat2x2_GrSLType: [[fallthrough]];
        case kHalf2x2_GrSLType:
            return 2;
        case kFloat3x3_GrSLType: [[fallthrough]];
        case kHalf3x3_GrSLType:
            return 3;
        case kFloat4x4_GrSLType: [[fallthrough]];
        case kHalf4x4_GrSLType:
            return 4;
        case kTexture2DSampler_GrSLType:    [[fallthrough]];
        case kSampler_GrSLType:             [[fallthrough]];
        case kTexture2D_GrSLType:
            return 0;
        case kTextureExternalSampler_GrSLType:
             return 0;
        case kTexture2DRectSampler_GrSLType:
             return 0;
        case kBool_GrSLType:
             return 1;
        case kInt_GrSLType:   [[fallthrough]];
        case kShort_GrSLType: [[fallthrough]];
        case kByte_GrSLType:
             return 1;
        case kUint_GrSLType:   [[fallthrough]];
        case kUShort_GrSLType: [[fallthrough]];
        case kUByte_GrSLType:
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

        int elementSize = grsltype_to_location_size(var.getType());
        SkASSERT(elementSize > 0);
        int numElements = 1;
        if (var.isArray() && !var.isUnsizedArray()) {
            numElements = var.getArrayCount();
        }
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
    finalize_helper(fGeomInputs);
    finalize_helper(fGeomOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
