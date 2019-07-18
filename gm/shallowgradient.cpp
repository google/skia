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
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"

typedef sk_sp<SkShader> (*MakeShaderProc)(const SkColor[], int count, const SkSize&);

static sk_sp<SkShader> shader_linear(const SkColor colors[], int count, const SkSize& size) {
    SkPoint pts[] = { { 0, 0 }, { size.width(), size.height() } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, count, SkTileMode::kClamp);
}

static sk_sp<SkShader> shader_radial(const SkColor colors[], int count, const SkSize& size) {
    SkPoint center = { size.width()/2, size.height()/2 };
    return SkGradientShader::MakeRadial(center, size.width()/2, colors, nullptr, count,
                                        SkTileMode::kClamp);
}

static sk_sp<SkShader> shader_conical(const SkColor colors[], int count, const SkSize& size) {
    SkPoint center = { size.width()/2, size.height()/2 };
    return SkGradientShader::MakeTwoPointConical(center, size.width()/64, center, size.width()/2,
                                                colors, nullptr, count, SkTileMode::kClamp);
}

static sk_sp<SkShader> shader_sweep(const SkColor colors[], int count, const SkSize& size) {
    return SkGradientShader::MakeSweep(size.width()/2, size.height()/2, colors, nullptr, count);
}

class ShallowGradientGM : public skiagm::GM {
public:
    ShallowGradientGM(MakeShaderProc proc, const char name[], bool dither)
        : fProc(proc), fName(name), fDither(dither) {}

private:
    MakeShaderProc fProc;
    const char* fName;
    bool fDither;

    SkString onShortName() override {
        return SkStringPrintf("shallow_gradient_%s%s", fName, fDither ? "" : "_nodither");
    }

    SkISize onISize() override { return {800, 800}; }

    void onDraw(SkCanvas* canvas) override {
        const SkColor colors[] = { 0xFF555555, 0xFF444444 };
        const int colorCount = SK_ARRAY_COUNT(colors);

        SkRect r = { 0, 0, this->width(), this->height() };
        SkSize size = SkSize::Make(r.width(), r.height());

        SkPaint paint;
        paint.setShader(fProc(colors, colorCount, size));
        paint.setDither(fDither);
        canvas->drawRect(r, paint);
    }
};

///////////////////////////////////////////////////////////////////////////////

#define M(PROC, DITHER) DEF_GM( return new ShallowGradientGM(shader_ ## PROC, #PROC, DITHER); )
M(linear,  true)
M(radial,  true)
M(conical, true)
M(sweep,   true)

M(linear,  false)
M(radial,  false)
M(conical, false)
M(sweep,   false)
#undef M
