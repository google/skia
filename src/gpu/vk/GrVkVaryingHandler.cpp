/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkVaryingHandler.h"

/** Returns the number of locations take up by a given GrSLType. We assume that all
    scalar values are 32 bits. */
static inline int grsltype_to_location_size(GrSLType type) {
    switch(type) {
        case kVoid_GrSLType:
            return 0;
        case kFloat_GrSLType:
            return 1;
        case kVec2f_GrSLType:
            return 1;
        case kVec3f_GrSLType:
            return 1;
        case kVec4f_GrSLType:
            return 1;
        case kVec2i_GrSLType:
            return 1;
        case kVec3i_GrSLType:
            return 1;
        case kVec4i_GrSLType:
            return 1;
        case kMat22f_GrSLType:
            return 2;
        case kMat33f_GrSLType:
            return 3;
        case kMat44f_GrSLType:
            return 4;
        case kTexture2DSampler_GrSLType:
            return 0;
        case kITexture2DSampler_GrSLType:
             return 0;
        case kTextureExternalSampler_GrSLType:
             return 0;
        case kTexture2DRectSampler_GrSLType:
             return 0;
        case kBufferSampler_GrSLType:
             return 0;
        case kBool_GrSLType:
             return 1;
        case kInt_GrSLType:
             return 1;
        case kUint_GrSLType:
             return 1;
        case kTexture2D_GrSLType:
             return 0;
        case kSampler_GrSLType:
             return 0;
        case kImageStorage2D_GrSLType:
            return 0;
        case kIImageStorage2D_GrSLType:
            return 0;
    }
    SkFAIL("Unexpected type");
    return -1;
}

void finalize_helper(GrVkVaryingHandler::VarArray& vars) {
    int locationIndex = 0;
    for (int i = 0; i < vars.count(); ++i) {
        GrShaderVar& var = vars[i];
        SkString location;
        location.appendf("location = %d", locationIndex);
        var.addLayoutQualifier(location.c_str());

        int elementSize = grsltype_to_location_size(var.getType());
        SkASSERT(elementSize);
        int numElements = 1;
        if (var.isArray()) {
           numElements = var.getArrayCount();
        }
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
