/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

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
        static const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(40));

        SkPaint blurPaint(paint);
        SkAutoTUnref<SkMaskFilter> mf(SkBlurMaskFilter::Create(kNormal_SkBlurStyle, kSigma));
        blurPaint.setMaskFilter(mf);

        SkTextBlobBuilder builder;

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        SkAutoTUnref<const SkTextBlob> blob(builder.build());
        canvas->drawTextBlob(blob.get(), 10, 200, blurPaint);
        canvas->drawTextBlob(blob.get(), 10, 200, paint);

        size_t len = strlen(text);
        canvas->drawText(text, len, 10, 500, blurPaint);
        canvas->drawText(text, len, 10, 500, paint);
}
