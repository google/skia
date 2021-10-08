/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GrDirectContext.h"

#include "src/core/SkCanvasPriv.h"
#include "src/core/SkConvertPixels.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

#include "tools/gpu/ProxyUtils.h"

static GrSurfaceProxyView create_view(GrDirectContext* dContext,
                                      const SkBitmap& src,
                                      GrSurfaceOrigin origin) {
    SkASSERT(src.colorType() == kRGBA_8888_SkColorType);

#define USE_LAZY_PROXIES 1 // Toggle this to generate the reference images

    if (USE_LAZY_PROXIES) {
        auto format = dContext->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                       GrRenderable::kNo);
        if (!format.isValid()) {
            return {};
        }

        sk_sp<GrTextureProxy> proxy = GrProxyProvider::MakeFullyLazyProxy(
                [src](GrResourceProvider* rp,
                      const GrSurfaceProxy::LazySurfaceDesc& desc)
                            -> GrSurfaceProxy::LazyCallbackResult {
                    SkASSERT(desc.fMipmapped == GrMipmapped::kNo);
                    GrMipLevel mipLevel = {src.getPixels(), src.rowBytes(), nullptr};
                    auto colorType = SkColorTypeToGrColorType(src.colorType());

                    return rp->createTexture(src.dimensions(),
                                             desc.fFormat,
                                             desc.fTextureType,
                                             colorType,
                                             desc.fRenderable,
                                             desc.fSampleCnt,
                                             desc.fBudgeted,
                                             desc.fFit,
                                             desc.fProtected,
                                             mipLevel);
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

    return sk_gpu_test::MakeTextureProxyViewFromData(dContext,
                                                     GrRenderable::kNo,
                                                     origin,
                                                     src.pixmap());
}

// Create an over large texture which is initialized to opaque black outside of the content
// rect. The inside of the content rect consists of a grey coordinate frame lacking the -Y axis.
// The -X and +X axes have a red and green dot at their ends (respectively). Finally, the content
// rect has a 1-pixel wide blue border.
static SkBitmap create_bitmap(SkIRect contentRect, SkISize fullSize, GrSurfaceOrigin origin) {

    const int kContentSize = contentRect.width();
    SkBitmap contentBM;

    {
        SkImageInfo contentInfo = SkImageInfo::Make(kContentSize, kContentSize,
                                                    kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        contentBM.allocPixels(contentInfo);

        contentBM.eraseColor(SK_ColorWHITE);

        const int halfM1 = kContentSize/2 - 1;

        // The coordinate frame
        contentBM.eraseArea(SkIRect::MakeXYWH(halfM1, 2,2, halfM1), SK_ColorGRAY);
        contentBM.eraseArea(SkIRect::MakeXYWH(2, halfM1, kContentSize-4, 2), SK_ColorGRAY);

        contentBM.eraseArea(SkIRect::MakeXYWH(2, halfM1, 2, 2), SK_ColorRED);
        contentBM.eraseArea(SkIRect::MakeXYWH(kContentSize-4, halfM1, 2, 2), SK_ColorGREEN);

        // The 1-pixel wide rim around the content rect
        contentBM.eraseArea(SkIRect::MakeXYWH(0, 0, kContentSize, 1), SK_ColorBLUE);
        contentBM.eraseArea(SkIRect::MakeXYWH(0, 0, 1, kContentSize), SK_ColorBLUE);
        contentBM.eraseArea(SkIRect::MakeXYWH(kContentSize-1, 0, 1, kContentSize), SK_ColorBLUE);
        contentBM.eraseArea(SkIRect::MakeXYWH(0, kContentSize-1, kContentSize, 1), SK_ColorBLUE);
    }

    SkBitmap bigBM;

    {
        const int kLeft = contentRect.fLeft;
        const int kTop  = contentRect.fTop;

        SkImageInfo bigInfo = SkImageInfo::Make(fullSize.fWidth, fullSize.fHeight,
                                                kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

        bigBM.allocPixels(bigInfo);

        bigBM.eraseColor(SK_ColorBLACK);

        const char* src = static_cast<const char*>(contentBM.getPixels());
        size_t srcRB = contentBM.rowBytes();
        size_t dstRB = bigBM.rowBytes();

        if (USE_LAZY_PROXIES && origin == kBottomLeft_GrSurfaceOrigin) {
            char* dst = static_cast<char*>(bigBM.getAddr(kLeft, fullSize.height() - kTop - 1));
            for (int y = 0; y < contentBM.height(); ++y) {
                memcpy(dst, src, srcRB);
                src = src + srcRB;
                dst = dst - dstRB;
            }
        } else {
            char* dst = static_cast<char*>(bigBM.getAddr(kLeft, kTop));
            SkRectMemcpy(dst, dstRB, src, srcRB,
                         contentBM.rowBytes(), contentBM.height());
        }

        bigBM.setAlphaType(kOpaque_SkAlphaType);
        bigBM.setImmutable();
    }

    return bigBM;
}

static void draw_texture(const GrCaps* caps,
                         skgpu::v1::SurfaceDrawContext* sdc,
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
// TODO: should we also test w/ mipmapping?
class LazyTilingGM : public GpuGM {
public:
    LazyTilingGM(GrSurfaceOrigin origin)
            : fOrigin(origin)
            , fContentRect(SkIRect::MakeXYWH(kLeftContentOffset, kTopContentOffset,
                                             kContentSize, kContentSize)) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkStringPrintf("lazytiling_%s", fOrigin == kTopLeft_GrSurfaceOrigin ? "tl" : "bl");
    }

    SkISize onISize() override {
        return SkISize::Make(kTotalWidth, kTotalHeight);
    }

    DrawResult onGpuSetup(GrDirectContext* dContext, SkString* errorMsg) override {
        if (!dContext || dContext->abandoned()) {
            return DrawResult::kSkip;
        }

        auto bm = create_bitmap(fContentRect,
                                { kLeftContentOffset + kContentSize + kRightContentPadding,
                                  kTopContentOffset  + kContentSize + kBottomContentPadding },
                                fOrigin);

        fView = create_view(dContext, bm, fOrigin);
        if (!fView.proxy()) {
            *errorMsg = "Failed to create proxy";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        SkSamplingOptions sampling(SkFilterMode::kNearest, SkMipmapMode::kNone);
        SkPaint p;

        auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
        if (!sdc) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        int y = kPad;
        for (auto yMode : { SkTileMode::kClamp, SkTileMode::kRepeat,
                            SkTileMode::kMirror, SkTileMode::kDecal }) {
            int x = kPad;
            for (auto xMode : { SkTileMode::kClamp, SkTileMode::kRepeat,
                                SkTileMode::kMirror, SkTileMode::kDecal }) {
                SkIRect cellRect = SkIRect::MakeXYWH(x, y, 2*kContentSize, 2*kContentSize);
                SkRect contentRect = SkRect::MakeXYWH(x+kContentSize/2, y+kContentSize/2,
                                                      kContentSize, kContentSize);

                SkMatrix texMatrix = SkMatrix::RectToRect(contentRect, SkRect::Make(fContentRect));

                draw_texture(rContext->priv().caps(),
                             sdc,
                             fView,
                             fContentRect,
                             cellRect,
                             texMatrix,
                             SkTileModeToWrapMode(xMode),
                             SkTileModeToWrapMode(yMode));

                x += 2*kContentSize+kPad;
            }

            y += 2*kContentSize+kPad;
        }

        return DrawResult::kOk;
    }

private:
    inline static constexpr int kLeftContentOffset = 8;
    inline static constexpr int kTopContentOffset = 16;
    inline static constexpr int kRightContentPadding = 24;
    inline static constexpr int kBottomContentPadding = 80;

    inline static constexpr int kPad = 4; // on-screen padding between cells

    inline static constexpr int kContentSize = 32;

    // Each cell in this GM's grid is a square - 2*kContentSize on a side
    inline static constexpr int kTotalWidth = (2*kContentSize+kPad) * kSkTileModeCount + kPad;
    inline static constexpr int kTotalHeight = (2*kContentSize+kPad) * kSkTileModeCount + kPad;

    GrSurfaceOrigin    fOrigin;
    SkIRect            fContentRect;
    GrSurfaceProxyView fView;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LazyTilingGM(kTopLeft_GrSurfaceOrigin);)
DEF_GM(return new LazyTilingGM(kBottomLeft_GrSurfaceOrigin);)

}  // namespace skiagm
