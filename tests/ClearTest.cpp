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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ClearOp, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
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
    rtContext->clear(&fullRect, kColor1, false);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check two full clears, same color
    rtContext->clear(&fullRect, kColor1, false);
    rtContext->clear(&fullRect, kColor1, false);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check two full clears, different colors
    rtContext->clear(&fullRect, kColor1, false);
    rtContext->clear(&fullRect, kColor2, false);
    if (!check_rect(rtContext.get(), fullRect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a full clear followed by a same color inset clear
    rtContext->clear(&fullRect, kColor1, false);
    rtContext->clear(&mid1Rect, kColor1, false);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a inset clear followed by same color full clear
    rtContext->clear(&mid1Rect, kColor1, false);
    rtContext->clear(&fullRect, kColor1, false);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Test a full clear followed by a different color inset clear
    rtContext->clear(&fullRect, kColor1, false);
    rtContext->clear(&mid1Rect, kColor2, false);
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
    rtContext->clear(&mid1Rect, kColor2, false);
    rtContext->clear(&fullRect, kColor1, false);
    if (!check_rect(rtContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    rtContext = newRTC(context, kW, kH);
    SkASSERT(rtContext);

    // Check three nested clears from largest to smallest where outermost and innermost are same
    // color.
    rtContext->clear(&fullRect, kColor1, false);
    rtContext->clear(&mid1Rect, kColor2, false);
    rtContext->clear(&mid2Rect, kColor1, false);
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
    rtContext->clear(&fullRect, kColor1, false);
    rtContext->clear(&mid2Rect, kColor1, false);
    rtContext->clear(&mid1Rect, kColor2, false);
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
#endif
