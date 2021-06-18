/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
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
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "tools/Resources.h"

#include <math.h>

// A tint filter maps colors to a given range (gradient), based on the input luminance:
//
//   c' = lerp(lo, hi, luma(c))
static sk_sp<SkColorFilter> MakeTintColorFilter(SkColor lo, SkColor hi, bool useSkSL) {
    const auto r_lo = SkColorGetR(lo),
    g_lo = SkColorGetG(lo),
    b_lo = SkColorGetB(lo),
    a_lo = SkColorGetA(lo),
    r_hi = SkColorGetR(hi),
    g_hi = SkColorGetG(hi),
    b_hi = SkColorGetB(hi),
    a_hi = SkColorGetA(hi);

    // We map component-wise:
    //
    //   r' = lo.r + (hi.r - lo.r) * luma
    //   g' = lo.g + (hi.g - lo.g) * luma
    //   b' = lo.b + (hi.b - lo.b) * luma
    //   a' = lo.a + (hi.a - lo.a) * luma
    //
    // The input luminance is stored in the alpha channel
    // (and RGB are cleared -- see SkLumaColorFilter). Thus:
    const float tint_matrix[] = {
        0, 0, 0, (r_hi - r_lo) / 255.0f, SkIntToScalar(r_lo) / 255.0f,
        0, 0, 0, (g_hi - g_lo) / 255.0f, SkIntToScalar(g_lo) / 255.0f,
        0, 0, 0, (b_hi - b_lo) / 255.0f, SkIntToScalar(b_lo) / 255.0f,
        0, 0, 0, (a_hi - a_lo) / 255.0f, SkIntToScalar(a_lo) / 255.0f,
    };

    sk_sp<SkColorFilter> inner = SkLumaColorFilter::Make(),
                         outer = SkColorFilters::Matrix(tint_matrix);

    // Prove that we can implement compose-color-filter using runtime effects
    if (useSkSL) {
        auto [effect, error] = SkRuntimeEffect::MakeForColorFilter(SkString(R"(
            uniform colorFilter inner;
            uniform colorFilter outer;
            half4 main(half4 c) { return sample(outer, sample(inner, c)); }
        )"));
        SkASSERT(effect);
        SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
        sk_sp<SkColorFilter> children[] = { inner, outer };
        return effect->makeColorFilter(nullptr, children, SK_ARRAY_COUNT(children));
    } else {
        return outer->makeComposed(inner);
    }
}

DEF_SIMPLE_GM(composeCF, canvas, 200, 200) {
    // This GM draws a simple color-filter network, using the existing "makeComposed" API, and also
    // using a runtime color filter that does the same thing.
    SkPaint paint;
    const SkColor gradient_colors[] = {SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED};
    paint.setShader(SkGradientShader::MakeSweep(
            50, 50, gradient_colors, nullptr, SK_ARRAY_COUNT(gradient_colors)));

    canvas->save();
    for (bool useSkSL : {false, true}) {
        auto cf0 = MakeTintColorFilter(0xff300000, 0xffa00000, useSkSL);  // red tint
        auto cf1 = MakeTintColorFilter(0xff003000, 0xff00a000, useSkSL);  // green tint

        paint.setColorFilter(cf0);
        canvas->drawRect({0, 0, 100, 100}, paint);
        canvas->translate(100, 0);

        paint.setColorFilter(cf1);
        canvas->drawRect({0, 0, 100, 100}, paint);

        canvas->restore();
        canvas->translate(0, 100);
    }
}
