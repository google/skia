/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/gpu/GrWrappedImageFactory.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedImageFactoryTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    // For now, while developing the API, we cheat and make the backendTexture renderable
    GrBackendTexture backendTex = context->createBackendTexture(32, 32, kRGBA_8888_SkColorType,
                                                                SkColors::kRed,
                                                                GrMipMapped::kNo,
                                                                GrRenderable::kYes);

    sk_sp<SkImage> img = SkImage::MakeFromTexture(context, backendTex, kTopLeft_GrSurfaceOrigin,
                                                  kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                                  SkColorSpace::MakeSRGB());

    SkImageInfo ii = SkImageInfo::Make({ 64, 64 }, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType, SkColorSpace::MakeSRGB());

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorBLACK);
    canvas->drawImage(std::move(img), 0, 0);

    context->flush();

    SkBitmap readback;
    readback.allocPixels(ii);

    SkAssertResult(surf->readPixels(readback, 0, 0));

    SkColor col = readback.getColor(16, 16);

    REPORTER_ASSERT(reporter, col == SK_ColorRED);

    context->deleteBackendTexture(backendTex);
}
