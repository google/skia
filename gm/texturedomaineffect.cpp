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
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrTextureProxy.h"
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
        return DrawResult::kSkip;
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
