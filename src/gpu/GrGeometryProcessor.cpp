/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGeometryProcessor.h"

#include "GrInvariantOutput.h"

void GrGeometryProcessor::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    if (fHasVertexColor) {
        if (fOpaqueVertexColors) {
            out->setUnknownOpaqueFourComponents();
        } else {
            out->setUnknownFourComponents();
        }
    } else {
        out->setKnownFourComponents(fColor);
    }
    this->onGetInvariantOutputColor(out);
}

void GrGeometryProcessor::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    this->onGetInvariantOutputCoverage(out);
}
