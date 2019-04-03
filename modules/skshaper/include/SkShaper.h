/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_DEFINED
#define SkShaper_DEFINED

#include "SkPoint.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkTextBlob.h"
#include "SkTypes.h"

#include <memory>

class SkFont;
class SkFontMgr;

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

    class FontRunIterator : public RunIterator {
    public:
        virtual const SkFont& currentFont() const = 0;
    };
    static std::unique_ptr<FontRunIterator>
    MakeFontMgrRunIterator(const char* utf8, size_t utf8Bytes,
                           const SkFont& font, sk_sp<SkFontMgr> fallback);

    class BiDiRunIterator : public RunIterator {
    public:
        /** The unicode bidi embedding level (even ltr, odd rtl) */
        virtual uint8_t currentLevel() const = 0;
    };
    #ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
    static std::unique_ptr<SkShaper::BiDiRunIterator>
    MakeIcuBiDiRunIterator(const char* utf8, size_t utf8Bytes, uint8_t bidiLevel);
    #endif

    class ScriptRunIterator : public RunIterator {
    public:
        /** Should be iso15924 codes. */
        virtual SkFourByteTag currentScript() const = 0;
    };
    #ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
    static std::unique_ptr<SkShaper::ScriptRunIterator>
    MakeHbIcuScriptRunIterator(const char* utf8, size_t utf8Bytes);
    #endif

    class LanguageRunIterator : public RunIterator {
    public:
        /** Should be BCP-47, c locale names may also work. */
        virtual const char* currentLanguage() const = 0;
    };
    static std::unique_ptr<SkShaper::LanguageRunIterator>
    MakeStdLanguageRunIterator(const char* utf8, size_t utf8Bytes);

    class RunHandler {
    public:
        virtual ~RunHandler() = default;

        struct Range {
            constexpr Range() : fBegin(0), fSize(0) {}
            constexpr Range(size_t begin, size_t size) : fBegin(begin), fSize(size) {}
            size_t fBegin;
            size_t fSize;
            constexpr size_t begin() const { return fBegin; }
            constexpr size_t end() const { return begin() + size(); }
            constexpr size_t size() const { return fSize; }
        };

        struct RunInfo {
            const SkFont& fFont;
            uint8_t fBidiLevel;
            SkVector fAdvance;
            size_t glyphCount;
            Range utf8Range;
        };

        struct Buffer {
            SkGlyphID* glyphs;  // required
            SkPoint* positions; // required
            SkPoint* offsets;   // optional
            uint32_t* clusters; // optional
            SkPoint point;
        };

        /** Called when beginning a line. */
        virtual void beginLine() = 0;

        /** Called once for each run in a line. Can compute baselines and offsets. */
        virtual void runInfo(const RunInfo&) = 0;

        /** Called after all runInfo calls for a line. */
        virtual void commitRunInfo() = 0;

        /** Called for each run in a line after commitRunInfo. The buffer will be filled out. */
        virtual Buffer runBuffer(const RunInfo&) = 0;

        /** Called after each runBuffer is filled out. */
        virtual void commitRunBuffer(const RunInfo&) = 0;

        /** Called when ending a line. */
        virtual void commitLine() = 0;
    };

    virtual void shape(const char* utf8, size_t utf8Bytes,
                       const SkFont& srcFont,
                       bool leftToRight,
                       SkScalar width,
                       RunHandler*) const = 0;

    virtual void shape(const char* utf8, size_t utf8Bytes,
                       FontRunIterator&,
                       BiDiRunIterator&,
                       ScriptRunIterator&,
                       LanguageRunIterator&,
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
    SkTextBlobBuilderRunHandler(const char* utf8Text, SkPoint offset)
        : fUtf8Text(utf8Text)
        , fOffset(offset) {}
    sk_sp<SkTextBlob> makeBlob();
    SkPoint endPoint() { return fOffset; }

    void beginLine() override;
    void runInfo(const RunInfo&) override;
    void commitRunInfo() override;
    Buffer runBuffer(const RunInfo&) override;
    void commitRunBuffer(const RunInfo&) override;
    void commitLine() override;

private:
    SkTextBlobBuilder fBuilder;
    char const * const fUtf8Text;
    uint32_t* fClusters;
    int fClusterOffset;
    int fGlyphCount;
    SkScalar fMaxRunAscent;
    SkScalar fMaxRunDescent;
    SkScalar fMaxRunLeading;
    SkPoint fCurrentPosition;
    SkPoint fOffset;
};

#endif  // SkShaper_DEFINED
