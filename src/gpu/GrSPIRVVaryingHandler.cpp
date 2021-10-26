/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSPIRVVaryingHandler.h"

/** Returns the number of locations take up by a given GrSLType. We assume that all
    scalar values are 32 bits. */
static inline int grsltype_to_location_size(GrSLType type) {
    // If a new GrSL type is added, this function will need to be updated.
    static_assert(kGrSLTypeCount == 41);

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
        case kInt2_GrSLType:
        case kUint2_GrSLType:
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
            return 1;
        case kInt3_GrSLType:
        case kUint3_GrSLType:
        case kShort3_GrSLType:
        case kUShort3_GrSLType:
            return 1;
        case kInt4_GrSLType:
        case kUint4_GrSLType:
        case kShort4_GrSLType:
        case kUShort4_GrSLType:
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
        case kBool2_GrSLType:
        case kBool3_GrSLType:
        case kBool4_GrSLType:
             return 1;
        case kInt_GrSLType: // fall through
        case kShort_GrSLType:
             return 1;
        case kUint_GrSLType: // fall through
        case kUShort_GrSLType:
             return 1;
        case kTexture2D_GrSLType:
             return 0;
        case kSampler_GrSLType:
             return 0;
        case kInput_GrSLType:
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

        int elementSize = grsltype_to_location_size(var.getType());
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
