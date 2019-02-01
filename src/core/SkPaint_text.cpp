/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkFontDescriptor.h"
#include "SkFontPriv.h"
#include "SkGraphics.h"
#include "SkPaintDefaults.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkSafeRange.h"
#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkShaderBase.h"
#include "SkStrike.h"
#include "SkStringUtils.h"
#include "SkTLazy.h"
#include "SkTextBlob.h"
#include "SkTextBlobPriv.h"
#include "SkTextFormatParams.h"
#include "SkTo.h"
#include "SkTypeface.h"

///////////////////////////////////////////////////////////////////////////////

static SkScalar mag2(SkScalar x, SkScalar y) {
    return x * x + y * y;
}

static bool tooBig(const SkMatrix& m, SkScalar ma2max) {
    return  mag2(m[SkMatrix::kMScaleX], m[SkMatrix::kMSkewY]) > ma2max
            ||
            mag2(m[SkMatrix::kMSkewX], m[SkMatrix::kMScaleY]) > ma2max;
}

bool SkFontPriv::TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM, SkScalar maxLimit) {
    SkASSERT(!ctm.hasPerspective());
    SkASSERT(!textM.hasPerspective());

    SkMatrix matrix;
    matrix.setConcat(ctm, textM);
    return tooBig(matrix, MaxCacheSize2(maxLimit));
}

SkScalar SkFontPriv::MaxCacheSize2(SkScalar maxLimit) {
    // we have a self-imposed maximum, just for memory-usage sanity
    const int limit = SkMin32(SkGraphics::GetFontCachePointSizeLimit(), maxLimit);
    const SkScalar maxSize = SkIntToScalar(limit);
    return maxSize * maxSize;
}

// return true if the paint is just a single color (i.e. not a shader). If its
// a shader, then we can't compute a const luminance for it :(
static bool just_a_color(const SkPaint& paint, SkColor* color) {
    SkColor c = paint.getColor();

    const auto* shader = as_SB(paint.getShader());
    if (shader && !shader->asLuminanceColor(&c)) {
        return false;
    }
    if (paint.getColorFilter()) {
        c = paint.getColorFilter()->filterColor(c);
    }
    if (color) {
        *color = c;
    }
    return true;
}

SkColor SkPaint::computeLuminanceColor() const {
    SkColor c;
    if (!just_a_color(*this, &c)) {
        c = SkColorSetRGB(0x7F, 0x80, 0x7F);
    }
    return c;
}
