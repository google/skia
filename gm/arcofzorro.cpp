/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GrDirectContext.h"
#include "include/utils/SkRandom.h"

#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/effects/GrTextureEffect.h"

#if 1
static GrSurfaceProxyView create_view(GrDirectContext* dContext,
                                      const SkBitmap& src,
                                      GrSurfaceOrigin origin) {
    SkASSERT(src.colorType() == kRGBA_8888_SkColorType);

    GrBackendFormat format = dContext->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                              GrRenderable::kNo);
    if (!format.isValid()) {
        return {};
    }

    sk_sp<GrTextureProxy> proxy = GrProxyProvider::MakeFullyLazyProxy(
            [src](GrResourceProvider* rp,
                  const GrSurfaceProxy::LazySurfaceDesc& desc)
                        -> GrSurfaceProxy::LazyCallbackResult {
                SkASSERT(desc.fMipmapped == GrMipmapped::kNo);
                GrMipLevel mipLevel = { src.getPixels(), src.rowBytes() };
                auto colorType = SkColorTypeToGrColorType(src.colorType());

                return rp->createTexture(src.dimensions(), desc.fFormat, colorType,
                                         desc.fRenderable, desc.fSampleCnt, desc.fBudgeted,
                                         desc.fFit, desc.fProtected, mipLevel);
            },
            format, GrRenderable::kNo, 1, GrProtected::kNo, *dContext->priv().caps(),
            GrSurfaceProxy::UseAllocator::kYes);

    if (!proxy) {
        return {};
    }

    auto swizzle = dContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                           GrColorType::kRGBA_8888);
    return GrSurfaceProxyView(std::move(proxy), origin, swizzle);
}

#else
static GrSurfaceProxyView create_view(GrDirectContext* dContext,
                                      const SkBitmap& src,
                                      GrSurfaceOrigin origin) {
    SkASSERT(src.colorType() == kRGBA_8888_SkColorType);

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    // TODO: should we also test w/ mipmapping?
    auto proxy = proxyProvider->createProxyFromBitmap(src,
                                                      GrMipmapped::kNo,
                                                      SkBackingFit::kExact,
                                                      SkBudgeted::kYes);
    if (!proxy) {
        return {};
    }

    auto swizzle = dContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                           GrColorType::kRGBA_8888);
    return GrSurfaceProxyView(std::move(proxy), origin, swizzle);
}
#endif

// Create an over large texture which is initialized to opaque black outside of the content
// rect. The content rect consists of a grey coordinate frame lacking the -Y axis. The
// -X and +X axes have a red and green dot at their ends (respectively). Finally, the content
// rect has a 1-pixel wide blue border.
static SkBitmap create_bitmap(SkIRect contentRect, SkISize fullSize) {

    const int kContentSize = contentRect.width();
    const int kLeft = contentRect.fLeft;
    const int kTop  = contentRect.fTop;

    SkImageInfo info = SkImageInfo::Make(fullSize.fWidth, fullSize.fHeight,
                                         kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(info);

    bm.eraseColor(SK_ColorBLACK);

    bm.eraseArea(contentRect, SK_ColorWHITE);

    const int halfM1 = kContentSize/2 - 1;

    // The coordinate frame
    bm.eraseArea(SkIRect::MakeXYWH(kLeft + halfM1,         kTop + 2,      2,              halfM1),
                 SK_ColorGRAY);
    bm.eraseArea(SkIRect::MakeXYWH(kLeft + 2,              kTop + halfM1, kContentSize-4, 2),
                 SK_ColorGRAY);
    bm.eraseArea(SkIRect::MakeXYWH(kLeft + 2,              kTop + halfM1, 2,              2),
                 SK_ColorRED);
    bm.eraseArea(SkIRect::MakeXYWH(kLeft + kContentSize-4, kTop + halfM1, 2,              2),
                 SK_ColorGREEN);

    // The 1-pixel wide rim around the content rect
    bm.eraseArea(SkIRect::MakeXYWH(kLeft, kTop, kContentSize, 1), SK_ColorBLUE);
    bm.eraseArea(SkIRect::MakeXYWH(kLeft, kTop, 1, kContentSize), SK_ColorBLUE);
    bm.eraseArea(SkIRect::MakeXYWH(kLeft+kContentSize-1, kTop, 1, kContentSize), SK_ColorBLUE);
    bm.eraseArea(SkIRect::MakeXYWH(kLeft, kTop+kContentSize-1, kContentSize, 1), SK_ColorBLUE);

    bm.setAlphaType(kOpaque_SkAlphaType);
    bm.setImmutable();

    return bm;
}

static void draw_texture(const GrCaps* caps,
                         GrSurfaceDrawContext* sdc,
                         const GrSurfaceProxyView& src,
                         const SkIRect& srcRect,
                         const SkIRect& drawRect,
                         const SkMatrix& mat,
                         GrSamplerState::WrapMode xTileMode,
                         GrSamplerState::WrapMode yTileMode) {
    GrSamplerState sampler(xTileMode, yTileMode, SkFilterMode::kNearest);

    auto fp = GrTextureEffect::MakeSubset(src, kOpaque_SkAlphaType, mat,
                                          sampler, SkRect::Make(srcRect), *caps);
    GrPaint paint;
    paint.setColorFragmentProcessor(std::move(fp));

    sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(), SkRect::Make(drawRect));
}

