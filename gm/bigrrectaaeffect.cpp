/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/ops/FillRectOp.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/ToolUtils.h"

#include <memory>
#include <utility>

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class BigRRectAAEffectGM : public GpuGM {
public:
    BigRRectAAEffectGM(const SkRRect& rrect, const char* name)
        : fRRect(rrect)
        , fName(name) {
        this->setBGColor(ToolUtils::color_to_565(SK_ColorBLUE));
        // Each test case draws the rrect with gaps around it.
        fTestWidth = SkScalarCeilToInt(rrect.width()) + 2 * kGap;
        fTestHeight = SkScalarCeilToInt(rrect.height()) + 2 * kGap;

        // Add a pad between test cases.
        fTestOffsetX = fTestWidth + kPad;
        fTestOffsetY = fTestHeight + kPad;

        // We draw two tests in x (fill and inv-fill) and pad around
        // all four sides of the image.
        fWidth = 2 * fTestOffsetX + kPad;
        fHeight = fTestOffsetY + kPad;
    }

protected:
    SkString onShortName() override {
        SkString name;
        name.printf("big_rrect_%s_aa_effect", fName);
        return name;
    }

    SkISize onISize() override { return SkISize::Make(fWidth, fHeight); }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
        if (!sdc) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        int y = kPad;
        int x = kPad;
        constexpr GrClipEdgeType kEdgeTypes[] = {
            GrClipEdgeType::kFillAA,
            GrClipEdgeType::kInverseFillAA,
        };
        SkRect testBounds = SkRect::MakeIWH(fTestWidth, fTestHeight);
        for (size_t et = 0; et < SK_ARRAY_COUNT(kEdgeTypes); ++et) {
            GrClipEdgeType edgeType = kEdgeTypes[et];
            canvas->save();
                canvas->translate(SkIntToScalar(x), SkIntToScalar(y));

                // Draw a background for the test case
                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                canvas->drawRect(testBounds, paint);

                SkRRect rrect = fRRect;
                rrect.offset(SkIntToScalar(x + kGap), SkIntToScalar(y + kGap));
                const auto& caps = *rContext->priv().caps()->shaderCaps();
                auto [success, fp] = GrRRectEffect::Make(/*inputFP=*/nullptr, edgeType, rrect,
                                                         caps);
                SkASSERT(success);
                if (success) {
                    SkASSERT(fp);
                    GrPaint grPaint;
                    grPaint.setColor4f({ 0, 0, 0, 1.f });
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                    grPaint.setCoverageFragmentProcessor(std::move(fp));

                    SkRect bounds = testBounds;
                    bounds.offset(SkIntToScalar(x), SkIntToScalar(y));

                    sdc->addDrawOp(skgpu::v1::FillRectOp::MakeNonAARect(
                            rContext, std::move(grPaint), SkMatrix::I(), bounds));
                }
            canvas->restore();
            x = x + fTestOffsetX;
        }

        return DrawResult::kOk;
    }

private:
    // pad between test cases
    inline static constexpr int kPad = 7;
    // gap between rect for each case that is rendered and exterior of rrect
    inline static constexpr int kGap = 3;

    SkRRect fRRect;
    int fWidth;
    int fHeight;
    int fTestWidth;
    int fTestHeight;
    int fTestOffsetX;
    int fTestOffsetY;
    const char* fName;
    using INHERITED = GM;
};

///////////////////////////////////////////////////////////////////////////////
// This value is motivated by bug chromium:477684. It has to be large to cause overflow in
// the shader
constexpr int kSize = 700;

DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeRect(SkRect::MakeIWH(kSize, kSize)), "rect"); )
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeOval(SkRect::MakeIWH(kSize, kSize)), "circle"); )
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeOval(SkRect::MakeIWH(kSize - 1, kSize - 10)), "ellipse"); )
// The next two have small linear segments between the corners
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeRectXY(SkRect::MakeIWH(kSize - 1, kSize - 10), kSize/2.f - 10.f, kSize/2.f - 10.f), "circular_corner"); )
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeRectXY(SkRect::MakeIWH(kSize - 1, kSize - 10), kSize/2.f - 10.f, kSize/2.f - 15.f), "elliptical_corner"); )

}  // namespace skiagm
