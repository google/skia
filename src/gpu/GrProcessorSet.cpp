/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorSet.h"

GrProcessorSet::GrProcessorSet(GrPaint&& paint) {
    fXPFactory = paint.fXPFactory;
    fColorFragmentProcessorCnt = paint.numColorFragmentProcessors();
    fFragmentProcessors.reset(paint.numTotalFragmentProcessors());
    int i = 0;
    for (auto& fp : paint.fColorFragmentProcessors) {
        fFragmentProcessors[i++] = fp.release();
    }
    for (auto& fp : paint.fCoverageFragmentProcessors) {
        fFragmentProcessors[i++] = fp.release();
    }
    fFlags = 0;
    if (paint.usesDistanceVectorField()) {
        fFlags |= kUseDistanceVectorField_Flag;
    }
    if (paint.getDisableOutputConversionToSRGB()) {
        fFlags |= kDisableOutputConversionToSRGB_Flag;
    }
    if (paint.getAllowSRGBInputs()) {
        fFlags |= kAllowSRGBInputs_Flag;
    }
}
