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
    static const uint32_t kSizes[] = {
        0,  // kVoid_GrSLType
        1,  // kFloat_GrSLType
        1,  // kVec2f_GrSLType
        1,  // kVec3f_GrSLType
        1,  // kVec4f_GrSLType
        2,  // kMat22f_GrSLType
        3,  // kMat33f_GrSLType
        4,  // kMat44f_GrSLType
        0,  // kTexture2DSampler_GrSLType
        0,  // kTexture2DISampler_GrSLType
        0,  // kTextureExternalSampler_GrSLType
        0,  // kTexture2DRectSampler_GrSLType
        0,  // kTextureBufferSampler_GrSLType
        1,  // kBool_GrSLType
        1,  // kInt_GrSLType
        1,  // kUint_GrSLType
        0,  // kTexture2D_GrSLType
        0,  // kSampler_GrSLType
    };
    return kSizes[type];

    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat22f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(7 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(8 == kTexture2DSampler_GrSLType);
    GR_STATIC_ASSERT(9 == kTexture2DISampler_GrSLType);
    GR_STATIC_ASSERT(10 == kTextureExternalSampler_GrSLType);
    GR_STATIC_ASSERT(11 == kTexture2DRectSampler_GrSLType);
    GR_STATIC_ASSERT(12 == kTextureBufferSampler_GrSLType);
    GR_STATIC_ASSERT(13 == kBool_GrSLType);
    GR_STATIC_ASSERT(14 == kInt_GrSLType);
    GR_STATIC_ASSERT(15 == kUint_GrSLType);
    GR_STATIC_ASSERT(16 == kTexture2D_GrSLType);
    GR_STATIC_ASSERT(17 == kSampler_GrSLType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kSizes) == kGrSLTypeCount);
}

void finalize_helper(GrVkVaryingHandler::VarArray& vars) {
    int locationIndex = 0;
    for (int i = 0; i < vars.count(); ++i) {
        GrGLSLShaderVar& var = vars[i];
        SkString location;
        location.appendf("location = %d", locationIndex);
        var.setLayoutQualifier(location.c_str());

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
