/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradient.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkColorPriv.h"

using namespace skiagm;

struct GradData {
    size_t           fCount;
    const SkColor4f* fColors;
    const float*     fPos;

    SkSpan<const SkColor4f> colors() const { return {fColors, fCount}; }
    SkSpan<const float> pos() const {
        if (fPos) {
            return {fPos, fCount};
        }
        return {};
    }

    SkGradient grad(SkTileMode tm) const { return {{this->colors(), this->pos(), tm}, {}}; }
};

constexpr SkColor4f gColors[] = {
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kWhite, SkColors::kBlack,
};

//constexpr SkScalar gPos[] = { SK_Scalar1*999/2000, SK_Scalar1*1001/2000 };

constexpr GradData gGradData[] = {
    { 40, gColors, nullptr },
    //  { 2, gColors, gPos },
    //  { 2, gCol2, nullptr },
};

static sk_sp<SkShader> MakeLinear(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    return SkShaders::LinearGradient(pts, data.grad(tm));
}

static sk_sp<SkShader> MakeRadial(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    const SkPoint pt{
        sk_float_midpoint(pts[0].fX, pts[1].fX),
        sk_float_midpoint(pts[0].fY, pts[1].fY),
    };
    return SkShaders::RadialGradient(pt, pt.fX, data.grad(tm));
}

static sk_sp<SkShader> MakeSweep(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    const SkPoint pt{
        sk_float_midpoint(pts[0].fX, pts[1].fX),
        sk_float_midpoint(pts[0].fY, pts[1].fY),
    };
    return SkShaders::SweepGradient(pt, data.grad(tm));
}


typedef sk_sp<SkShader> (*GradMaker)(const SkPoint pts[2], const GradData&, SkTileMode);

constexpr GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep,
};

///////////////////////////////////////////////////////////////////////////////

class GradientsGM : public GM {
public:
    GradientsGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString getName() const override { return SkString("gradient_dirty_laundry"); }
    SkISize getISize() override { return SkISize::Make(640, 615); }

    void onDraw(SkCanvas* canvas) override {
        SkPoint pts[2] = { { 0, 0 },
                           { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkTileMode tm = SkTileMode::kClamp;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < std::size(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < std::size(gGradMakers); j++) {
                paint.setShader(gGradMakers[j](pts, gGradData[i], tm));
                canvas->drawRect(r, paint);
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

private:
    using INHERITED = GM;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new GradientsGM; )
