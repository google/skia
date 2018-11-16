// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkShaper.h"

#include "SkCanvas.h"
#include "SkTextBlob.h"

SkPoint SkShaper::DrawText(SkCanvas* canvas, const char* text, size_t size,
                           SkPoint xy, const SkPaint& paint, SkScalar width) {
    SkASSERT(paint.getTextEncoding() == SkPaint::kUTF8_TextEncoding);
    SkShaper shaper(paint.refTypeface());
    if (shaper.good()) {
        SkTextBlobBuilder builder;
        SkPoint p = shaper.shape(&builder, paint, (const char*)text, size, true, {0, 0}, width);
        if (sk_sp<SkTextBlob> blob = builder.make()) {
            canvas->drawTextBlob(blob.get(), xy.x(), xy.y(), paint);
            return p + xy;
        }
    }
    canvas->drawText(text, size, xy.x(), xy.y(), paint);  // Fallback.
    return xy + SkPoint{paint.measureText(text, size), paint.getFontSpacing()};
}
