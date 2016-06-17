/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkVaryingHandler.h"


void finalize_helper(GrVkVaryingHandler::VarArray& vars) {
    for (int i = 0; i < vars.count(); ++i) {
        SkString location;
        location.appendf("location = %d", i);
        vars[i].setLayoutQualifier(location.c_str());
    }
}

void GrVkVaryingHandler::onFinalize() {
    finalize_helper(fVertexInputs);
    finalize_helper(fVertexOutputs);
    finalize_helper(fGeomInputs);
    finalize_helper(fGeomOutputs);
    finalize_helper(fFragInputs);
    finalize_helper(fFragOutputs);
}
