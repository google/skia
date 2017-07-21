/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrRenderTargetContextPriv.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkGradientShader.h"
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
        // TODO: do this with surfaces & images and gpu backend
        SkImageInfo ii = SkImageInfo::Make(kTargetWidth, kTargetHeight, kN32_SkColorType,
                                           kPremul_SkAlphaType);
        fBmp.allocPixels(ii);
        SkCanvas canvas(fBmp);
        canvas.clear(0x00000000);
        SkPaint paint;

        SkColor colors1[] = { SK_ColorCYAN, SK_ColorLTGRAY, SK_ColorGRAY };
        paint.setShader(SkGradientShader::MakeSweep(65.f, 75.f, colors1, nullptr,
                                                    SK_ARRAY_COUNT(colors1)));
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f, fBmp.width() + 10.f, fBmp.height() + 10.f),
                        paint);

        SkColor colors2[] = { SK_ColorMAGENTA, SK_ColorLTGRAY, SK_ColorYELLOW };
        paint.setShader(SkGradientShader::MakeSweep(45.f, 55.f, colors2, nullptr,
                                                    SK_ARRAY_COUNT(colors2)));
        paint.setBlendMode(SkBlendMode::kDarken);
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f, fBmp.width() + 10.f, fBmp.height() + 10.f),
                        paint);

        SkColor colors3[] = { SK_ColorBLUE, SK_ColorLTGRAY, SK_ColorGREEN };
        paint.setShader(SkGradientShader::MakeSweep(25.f, 35.f, colors3, nullptr,
                                                    SK_ARRAY_COUNT(colors3)));
        paint.setBlendMode(SkBlendMode::kLighten);
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f, fBmp.width() + 10.f, fBmp.height() + 10.f),
                        paint);
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

        GrSurfaceDesc desc;
        desc.fWidth = fBmp.width();
        desc.fHeight = fBmp.height();
        desc.fConfig = SkImageInfo2GrPixelConfig(fBmp.info(), *context->caps());

        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                 desc, SkBudgeted::kYes,
                                                                 fBmp.getPixels(),
                                                                 fBmp.rowBytes()));
        if (!proxy) {
            return;
        }

        SkTArray<SkMatrix> textureMatrices;
        textureMatrices.push_back() = SkMatrix::I();
        textureMatrices.push_back() = SkMatrix::MakeScale(1.5f, 0.85f);
        textureMatrices.push_back();
        textureMatrices.back().setRotate(45.f, proxy->width() / 2.f, proxy->height() / 2.f);

        const SkIRect texelDomains[] = {
            fBmp.bounds(),
            SkIRect::MakeXYWH(fBmp.width() / 4, fBmp.height() / 4,
                              fBmp.width() / 2, fBmp.height() / 2),
        };

        SkRect renderRect = SkRect::Make(fBmp.bounds());
        renderRect.outset(kDrawPad, kDrawPad);

        SkScalar y = kDrawPad + kTestPad;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (size_t d = 0; d < SK_ARRAY_COUNT(texelDomains); ++d) {
                SkScalar x = kDrawPad + kTestPad;
                for (int m = 0; m < GrTextureDomain::kModeCount; ++m) {
                    GrTextureDomain::Mode mode = (GrTextureDomain::Mode) m;
                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                    sk_sp<GrFragmentProcessor> fp(
                        GrTextureDomainEffect::Make(
                                   proxy,
                                   nullptr, textureMatrices[tm],
                                   GrTextureDomain::MakeTexelDomainForMode(texelDomains[d], mode),
                                   mode, GrSamplerParams::kNone_FilterMode));

                    if (!fp) {
                        continue;
                    }
                    const SkMatrix viewMatrix = SkMatrix::MakeTrans(x, y);
                    grPaint.addColorFragmentProcessor(std::move(fp));
                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrRectOpFactory::MakeNonAAFill(std::move(grPaint), viewMatrix,
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
    SkBitmap fBmp;

    typedef GM INHERITED;
};

DEF_GM(return new TextureDomainEffect;)
}

#endif
