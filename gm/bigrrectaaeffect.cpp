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
#include "include/gpu/GrContext.h"
#include "include/private/GrSharedEnums.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/ToolUtils.h"

#include <memory>
#include <utility>

namespace {
class BigRRectAAEffectGM : public skiagm::GpuGM {
public:
    BigRRectAAEffectGM(const SkRRect& rrect, const char* name)
        : fRRect(rrect)
        , fName(name) {
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

private:
    SkString onShortName() override { return SkString(fName); }

    SkISize onISize() override { return {fWidth, fHeight}; }

    void onOnceBeforeDraw() override {
        this->setBGColor(ToolUtils::color_to_565(SK_ColorBLUE));
    }

    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {
        SkPaint paint;

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
                const auto& caps = *renderTargetContext->caps()->shaderCaps();
                auto fp = GrRRectEffect::Make(edgeType, rrect, caps);
                SkASSERT(fp);
                if (fp) {
                    GrPaint grPaint;
                    grPaint.setColor4f({ 0, 0, 0, 1.f });
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                    grPaint.addCoverageFragmentProcessor(std::move(fp));

                    SkRect bounds = testBounds;
                    bounds.offset(SkIntToScalar(x), SkIntToScalar(y));

                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrFillRectOp::MakeNonAARect(context, std::move(grPaint),
                                                        SkMatrix::I(), bounds));
                }
            canvas->restore();
            x = x + fTestOffsetX;
        }
    }

    // pad between test cases
    static constexpr int kPad = 7;
    // gap between rect for each case that is rendered and exterior of rrect
    static constexpr int kGap = 3;

    SkRRect fRRect;
    int fWidth;
    int fHeight;
    int fTestWidth;
    int fTestHeight;
    int fTestOffsetX;
    int fTestOffsetY;
    const char* fName;
};

///////////////////////////////////////////////////////////////////////////////
// This value is motivated by bug chromium:477684. It has to be large to cause overflow in
// the shader
constexpr float kSize = 700;

#define M(RRECT, N) DEF_GM( return new BigRRectAAEffectGM(RRECT, "big_rrect_" N "_aa_effect"); )

M(SkRRect::MakeRect({0, 0, kSize, kSize}), "rect")
M(SkRRect::MakeOval({0, 0, kSize, kSize}), "circle")
M(SkRRect::MakeOval({0, 0, kSize - 1, kSize - 10}), "ellipse")
// The next two have small linear segments between the corners
M(SkRRect::MakeRectXY({0, 0, kSize - 1, kSize - 10}, 0.5f * kSize - 10, 0.5f * kSize - 10),
  "circular_corner")
M(SkRRect::MakeRectXY({0, 0, kSize - 1, kSize - 10}, 0.5f * kSize - 10, 0.5f * kSize - 15),
  "elliptical_corner")

#undef M
}  // namespace
