/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrSamplerState.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTextureProxy.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrFillRectOp.h"

#include <memory>
#include <utility>

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
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        sk_sp<GrTextureProxy> proxy;
        GrMipMapped mipMapped = fFilter == GrSamplerState::Filter::kMipMap &&
                                context->priv().caps()->mipMapSupport()
                ? GrMipMapped::kYes : GrMipMapped::kNo;
        proxy = proxyProvider->createProxyFromBitmap(fBitmap, mipMapped);
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

        SkRect renderRect = SkRect::Make(fBitmap.bounds());
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
    SkBitmap fBitmap;
    GrSamplerState::Filter fFilter;

    typedef GM INHERITED;
};

DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kNearest);)
DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kBilerp);)
DEF_GM(return new TextureDomainEffect(GrSamplerState::Filter::kMipMap);)

}
