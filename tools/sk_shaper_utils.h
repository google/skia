// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef sk_shaper_utils_DEFINED
#define sk_shaper_utils_DEFINED

#include "SkScalar.h"
#include "SkString.h"

#include <cstddef>
#include <cstring>

class SkCanvas;
class SkFont;
class SkPaint;

void SkDrawShapedUTF8(SkCanvas* canvas, const char* text, size_t byteLength,
                    SkScalar x, SkScalar y, const SkFont& font,
                    const SkPaint& paint);

static inline void SkDrawShapedString(SkCanvas* canvas, const char* text,
                                    SkScalar x, SkScalar y, const SkFont& font,
                                    const SkPaint& paint) {
    SkDrawShapedUTF8(canvas, text, strlen(text), x, y, font, paint);
}

static inline void SkDrawShapedString(SkCanvas* canvas, const SkString& text,
                                    SkScalar x, SkScalar y, const SkFont& font,
                                    const SkPaint& paint) {
    SkDrawShapedUTF8(canvas, text.c_str(), text.size(), x, y, font, paint);
}
#endif  // sk_shaper_utils_DEFINED
