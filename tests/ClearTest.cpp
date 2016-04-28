/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrRenderTarget.h"
#include "GrTexture.h"
#include "GrTextureProvider.h"

static bool check_rect(GrDrawContext* dc, const SkIRect& rect, uint32_t expectedValue,
                       uint32_t* actualValue, int* failX, int* failY) {
    GrRenderTarget* rt = dc->accessRenderTarget();
    int w = rect.width();
    int h = rect.height();
    SkAutoTDeleteArray<uint32_t> pixels(new uint32_t[w * h]);
    memset(pixels.get(), ~expectedValue, sizeof(uint32_t) * w * h);
    rt->readPixels(rect.fLeft, rect.fTop, w, h, kRGBA_8888_GrPixelConfig, pixels.get());
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

// We only really need the DC, but currently the DC doesn't own the RT so we also ref it, but that
// could be dropped when DC is a proper owner of its RT.
static bool reset_dc(sk_sp<GrDrawContext>* dc, SkAutoTUnref<GrSurface>* rtKeepAlive,
                     GrContext* context, int w, int h) {
    SkDEBUGCODE(uint32_t oldID = 0;)
    if (*dc) {
        SkDEBUGCODE(oldID = (*dc)->accessRenderTarget()->getUniqueID();)
        rtKeepAlive->reset(nullptr);
        dc->reset(nullptr);
    }
    context->freeGpuResources();

    GrTextureDesc desc;
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;

    rtKeepAlive->reset(context->textureProvider()->createTexture(desc, SkBudgeted::kYes));
    if (!(*rtKeepAlive)) {
        return false;
    }
    GrRenderTarget* rt = (*rtKeepAlive)->asRenderTarget();
    SkASSERT(rt->getUniqueID() != oldID);
    *dc = context->drawContext(sk_ref_sp(rt));
    return *dc != nullptr;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ClearBatch, reporter, ctxInfo) {
    GrContext* context = ctxInfo.fGrContext;
    static const int kW = 10;
    static const int kH = 10;

    SkIRect fullRect = SkIRect::MakeWH(kW, kH);
    sk_sp<GrDrawContext> drawContext;
    SkAutoTUnref<GrSurface> rtKeepAlive;

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

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Check a full clear
    drawContext->clear(&fullRect, kColor1, false);
    if (!check_rect(drawContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Check two full clears, same color
    drawContext->clear(&fullRect, kColor1, false);
    drawContext->clear(&fullRect, kColor1, false);
    if (!check_rect(drawContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Check two full clears, different colors
    drawContext->clear(&fullRect, kColor1, false);
    drawContext->clear(&fullRect, kColor2, false);
    if (!check_rect(drawContext.get(), fullRect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Test a full clear followed by a same color inset clear
    drawContext->clear(&fullRect, kColor1, false);
    drawContext->clear(&mid1Rect, kColor1, false);
    if (!check_rect(drawContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Test a inset clear followed by same color full clear
    drawContext->clear(&mid1Rect, kColor1, false);
    drawContext->clear(&fullRect, kColor1, false);
    if (!check_rect(drawContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Test a full clear followed by a different color inset clear
    drawContext->clear(&fullRect, kColor1, false);
    drawContext->clear(&mid1Rect, kColor2, false);
    if (!check_rect(drawContext.get(), mid1Rect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(drawContext.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Test a inset clear followed by a different full clear
    drawContext->clear(&mid1Rect, kColor2, false);
    drawContext->clear(&fullRect, kColor1, false);
    if (!check_rect(drawContext.get(), fullRect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Check three nested clears from largest to smallest where outermost and innermost are same
    // color.
    drawContext->clear(&fullRect, kColor1, false);
    drawContext->clear(&mid1Rect, kColor2, false);
    drawContext->clear(&mid2Rect, kColor1, false);
    if (!check_rect(drawContext.get(), mid2Rect, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }
    if (!check_rect(drawContext.get(), innerLeftEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), innerTopEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), innerRightEdge, kColor2, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), innerBottomEdge, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(drawContext.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }

    if (!reset_dc(&drawContext, &rtKeepAlive, context, kW, kH)) {
        ERRORF(reporter, "Could not create draw context.");
        return;
    }
    // Swap the order of the second two clears in the above test.
    drawContext->clear(&fullRect, kColor1, false);
    drawContext->clear(&mid2Rect, kColor1, false);
    drawContext->clear(&mid1Rect, kColor2, false);
    if (!check_rect(drawContext.get(), mid1Rect, kColor2, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor2, actualValue,
               failX, failY);
    }
    if (!check_rect(drawContext.get(), outerLeftEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerTopEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerRightEdge, kColor1, &actualValue, &failX, &failY) ||
        !check_rect(drawContext.get(), outerBottomEdge, kColor1, &actualValue, &failX, &failY)) {
        ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d).", kColor1, actualValue,
               failX, failY);
    }
}
#endif
