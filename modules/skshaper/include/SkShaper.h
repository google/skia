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
#include "SkSpan.h"
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
    static std::unique_ptr<SkShaper> MakePrimitive();
    #ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
    static std::unique_ptr<SkShaper> MakeHarfBuzz();
    #endif

    static std::unique_ptr<SkShaper> Make();

    SkShaper();
    virtual ~SkShaper();

    class RunHandler {
    public:
        virtual ~RunHandler() = default;

        struct RunInfo {
            SkVector fAdvance;
            SkScalar fAscent,
                     fDescent,
                     fLeading;
        };

        struct Buffer {
            SkGlyphID* glyphs;    // required
            SkPoint*   positions; // required
            uint32_t*  clusters;  // optional
        };

        // Callback per glyph run.
        virtual Buffer newRunBuffer(const RunInfo&, const SkFont&, int glyphCount,
                                    SkSpan<const char> utf8) = 0;

        // Called after run information is filled out.
        virtual void commitRun() = 0;
        // Callback per line.
        virtual void commitLine() = 0;
    };

    virtual SkPoint shape(RunHandler* handler,
                          const SkFont& srcFont,
                          const char* utf8text,
                          size_t textBytes,
                          bool leftToRight,
                          SkPoint point,
                          SkScalar width) const = 0;

private:
    SkShaper(const SkShaper&) = delete;
    SkShaper& operator=(const SkShaper&) = delete;
};

/**
 * Helper for shaping text directly into a SkTextBlob.
 */
class SkTextBlobBuilderRunHandler final : public SkShaper::RunHandler {
public:
    SkTextBlobBuilderRunHandler(const char* utf8Text) : fUtf8Text(utf8Text) {}
    sk_sp<SkTextBlob> makeBlob();

    SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo&, const SkFont&, int, SkSpan<const char>) override;
    void commitRun() override;
    void commitLine() override {}

private:
    SkTextBlobBuilder fBuilder;
    char const * const fUtf8Text;
    uint32_t* fClusters;
    int fClusterOffset;
    int fGlyphCount;
};

#endif  // SkShaper_DEFINED
