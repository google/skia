/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorSet.h"

constexpr int GrProcessorSet::kMaxColorFragmentProcessors;

GrProcessorSet::GrProcessorSet(GrPaint&& paint) {
    fXPFactory = paint.fXPFactory;
    SkASSERT(paint.numColorFragmentProcessors() <= kMaxColorFragmentProcessors);
    fColorFragmentProcessorCnt =
            SkTMin(paint.numColorFragmentProcessors(), kMaxColorFragmentProcessors);
    int totalCnt = fColorFragmentProcessorCnt + paint.numCoverageFragmentProcessors();
    fFragmentProcessors.reset(totalCnt);
    int i;
    for (i = 0; i < fColorFragmentProcessorCnt; ++i) {
        fFragmentProcessors[i] = paint.fColorFragmentProcessors[i].release();
    }
    for (auto& fp : paint.fCoverageFragmentProcessors) {
        fFragmentProcessors[i++] = fp.release();
    }
}
