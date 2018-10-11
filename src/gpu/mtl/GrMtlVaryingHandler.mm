/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlVaryingHandler.h"

static void finalize_helper(GrMtlVaryingHandler::VarArray& vars) {
    int locationIndex;
    for (locationIndex = 0; locationIndex < vars.count(); locationIndex++) {
        GrShaderVar& var = vars[locationIndex];
        // Metal only allows scalars (including bool and char) and vectors as varyings
        SkASSERT(GrSLTypeVecLength(var.getType()) != -1);

        SkString location;
        location.appendf("location = %d", locationIndex);
        var.addLayoutQualifier(location.c_str());
    }
    // The max number of inputs is 60 for iOS and 32 for macOS. The max number of components is 60
    // for iOS and 128 for macOS. To be conservative, we are going to assert that we have less than
    // 15 varyings because in the worst case scenario, they are all vec4s (15 * 4 = 60). If we hit
    // this assert, we can implement a function in GrMtlCaps to be less conservative.
    SkASSERT(locationIndex <= 15);
}

void GrMtlVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fGeomInputs);
    finalize_helper(fGeomOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
