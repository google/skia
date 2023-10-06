/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/gpu/TestOps.h"

#include <memory>
#include <utility>

using namespace skia_private;

using MipmapMode = GrSamplerState::MipmapMode;
using Filter     = GrSamplerState::Filter;
using Wrap       = GrSamplerState::WrapMode;

namespace skiagm {
/**
 * This GM directly exercises GrTextureEffect::MakeTexelSubset.
 */
class TexelSubset : public GpuGM {
public:
    TexelSubset(Filter filter, MipmapMode mm, bool upscale)
            : fFilter(filter), fMipmapMode(mm), fUpscale(upscale) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override {
        SkString name("texel_subset");
        switch (fFilter) {
            case Filter::kNearest:
                name.append("_nearest");
                break;
            case Filter::kLinear:
                name.append("_linear");
                break;
        }
        switch (fMipmapMode) {
            case MipmapMode::kNone:
                break;
            case MipmapMode::kNearest:
                name.append("_mipmap_nearest");
                break;
            case MipmapMode::kLinear:
                name.append("_mipmap_linear");
                break;
        }
        name.append(fUpscale ? "_up" : "_down");
        return name;
    }

    SkISize getISize() override {
        static constexpr int kN = GrSamplerState::kWrapModeCount;
        int w = kTestPad + 2*kN*(kImageSize.width()  + 2*kDrawPad + kTestPad);
        int h = kTestPad + 2*kN*(kImageSize.height() + 2*kDrawPad + kTestPad);
        return {w, h};
    }

    void onOnceBeforeDraw() override {
        SkAssertResult(ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &fBitmap));
        // Make the bitmap non-square to detect any width/height confusion.
        fBitmap.extractSubset(&fBitmap, SkIRect::MakeSize(fBitmap.dimensions()).makeInset(0, 20));
        SkASSERT(fBitmap.dimensions() == kImageSize);
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);
        if (!sdc) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        skgpu::Mipmapped mipmapped =
                (fMipmapMode != MipmapMode::kNone) ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
        if (mipmapped == skgpu::Mipmapped::kYes && !rContext->priv().caps()->mipmapSupport()) {
            return DrawResult::kSkip;
        }
        auto view = std::get<0>(GrMakeCachedBitmapProxyView(
                rContext, fBitmap, /*label=*/"DrawResult_Draw_BitMap", mipmapped));
        if (!view) {
            *errorMsg = "Failed to create proxy.";
            return DrawResult::kFail;
        }

        SkIRect texelSubset;
        // Use a smaller subset when upscaling so that wrap is hit on all sides of the rect we
        // will draw.
        if (fUpscale) {
            texelSubset = SkIRect::MakeXYWH(fBitmap.width()/3 - 1, 2*fBitmap.height()/5 - 1,
                                            fBitmap.width()/4 + 2,   fBitmap.height()/5 + 2);
        } else {
            texelSubset = SkIRect::MakeXYWH(  fBitmap.width()/8 - 1,   fBitmap.height()/7 - 1,
                                            3*fBitmap.width()/5 + 2, 4*fBitmap.height()/5 + 2);
        }

        TArray<SkMatrix> textureMatrices;

        SkRect a = SkRect::Make(texelSubset);
        SkRect b = fUpscale ? a.makeInset (.31f * a.width(), .31f * a.height())
                            : a.makeOutset(.25f * a.width(), .25f * a.height());
        textureMatrices.push_back() = SkMatrix::RectToRect(a, b);

        b = fUpscale ? a.makeInset (.25f * a.width(), .35f * a.height())
                     : a.makeOutset(.20f * a.width(), .35f * a.height());
        textureMatrices.push_back() = SkMatrix::RectToRect(a, b);
        textureMatrices.back().preRotate(45.f, a.centerX(), a.centerY());
        textureMatrices.back().postSkew(.05f, -.05f);

        SkBitmap subsetBmp;
        fBitmap.extractSubset(&subsetBmp, texelSubset);
        subsetBmp.setImmutable();
        auto subsetView = std::get<0>(GrMakeCachedBitmapProxyView(
                rContext, subsetBmp, /*label=*/"DrawResult_Draw_SubsetBitMap", mipmapped));

        SkRect localRect = SkRect::Make(fBitmap.bounds()).makeOutset(kDrawPad, kDrawPad);

        auto size = this->getISize();

