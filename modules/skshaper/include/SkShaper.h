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
#include "SkTextBlob.h"
#include "SkTypeface.h"

class SkFont;

/**
   Shapes text using HarfBuzz and places the shaped text into a
   client-managed buffer.

   If compiled without HarfBuzz, fall back on SkPaint::textToGlyphs.
 */
class SkShaper {
public:
    SkShaper(sk_sp<SkTypeface> face);
    ~SkShaper();

    class LineHandler {
    public:
        virtual ~LineHandler() = default;

        struct Buffer {
            SkGlyphID* glyphs;    // required
            SkPoint*   positions; // required
            char*      utf8text;  // optional
            uint32_t*  clusters;  // optional
        };

        virtual Buffer newLineBuffer(const SkFont&, int glyphCount, int utf8textCount) = 0;
    };

    bool good() const;
    SkPoint shape(LineHandler* handler,
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

/**
 * Helper for shaping text directly into a SkTextBlob.
 */
class SkTextBlobBuilderLineHandler final : public SkShaper::LineHandler {
public:
    sk_sp<SkTextBlob> makeBlob();

    SkShaper::LineHandler::Buffer newLineBuffer(const SkFont&, int, int) override;

private:
    SkTextBlobBuilder fBuilder;
};

#endif  // SkShaper_DEFINED
