/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrDrawOpAtlas.h"
#include "GrOpFlushState.h"

static void handle_eviction(GrDrawOpAtlas::AtlasID id, void*) {
}

static void test_resizing(skiatest::Reporter* reporter, GrContext* context,
                          int minX, int minY, int maxX, int maxY) {

    GrGpu* gpu = context->getGpu();

    GrOpFlushState flushState(gpu, nullptr);

    std::unique_ptr<GrDrawOpAtlas> atlas = GrDrawOpAtlas::Make(context, kAlpha_8_GrPixelConfig,
                                                               SkISize::Make(64, 64),
                                                               SkIPoint::Make(minX, minY),
                                                               SkIPoint::Make(maxX, maxY),
                                                               handle_eviction, nullptr);

    int numIters = (maxX-minX) + (maxY-minY);

    SkISize atlasSize = atlas->curAtlasSize();

    // Should be able to expand the atlas 'numIters' times
    for (int i = 0; i < numIters; ++i) {
        REPORTER_ASSERT(reporter, atlas->grow());
        SkISize newSize = atlas->curAtlasSize();
        REPORTER_ASSERT(reporter, newSize.fWidth >= atlasSize.fWidth);
        REPORTER_ASSERT(reporter, newSize.fHeight >= atlasSize.fHeight);
        REPORTER_ASSERT(reporter, newSize.fWidth > atlasSize.fWidth ||
                                  newSize.fHeight > atlasSize.fHeight);
        atlasSize = newSize;
    }

    REPORTER_ASSERT(reporter, !atlas->grow());

    // Should be able to shrink it back down 'numIters' times too
    for (int i = 0; i < numIters; ++i) {
        REPORTER_ASSERT(reporter, atlas->shrink(&flushState));
        SkISize newSize = atlas->curAtlasSize();
        REPORTER_ASSERT(reporter, newSize.fWidth <= atlasSize.fWidth);
        REPORTER_ASSERT(reporter, newSize.fHeight <= atlasSize.fHeight);
        REPORTER_ASSERT(reporter, newSize.fWidth < atlasSize.fWidth ||
                                  newSize.fHeight < atlasSize.fHeight);
        atlasSize = newSize;
    }

    REPORTER_ASSERT(reporter, !atlas->shrink(&flushState));
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DrawOpAtlasTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    // Test resizing symmetrically
    test_resizing(reporter, context, 1, 1, 4, 4);

    // Force resizing only in X
    test_resizing(reporter, context, 1, 1, 4, 1);

    // Force resizing only in Y
    test_resizing(reporter, context, 1, 1, 1, 4);
}

#endif
