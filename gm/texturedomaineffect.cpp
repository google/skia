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
#include "SkSurface.h"
#include "effects/GrTextureDomain.h"
#include "ops/GrDrawOp.h"
#include "ops/GrRectOpFactory.h"

namespace skiagm {
/**
 * This GM directly exercises GrTextureDomainEffect.
 */
class TextureDomainEffect : public GM {
public:
    TextureDomainEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("texture_domain_effect");
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

    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
        sk_sp<GrTextureProxy> proxy = proxyProvider->createTextureProxy(
                fImage, kNone_GrSurfaceFlags, 1, SkBudgeted::kYes, SkBackingFit::kExact);
        if (!proxy) {
            return;
        }

        SkTArray<SkMatrix> textureMatrices;
        textureMatrices.push_back() = SkMatrix::I();
        textureMatrices.push_back() = SkMatrix::MakeScale(1.5f, 0.85f);
        textureMatrices.push_back();
        textureMatrices.back().setRotate(45.f, proxy->width() / 2.f, proxy->height() / 2.f);


        const SkIRect texelDomains[] = {
            fImage->bounds(),
            SkIRect::MakeXYWH(fImage->width() / 4, fImage->height() / 4,
                              fImage->width() / 2, fImage->height() / 2),
        };

        SkRect renderRect = SkRect::Make(fImage->bounds());
        renderRect.outset(kDrawPad, kDrawPad);

        SkScalar y = kDrawPad + kTestPad;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (size_t d = 0; d < SK_ARRAY_COUNT(texelDomains); ++d) {
                SkScalar x = kDrawPad + kTestPad;
                for (int m = 0; m < GrTextureDomain::kModeCount; ++m) {
                    GrTextureDomain::Mode mode = (GrTextureDomain::Mode) m;
                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                    auto fp = GrTextureDomainEffect::Make(
                            proxy, textureMatrices[tm],
                            GrTextureDomain::MakeTexelDomainForMode(texelDomains[d], mode), mode,
                            GrSamplerState::Filter::kNearest);

                    if (!fp) {
                        continue;
                    }
                    const SkMatrix viewMatrix = SkMatrix::MakeTrans(x, y);
                    grPaint.addColorFragmentProcessor(std::move(fp));
                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrRectOpFactory::MakeNonAAFill(context, std::move(grPaint), viewMatrix,
                                                           renderRect, GrAAType::kNone));
                    x += renderRect.width() + kTestPad;
                }
                y += renderRect.height() + kTestPad;
            }
        }
    }

private:
    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;;
    static constexpr int      kTargetWidth = 100;
    static constexpr int      kTargetHeight = 100;
    sk_sp<SkImage> fImage;

    typedef GM INHERITED;
};

DEF_GM(return new TextureDomainEffect;)
}
