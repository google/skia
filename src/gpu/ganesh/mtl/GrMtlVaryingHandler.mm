/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlVaryingHandler.h"

#include "src/gpu/ganesh/mtl/GrMtlTypesPriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

static void finalize_helper(GrMtlVaryingHandler::VarArray& vars) {
    int locationIndex = 0;

    SkDEBUGCODE(int componentCount = 0);
    for (GrShaderVar& var : vars.items()) {
        // Metal only allows scalars (including bool and char) and vectors as varyings
        SkASSERT(SkSLTypeVecLength(var.getType()) != -1);
        SkDEBUGCODE(componentCount += SkSLTypeVecLength(var.getType()));

        SkString location;
        location.appendf("location = %d", locationIndex);
        var.addLayoutQualifier(location.c_str());
        ++locationIndex;
    }
    // The max number of inputs is 60 for iOS and 32 for macOS. The max number of components is 60
    // for iOS and 128 for macOS. To be conservative, we are going to assert that we have less than
    // 32 varyings and less than 60 components across all varyings. If we hit this assert, we can
    // implement a function in GrMtlCaps to be less conservative.
    SkASSERT(locationIndex <= 32);
    SkASSERT(componentCount <= 60);
}

void GrMtlVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}

GR_NORETAIN_END