namespace skiagm {

// This GM exercises all the different tile modes for a texture that cannot be normalized
// early (i.e., rectangle or fully-lazy).
class LazyTilingGM : public GpuGM {
public:
    LazyTilingGM()
            : fTopLeftContentRect(SkIRect::MakeXYWH(kLeftContentOffset, kTopContentOffset, kContentSize, kContentSize)) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("lazytiling");
    }

    SkISize onISize() override {
        return SkISize::Make(kPanelWidth, kPanelHeight);
    }

    DrawResult onGpuSetup(GrDirectContext* dContext, SkString* errorMsg) override {
        if (!dContext || dContext->abandoned()) {
            return DrawResult::kSkip;
        }

        fTopLeft = create_bitmap(fTopLeftContentRect,
                                 { kLeftContentOffset + kContentSize + kRightContentPadding,
                                   kTopContentOffset  + kContentSize + kBottomContentPadding });

#if 0
        GrBitmapTextureMaker maker(dContext, fTopLeft, GrImageTexGenPolicy::kDraw);

        fView = maker.view(GrMipmapped::kNo);
#else
        fView = create_view(dContext, fTopLeft, kTopLeft_GrSurfaceOrigin);
#endif
        if (!fView.proxy()) {
            *errorMsg = "Failed to create proxy";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onOnceBeforeDraw() override {
    }

    void onDraw(GrRecordingContext* rContext, GrSurfaceDrawContext* sdc,
                SkCanvas* canvas) override {
        SkSamplingOptions sampling(SkFilterMode::kNearest, SkMipmapMode::kNone);
        SkPaint p;

        int y = kPad;
        for (auto yMode : { SkTileMode::kClamp, SkTileMode::kRepeat,
                            SkTileMode::kMirror, SkTileMode::kDecal }) {
            int x = kPad;
            for (auto xMode : { SkTileMode::kClamp, SkTileMode::kRepeat,
                                SkTileMode::kMirror, SkTileMode::kDecal }) {
                SkIRect cellRect = SkIRect::MakeXYWH(x, y, 2*kContentSize, 2*kContentSize);
                SkRect contentRect = SkRect::MakeXYWH(x+kContentSize/2, y+kContentSize/2,
                                                      kContentSize, kContentSize);

                SkMatrix texMatrix = SkMatrix::MakeRectToRect(contentRect,
                                                              SkRect::Make(fTopLeftContentRect),
                                                              SkMatrix::ScaleToFit::kFill_ScaleToFit);

                draw_texture(rContext->priv().caps(),
                             sdc,
                             fView,
                             fTopLeftContentRect,
                             cellRect,
                             texMatrix,
                             SkTileModeToWrapMode(xMode),
                             SkTileModeToWrapMode(yMode));

                x += 2*kContentSize+kPad;
            }

            y += 2*kContentSize+kPad;
        }

    }

private:
    static constexpr int kLeftContentOffset = 8;
    static constexpr int kTopContentOffset = 16;
    static constexpr int kRightContentPadding = 24;
    static constexpr int kBottomContentPadding = 80;

    static constexpr int kPad = 4; // on-screen padding between cells

    static constexpr int kContentSize = 32;

    static constexpr int kPanelWidth = (2*kContentSize+kPad) * kSkTileModeCount + kPad;
    static constexpr int kPanelHeight = (2*kContentSize+kPad) * kSkTileModeCount + kPad;


    SkIRect            fTopLeftContentRect;
    SkBitmap           fTopLeft;

    GrSurfaceProxyView fView;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LazyTilingGM;)

}  // namespace skiagm
