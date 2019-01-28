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

    class RunHandler {
    public:
        virtual ~RunHandler() = default;

        struct RunInfo {
            size_t   fLineIndex;
            SkVector fAdvance;
            SkScalar fAscent,
                     fDescent,
                     fLeading;
        };

        struct Buffer {
            SkGlyphID* glyphs;    // required
            SkPoint*   positions; // required
            char*      utf8text;  // optional
            uint32_t*  clusters;  // optional
        };

        virtual Buffer newRunBuffer(const RunInfo&, const SkFont&, int glyphCount,
                                    int utf8textCount) = 0;
    };

    bool good() const;
    SkPoint shape(RunHandler* handler,
                  const SkFont& srcFont,
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
class SkTextBlobBuilderRunHandler final : public SkShaper::RunHandler {
public:
    sk_sp<SkTextBlob> makeBlob();

    SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo&, const SkFont&, int, int) override;

private:
    SkTextBlobBuilder fBuilder;
};

#endif  // SkShaper_DEFINED
