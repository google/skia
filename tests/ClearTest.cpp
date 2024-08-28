/*
 * Copyright 2016 Google Inc.
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
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/ClearOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/OpsTask.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <memory>

class GrRecordingContext;

using SurfaceDrawContext = skgpu::ganesh::SurfaceDrawContext;
using ClearOp = skgpu::ganesh::ClearOp;

static bool pixel_matches(const SkPixmap& pm, int x, int y, uint32_t expectedValue,
                          uint32_t* actualValue, int* failX, int* failY) {
    uint32_t pixel = pm.addr32()[y * pm.width() + x];
    if (pixel != expectedValue) {
        *actualValue = pixel;
        *failX = x;
        *failY = y;
        return false;
    }

    return true;
}

static bool check_rect(GrDirectContext* dContext,
                       SurfaceDrawContext* sdc,
                       const SkIRect& rect,
                       uint32_t expectedValue,
                       uint32_t* actualValue,
                       int* failX,
                       int* failY) {
    int w = sdc->width();
    int h = sdc->height();

    SkImageInfo dstInfo = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(dstInfo);

    readback.erase(~expectedValue);
    if (!sdc->readPixels(dContext, readback, {0, 0})) {
        return false;
    }

    SkASSERT(rect.fTop < rect.fBottom && rect.fLeft < rect.fRight);
    for (int y = rect.fTop; y < rect.fBottom; ++y) {
        for (int x = rect.fLeft; x < rect.fRight; ++x) {
            if (!pixel_matches(readback, x, y, expectedValue, actualValue, failX, failY)) {
                return false;
            }
        }
    }
    return true;
}

// Check a 1-pixel wide ring 'inset' from the outer edge
static bool check_ring(GrDirectContext* dContext,
                       SurfaceDrawContext* sdc,
                       int inset,
                       uint32_t expectedValue,
                       uint32_t* actualValue,
                       int* failX,
                       int* failY) {
    int w = sdc->width();
    int h = sdc->height();

    SkImageInfo dstInfo = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(dstInfo);

    readback.erase(~expectedValue);
    if (!sdc->readPixels(dContext, readback, {0, 0})) {
        return false;
    }

    for (int y = inset; y < h-inset; ++y) {
        if (!pixel_matches(readback, inset, y, expectedValue, actualValue, failX, failY) ||
            !pixel_matches(readback, w-1-inset, y, expectedValue, actualValue, failX, failY)) {
            return false;
        }
    }
    for (int x = inset+1; x < w-inset-1; ++x) {
        if (!pixel_matches(readback, x, inset, expectedValue, actualValue, failX, failY) ||
            !pixel_matches(readback, x, h-1-inset, expectedValue, actualValue, failX, failY)) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<SurfaceDrawContext> newSDC(GrRecordingContext* rContext, int w, int h) {
    return SurfaceDrawContext::Make(rContext, GrColorType::kRGBA_8888, nullptr,
                                    SkBackingFit::kExact, {w, h}, SkSurfaceProps(),
                                    /*label=*/{});
}

