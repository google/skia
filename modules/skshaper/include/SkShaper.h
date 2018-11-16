/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_DEFINED
#define SkShaper_DEFINED

#include <cstring>
#include <memory>

#include "SkPoint.h"
#include "SkTypeface.h"

class SkCanvas;
class SkPaint;
class SkTextBlobBuilder;

/**
   Shapes text using HarfBuzz and places the shaped text into a
   TextBlob.

   If compiled without HarfBuzz, fall back on SkPaint::textToGlyphs.
 */
class SkShaper {
public:
    SkShaper(sk_sp<SkTypeface> face);
    ~SkShaper();

    bool good() const;
    SkPoint shape(SkTextBlobBuilder* dest,
                   const SkPaint& srcPaint,
                   const char* utf8text,
                   size_t textBytes,
                   bool leftToRight,
                   SkPoint point,
                   SkScalar width) const;

    static SkPoint DrawText(SkCanvas* canvas, const char* text, size_t size,
                            SkPoint xy, const SkPaint& paint, SkScalar width = FLT_MAX);

    static SkPoint DrawString(SkCanvas* canvas, const char* string,
                              SkPoint xy, const SkPaint& paint, SkScalar width = FLT_MAX) {
        return DrawText(canvas, string, strlen(string), xy, paint, width);
    }

private:
    SkShaper(const SkShaper&) = delete;
    SkShaper& operator=(const SkShaper&) = delete;

    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

#endif  // SkShaper_DEFINED
