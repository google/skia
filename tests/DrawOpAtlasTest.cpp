/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrDrawOpAtlas.h"

static void handle_eviction(GrDrawOpAtlas::AtlasID id, void*) {
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DrawOpAtlasTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    std::unique_ptr<GrDrawOpAtlas> atlas = GrDrawOpAtlas::Make(context, kAlpha_8_GrPixelConfig,
                                                               64, 128,
                                                               1, 1,
                                                               4, 4, handle_eviction, nullptr);


    // Should be able to expand the atlas 6 times
    for (int i = 0; i < 6; ++i) {
        REPORTER_ASSERT(reporter, atlas->enbiggen());
    }

    REPORTER_ASSERT(reporter, !atlas->enbiggen());

    // Should be able to shrink it back down 6 times too
    for (int i = 0; i < 6; ++i) {
        REPORTER_ASSERT(reporter, atlas->ensmallen());
    }

    REPORTER_ASSERT(reporter, !atlas->ensmallen());
}

#endif
