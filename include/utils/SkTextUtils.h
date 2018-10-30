/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextUtils_DEFINED
#define SkTextUtils_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkFont.h"
#include "SkString.h"

class SkTextUtils {
public:
    enum Align {
        kLeft_Align,
        kCenter_Align,
        kRight_Align,
    };

    static void DrawText(SkCanvas*, const void* text, size_t size, SkScalar x, SkScalar y,
                          const SkPaint&, Align = kLeft_Align);

    static void DrawString(SkCanvas* canvas, const char text[], SkScalar x, SkScalar y,
                           const SkPaint& paint, Align align = kLeft_Align) {
        DrawText(canvas, text, strlen(text), x, y, paint, align);
    }
    static void DrawString(SkCanvas* canvas, const SkString& str, SkScalar x, SkScalar y,
                           const SkPaint& paint, Align align = kLeft_Align) {
        DrawText(canvas, str.c_str(), str.size(), x, y, paint, align);
    }
};

#endif
