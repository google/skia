/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkClipStack.h"
#include "SkRRect.h"
#include "SkTextUtils.h"

#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrContextPriv.h"
#include "GrReducedClip.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "GrStencilClip.h"
#include "effects/GrTextureDomain.h"

constexpr static SkIRect kDeviceRect = {0, 0, 600, 600};
constexpr static SkIRect kCoverRect = {50, 50, 550, 550};

namespace skiagm {

////////////////////////////////////////////////////////////////////////////////////////////////////

class WindowRectanglesBaseGM : public GM {
protected:
    virtual DrawResult onCoverClipStack(const SkClipStack&, SkCanvas*, SkString* errorMsg) = 0;

private:
    SkISize onISize() override { return SkISize::Make(kDeviceRect.width(), kDeviceRect.height()); }
    DrawResult onDraw(SkCanvas*, SkString* errorMsg) final;
};

DrawResult WindowRectanglesBaseGM::onDraw(SkCanvas* canvas, SkString* errorMsg) {
    sk_tool_utils::draw_checkerboard(canvas, 0xffffffff, 0xffc6c3c6, 25);

    SkClipStack stack;
    stack.clipRect(SkRect::MakeXYWH(370.75, 80.25, 149, 100), SkMatrix::I(),
                   kDifference_SkClipOp, false);
    stack.clipRect(SkRect::MakeXYWH(80.25, 420.75, 150, 100), SkMatrix::I(),
                   kDifference_SkClipOp, true);
    stack.clipRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(200, 200, 200, 200), 60, 45),
                    SkMatrix::I(), kDifference_SkClipOp, true);

    SkRRect nine;
    nine.setNinePatch(SkRect::MakeXYWH(550 - 30.25 - 100, 370.75, 100, 150), 12, 35, 23, 20);
    stack.clipRRect(nine, SkMatrix::I(), kDifference_SkClipOp, true);

    SkRRect complx;
    SkVector complxRadii[4] = {{6, 4}, {8, 12}, {16, 24}, {48, 32}};
    complx.setRectRadii(SkRect::MakeXYWH(80.25, 80.75, 100, 149), complxRadii);
    stack.clipRRect(complx, SkMatrix::I(), kDifference_SkClipOp, false);

    return this->onCoverClipStack(stack, canvas, errorMsg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Draws a clip that will exercise window rectangles if they are supported.
 */
class WindowRectanglesGM : public WindowRectanglesBaseGM {
private:
    SkString onShortName() final { return SkString("windowrectangles"); }
    DrawResult onCoverClipStack(const SkClipStack&, SkCanvas*, SkString* errorMsg) final;
};

DrawResult WindowRectanglesGM::onCoverClipStack(const SkClipStack& stack, SkCanvas* canvas,
                                                SkString* errorMsg) {
    SkPaint paint;
    paint.setColor(0xff00aa80);

    // Set up the canvas's clip to match our SkClipStack.
    SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);
    for (const SkClipStack::Element* element = iter.next(); element; element = iter.next()) {
        SkClipOp op = element->getOp();
        bool isAA = element->isAA();
        switch (element->getDeviceSpaceType()) {
            case SkClipStack::Element::DeviceSpaceType::kPath:
                canvas->clipPath(element->getDeviceSpacePath(), op, isAA);
                break;
            case SkClipStack::Element::DeviceSpaceType::kRRect:
                canvas->clipRRect(element->getDeviceSpaceRRect(), op, isAA);
                break;
            case SkClipStack::Element::DeviceSpaceType::kRect:
                canvas->clipRect(element->getDeviceSpaceRect(), op, isAA);
                break;
            case SkClipStack::Element::DeviceSpaceType::kEmpty:
                canvas->clipRect({ 0, 0, 0, 0 }, kIntersect_SkClipOp, false);
                break;
        }
    }

    canvas->drawRect(SkRect::Make(kCoverRect), paint);
    return DrawResult::kOk;
}

DEF_GM( return new WindowRectanglesGM(); )

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr static int kNumWindows = 8;

