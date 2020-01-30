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
#include "tools/gpu/TestOps.h"

#include <memory>
#include <utility>

namespace skiagm {
/**
 * This GM directly exercises GrTextureEffect::MakeTexelSubset.
 */
class TextureDomainEffect : public GpuGM {
public:
    TextureDomainEffect(GrSamplerState::Filter filter) : fFilter(filter) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        SkString name("texture_domain_effect");
        if (fFilter == GrSamplerState::Filter::kBilerp) {
            name.append("_bilerp");
        } else if (fFilter == GrSamplerState::Filter::kMipMap) {
            name.append("_mipmap");
        }
        return name;
    }

    SkISize onISize() override {
        const SkScalar canvasWidth =
                kDrawPad + 2 * ((kTargetWidth + 2 * kDrawPad) * GrSamplerState::kWrapModeCount +
                                kTestPad * GrSamplerState::kWrapModeCount);
        return SkISize::Make(SkScalarCeilToInt(canvasWidth), 800);
    }

    void onOnceBeforeDraw() override {
        fBitmap.allocN32Pixels(kTargetWidth, kTargetHeight);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;

        SkColor colors1[] = { SK_ColorCYAN, SK_ColorLTGRAY, SK_ColorGRAY };
        paint.setShader(SkGradientShader::MakeSweep(65.f, 75.f, colors1, nullptr,
                                                    SK_ARRAY_COUNT(colors1)));
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f, kTargetWidth + 10.f, kTargetHeight + 10.f),
                        paint);

        SkColor colors2[] = { SK_ColorMAGENTA, SK_ColorLTGRAY, SK_ColorYELLOW };
        paint.setShader(SkGradientShader::MakeSweep(45.f, 55.f, colors2, nullptr,
                                                    SK_ARRAY_COUNT(colors2)));
        paint.setBlendMode(SkBlendMode::kDarken);
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f, kTargetWidth + 10.f, kTargetHeight + 10.f),
                        paint);

        SkColor colors3[] = { SK_ColorBLUE, SK_ColorLTGRAY, SK_ColorGREEN };
        paint.setShader(SkGradientShader::MakeSweep(25.f, 35.f, colors3, nullptr,
                                                    SK_ARRAY_COUNT(colors3)));
        paint.setBlendMode(SkBlendMode::kLighten);
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f, kTargetWidth + 10.f, kTargetHeight + 10.f),
                        paint);
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

        SkTArray<SkMatrix> textureMatrices;
        textureMatrices.push_back() = SkMatrix::I();
        textureMatrices.push_back() = SkMatrix::MakeScale(1.5f, 0.85f);
        textureMatrices.push_back();
        textureMatrices.back().setRotate(45.f, proxy->width() / 2.f, proxy->height() / 2.f);

        const SkIRect texelDomains[] = {
            fBitmap.bounds(),
            SkIRect::MakeXYWH(fBitmap.width() / 4 - 1, fBitmap.height() / 4 - 1,
                              fBitmap.width() / 2 + 2, fBitmap.height() / 2 + 2),
        };

        sk_sp<GrTextureProxy> subsetProxies[SK_ARRAY_COUNT(texelDomains)];
        for (size_t d = 0; d < SK_ARRAY_COUNT(texelDomains); ++d) {
            SkBitmap subset;
            fBitmap.extractSubset(&subset, texelDomains[d]);
            subset.setImmutable();
            GrBitmapTextureMaker maker(context, subset);
            std::tie(subsetProxies[d], std::ignore) = maker.refTextureProxy(mipMapped);
        }

        SkRect localRect = SkRect::Make(fBitmap.bounds()).makeOutset(kDrawPad, kDrawPad);

        SkScalar y = kDrawPad + kTestPad;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (size_t d = 0; d < SK_ARRAY_COUNT(texelDomains); ++d) {
                SkScalar x = kDrawPad + kTestPad;
                for (int m = 0; m < GrSamplerState::kWrapModeCount; ++m) {
                    auto wm = static_cast<GrSamplerState::WrapMode>(m);
                    if (fFilter != GrSamplerState::Filter::kNearest &&
                        (wm == GrSamplerState::WrapMode::kRepeat ||
                         wm == GrSamplerState::WrapMode::kMirrorRepeat)) {
                        // [Mirror] Repeat mode doesn't produce correct results with bilerp
                        // filtering
                        continue;
                    }
                    GrSamplerState sampler(wm, fFilter);
                    const auto& caps = *context->priv().caps();
                    auto fp1 = GrTextureEffect::MakeTexelSubset(proxy,
                                                                fBitmap.alphaType(),
                                                                textureMatrices[tm],
                                                                sampler,
                                                                texelDomains[d],
                                                                caps);
                    if (!fp1) {
                        continue;
                    }
                    auto drawRect = localRect.makeOffset(x, y);
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
                            SkMatrix::MakeTrans(-texelDomains[d].topLeft()), textureMatrices[tm]);

                    // Now draw with a subsetted proxy using fixed function texture sampling rather
                    // than a texture subset as a comparison.
                    drawRect = localRect.makeOffset(x, y);
                    auto fp2 = GrTextureEffect::Make(subsetProxies[d], fBitmap.alphaType(),
                                                     subsetTextureMatrix,
                                                     GrSamplerState(wm, fFilter), caps);
                    if (auto op = sk_gpu_test::test_ops::MakeRect(context, std::move(fp2), drawRect,
                                                                  localRect)) {
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }
                    x += localRect.width() + kTestPad;
                }
                y += localRect.height() + kTestPad;
            }
        }
        return DrawResult::kOk;
    }

private:
    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    static constexpr int      kTargetWidth = 100;
    static constexpr int      kTargetHeight = 100;
    SkBitmap fBitmap;
    GrSamplerState::Filter fFilter;

    typedef GM INHERITED;
};

DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kNearest);)
DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kBilerp);)
DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kMipMap);)

}
