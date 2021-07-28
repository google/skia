/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/ops/GrClearOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include <cstdint>
#include <memory>

static bool check_rect(GrDirectContext* dContext,
                       skgpu::v1::SurfaceDrawContext* sdc,
                       const SkIRect& rect,
                       uint32_t expectedValue,
                       uint32_t* actualValue,
                       int* failX,
                       int* failY) {
    int w = rect.width();
    int h = rect.height();

    SkImageInfo dstInfo = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(dstInfo);

    readback.erase(~expectedValue);
    if (!sdc->readPixels(dContext, readback, {rect.fLeft, rect.fTop})) {
        return false;
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t pixel = readback.addr32()[y * w + x];
            if (pixel != expectedValue) {
                *actualValue = pixel;
                *failX = x + rect.fLeft;
                *failY = y + rect.fTop;
                return false;
            }
        }
    }
    return true;
}

std::unique_ptr<skgpu::v1::SurfaceDrawContext> newSDC(GrRecordingContext* rContext, int w, int h) {
    return skgpu::v1::SurfaceDrawContext::Make(
            rContext, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact, {w, h},
            SkSurfaceProps());
}

static void clear_op_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    static const int kW = 10;
    static const int kH = 10;

    SkIRect fullRect = SkIRect::MakeWH(kW, kH);
    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc;

    // A rectangle that is inset by one on all sides and the 1-pixel wide rectangles that surround
    // it.
    SkIRect mid1Rect = SkIRect::MakeXYWH(1, 1, kW-2, kH-2);
    SkIRect outerLeftEdge = SkIRect::MakeXYWH(0, 0, 1, kH);
    SkIRect outerTopEdge = SkIRect::MakeXYWH(0, 0, kW, 1);
    SkIRect outerRightEdge = SkIRect::MakeXYWH(kW-1, 0, 1, kH);
    SkIRect outerBottomEdge = SkIRect::MakeXYWH(0, kH-1, kW, 1);

    // A rectangle that is inset by two on all sides and the 1-pixel wide rectangles that surround
    // it.
    SkIRect mid2Rect = SkIRect::MakeXYWH(2, 2, kW-4, kH-4);
    SkIRect innerLeftEdge = SkIRect::MakeXYWH(1, 1, 1, kH-2);
    SkIRect innerTopEdge = SkIRect::MakeXYWH(1, 1, kW-2, 1);
    SkIRect innerRightEdge = SkIRect::MakeXYWH(kW-2, 1, 1, kH-2);
    SkIRect innerBottomEdge = SkIRect::MakeXYWH(1, kH-2, kW-2, 1);

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
    if (!check_rect(dContext, sdc.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
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
    if (!check_rect(dContext, sdc.get(), innerLeftEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), innerTopEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), innerRightEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), innerBottomEdge, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(dContext, sdc.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
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
    if (!check_rect(dContext, sdc.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(dContext, sdc.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
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

            GrOpsTask* ops = sdc->getOpsTask();
            REPORTER_ASSERT(reporter, ops->numOpChains() == 1);

            const GrClearOp& clearOp = ops->getChain(0)->cast<GrClearOp>();

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

            GrOpsTask* ops = sdc->getOpsTask();
            REPORTER_ASSERT(reporter, ops->numOpChains() == 1);

            const GrClearOp& clearOp = ops->getChain(0)->cast<GrClearOp>();

            constexpr std::array<float, 4> kExpected { 1, 1, 1, 1 };
            REPORTER_ASSERT(reporter, clearOp.color() == kExpected);
            REPORTER_ASSERT(reporter, !clearOp.stencilInsideMask());

            dContext->flushAndSubmit();
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ClearOp, reporter, ctxInfo) {
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

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes, ii);
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
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FullScreenClearWithLayers, reporter, ctxInfo) {
    // Regular clear
    fullscreen_clear_with_layer_test(reporter, ctxInfo.directContext());

    // Use draws for clears
    GrContextOptions options(ctxInfo.options());
    options.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
    sk_gpu_test::GrContextFactory workaroundFactory(options);
    fullscreen_clear_with_layer_test(reporter, workaroundFactory.get(ctxInfo.type()));
}
