/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlVaryingHandler.h"

// TODO: this class is basically copy and pasted from GrVkVaryingHandler so that we can have
// some shaders working. The SkSL Metal code generator was written to work with GLSL generated for
// the Ganesh Vulkan backend, so it should all work. There might be better ways to do things in
// Metal and/or some Vulkan GLSLisms left in.

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
    return -1;
}

void finalize_helper(GrMtlVaryingHandler::VarArray& vars) {
    int locationIndex = 0;
    for (int i = 0; i < vars.count(); ++i) {
        GrShaderVar& var = vars[i];
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
}

void GrMtlVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fGeomInputs);
    finalize_helper(fGeomOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
