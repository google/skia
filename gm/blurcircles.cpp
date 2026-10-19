/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBlurMask.h"
#include "tools/fonts/FontToolUtils.h"

#include <array>

class BlurCirclesGM : public skiagm::GM {
public:
    BlurCirclesGM() { }

protected:
    bool runAsBench() const override { return true; }

    SkString getName() const override { return SkString("blurcircles"); }

    SkISize getISize() override { return SkISize::Make(950, 950); }

    void onOnceBeforeDraw() override {
        static constexpr std::array<float, kNumBlurs> blurRadii = {1.f, 5.f, 10.f, 20.f};

        for (int i = 0; i < kNumBlurs; ++i) {
            fBlurFilters[i] = SkMaskFilter::MakeBlur(
                                    kNormal_SkBlurStyle,
                                    SkBlurMask::ConvertRadiusToSigma(blurRadii[i]));
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(1.5f, 1.5f);
        canvas->translate(50,50);

        static constexpr auto circleRadii = std::to_array<float>({5.f, 10.f, 25.f, 50.f});

        for (size_t i = 0; i < kNumBlurs; ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, 150.f*i);
            for (size_t j = 0; j < std::size(circleRadii); ++j) {
                SkPaint paint;
                paint.setColor(SK_ColorBLACK);
                paint.setMaskFilter(fBlurFilters[i]);

                static constexpr SkPoint kCenter = {50.f, 50.f};
                // Throw a rotation in the mix to make sure GPU fast path handles it correctly.
                canvas->save();
                canvas->rotate(j*22.f, kCenter.fX, kCenter.fY);
                canvas->drawCircle(kCenter, circleRadii[j], paint);
                canvas->restore();
                canvas->translate(150.f, 0.f);
            }
        }
    }
private:
    inline static constexpr int kNumBlurs = 4;

    std::array<sk_sp<SkMaskFilter>, kNumBlurs> fBlurFilters;

    using INHERITED =         skiagm::GM;
};

DEF_GM(return new BlurCirclesGM();)

DEF_SIMPLE_GM(blurcircles_xform, canvas, 1280, 1020) {
    static constexpr float kRadius = 10.f;
    static constexpr float kCellSize = 140.f;
    static constexpr float kLabelHeight = 30.f;
    static constexpr std::array<float, 3> kSigmas = {1.f, 2.5f, 5.f};
    static constexpr float kSectionHeight = kLabelHeight + kSigmas.size() * kCellSize;

    const SkMatrix matrices[] = {
        SkMatrix(),
        SkMatrix::Scale(2.f, 2.f),
        SkMatrix::RotateDeg(30.f),
        SkMatrix::Scale(3.f, 1.f),
        SkMatrix::Scale(0.5f, 3.f),
        SkMatrix::RotateDeg(30.f) * SkMatrix::Scale(3.f, 1.f),  // Non-uniform scale, then rotate.
        // Rotate, then non-uniform scale. A circle is unchanged by the rotation, so this should
        // look the same as Scale(3, 1).
        SkMatrix::Scale(3.f, 1.f) * SkMatrix::RotateDeg(30.f),
        SkMatrix::Skew(1.f, 0.f),
    };

    static constexpr struct {
        bool fRespectCTM;
        const char* fLabel;
    } kSections[] = {
        {true,  "respectCTM = true"},
        {false, "respectCTM = false"},
    };

    SkFont font = ToolUtils::DefaultPortableFont();
    font.setSize(16.f);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    for (size_t section = 0; section < std::size(kSections); section++) {
        const float sectionTop = section * kSectionHeight;
        canvas->drawString(kSections[section].fLabel, 10.f, sectionTop + kLabelHeight * 0.5 + 10.0,
                           font, SkPaint());

        for (size_t row = 0; row < kSigmas.size(); row++) {
            paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, kSigmas[row],
                                                       kSections[section].fRespectCTM));
            for (size_t col = 0; col < std::size(matrices); ++col) {
                SkAutoCanvasRestore autoRestore(canvas, true);
                canvas->translate(kCellSize * (col + 0.5f) + 0.25f,
                                  sectionTop + kLabelHeight + kCellSize * (row + 0.5f) + 0.75f);
                canvas->concat(matrices[col]);
                canvas->drawCircle(0.f, 0.f, kRadius, paint);
            }
        }
    }
}
