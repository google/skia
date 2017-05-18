/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkTextBlob.h"

// This test ensures that glyphs whose point size is less than the SkGlyphCache's maxmium, but
// who have a large blur, are still handled correctly
DEF_SIMPLE_GM(largeglyphblur, canvas, 1920, 600) {
        const char text[] = "Hamburgefons";

        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(256);
        paint.setAntiAlias(true);

        // setup up maskfilter
        const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(40));

        SkPaint blurPaint(paint);
        blurPaint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, kSigma));

        SkTextBlobBuilder builder;

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        sk_sp<SkTextBlob> blob(builder.make());
        canvas->drawTextBlob(blob, 10, 200, blurPaint);
        canvas->drawTextBlob(blob, 10, 200, paint);

        size_t len = strlen(text);
        canvas->drawText(text, len, 10, 500, blurPaint);
        canvas->drawText(text, len, 10, 500, paint);
}
