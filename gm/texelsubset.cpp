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
#include "include/effects/SkGradientShader.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrTextureProxy.h"
#include "tools/Resources.h"
#include "tools/gpu/TestOps.h"

#include <memory>
#include <utility>

namespace skiagm {
/**
 * This GM directly exercises GrTextureEffect::MakeTexelSubset.
 */
class TexelSubset : public GpuGM {
public:
    TexelSubset(GrSamplerState::Filter filter) : fFilter(filter) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        SkString name("texel_subset");
        if (fFilter == GrSamplerState::Filter::kBilerp) {
            name.append("_bilerp");
        } else if (fFilter == GrSamplerState::Filter::kMipMap) {
            name.append("_mipmap");
        }
        return name;
    }

    SkISize onISize() override {
        int n = GrSamplerState::kWrapModeCount;
        if (fFilter != GrSamplerState::Filter::kNearest) {
            // Account for not supporting kMirror or kMirrorRepeat with filtering.
            n -= 2;
        }
        const SkScalar w = kTestPad + 2 * n * (fBitmap.width()  + 2 * kDrawPad + kTestPad);
        const SkScalar h = kTestPad + 2 * n * (fBitmap.height() + 2 * kDrawPad + kTestPad);
        return SkISize::Make(SkScalarCeilToInt(w), SkScalarCeilToInt(h));
    }

    void onOnceBeforeDraw() override {
        GetResourceAsBitmap("images/mandrill_128.png", &fBitmap);
        // Make the bitmap non-square to detect any width/height confusion.
        fBitmap.extractSubset(&fBitmap, SkIRect::MakeSize(fBitmap.dimensions()).makeInset(0, 20));
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrMipMapped mipMapped = fFilter == GrSamplerState::Filter::kMipMap &&
                                context->priv().caps()->mipMapSupport()
                ? GrMipMapped::kYes : GrMipMapped::kNo;
        GrBitmapTextureMaker maker(context, fBitmap);
        auto [proxy, grCT] = maker.refTextureProxy(mipMapped);
        if (!proxy) {
            *errorMsg = "Failed to create proxy.";
            return DrawResult::kFail;
        }

        // All matrices are configured to avoid sampling only the base level when using matrices.
        SkTArray<SkMatrix> textureMatrices;
        textureMatrices.push_back().setScale(1.1, 1.1);
        textureMatrices.push_back().setRotate(45.f, proxy->width() / 2.f, proxy->height() / 2.f);
        textureMatrices.back().postScale(1.3f, 2.7f);

        auto texelSubset = SkIRect::MakeXYWH(  fBitmap.width()/8 - 1,   fBitmap.height()/7 - 1,
                                             3*fBitmap.width()/5 + 2, 4*fBitmap.height()/5 + 2);

        sk_sp<GrTextureProxy> subsetProxy;
        SkBitmap subsetBmp;
        fBitmap.extractSubset(&subsetBmp, texelSubset);
        subsetBmp.setImmutable();
        GrBitmapTextureMaker subsetMaker(context, subsetBmp);
        std::tie(subsetProxy, std::ignore) = subsetMaker.refTextureProxy(mipMapped);

        SkRect localRect = SkRect::Make(fBitmap.bounds()).makeOutset(kDrawPad, kDrawPad);

        auto size = this->onISize();

        SkScalar y = kDrawPad + kTestPad;
        SkRect drawRect;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (int mx = 0; mx < GrSamplerState::kWrapModeCount; ++mx) {
                SkScalar x = kDrawPad + kTestPad;
                auto wmx = static_cast<GrSamplerState::WrapMode>(mx);
                if (fFilter != GrSamplerState::Filter::kNearest &&
                    (wmx == GrSamplerState::WrapMode::kRepeat ||
                     wmx == GrSamplerState::WrapMode::kMirrorRepeat)) {
                    // [Mirror] Repeat mode doesn't produce correct results with bilerp
                    // filtering
                    continue;
                }
                for (int my = 0; my < GrSamplerState::kWrapModeCount; ++my) {
                    auto wmy = static_cast<GrSamplerState::WrapMode>(my);
                    if (fFilter != GrSamplerState::Filter::kNearest &&
                        (wmy == GrSamplerState::WrapMode::kRepeat ||
                         wmy == GrSamplerState::WrapMode::kMirrorRepeat)) {
                        continue;
                    }
                    GrSamplerState sampler(wmx, wmy, fFilter);
                    const auto& caps = *context->priv().caps();
                    auto fp1 = GrTextureEffect::MakeTexelSubset(proxy,
                                                                fBitmap.alphaType(),
                                                                textureMatrices[tm],
                                                                sampler,
                                                                texelSubset,
                                                                caps);
                    if (!fp1) {
                        continue;
                    }
                    drawRect = localRect.makeOffset(x, y);
                    // Throw a translate in the local matrix just to test having something other
                    // than identity. Compensate with an offset local rect.
                    static constexpr SkVector kT = {-100, 300};
                    if (auto op = sk_gpu_test::test_ops::MakeRect(context,
                                                                  std::move(fp1),
                                                                  drawRect,
                                                                  localRect.makeOffset(kT),
                                                                  SkMatrix::MakeTrans(-kT))) {
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }
                    x += localRect.width() + kTestPad;

                    SkMatrix subsetTextureMatrix = SkMatrix::Concat(
                            SkMatrix::MakeTrans(-texelSubset.topLeft()), textureMatrices[tm]);

                    // Now draw with a subsetted proxy using fixed function texture sampling rather
                    // than a texture subset as a comparison.
                    drawRect = localRect.makeOffset(x, y);
                    auto fp2 = GrTextureEffect::Make(subsetProxy, fBitmap.alphaType(),
                                                     subsetTextureMatrix,
                                                     GrSamplerState(wmx, wmy, fFilter), caps);
                    if (auto op = sk_gpu_test::test_ops::MakeRect(context, std::move(fp2), drawRect,
                                                                  localRect)) {
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }
                    if (my < GrSamplerState::kWrapModeCount - 1) {
                        SkPaint paint;
                        SkScalar midX = drawRect.right() + kTestPad / 2.f;
                        canvas->drawLine({midX, 0}, {midX, (float)size.fHeight}, paint);
                    }
                    x += localRect.width() + kTestPad;
                }
                if (mx < GrSamplerState::kWrapModeCount - 1) {
                    SkPaint paint;
                    SkScalar midY = drawRect.bottom() + kTestPad / 2.f;
                    canvas->drawLine({0, midY}, {(float)size.fWidth, midY}, paint);
                }
                y += localRect.height() + kTestPad;
            }
            if (tm < textureMatrices.count() - 1) {
                SkPaint paint;
                paint.setColor(SK_ColorRED);
                SkScalar midY = drawRect.bottom() + kTestPad / 2.f;
                canvas->drawLine({0, midY}, {(float)size.fWidth, midY}, paint);
            }
        }
        return DrawResult::kOk;
    }

private:
    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    SkBitmap fBitmap;
    GrSamplerState::Filter fFilter;

    typedef GM INHERITED;
};

DEF_GM(return new TexelSubset(GrSamplerState::Filter::kNearest);)
DEF_GM(return new TexelSubset(GrSamplerState::Filter::kBilerp);)
DEF_GM(return new TexelSubset(GrSamplerState::Filter::kMipMap);)

}
