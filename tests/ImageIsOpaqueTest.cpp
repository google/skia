/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif
#include "SkImage.h"
#include "SkSurface.h"

#include "Test.h"

DEF_TEST(ImageIsOpaqueTest, reporter) {
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    SkAutoTUnref<SkSurface> surfaceTransparent(SkSurface::NewRaster(infoTransparent));
    REPORTER_ASSERT(reporter, !surfaceTransparent->newImageSnapshot()->isOpaque());

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surfaceOpaque(SkSurface::NewRaster(infoOpaque));
    REPORTER_ASSERT(reporter, surfaceOpaque->newImageSnapshot()->isOpaque());
}

#if SK_SUPPORT_GPU

DEF_GPUTEST(ImageIsOpaqueTest_GPU, reporter, factory) {
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;

        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }

        GrContext* context = factory->get(glCtxType);

        if (NULL == context) {
            continue;
        }

        SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
        SkAutoTUnref<SkSurface> surfaceTransparent(SkSurface::NewRenderTarget(context, infoTransparent));
        REPORTER_ASSERT(reporter, !surfaceTransparent->newImageSnapshot()->isOpaque());

        SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
        SkAutoTUnref<SkSurface> surfaceOpaque(SkSurface::NewRenderTarget(context, infoOpaque));
        REPORTER_ASSERT(reporter, !surfaceOpaque->newImageSnapshot()->isOpaque());

    }
}

#endif
