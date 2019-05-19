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
#include "include/effects/SkGradientShader.h"

using namespace skiagm;

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

constexpr SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
};

//constexpr SkScalar gPos[] = { SK_Scalar1*999/2000, SK_Scalar1*1001/2000 };

constexpr GradData gGradData[] = {
    { 40, gColors, nullptr },
    //  { 2, gColors, gPos },
    //  { 2, gCol2, nullptr },
};

static sk_sp<SkShader> MakeLinear(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    return SkGradientShader::MakeLinear(pts, data.fColors, data.fPos, data.fCount, tm);
}

static sk_sp<SkShader> MakeRadial(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    const SkPoint pt{ SkScalarAve(pts[0].fX, pts[1].fX), SkScalarAve(pts[0].fY, pts[1].fY) };
    return SkGradientShader::MakeRadial(pt, pt.fX, data.fColors, data.fPos, data.fCount, tm);
}

static sk_sp<SkShader> MakeSweep(const SkPoint pts[2], const GradData& data, SkTileMode) {
    const SkPoint pt{ SkScalarAve(pts[0].fX, pts[1].fX), SkScalarAve(pts[0].fY, pts[1].fY) };
    return SkGradientShader::MakeSweep(pt.fX, pt.fY, data.fColors, data.fPos, data.fCount);
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
    SkString onShortName() override { return SkString("gradient_dirty_laundry"); }
    SkISize onISize() override { return SkISize::Make(640, 615); }

    void onDraw(SkCanvas* canvas) override {
        SkPoint pts[2] = { { 0, 0 },
                           { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkTileMode tm = SkTileMode::kClamp;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                paint.setShader(gGradMakers[j](pts, gGradData[i], tm));
                canvas->drawRect(r, paint);
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new GradientsGM; )