/**
 * Visualizes the mask (alpha or stencil) for a clip with several window rectangles. The purpose of
 * this test is to verify that window rectangles are being used during clip mask generation, and to
 * visualize where the window rectangles are placed.
 *
 * We use window rectangles when generating the clip mask because there is no need to invest time
 * defining those regions where window rectangles will be in effect during the actual draw anyway.
 *
 * This test works by filling the entire clip mask with a small checkerboard pattern before drawing
 * it, and then covering the mask with a solid color once it has been generated. The regions inside
 * window rectangles or outside the scissor should still have the initial checkerboard intact.
 */
class WindowRectanglesMaskGM : public WindowRectanglesBaseGM {
private:
    constexpr static int kMaskCheckerSize = 5;
    SkString onShortName() final { return SkString("windowrectangles_mask"); }
    DrawResult onCoverClipStack(const SkClipStack&, SkCanvas*, SkString* errorMsg) final;
    void visualizeAlphaMask(GrContext*, GrRenderTargetContext*, const GrReducedClip&, GrPaint&&);
    void visualizeStencilMask(GrContext*, GrRenderTargetContext*, const GrReducedClip&, GrPaint&&);
    void stencilCheckerboard(GrRenderTargetContext*, bool flip);
};

/**
 * Base class for GrClips that visualize a clip mask.
 */
class MaskOnlyClipBase : public GrClip {
private:
    bool quickContains(const SkRect&) const final { return false; }
    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA*) const final { return false; }
    void getConservativeBounds(int width, int height, SkIRect* rect, bool* iior) const final {
        rect->set(0, 0, width, height);
        if (iior) {
            *iior = false;
        }
    }
};

/**
 * This class clips a cover by an alpha mask. We use it to visualize the alpha clip mask.
 */
class AlphaOnlyClip final : public MaskOnlyClipBase {
public:
    AlphaOnlyClip(sk_sp<GrTextureProxy> mask, int x, int y) : fMask(mask), fX(x), fY(y) {}

private:
    bool apply(GrRecordingContext*, GrRenderTargetContext*, bool, bool, GrAppliedClip* out,
               SkRect* bounds) const override {
        int w = fMask->width();
        int h = fMask->height();
        out->addCoverageFP(GrDeviceSpaceTextureDecalFragmentProcessor::Make(
                fMask, SkIRect::MakeWH(w, h), {fX, fY}));
        return true;
    }
    sk_sp<GrTextureProxy> fMask;
    int fX;
    int fY;
};

/**
 * Makes a clip object that enforces the stencil clip bit. Used to visualize the stencil mask.
 */
static GrStencilClip make_stencil_only_clip() {
    return GrStencilClip(SkClipStack::kEmptyGenID);
};

DrawResult WindowRectanglesMaskGM::onCoverClipStack(const SkClipStack& stack, SkCanvas* canvas,
                                                        SkString* errorMsg) {
    GrContext* ctx = canvas->getGrContext();
    GrRenderTargetContext* rtc = canvas->internal_private_accessTopLayerRenderTargetContext();
    if (!ctx || !rtc) {
        *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }
    if (rtc->priv().maxWindowRectangles() < kNumWindows) {
        *errorMsg = "Requires at least 8 window rectangles. "
                    "(Are you off FBO 0? Use sRGB to force offscreen rendering.)";
        return DrawResult::kSkip;
    }

    const GrReducedClip reducedClip(stack, SkRect::Make(kCoverRect), rtc->caps(), kNumWindows);

    GrPaint paint;
    if (GrFSAAType::kNone == rtc->fsaaType()) {
        paint.setColor4f({ 0, 0.25f, 1, 1 });
        this->visualizeAlphaMask(ctx, rtc, reducedClip, std::move(paint));
    } else {
        paint.setColor4f({ 1, 0.25f, 0.25f, 1 });
        this->visualizeStencilMask(ctx, rtc, reducedClip, std::move(paint));
    }
    return DrawResult::kOk;
}

