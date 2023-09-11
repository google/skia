/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstdint>

struct GrContextOptions;

static void check_pixels(skiatest::Reporter* reporter, const SkBitmap& bitmap,
                         GrSurfaceOrigin origin) {
    const uint32_t* canvasPixels = static_cast<const uint32_t*>(bitmap.getPixels());

    bool failureFound = false;
    bool foundNonBlue = false;

    for (int cy = 0; cy < 8 && !failureFound; ++cy) {
        int cx = 4; // Just need to check one column;
        SkPMColor canvasPixel = canvasPixels[cy * 8 + cx];
        // We don't know which way the GPU will snap so the non-blue line could either be at row
        // 3 or 4 since we drew the line at a y value of 4. We check that one of those two values
        // is green and all the rest are blue. The key thing is that we should not get any red
        // values since the green line in the saveLayer should snap the same way and overwrite the
        // red line.
        if (cy == 3) {
            if (canvasPixel != 0xFFFF0000 && canvasPixel != 0xFF00FF00) {
                failureFound = true;
                ERRORF(reporter, "Wrong color at %d, %d. Got 0x%08x when we expected Blue or Green."
                       " Origin is: %s",
                       cx, cy, canvasPixel, GrSurfaceOriginToStr(origin));
            }
            if (canvasPixel != 0XFFFF0000) {
                foundNonBlue = true;
            }
        } else {
            SkPMColor expectedPixel;
            if (cy == 4 && !foundNonBlue) {
                expectedPixel = 0xFF00FF00; // Green
            } else {
                expectedPixel = 0xFFFF0000; // Blue
            }
            if (canvasPixel != expectedPixel) {
                failureFound = true;
                ERRORF(reporter,
                       "Wrong color at %d, %d. Got 0x%08x when we expected 0x%08x. Origin is: %s",
                       cx, cy, canvasPixel, expectedPixel, GrSurfaceOriginToStr(origin));
            }
        }
    }
}

static void run_test(skiatest::Reporter* reporter,
                     GrDirectContext* context,
                     GrSurfaceOrigin origin) {
    auto beTexture = context->createBackendTexture(8,
                                                   8,
                                                   kRGBA_8888_SkColorType,
                                                   skgpu::Mipmapped::kNo,
                                                   GrRenderable::kYes,
                                                   GrProtected::kNo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    if (!beTexture.isValid()) {
        return;
    }

    auto surface = SkSurfaces::WrapBackendTexture(
            context, beTexture, origin, 0, kRGBA_8888_SkColorType, nullptr, nullptr);
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SK_ColorBLUE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawLine({ 0,4 }, { 8,4 }, paint);

    SkRect bounds = SkRect::MakeWH(8, 8);
    SkPaint layerPaint;
    canvas->saveLayer(bounds, &paint);
    paint.setColor(SK_ColorGREEN);
    canvas->drawLine({ 0,4 }, { 8,4 }, paint);
    canvas->restore();

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    surface->readPixels(bitmap, 0, 0);

    check_pixels(reporter, bitmap, origin);

    context->deleteBackendTexture(beTexture);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SaveLayerOrigin,
                                       reporter,
                                       context_info,
                                       CtsEnforcement::kApiLevel_T) {
    GrDirectContext* context = context_info.directContext();
    run_test(reporter, context, kBottomLeft_GrSurfaceOrigin);
    run_test(reporter, context, kTopLeft_GrSurfaceOrigin);
}
