/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContextPriv.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkSurface.h"
#include "effects/GrTextureDomain.h"
#include "ops/GrDrawOp.h"
#include "ops/GrFillRectOp.h"

namespace skiagm {
/**
 * This GM directly exercises GrTextureDomainEffect.
 */
class TextureDomainEffect : public GpuGM {
public:
    TextureDomainEffect(GrSamplerState::Filter filter)
            : fFilter(filter) {
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
        const SkScalar canvasWidth = kDrawPad +
                (kTargetWidth + 2 * kDrawPad) * GrTextureDomain::kModeCount +
                kTestPad * GrTextureDomain::kModeCount;
        return SkISize::Make(SkScalarCeilToInt(canvasWidth), 800);
    }

    void onOnceBeforeDraw() override {
        // TODO: do this with gpu backend
        SkImageInfo ii = SkImageInfo::Make(kTargetWidth, kTargetHeight, kN32_SkColorType,
                                           kPremul_SkAlphaType);
        auto surface = SkSurface::MakeRaster(ii);
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(0x00000000);
        SkPaint paint;

        SkColor colors1[] = { SK_ColorCYAN, SK_ColorLTGRAY, SK_ColorGRAY };
        paint.setShader(SkGradientShader::MakeSweep(65.f, 75.f, colors1, nullptr,
                                                    SK_ARRAY_COUNT(colors1)));
        canvas->drawOval(SkRect::MakeXYWH(-5.f, -5.f, kTargetWidth + 10.f, kTargetHeight + 10.f),
                         paint);

        SkColor colors2[] = { SK_ColorMAGENTA, SK_ColorLTGRAY, SK_ColorYELLOW };
        paint.setShader(SkGradientShader::MakeSweep(45.f, 55.f, colors2, nullptr,
                                                    SK_ARRAY_COUNT(colors2)));
        paint.setBlendMode(SkBlendMode::kDarken);
        canvas->drawOval(SkRect::MakeXYWH(-5.f, -5.f, kTargetWidth + 10.f, kTargetHeight + 10.f),
                         paint);

        SkColor colors3[] = { SK_ColorBLUE, SK_ColorLTGRAY, SK_ColorGREEN };
        paint.setShader(SkGradientShader::MakeSweep(25.f, 35.f, colors3, nullptr,
                                                    SK_ARRAY_COUNT(colors3)));
        paint.setBlendMode(SkBlendMode::kLighten);
        canvas->drawOval(SkRect::MakeXYWH(-5.f, -5.f, kTargetWidth + 10.f, kTargetHeight + 10.f),
                         paint);

        fImage = surface->makeImageSnapshot();
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        sk_sp<GrTextureProxy> proxy;
        if (fFilter == GrSamplerState::Filter::kMipMap && context->priv().caps()->mipMapSupport()) {
            SkBitmap copy;
            SkImageInfo info = fImage->imageInfo().makeColorType(kN32_SkColorType);
            if (!copy.tryAllocPixels(info) || !fImage->readPixels(copy.pixmap(), 0, 0)) {
                *errorMsg = "Failed to read pixels.";
                return DrawResult::kFail;
            }
            proxy = proxyProvider->createProxyFromBitmap(copy, GrMipMapped::kYes);
        } else {
            proxy = proxyProvider->createTextureProxy(
                fImage, kNone_GrSurfaceFlags, 1, SkBudgeted::kYes, SkBackingFit::kExact);
        }
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
            fImage->bounds(),
            SkIRect::MakeXYWH(fImage->width() / 4 - 1, fImage->height() / 4 - 1,
                              fImage->width() / 2 + 2, fImage->height() / 2 + 2),
        };

        SkRect renderRect = SkRect::Make(fImage->bounds());
        renderRect.outset(kDrawPad, kDrawPad);

        SkScalar y = kDrawPad + kTestPad;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (size_t d = 0; d < SK_ARRAY_COUNT(texelDomains); ++d) {
                SkScalar x = kDrawPad + kTestPad;
                for (int m = 0; m < GrTextureDomain::kModeCount; ++m) {
                    GrTextureDomain::Mode mode = (GrTextureDomain::Mode) m;
                    if (fFilter != GrSamplerState::Filter::kNearest &&
                        mode == GrTextureDomain::kRepeat_Mode) {
                        // Repeat mode doesn't produce correct results with bilerp filtering
                        continue;
                    }

                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                    auto fp = GrTextureDomainEffect::Make(
                            proxy, textureMatrices[tm],
                            GrTextureDomain::MakeTexelDomain(texelDomains[d], mode),
                            mode, fFilter);

                    if (!fp) {
                        continue;
                    }
                    const SkMatrix viewMatrix = SkMatrix::MakeTrans(x, y);
                    grPaint.addColorFragmentProcessor(std::move(fp));
                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrFillRectOp::Make(context, std::move(grPaint), GrAAType::kNone,
                                               viewMatrix, renderRect));
                    x += renderRect.width() + kTestPad;
                }
                y += renderRect.height() + kTestPad;
            }
        }
        return DrawResult::kOk;
    }

private:
    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    static constexpr int      kTargetWidth = 100;
    static constexpr int      kTargetHeight = 100;
    sk_sp<SkImage> fImage;
    GrSamplerState::Filter fFilter;

    typedef GM INHERITED;
};

DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kNearest);)
DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kBilerp);)
DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kMipMap);)

}
