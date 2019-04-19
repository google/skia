/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "gm.h"

#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkMaskFilter.h"
#include "SkTextBlob.h"

// This test ensures that glyphs whose point size is less than the SkStrike's maxmium, but
// who have a large blur, are still handled correctly
DEF_SIMPLE_GM(largeglyphblur, canvas, 1920, 600) {
    const char text[] = "Hamburgefons";

    SkFont font(ToolUtils::create_portable_typeface(), 256);
    auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);

    // setup up maskfilter
    const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(40));

    SkPaint blurPaint;
    blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, kSigma));

    canvas->drawTextBlob(blob, 10, 200, blurPaint);
    canvas->drawTextBlob(blob, 10, 200, SkPaint());

    size_t len = strlen(text);
    canvas->drawSimpleText(text, len, kUTF8_SkTextEncoding, 10, 500, font, blurPaint);
    canvas->drawSimpleText(text, len, kUTF8_SkTextEncoding, 10, 500, font, SkPaint());
}
