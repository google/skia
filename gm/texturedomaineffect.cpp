
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
#include "GrTest.h"
#include "effects/GrTextureDomain.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkGradientShader.h"

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
        fBmp.allocN32Pixels(kTargetWidth, kTargetHeight);
        SkCanvas canvas(fBmp);
        canvas.clear(0x00000000);
        SkPaint paint;

        SkColor colors1[] = { SK_ColorCYAN, SK_ColorLTGRAY, SK_ColorGRAY };
        paint.setShader(SkGradientShader::CreateSweep(65.f, 75.f, colors1,
                                                      NULL, SK_ARRAY_COUNT(colors1)))->unref();
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f,
                                         fBmp.width() + 10.f, fBmp.height() + 10.f), paint);

        SkColor colors2[] = { SK_ColorMAGENTA, SK_ColorLTGRAY, SK_ColorYELLOW };
        paint.setShader(SkGradientShader::CreateSweep(45.f, 55.f, colors2, NULL,
                                                      SK_ARRAY_COUNT(colors2)))->unref();
        paint.setXfermodeMode(SkXfermode::kDarken_Mode);
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f,
                                         fBmp.width() + 10.f, fBmp.height() + 10.f), paint);

        SkColor colors3[] = { SK_ColorBLUE, SK_ColorLTGRAY, SK_ColorGREEN };
        paint.setShader(SkGradientShader::CreateSweep(25.f, 35.f, colors3, NULL,
                                                      SK_ARRAY_COUNT(colors3)))->unref();
        paint.setXfermodeMode(SkXfermode::kLighten_Mode);
        canvas.drawOval(SkRect::MakeXYWH(-5.f, -5.f,
                                         fBmp.width() + 10.f, fBmp.height() + 10.f), paint);
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
        if (NULL == rt) {
            return;
        }
        GrContext* context = rt->getContext();
        if (NULL == context) {
            this->drawGpuOnlyMessage(canvas);
            return;
        }

        GrTestTarget tt;
        context->getTestTarget(&tt);
        if (NULL == tt.target()) {
            SkDEBUGFAIL("Couldn't get Gr test target.");
            return;
        }

        SkAutoTUnref<GrTexture> texture(GrRefCachedBitmapTexture(context, fBmp, NULL));
        if (!texture) {
            return;
        }

        SkTArray<SkMatrix> textureMatrices;
        textureMatrices.push_back().setIDiv(texture->width(), texture->height());
        textureMatrices.push_back() = textureMatrices[0];
        textureMatrices.back().postScale(1.5f, 0.85f);
        textureMatrices.push_back() = textureMatrices[0];
        textureMatrices.back().preRotate(45.f, texture->width() / 2.f, texture->height() / 2.f);

        const SkIRect texelDomains[] = {
            fBmp.bounds(),
            SkIRect::MakeXYWH(fBmp.width() / 4,
                              fBmp.height() / 4,
                              fBmp.width() / 2,
                              fBmp.height() / 2),
        };

        SkRect renderRect = SkRect::Make(fBmp.bounds());
        renderRect.outset(kDrawPad, kDrawPad);

        SkScalar y = kDrawPad + kTestPad;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (size_t d = 0; d < SK_ARRAY_COUNT(texelDomains); ++d) {
                SkScalar x = kDrawPad + kTestPad;
                for (int m = 0; m < GrTextureDomain::kModeCount; ++m) {
                    GrTextureDomain::Mode mode = (GrTextureDomain::Mode) m;
                    GrPipelineBuilder pipelineBuilder;
                    SkAutoTUnref<GrFragmentProcessor> fp(
                        GrTextureDomainEffect::Create(pipelineBuilder.getProcessorDataManager(),
                                                      texture, textureMatrices[tm],
                                                GrTextureDomain::MakeTexelDomain(texture,
                                                                                texelDomains[d]),
                                                mode, GrTextureParams::kNone_FilterMode));

                    if (!fp) {
                        continue;
                    }
                    const SkMatrix viewMatrix = SkMatrix::MakeTrans(x, y);
                    pipelineBuilder.setRenderTarget(rt);
                    pipelineBuilder.addColorProcessor(fp);

                    tt.target()->drawSimpleRect(pipelineBuilder,
                                                GrColor_WHITE,
                                                viewMatrix,
                                                renderRect);
                    x += renderRect.width() + kTestPad;
                }
                y += renderRect.height() + kTestPad;
            }
        }
    }

private:
    static const SkScalar kDrawPad;
    static const SkScalar kTestPad;
    static const int      kTargetWidth = 100;
    static const int      kTargetHeight = 100;
    SkBitmap fBmp;

    typedef GM INHERITED;
};

// Windows builds did not like SkScalar initialization in class :(
const SkScalar TextureDomainEffect::kDrawPad = 10.f;
const SkScalar TextureDomainEffect::kTestPad = 10.f;

DEF_GM( return SkNEW(TextureDomainEffect); )
}

#endif