        SkScalar y = kDrawPad + kTestPad;
        SkRect drawRect;
        for (int tm = 0; tm < textureMatrices.size(); ++tm) {
            for (int my = 0; my < GrSamplerState::kWrapModeCount; ++my) {
                SkScalar x = kDrawPad + kTestPad;
                auto wmy = static_cast<Wrap>(my);
                for (int mx = 0; mx < GrSamplerState::kWrapModeCount; ++mx) {
                    auto wmx = static_cast<Wrap>(mx);

                    const auto& caps = *rContext->priv().caps();

                    GrSamplerState sampler(wmx, wmy, fFilter, fMipmapMode);

                    drawRect = localRect.makeOffset(x, y);

                    std::unique_ptr<GrFragmentProcessor> fp1;
                    fp1 = GrTextureEffect::MakeSubset(view,
                                                      fBitmap.alphaType(),
                                                      textureMatrices[tm],
                                                      sampler,
                                                      SkRect::Make(texelSubset),
                                                      caps);
                    if (!fp1) {
                        continue;
                    }

                    // Throw a translate in the local matrix just to test having something other
                    // than identity. Compensate with an offset local rect.
                    static constexpr SkVector kT = {-100, 300};
                    if (auto op = sk_gpu_test::test_ops::MakeRect(rContext,
                                                                  std::move(fp1),
                                                                  drawRect,
                                                                  localRect.makeOffset(kT),
                                                                  SkMatrix::Translate(-kT))) {
                        sdc->addDrawOp(std::move(op));
                    }

                    x += localRect.width() + kTestPad;

                    // Now draw with a subsetted proxy using fixed function texture sampling
                    // rather than a texture subset as a comparison.
                    drawRect = localRect.makeOffset(x, y);
                    SkMatrix subsetTextureMatrix = SkMatrix::Concat(
                            SkMatrix::Translate(-texelSubset.topLeft()), textureMatrices[tm]);

                    auto fp2 = GrTextureEffect::Make(subsetView,
                                                     fBitmap.alphaType(),
                                                     subsetTextureMatrix,
                                                     sampler,
                                                     caps);
                    if (auto op = sk_gpu_test::test_ops::MakeRect(rContext, std::move(fp2),
                                                                  drawRect, localRect)) {
                        sdc->addDrawOp(std::move(op));
                    }

                    if (mx < GrSamplerState::kWrapModeCount - 1) {
                        SkScalar midX =
                                SkScalarFloorToScalar(drawRect.right() + kTestPad/2.f) + 0.5f;
                        canvas->drawLine({midX, -1}, {midX, (float)size.fHeight+1}, {});
                    }
                    x += localRect.width() + kTestPad;
                }
                if (my < GrSamplerState::kWrapModeCount - 1) {
                    SkScalar midY = SkScalarFloorToScalar(drawRect.bottom() + kTestPad/2.f) + 0.5f;
                    canvas->drawLine({-1, midY}, {(float)size.fWidth+1, midY}, {});
                }
                y += localRect.height() + kTestPad;
            }
            if (tm < textureMatrices.size() - 1) {
                SkPaint paint;
                paint.setColor(SK_ColorRED);
                SkScalar midY = SkScalarFloorToScalar(drawRect.bottom() + kTestPad/2.f) + 0.5f;
                canvas->drawLine({-1, midY}, {(float)size.fWidth + 1, midY}, paint);
            }
        }
        return DrawResult::kOk;
    }

private:
    inline static constexpr SkISize kImageSize = {128, 88};
    inline static constexpr SkScalar kDrawPad = 10.f;
    inline static constexpr SkScalar kTestPad = 10.f;
    SkBitmap fBitmap;
    Filter fFilter;
    MipmapMode fMipmapMode;
    bool fUpscale;

    using INHERITED = GM;
};

DEF_GM(return new TexelSubset(Filter::kNearest, MipmapMode::kNone   , false);)
DEF_GM(return new TexelSubset(Filter::kNearest, MipmapMode::kNone   , true );)
DEF_GM(return new TexelSubset(Filter::kLinear , MipmapMode::kNone   , false);)
DEF_GM(return new TexelSubset(Filter::kLinear , MipmapMode::kNone   , true );)
// It doesn't make sense to have upscaling MIP map.
DEF_GM(return new TexelSubset(Filter::kNearest, MipmapMode::kNearest, false);)
DEF_GM(return new TexelSubset(Filter::kLinear , MipmapMode::kNearest, false);)
DEF_GM(return new TexelSubset(Filter::kNearest, MipmapMode::kLinear , false);)
DEF_GM(return new TexelSubset(Filter::kLinear , MipmapMode::kLinear , false);)

}  // namespace skiagm
