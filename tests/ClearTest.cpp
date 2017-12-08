/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrRenderTargetContext.h"

#include "SkCanvas.h"
#include "SkSurface.h"

static bool check_rect(GrRenderTargetContext* rtc, const SkIRect& rect, uint32_t expectedValue,
                       uint32_t* actualValue, int* failX, int* failY) {
    int w = rect.width();
    int h = rect.height();
    std::unique_ptr<uint32_t[]> pixels(new uint32_t[w * h]);
    memset(pixels.get(), ~expectedValue, sizeof(uint32_t) * w * h);

    SkImageInfo dstInfo = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    if (!rtc->readPixels(dstInfo, pixels.get(), 0, rect.fLeft, rect.fTop)) {
        return false;
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t pixel = pixels.get()[y * w + x];
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

sk_sp<GrRenderTargetContext> newRTC(GrContext* context, int w, int h) {
    return context->makeDeferredRenderTargetContext(SkBackingFit::kExact, w, h,
                                                    kRGBA_8888_GrPixelConfig, nullptr);
}

static void clear_op_test(skiatest::Reporter* reporter, GrContext* context) {
    static const int kW = 10;
    static const int kH = 10;

    SkIRect fullRect = SkIRect::MakeWH(kW, kH);
    sk_sp<GrRenderTargetContext> rtContext;

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

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check a full clear
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check two full clears, same color
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check two full clears, different colors
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&fullRect, kColor2, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), fullRect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a full clear followed by a same color inset clear
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&mid1Rect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a inset clear followed by same color full clear
    rtContext->clear(&mid1Rect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a full clear followed by a different color inset clear
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&mid1Rect, kColor2, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), mid1Rect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(rtContext.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a inset clear followed by a different full clear
    rtContext->clear(&mid1Rect, kColor2, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check three nested clears from largest to smallest where outermost and innermost are same
    // color.
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&mid1Rect, kColor2, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&mid2Rect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), mid2Rect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }
    if (!check_rect(rtContext.get(), innerLeftEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), innerTopEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), innerRightEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), innerBottomEdge, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(rtContext.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Swap the order of the second two clears in the above test.
    rtContext->clear(&fullRect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&mid2Rect, kColor1, GrRenderTargetContext::CanClearFullscreen::kNo);
    rtContext->clear(&mid1Rect, kColor2, GrRenderTargetContext::CanClearFullscreen::kNo);
    if (!check_rect(rtContext.get(), mid1Rect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(rtContext.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(rtContext.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ClearOp, reporter, ctxInfo) {
    clear_op_test(reporter, ctxInfo.grContext());
    if (ctxInfo.backend() == kOpenGL_GrBackend) {
        GrContextOptions options(ctxInfo.options());
        options.fUseDrawInsteadOfGLClear = GrContextOptions::Enable::kYes;
        sk_gpu_test::GrContextFactory workaroundFactory(options);
        clear_op_test(reporter, workaroundFactory.get(ctxInfo.type()));
    }
}

void fullscreen_clear_with_layer_test(skiatest::Reporter* reporter, GrContext* context) {
    const SkImageInfo ii = SkImageInfo::Make(400, 77, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii);
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
    fullscreen_clear_with_layer_test(reporter, ctxInfo.grContext());
    if (ctxInfo.backend() == kOpenGL_GrBackend) {
        GrContextOptions options(ctxInfo.options());
        options.fUseDrawInsteadOfGLClear = GrContextOptions::Enable::kYes;
        sk_gpu_test::GrContextFactory workaroundFactory(options);
        fullscreen_clear_with_layer_test(reporter, workaroundFactory.get(ctxInfo.type()));
    }
}

#endif
