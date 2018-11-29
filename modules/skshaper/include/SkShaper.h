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
#include "SkFontTypes.h"
#include "SkTextUtils.h"

class SkFont;
class SkTextBlobBuilder;

struct SkText {
    const void* fText = nullptr;
    size_t fTextBytes = 0;
    SkTextEncoding fTextEncoding = kUTF8_SkTextEncoding;
};

SkPoint SkShape(SkTextBlobBuilder* dest,
                const SkFont& font,
                SkText text,
                bool leftToRight,
                SkTextUtils::Align align,
                SkPoint point,
                SkScalar width);

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
