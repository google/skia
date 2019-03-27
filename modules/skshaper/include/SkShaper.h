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
    static std::unique_ptr<SkShaper> MakePrimitive();
    #ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
    static std::unique_ptr<SkShaper> MakeHarfBuzz();
    #endif

    static std::unique_ptr<SkShaper> Make();

    SkShaper();
    virtual ~SkShaper();

    class RunIterator {
    public:
        virtual ~RunIterator() = default;
        /** Set state to that of current run and move iterator to end of that run. */
        virtual void consume() = 0;
        /** Offset to one past the last (utf8) element in the current run. */
        virtual size_t endOfCurrentRun() const = 0;
        /** Return true if consume should no longer be called. */
        virtual bool atEnd() const = 0;
    };
    class BiDiRunIterator : public RunIterator {
    public:
        /** The unicode bidi embedding level (even ltr, odd rtl) */
        virtual uint8_t currentLevel() const = 0;
    };
    class ScriptRunIterator : public RunIterator {
    public:
        /** Should be iso15924 codes. */
        virtual SkFourByteTag currentScript() const = 0;
    };
    class LanguageRunIterator : public RunIterator {
    public:
        /** Should be BCP-47, c locale names may also work. */
        virtual const char* currentLanguage() const = 0;
    };
    class FontRunIterator : public RunIterator {
    public:
        virtual const SkFont& currentFont() const = 0;
    };

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

        struct Range {
            constexpr Range() : fBegin(0), fSize(0) {}
            constexpr Range(size_t begin, size_t size) : fBegin(begin), fSize(size) {}
            size_t fBegin;
            size_t fSize;
            constexpr size_t begin() const { return fBegin; }
            constexpr size_t end() const { return begin() + size(); }
            constexpr size_t size() const { return fSize; }
        };

        // Callback per glyph run.
        virtual Buffer newRunBuffer(const RunInfo&, const SkFont&, size_t glyphCount,
                                    Range utf8Range) = 0;

        // Called after run information is filled out.
        virtual void commitRun() = 0;
        // Callback per line.
        virtual void commitLine() = 0;
    };

    virtual SkPoint shape(RunHandler* handler,
                          const SkFont& srcFont,
                          const char* utf8, size_t utf8Bytes,
                          bool leftToRight,
                          SkPoint point,
                          SkScalar width) const = 0;

    virtual SkPoint shape(const char* utf8, size_t utf8Bytes,
                          FontRunIterator&,
                          BiDiRunIterator&,
                          ScriptRunIterator&,
                          LanguageRunIterator&,
                          SkPoint point,
                          SkScalar width,
                          RunHandler*) const = 0;

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

    Buffer newRunBuffer(const RunInfo&, const SkFont&, size_t, Range) override;
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
