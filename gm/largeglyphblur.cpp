/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkMaskFilter.h"
#include "SkTextBlob.h"

// This test ensures that glyphs whose point size is less than the SkStrike's maxmium, but
// who have a large blur, are still handled correctly
DEF_SIMPLE_GM(largeglyphblur, canvas, 1920, 600) {
    const char text[] = "Hamburgefons";

    SkFont font(sk_tool_utils::create_portable_typeface(), 256);
    auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);

    // setup up maskfilter
    const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(40));

    SkPaint blurPaint;
    blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, kSigma));

    canvas->drawTextBlob(blob, 10, 200, blurPaint);
    canvas->drawTextBlob(blob, 10, 200, SkPaint());

    canvas->drawString(text, 10, 500, font, blurPaint);
    canvas->drawString(text, 10, 500, font, SkPaint());
}
