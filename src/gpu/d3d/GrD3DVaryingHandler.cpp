/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DVaryingHandler.h"

/** Returns the number of locations take up by a given GrSLType. We assume that all
    scalar values are 32 bits. */
static inline int grsltype_to_location_size(GrSLType type) {
    switch(type) {
        case kVoid_GrSLType:
            return 0;
        case kFloat_GrSLType: // fall through
        case kHalf_GrSLType:
            return 1;
        case kFloat2_GrSLType: // fall through
        case kHalf2_GrSLType:
            return 1;
        case kFloat3_GrSLType:
        case kHalf3_GrSLType:
            return 1;
        case kFloat4_GrSLType:
        case kHalf4_GrSLType:
            return 1;
        case kUint2_GrSLType:
            return 1;
        case kInt2_GrSLType:
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kByte2_GrSLType:
        case kUByte2_GrSLType:
            return 1;
        case kInt3_GrSLType:
        case kShort3_GrSLType:
        case kUShort3_GrSLType:
        case kByte3_GrSLType:
        case kUByte3_GrSLType:
            return 1;
        case kInt4_GrSLType:
        case kShort4_GrSLType:
        case kUShort4_GrSLType:
        case kByte4_GrSLType:
        case kUByte4_GrSLType:
            return 1;
        case kFloat2x2_GrSLType:
        case kHalf2x2_GrSLType:
            return 2;
        case kFloat3x3_GrSLType:
        case kHalf3x3_GrSLType:
            return 3;
        case kFloat4x4_GrSLType:
        case kHalf4x4_GrSLType:
            return 4;
        case kTexture2DSampler_GrSLType:
        case kSampler_GrSLType:
        case kTexture2D_GrSLType:
            return 0;
        case kTextureExternalSampler_GrSLType:
             return 0;
        case kTexture2DRectSampler_GrSLType:
             return 0;
        case kBool_GrSLType:
             return 1;
        case kInt_GrSLType: // fall through
        case kShort_GrSLType:
        case kByte_GrSLType:
             return 1;
        case kUint_GrSLType: // fall through
        case kUShort_GrSLType:
        case kUByte_GrSLType:
             return 1;
    }
    SK_ABORT("Unexpected type");
}

static void finalize_helper(GrD3DVaryingHandler::VarArray& vars) {
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
    // Since this is based off of Spir-V, Vulkan requires at least 64 locations to be supported
    // for both vertex output and fragment input. If we ever hit this assert or crash cause our
    // output struct from the HLSL shader is too large we'll have to adjust this value. But for now
    // using the vulkan value of 64 seems an okay starting point.
    SkASSERT(locationIndex <= 64);
}

void GrD3DVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fGeomInputs);
    finalize_helper(fGeomOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
