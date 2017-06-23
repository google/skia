/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"
#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkSurface.h"
#include "Test.h"

// Tests that MIP maps are created and invalidated as expected when drawing to and from GrTextures.
DEF_GPUTEST_FOR_NULLGL_CONTEXT(GrTextureMipMapInvalidationTest, reporter, ctxInfo) {
    auto isMipped = [] (SkSurface* surf) {
        return surf->makeImageSnapshot()->getTexture()->texturePriv().hasMipMaps();
    };

    auto mipsAreDirty = [] (SkSurface* surf) {
        return surf->makeImageSnapshot()->getTexture()->texturePriv().mipMapsAreDirty();
    };

    GrContext* context = ctxInfo.grContext();
    auto info = SkImageInfo::MakeN32Premul(256, 256);
    auto surf1 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info);
    auto surf2 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info);
    // Draw something just in case we ever had a solid color optimization
    surf1->getCanvas()->drawCircle(128, 128, 50, SkPaint());
    surf1->getCanvas()->flush();

    // No mipmaps initially
    REPORTER_ASSERT(reporter, !isMipped(surf1.get()));

    // Painting with downscale and medium filter quality should result in mipmap creation
    SkPaint paint;
    paint.setFilterQuality(kMedium_SkFilterQuality);
    surf2->getCanvas()->scale(0.2f, 0.2f);
    surf2->getCanvas()->drawImage(surf1->makeImageSnapshot(), 0, 0, &paint);
    surf2->getCanvas()->flush();
    REPORTER_ASSERT(reporter, isMipped(surf1.get()));
    REPORTER_ASSERT(reporter, !mipsAreDirty(surf1.get()));

    // Changing the contents of the surface should invalidate the mipmap, but not de-allocate
    surf1->getCanvas()->drawCircle(128, 128, 100, SkPaint());
    surf1->getCanvas()->flush();
    REPORTER_ASSERT(reporter, isMipped(surf1.get()));
    REPORTER_ASSERT(reporter, mipsAreDirty(surf1.get()));
}

#endif
