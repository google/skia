/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_DEFINED
#define SkShaper_DEFINED

#include <memory>

#include "SkPoint.h"
#include "SkTypeface.h"

class SkFont;
class SkTextBlobBuilder;

/**
   Shapes text using HarfBuzz and passes the shaped text lines to a
   user-priveded handler.

   If compiled without HarfBuzz, fall back on SkPaint::textToGlyphs.
 */
class SkShaper {
public:
    SkShaper(sk_sp<SkTypeface> face);
    ~SkShaper();

    class LineHandler {
    public:
        virtual ~LineHandler() = default;

        virtual void operator()(const SkGlyphID glyphs[], int count, const SkPoint pos[]) = 0;
    };

    bool good() const;
    SkPoint shape(LineHandler* dest,
                  const SkFont& srcPaint,
                  const char* utf8text,
                  size_t textBytes,
                  bool leftToRight,
                  SkPoint point,
                  SkScalar width) const;

private:
    SkShaper(const SkShaper&) = delete;
    SkShaper& operator=(const SkShaper&) = delete;

    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

#endif  // SkShaper_DEFINED
