/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextUtils.h"
#include "SkTextBlob.h"

void SkTextUtils::DrawText(SkCanvas* canvas, const void* text, size_t size, SkScalar x, SkScalar y,
                            const SkPaint& paint, Align align) {

    SkFont font = SkFont::LEGACY_ExtractFromPaint(paint);

    if (align != kLeft_Align) {
        SkScalar width = font.measureText(text, size, paint.getTextEncoding());
        if (align == kCenter_Align) {
            width *= 0.5f;
        }
        x -= width;
    }

    canvas->drawTextBlob(SkTextBlob::MakeFromText(text, size, font, paint.getTextEncoding()), x, y, paint);
}