static void clear_op_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    static const int kW = 10;
    static const int kH = 10;

    SkIRect fullRect = SkIRect::MakeWH(kW, kH);
    std::unique_ptr<SurfaceDrawContext> sdc;

    // A rectangle that is inset by one on all sides and the 1-pixel wide rectangles that surround
    // it.
    SkIRect mid1Rect = SkIRect::MakeXYWH(1, 1, kW-2, kH-2);

    // A rectangle that is inset by two on all sides and the 1-pixel wide rectangles that surround
    // it.
    SkIRect mid2Rect = SkIRect::MakeXYWH(2, 2, kW-4, kH-4);

    uint32_t actualValue;
    int failX, failY;

    static const GrColor kColor1 = 0xABCDEF01;
    static const GrColor kColor2 = ~kColor1;
    static const SkPMColor4f kColor1f = SkPMColor4f::FromBytes_RGBA(kColor1);
    static const SkPMColor4f kColor2f = SkPMColor4f::FromBytes_RGBA(kColor2);

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Check a full clear
    sdc->clear(fullRect, kColor1f);
    if (!check_rect(dContext, sdc.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Check two full clears, same color
    sdc->clear(fullRect, kColor1f);
    sdc->clear(fullRect, kColor1f);
    if (!check_rect(dContext, sdc.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Check two full clears, different colors
    sdc->clear(fullRect, kColor1f);
    sdc->clear(fullRect, kColor2f);
    if (!check_rect(dContext, sdc.get(), fullRect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Test a full clear followed by a same color inset clear
    sdc->clear(fullRect, kColor1f);
    sdc->clear(mid1Rect, kColor1f);
    if (!check_rect(dContext, sdc.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Test a inset clear followed by same color full clear
    sdc->clear(mid1Rect, kColor1f);
    sdc->clear(fullRect, kColor1f);
    if (!check_rect(dContext, sdc.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Test a full clear followed by a different color inset clear
    sdc->clear(fullRect, kColor1f);
    sdc->clear(mid1Rect, kColor2f);
    if (!check_rect(dContext, sdc.get(), mid1Rect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }

    if (!check_ring(dContext, sdc.get(), /*inset=*/ 0, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Test a inset clear followed by a different full clear
    sdc->clear(mid1Rect, kColor2f);
    sdc->clear(fullRect, kColor1f);
    if (!check_rect(dContext, sdc.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Check three nested clears from largest to smallest where outermost and innermost are same
    // color.
    sdc->clear(fullRect, kColor1f);
    sdc->clear(mid1Rect, kColor2f);
    sdc->clear(mid2Rect, kColor1f);
    if (!check_rect(dContext, sdc.get(), mid2Rect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!check_ring(dContext, sdc.get(), /*inset=*/ 1, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }

    if (!check_ring(dContext, sdc.get(), /*inset=*/ 0, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    sdc = newSDC(dContext, kW, kH);
    SkASSERT(sdc);

    // Swap the order of the second two clears in the above test.
    sdc->clear(fullRect, kColor1f);
    sdc->clear(mid2Rect, kColor1f);
    sdc->clear(mid1Rect, kColor2f);
    if (!check_rect(dContext, sdc.get(), mid1Rect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_ring(dContext, sdc.get(), /*inset=*/ 0, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    // Clear calls need to remain ClearOps for the following combining-tests to work as expected
    if (!dContext->priv().caps()->performColorClearsAsDraws() &&
        !dContext->priv().caps()->performStencilClearsAsDraws() &&
        !dContext->priv().caps()->performPartialClearsAsDraws()) {
        static constexpr SkIRect kScissorRect = SkIRect::MakeXYWH(1, 1, kW-1, kH-1);

        // Try combining a pure-color clear w/ a combined stencil & color clear
        // (re skbug.com/10963)
        {
            sdc = newSDC(dContext, kW, kH);
            SkASSERT(sdc);

            sdc->clearStencilClip(kScissorRect, true);
            // This color clear can combine w/ the preceding stencil clear
            sdc->clear(kScissorRect, SK_PMColor4fWHITE);

            // This should combine w/ the prior combined clear and overwrite the color
            sdc->clear(kScissorRect, SK_PMColor4fBLACK);

            auto opsTask = sdc->getOpsTask();
            REPORTER_ASSERT(reporter, opsTask->numOpChains() == 1);

            const ClearOp& clearOp = opsTask->getChain(0)->cast<ClearOp>();

            constexpr std::array<float, 4> kExpected { 0, 0, 0, 1 };
            REPORTER_ASSERT(reporter, clearOp.color() == kExpected);
            REPORTER_ASSERT(reporter, clearOp.stencilInsideMask());

            dContext->flushAndSubmit();
        }

        // Try combining a pure-stencil clear w/ a combined stencil & color clear
        // (re skbug.com/10963)
        {
            sdc = newSDC(dContext, kW, kH);
            SkASSERT(sdc);

            sdc->clearStencilClip(kScissorRect, true);
            // This color clear can combine w/ the preceding stencil clear
            sdc->clear(kScissorRect, SK_PMColor4fWHITE);

            // This should combine w/ the prior combined clear and overwrite the 'insideStencilMask'
            // field
            sdc->clearStencilClip(kScissorRect, false);

            auto opsTask = sdc->getOpsTask();
            REPORTER_ASSERT(reporter, opsTask->numOpChains() == 1);

            const ClearOp& clearOp = opsTask->getChain(0)->cast<ClearOp>();

            constexpr std::array<float, 4> kExpected { 1, 1, 1, 1 };
            REPORTER_ASSERT(reporter, clearOp.color() == kExpected);
            REPORTER_ASSERT(reporter, !clearOp.stencilInsideMask());

            dContext->flushAndSubmit();
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ClearOp, reporter, ctxInfo, CtsEnforcement::kApiLevel_T) {
    // Regular clear
    clear_op_test(reporter, ctxInfo.directContext());

    // Force drawing for clears
    GrContextOptions options(ctxInfo.options());
    options.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
    sk_gpu_test::GrContextFactory workaroundFactory(options);
    clear_op_test(reporter, workaroundFactory.get(ctxInfo.type()));
}

void fullscreen_clear_with_layer_test(skiatest::Reporter* reporter, GrRecordingContext* rContext) {
    const SkImageInfo ii = SkImageInfo::Make(400, 77, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kYes, ii);
    SkCanvas* canvas = surf->getCanvas();

    SkPaint paints[2];
    paints[0].setColor(SK_ColorGREEN);
    paints[1].setColor(SK_ColorGRAY);

    static const int kLeftX = 158;
    static const int kMidX = 258;
    static const int kRightX = 383;
    static const int kTopY = 26;
    static const int kBotY = 51;

    const SkRect rects[2] = {
        { kLeftX, kTopY, kMidX, kBotY },
        { kMidX, kTopY, kRightX, kBotY },
    };

    for (int i = 0; i < 2; ++i) {
        // the bounds parameter is required to cause a full screen clear
        canvas->saveLayer(&rects[i], nullptr);
            canvas->drawRect(rects[i], paints[i]);
        canvas->restore();
    }

    SkBitmap bm;
    bm.allocPixels(ii, 0);

    SkAssertResult(surf->readPixels(bm, 0, 0));

    bool isCorrect = true;
    for (int y = kTopY; isCorrect && y < kBotY; ++y) {
        const uint32_t* sl = bm.getAddr32(0, y);

        for (int x = kLeftX; x < kMidX; ++x) {
            if (SK_ColorGREEN != sl[x]) {
                isCorrect = false;
                break;
            }
        }

        for (int x = kMidX; x < kRightX; ++x) {
            if (SK_ColorGRAY != sl[x]) {
                isCorrect = false;
                break;
            }
        }
    }

    REPORTER_ASSERT(reporter, isCorrect);
}
// From crbug.com/768134
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(FullScreenClearWithLayers,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    // Regular clear
    fullscreen_clear_with_layer_test(reporter, ctxInfo.directContext());

    // Use draws for clears
    GrContextOptions options(ctxInfo.options());
    options.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
    sk_gpu_test::GrContextFactory workaroundFactory(options);
    fullscreen_clear_with_layer_test(reporter, workaroundFactory.get(ctxInfo.type()));
}