void WindowRectanglesMaskGM::visualizeAlphaMask(GrContext* ctx, GrRenderTargetContext* rtc,
                                                const GrReducedClip& reducedClip, GrPaint&& paint) {
    const int padRight = (kDeviceRect.right() - kCoverRect.right()) / 2;
    const int padBottom = (kDeviceRect.bottom() - kCoverRect.bottom()) / 2;
    const GrBackendFormat format =
            ctx->priv().caps()->getBackendFormatFromColorType(kAlpha_8_SkColorType);
    sk_sp<GrRenderTargetContext> maskRTC(
        ctx->priv().makeDeferredRenderTargetContextWithFallback(
                                                         format, SkBackingFit::kExact,
                                                         kCoverRect.width() + padRight,
                                                         kCoverRect.height() + padBottom,
                                                         kAlpha_8_GrPixelConfig, nullptr));
    if (!maskRTC) {
        return;
    }

    // Draw a checker pattern into the alpha mask so we can visualize the regions left untouched by
    // the clip mask generation.
    this->stencilCheckerboard(maskRTC.get(), true);
    maskRTC->clear(nullptr, SK_PMColor4fWHITE, GrRenderTargetContext::CanClearFullscreen::kYes);
    maskRTC->priv().drawAndStencilRect(make_stencil_only_clip(), &GrUserStencilSettings::kUnused,
                                       SkRegion::kDifference_Op, false, GrAA::kNo, SkMatrix::I(),
                                       SkRect::MakeIWH(maskRTC->width(), maskRTC->height()));
    reducedClip.drawAlphaClipMask(maskRTC.get());

    int x = kCoverRect.x() - kDeviceRect.x(),
        y = kCoverRect.y() - kDeviceRect.y();

    // Now visualize the alpha mask by drawing a rect over the area where it is defined. The regions
    // inside window rectangles or outside the scissor should still have the initial checkerboard
    // intact. (This verifies we didn't spend any time modifying those pixels in the mask.)
    AlphaOnlyClip clip(maskRTC->asTextureProxyRef(), x, y);
    rtc->drawRect(clip, std::move(paint), GrAA::kYes, SkMatrix::I(),
                  SkRect::Make(SkIRect::MakeXYWH(x, y, maskRTC->width(), maskRTC->height())));
}

void WindowRectanglesMaskGM::visualizeStencilMask(GrContext* ctx, GrRenderTargetContext* rtc,
                                                  const GrReducedClip& reducedClip,
                                                  GrPaint&& paint) {
    // Draw a checker pattern into the stencil buffer so we can visualize the regions left untouched
    // by the clip mask generation.
    this->stencilCheckerboard(rtc, false);
    reducedClip.drawStencilClipMask(ctx, rtc);

    // Now visualize the stencil mask by covering the entire render target. The regions inside
    // window rectangles or outside the scissor should still have the initial checkerboard intact.
    // (This verifies we didn't spend any time modifying those pixels in the mask.)
    rtc->drawPaint(make_stencil_only_clip(), std::move(paint), SkMatrix::I());
}

void WindowRectanglesMaskGM::stencilCheckerboard(GrRenderTargetContext* rtc, bool flip) {
    constexpr static GrUserStencilSettings kSetClip(
        GrUserStencilSettings::StaticInit<
        0,
        GrUserStencilTest::kAlways,
        0,
        GrUserStencilOp::kSetClipBit,
        GrUserStencilOp::kKeep,
        0>()
    );

    rtc->priv().clearStencilClip(GrFixedClip::Disabled(), false);

    for (int y = 0; y < kDeviceRect.height(); y += kMaskCheckerSize) {
        for (int x = (y & 1) == flip ? 0 : kMaskCheckerSize;
             x < kDeviceRect.width(); x += 2 * kMaskCheckerSize) {
            SkIRect checker = SkIRect::MakeXYWH(x, y, kMaskCheckerSize, kMaskCheckerSize);
            rtc->priv().stencilRect(
                    GrNoClip(), &kSetClip, GrAA::kNo, SkMatrix::I(), SkRect::Make(checker));
        }
    }
}

DEF_GM( return new WindowRectanglesMaskGM(); )

}
