/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProcessorTestData.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrRecordingContextPriv.h"

#if GR_TEST_UTILS

GrProcessorTestData::GrProcessorTestData(SkRandom* random, GrRecordingContext* context,
                                         int numViews, const ViewInfo views[],
                                         std::unique_ptr<GrFragmentProcessor> inputFP)
        : fRandom(random), fContext(context), fInputFP(std::move(inputFP)) {
    fViews.reset(views, numViews);
    fArena = std::unique_ptr<SkArenaAlloc>(new SkArenaAlloc(1000));
}

GrProxyProvider* GrProcessorTestData::proxyProvider() { return fContext->priv().proxyProvider(); }

const GrCaps* GrProcessorTestData::caps() { return fContext->priv().caps(); }

GrProcessorTestData::ViewInfo GrProcessorTestData::randomView() {
    SkASSERT(!fViews.empty());
    return fViews[fRandom->nextULessThan(fViews.count())];
}

GrProcessorTestData::ViewInfo GrProcessorTestData::randomAlphaOnlyView() {
    int numAlphaOnly = 0;
    for (const auto& [v, ct, at] : fViews) {
        if (GrColorTypeIsAlphaOnly(ct)) {
            ++numAlphaOnly;
        }
    }
    SkASSERT(numAlphaOnly);
    int idx = fRandom->nextULessThan(numAlphaOnly);
    for (const auto& [v, ct, at] : fViews) {
        if (GrColorTypeIsAlphaOnly(ct) && !idx--) {
            return {v, ct, at};
        }
    }
    SkUNREACHABLE;
}

#endif  // GR_TEST_UTILS
