/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextUtils_DEFINED
#define SkTextUtils_DEFINED

#include "SkCanvas.h"
#include "SkFont.h"
#include "SkPaint.h"
#include "SkString.h"

class SkTextUtils {
public:
    enum Align {
        kLeft_Align,
        kCenter_Align,
        kRight_Align,
    };

    static void Draw(SkCanvas*, const void* text, size_t size, SkTextEncoding,
                     SkScalar x, SkScalar y, const SkFont&, const SkPaint&, Align = kLeft_Align);

    static void DrawString(SkCanvas* canvas, const char text[], SkScalar x, SkScalar y,
                           const SkFont& font, const SkPaint& paint, Align align = kLeft_Align) {
        SkASSERT(paint.getTextEncoding() == kUTF8_SkTextEncoding);
        Draw(canvas, text, strlen(text), kUTF8_SkTextEncoding, x, y, font, paint, align);
    }

#if 1
    static void DrawString(SkCanvas* canvas, const char text[], SkScalar x, SkScalar y,
                           const SkPaint& paint, Align align = kLeft_Align) {
        SkASSERT(paint.getTextEncoding() == kUTF8_SkTextEncoding);
        Draw(canvas, text, strlen(text), kUTF8_SkTextEncoding, x, y,
             SkFont::LEGACY_ExtractFromPaint(paint), paint, align);
    }

    static void DrawText(SkCanvas* canvas, const void* text, size_t size, SkScalar x, SkScalar y,
                         const SkPaint& paint, Align align = kLeft_Align) {
        Draw(canvas, text, size, paint.getTextEncoding(), x, y,
             SkFont::LEGACY_ExtractFromPaint(paint), paint, align);
    }

    static void DrawString(SkCanvas* canvas, const SkString& str, SkScalar x, SkScalar y,
                           const SkPaint& paint, Align align = kLeft_Align) {
        DrawText(canvas, str.c_str(), str.size(), x, y, paint, align);
    }
#endif
};

#endif
