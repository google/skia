/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextUtils.h"
#include "SkTextBlob.h"

void SkTextUtils::Draw(SkCanvas* canvas, const void* text, size_t size, SkTextEncoding encoding,
                       SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint,
                       Align align) {
    if (align != kLeft_Align) {
        SkScalar width = font.measureText(text, size, encoding);
        if (align == kCenter_Align) {
            width *= 0.5f;
        }
        x -= width;
    }

    canvas->drawTextBlob(SkTextBlob::MakeFromText(text, size, font, encoding), x, y, paint);
}

