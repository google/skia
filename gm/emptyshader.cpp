/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradient.h"
#include "src/shaders/SkEmptyShader.h"

namespace skiagm {

namespace {

sk_sp<SkShader> empty(SkRect r) { return sk_make_sp<SkEmptyShader>(); }

sk_sp<SkShader> degen_sweep(SkRect r) {
    // A too small angle between start and end falls back to an empty shader
    const float startAngle = 0.0f;
    const float endAngle = nextafter(startAngle, 360.0f);
    const SkColor4f colors[2] = { SkColors::kRed, SkColors::kGreen };

    return SkShaders::SweepGradient(r.center(), startAngle, endAngle,
                                    {{colors, {}, SkTileMode::kDecal}, {}});
}

sk_sp<SkShader> degen_linear(SkRect r) {
    // Having the two positions be the same causes a fallback to an empty shader
    const SkPoint pts[2] = { r.center(), r.center() };
    const SkColor4f colors[2] = { SkColors::kRed, SkColors::kGreen };

    return SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kDecal}, {}});
}

sk_sp<SkShader> degen_radial(SkRect r) {
    const SkColor4f colors[2] = { SkColors::kRed, SkColors::kGreen };

    // Having a radius of 0.0 causes a fallback to an empty shader
    return SkShaders::RadialGradient(r.center(), 0, {{colors, {}, SkTileMode::kDecal}, {}});
}

sk_sp<SkShader> degen_conical(SkRect r) {
    const SkColor4f colors[2] = { SkColors::kRed, SkColors::kGreen };

    // Having the start and end radii be the same causes a fallback to an empty shader
    return SkShaders::TwoPointConicalGradient(r.center(), /* startRadius= */ 0.0f,
                                              r.center(), /* endRadius= */ 0.0f,
                                              {{colors, {}, SkTileMode::kDecal}, {}});
}

} // anonymous namespace

class EmptyShaderGM : public GM {
public:
    EmptyShaderGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("emptyshader"); }

    SkISize getISize() override { return SkISize::Make(128, 88); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);

        int left = kPad, top = kPad;
        for (auto f : { empty, degen_sweep, degen_linear, degen_radial, degen_conical }) {
            SkRect r = SkRect::MakeXYWH(left, top, kSize, kSize);

            SkPaint p;
            p.setColor(SK_ColorBLUE);
            p.setShader(f(r));

            canvas->drawRect(r, p);
            canvas->drawRect(r, stroke);

            left += kSize + kPad;
            if (left >= this->getISize().width()) {
                left = kPad;
                top += kSize + kPad;
            }
        }
    }

private:
    static constexpr int kPad = 8;
    static constexpr int kSize = 32;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new EmptyShaderGM;)

}  // namespace skiagm
