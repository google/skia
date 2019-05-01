/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkBlurMask.h"
#include "tools/ToolUtils.h"

#include <string.h>

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
