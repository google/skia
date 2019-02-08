/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmaskEnum.h"
#include "SkFont.h"
#include "SkFontArguments.h"
#include "SkFontMetrics.h"
#include "SkFontMgr.h"
#include "SkMalloc.h"
#include "SkPoint.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkShaper.h"
#include "SkSpan.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDPQueue.h"
#include "SkTFitsIn.h"
#include "SkTLazy.h"
#include "SkTemplates.h"
#include "SkTo.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "SkUTF.h"

#include <hb.h>
#include <hb-ot.h>
#include <unicode/ubrk.h>
#include <unicode/ubidi.h>
#include <unicode/ustring.h>
#include <unicode/urename.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include <cstring>
#include <locale>
#include <memory>
#include <utility>

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

namespace skstd {
template <> struct is_bitmask_enum<hb_buffer_flags_t> : std::true_type {};
}

namespace {
template <class T, void(*P)(T*)> using resource = std::unique_ptr<T, SkFunctionWrapper<void, T, P>>;
using HBBlob   = resource<hb_blob_t     , hb_blob_destroy  >;
using HBFace   = resource<hb_face_t     , hb_face_destroy  >;
using HBFont   = resource<hb_font_t     , hb_font_destroy  >;
using HBBuffer = resource<hb_buffer_t   , hb_buffer_destroy>;
using ICUBiDi  = resource<UBiDi         , ubidi_close      >;
using ICUBrk   = resource<UBreakIterator, ubrk_close       >;

HBBlob stream_to_blob(std::unique_ptr<SkStreamAsset> asset) {
    size_t size = asset->getLength();
    HBBlob blob;
    if (const void* base = asset->getMemoryBase()) {
        blob.reset(hb_blob_create((char*)base, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, asset.release(),
                                  [](void* p) { delete (SkStreamAsset*)p; }));
    } else {
        // SkDebugf("Extra SkStreamAsset copy\n");
        void* ptr = size ? sk_malloc_throw(size) : nullptr;
        asset->read(ptr, size);
        blob.reset(hb_blob_create((char*)ptr, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, ptr, sk_free));
    }
    SkASSERT(blob);
    hb_blob_make_immutable(blob.get());
    return blob;
}

HBFont create_hb_font(SkTypeface* tf) {
    if (!tf) {
        return nullptr;
    }
    int index;
    std::unique_ptr<SkStreamAsset> typefaceAsset(tf->openStream(&index));
    if (!typefaceAsset) {
        SkString name;
        tf->getFamilyName(&name);
        SkDebugf("Typeface '%s' has no data :(\n", name.c_str());
        return nullptr;
    }
    HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
    HBFace face(hb_face_create(blob.get(), (unsigned)index));
    SkASSERT(face);
    if (!face) {
        return nullptr;
    }
    hb_face_set_index(face.get(), (unsigned)index);
    hb_face_set_upem(face.get(), tf->getUnitsPerEm());

    HBFont font(hb_font_create(face.get()));
    SkASSERT(font);
    if (!font) {
        return nullptr;
    }
    hb_ot_font_set_funcs(font.get());
    int axis_count = tf->getVariationDesignPosition(nullptr, 0);
    if (axis_count > 0) {
        SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> axis_values(axis_count);
        if (tf->getVariationDesignPosition(axis_values, axis_count) == axis_count) {
            hb_font_set_variations(font.get(),
                                   reinterpret_cast<hb_variation_t*>(axis_values.get()),
                                   axis_count);
        }
    }
    return font;
}

/** this version replaces invalid utf-8 sequences with code point U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    if (val < 0) {
        return 0xFFFD;  // REPLACEMENT CHARACTER
    }
    return val;
}

class RunIterator {
public:
    virtual ~RunIterator() {}
    virtual void consume() = 0;
    // Pointer one past the last (utf8) element in the current run.
    virtual const char* endOfCurrentRun() const = 0;
    virtual bool atEnd() const = 0;
    bool operator<(const RunIterator& that) const {
        return this->endOfCurrentRun() < that.endOfCurrentRun();
    }
};

class BiDiRunIterator : public RunIterator {
public:
    static SkTLazy<BiDiRunIterator> Make(const char* utf8, size_t utf8Bytes, UBiDiLevel level) {
        SkTLazy<BiDiRunIterator> ret;

        // ubidi only accepts utf16 (though internally it basically works on utf32 chars).
        // We want an ubidi_setPara(UBiDi*, UText*, UBiDiLevel, UBiDiLevel*, UErrorCode*);
        if (!SkTFitsIn<int32_t>(utf8Bytes)) {
            SkDebugf("Bidi error: text too long");
            return ret;
        }

        UErrorCode status = U_ZERO_ERROR;

        // Getting the length like this seems to always set U_BUFFER_OVERFLOW_ERROR
        int32_t utf16Units;
        u_strFromUTF8(nullptr, 0, &utf16Units, utf8, utf8Bytes, &status);
        status = U_ZERO_ERROR;
        std::unique_ptr<UChar[]> utf16(new UChar[utf16Units]);
        u_strFromUTF8(utf16.get(), utf16Units, nullptr, utf8, utf8Bytes, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Invalid utf8 input: %s", u_errorName(status));
            return ret;
        }

        ICUBiDi bidi(ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDebugf("Bidi error: %s", u_errorName(status));
            return ret;
        }
        SkASSERT(bidi);

        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        ubidi_setPara(bidi.get(), utf16.get(), utf16Units, level, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Bidi error: %s", u_errorName(status));
            return ret;
        }

        ret.init(utf8, utf8 + utf8Bytes, std::move(bidi));
        return ret;
    }
    BiDiRunIterator(const char* utf8, const char* end, ICUBiDi bidi)
        : fBidi(std::move(bidi))
        , fEndOfCurrentRun(utf8)
        , fEndOfAllRuns(end)
        , fUTF16LogicalPosition(0)
        , fLevel(UBIDI_DEFAULT_LTR)
    {}
    void consume() override {
        SkASSERT(fUTF16LogicalPosition < ubidi_getLength(fBidi.get()));
        int32_t endPosition = ubidi_getLength(fBidi.get());
        fLevel = ubidi_getLevelAt(fBidi.get(), fUTF16LogicalPosition);
        SkUnichar u = utf8_next(&fEndOfCurrentRun, fEndOfAllRuns);
        fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        UBiDiLevel level;
        while (fUTF16LogicalPosition < endPosition) {
            level = ubidi_getLevelAt(fBidi.get(), fUTF16LogicalPosition);
            if (level != fLevel) {
                break;
            }
            u = utf8_next(&fEndOfCurrentRun, fEndOfAllRuns);
            fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        }
    }
    const char* endOfCurrentRun() const override {
        return fEndOfCurrentRun;
    }
    bool atEnd() const override {
        return fUTF16LogicalPosition == ubidi_getLength(fBidi.get());
    }

    UBiDiLevel currentLevel() const {
        return fLevel;
    }
private:
    ICUBiDi fBidi;
    const char* fEndOfCurrentRun;
    const char* fEndOfAllRuns;
    int32_t fUTF16LogicalPosition;
    UBiDiLevel fLevel;
};

class ScriptRunIterator : public RunIterator {
public:
    static SkTLazy<ScriptRunIterator> Make(const char* utf8, size_t utf8Bytes,
                                           hb_unicode_funcs_t* hbUnicode)
    {
        SkTLazy<ScriptRunIterator> ret;
        ret.init(utf8, utf8Bytes, hbUnicode);
        return ret;
    }
    ScriptRunIterator(const char* utf8, size_t utf8Bytes, hb_unicode_funcs_t* hbUnicode)
        : fCurrent(utf8), fEnd(fCurrent + utf8Bytes)
        , fHBUnicode(hbUnicode)
        , fCurrentScript(HB_SCRIPT_UNKNOWN)
    {}
    void consume() override {
        SkASSERT(fCurrent < fEnd);
        SkUnichar u = utf8_next(&fCurrent, fEnd);
        fCurrentScript = hb_unicode_script(fHBUnicode, u);
        while (fCurrent < fEnd) {
            const char* prev = fCurrent;
            u = utf8_next(&fCurrent, fEnd);
            const hb_script_t script = hb_unicode_script(fHBUnicode, u);
            if (script != fCurrentScript) {
                if (fCurrentScript == HB_SCRIPT_INHERITED || fCurrentScript == HB_SCRIPT_COMMON) {
                    fCurrentScript = script;
                } else if (script == HB_SCRIPT_INHERITED || script == HB_SCRIPT_COMMON) {
                    continue;
                } else {
                    fCurrent = prev;
                    break;
                }
            }
        }
        if (fCurrentScript == HB_SCRIPT_INHERITED) {
            fCurrentScript = HB_SCRIPT_COMMON;
        }
    }
    const char* endOfCurrentRun() const override {
        return fCurrent;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    hb_script_t currentScript() const {
        return fCurrentScript;
    }
private:
    const char* fCurrent;
    const char* fEnd;
    hb_unicode_funcs_t* fHBUnicode;
    hb_script_t fCurrentScript;
};

class FontRunIterator : public RunIterator {
public:
    static SkTLazy<FontRunIterator> Make(const char* utf8, size_t utf8Bytes,
                                         SkFont font,
                                         sk_sp<SkFontMgr> fallbackMgr)
    {
        SkTLazy<FontRunIterator> ret;
        font.setTypeface(font.refTypefaceOrDefault());
        HBFont hbFont = create_hb_font(font.getTypeface());
        if (!hbFont) {
            SkDebugf("create_hb_font failed!\n");
            return ret;
        }
        ret.init(utf8, utf8Bytes, std::move(font), std::move(hbFont), std::move(fallbackMgr));
        return ret;
    }
    FontRunIterator(const char* utf8, size_t utf8Bytes, SkFont font,
                    HBFont hbFont, sk_sp<SkFontMgr> fallbackMgr)
        : fCurrent(utf8), fEnd(fCurrent + utf8Bytes)
        , fFallbackMgr(std::move(fallbackMgr))
        , fHBFont(std::move(hbFont)), fFont(std::move(font))
        , fFallbackHBFont(nullptr), fFallbackFont(fFont)
        , fCurrentHBFont(fHBFont.get()), fCurrentFont(&fFont)
    {
        fFallbackFont.setTypeface(nullptr);
    }
    void consume() override {
        SkASSERT(fCurrent < fEnd);
        SkUnichar u = utf8_next(&fCurrent, fEnd);
        // If the starting typeface can handle this character, use it.
        if (fFont.getTypeface()->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1)) {
            fCurrentFont = &fFont;
            fCurrentHBFont = fHBFont.get();
        // If the current fallback can handle this character, use it.
        } else if (fFallbackFont.getTypeface() &&
                   fFallbackFont.getTypeface()->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1))
        {
            fCurrentFont = &fFallbackFont;
            fCurrentHBFont = fFallbackHBFont.get();
        // If not, try to find a fallback typeface
        } else {
            fFallbackFont.setTypeface(sk_ref_sp(fFallbackMgr->matchFamilyStyleCharacter(
                nullptr, fFont.getTypeface()->fontStyle(), nullptr, 0, u)));
            fFallbackHBFont = create_hb_font(fFallbackFont.getTypeface());
            fCurrentFont = &fFallbackFont;
            fCurrentHBFont = fFallbackHBFont.get();
        }

        while (fCurrent < fEnd) {
            const char* prev = fCurrent;
            u = utf8_next(&fCurrent, fEnd);

            // If not using initial typeface and initial typeface has this character, stop fallback.
            if (fCurrentFont->getTypeface() != fFont.getTypeface() &&
                fFont.getTypeface()->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1))
            {
                fCurrent = prev;
                return;
            }
            // If the current typeface cannot handle this character, stop using it.
            if (!fCurrentFont->getTypeface()->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1)) {
                fCurrent = prev;
                return;
            }
        }
    }
    const char* endOfCurrentRun() const override {
        return fCurrent;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    SkFont* currentFont() const {
        return fCurrentFont;
    }
    hb_font_t* currentHBFont() const {
        return fCurrentHBFont;
    }
private:
    const char* fCurrent;
    const char* fEnd;
    sk_sp<SkFontMgr> fFallbackMgr;
    HBFont fHBFont;
    SkFont fFont;
    HBFont fFallbackHBFont;
    SkFont fFallbackFont;
    hb_font_t* fCurrentHBFont;
    SkFont* fCurrentFont;
};

class LanguageRunIterator : public RunIterator {
public:
    static SkTLazy<LanguageRunIterator> Make(const char* utf8, size_t utf8Bytes) {
        SkTLazy<LanguageRunIterator> ret;
        ret.init(utf8, utf8Bytes);
        return ret;
    }
    LanguageRunIterator(const char* utf8, size_t utf8Bytes)
        : fCurrent(utf8), fEnd(fCurrent + utf8Bytes)
    , fLanguage(hb_language_from_string(std::locale().name().c_str(), -1))
    { }
    void consume() override {
        // Ideally something like cld2/3 could be used, or user signals.
        SkASSERT(fCurrent < fEnd);
        fCurrent = fEnd;
    }
    const char* endOfCurrentRun() const override {
        return fCurrent;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    hb_language_t currentLanguage() const {
        return fLanguage;
    }
private:
    const char* fCurrent;
    const char* fEnd;
    hb_language_t fLanguage;
};

class RunIteratorQueue {
public:
    void insert(RunIterator* runIterator) {
        fRunIterators.insert(runIterator);
    }

    bool advanceRuns() {
        const RunIterator* leastRun = fRunIterators.peek();
        if (leastRun->atEnd()) {
            SkASSERT(this->allRunsAreAtEnd());
            return false;
        }
        const char* leastEnd = leastRun->endOfCurrentRun();
        RunIterator* currentRun = nullptr;
        SkDEBUGCODE(const char* previousEndOfCurrentRun);
        while ((currentRun = fRunIterators.peek())->endOfCurrentRun() <= leastEnd) {
            fRunIterators.pop();
            SkDEBUGCODE(previousEndOfCurrentRun = currentRun->endOfCurrentRun());
            currentRun->consume();
            SkASSERT(previousEndOfCurrentRun < currentRun->endOfCurrentRun());
            fRunIterators.insert(currentRun);
        }
        return true;
    }

    const char* endOfCurrentRun() const {
        return fRunIterators.peek()->endOfCurrentRun();
    }

private:
    bool allRunsAreAtEnd() const {
        for (int i = 0; i < fRunIterators.count(); ++i) {
            if (!fRunIterators.at(i)->atEnd()) {
                return false;
            }
        }
        return true;
    }

    static bool CompareRunIterator(RunIterator* const& a, RunIterator* const& b) {
        return *a < *b;
    }
    SkTDPQueue<RunIterator*, CompareRunIterator> fRunIterators;
};

struct ShapedGlyph {
    SkGlyphID fID;
    uint32_t fCluster;
    SkPoint fOffset;
    SkVector fAdvance;
    bool fMayLineBreakBefore;
    bool fMustLineBreakBefore;
    bool fHasVisual;
    bool fGraphemeBreakBefore;
    bool fUnsafeToBreak;
};
struct ShapedRun {
    ShapedRun(SkSpan<const char> utf8, const SkFont& font, UBiDiLevel level,
              std::unique_ptr<ShapedGlyph[]> glyphs, int numGlyphs)
        : fUtf8(utf8), fFont(font), fLevel(level)
        , fGlyphs(std::move(glyphs)), fNumGlyphs(numGlyphs)
    {}

    SkSpan<const char> fUtf8;
    SkFont fFont;
    UBiDiLevel fLevel;
    std::unique_ptr<ShapedGlyph[]> fGlyphs;
    int fNumGlyphs;
    SkVector fAdvance = { 0, 0 };
};
struct ShapedLine {
    SkTArray<ShapedRun> runs;
    SkVector fAdvance = { 0, 0 };
};

static constexpr bool is_LTR(UBiDiLevel level) {
    return (level & 1) == 0;
}

static void append(SkShaper::RunHandler* handler, const SkShaper::RunHandler::RunInfo& runInfo,
                   const ShapedRun& run, int start, int end,
                   SkPoint* p) {
    const unsigned len = end - start;

    const auto buffer = handler->newRunBuffer(runInfo, run.fFont, len, run.fUtf8);
    SkASSERT(buffer.glyphs);
    SkASSERT(buffer.positions);

    for (unsigned i = 0; i < len; i++) {
        // Glyphs are in logical order, but output ltr since PDF readers seem to expect that.
        const ShapedGlyph& glyph = run.fGlyphs[is_LTR(run.fLevel) ? start + i : end - 1 - i];
        buffer.glyphs[i] = glyph.fID;
        buffer.positions[i] = SkPoint::Make(p->fX + glyph.fOffset.fX, p->fY - glyph.fOffset.fY);
        if (buffer.clusters) {
            buffer.clusters[i] = glyph.fCluster;
        }
        p->fX += glyph.fAdvance.fX;
        p->fY += glyph.fAdvance.fY;
    }
    handler->commitRun();
}

static void emit(const ShapedLine& line, SkShaper::RunHandler* handler,
                 SkPoint point, SkPoint& currentPoint)
{
    // Reorder the runs and glyphs per line and write them out.
    SkScalar maxAscent = 0;
    SkScalar maxDescent = 0;
    SkScalar maxLeading = 0;
    for (const ShapedRun& run : line.runs) {
        SkFontMetrics metrics;
        run.fFont.getMetrics(&metrics);
        maxAscent = SkTMin(maxAscent, metrics.fAscent);
        maxDescent = SkTMax(maxDescent, metrics.fDescent);
        maxLeading = SkTMax(maxLeading, metrics.fLeading);
    }

    int numRuns = line.runs.size();
    SkAutoSTMalloc<4, UBiDiLevel> runLevels(numRuns);
    for (int i = 0; i < numRuns; ++i) {
        runLevels[i] = line.runs[i].fLevel;
    }
    SkAutoSTMalloc<4, int32_t> logicalFromVisual(numRuns);
    ubidi_reorderVisual(runLevels, numRuns, logicalFromVisual);

    currentPoint.fY -= maxAscent;

    for (int i = 0; i < numRuns; ++i) {
        int logicalIndex = logicalFromVisual[i];

        const auto& run = line.runs[logicalIndex];
        const SkShaper::RunHandler::RunInfo info = {
            run.fAdvance,
            maxAscent,
            maxDescent,
            maxLeading,
        };
        append(handler, info, run, 0, run.fNumGlyphs, &currentPoint);
    }

    currentPoint.fY += maxDescent + maxLeading;
    currentPoint.fX = point.fX;

    handler->commitLine();
}

struct ShapedRunGlyphIterator {
    ShapedRunGlyphIterator(const SkTArray<ShapedRun>& origRuns)
        : fRuns(&origRuns), fRunIndex(0), fGlyphIndex(0)
    { }

    ShapedRunGlyphIterator(const ShapedRunGlyphIterator& that) = default;
    ShapedRunGlyphIterator& operator=(const ShapedRunGlyphIterator& that) = default;
    bool operator==(const ShapedRunGlyphIterator& that) const {
        return fRuns == that.fRuns &&
               fRunIndex == that.fRunIndex &&
               fGlyphIndex == that.fGlyphIndex;
    }
    bool operator!=(const ShapedRunGlyphIterator& that) const {
        return fRuns != that.fRuns ||
               fRunIndex != that.fRunIndex ||
               fGlyphIndex != that.fGlyphIndex;
    }

    ShapedGlyph* next() {
        const SkTArray<ShapedRun>& runs = *fRuns;
        SkASSERT(fRunIndex < runs.count());
        SkASSERT(fGlyphIndex < runs[fRunIndex].fNumGlyphs);

        ++fGlyphIndex;
        if (fGlyphIndex == runs[fRunIndex].fNumGlyphs) {
            fGlyphIndex = 0;
            ++fRunIndex;
            if (fRunIndex >= runs.count()) {
                return nullptr;
            }
        }
        return &runs[fRunIndex].fGlyphs[fGlyphIndex];
    }

    ShapedGlyph* current() {
        const SkTArray<ShapedRun>& runs = *fRuns;
        if (fRunIndex >= runs.count()) {
            return nullptr;
        }
        return &runs[fRunIndex].fGlyphs[fGlyphIndex];
    }

    const SkTArray<ShapedRun>* fRuns;
    int fRunIndex;
    int fGlyphIndex;
};

}  // namespace

struct SkShaper::Impl {
    HBFont fHarfBuzzFont;
    HBBuffer fBuffer;
    sk_sp<SkTypeface> fTypeface;
    ICUBrk fLineBreakIterator;
    ICUBrk fGraphemeBreakIterator;

    SkPoint shapeCorrect(RunHandler* handler,
                         const char* utf8,
                         size_t utf8Bytes,
                         SkPoint point,
                         SkScalar width,
                         RunIteratorQueue& runSegmenter,
                         const BiDiRunIterator* bidi,
                         const LanguageRunIterator* language,
                         const ScriptRunIterator* script,
                         const FontRunIterator* font) const;

    SkPoint shapeOk(RunHandler* handler,
                    const char* utf8,
                    size_t utf8Bytes,
                    SkPoint point,
                    SkScalar width,
                    RunIteratorQueue& runSegmenter,
                    const BiDiRunIterator* bidi,
                    const LanguageRunIterator* language,
                    const ScriptRunIterator* script,
                    const FontRunIterator* font) const;

    ShapedRun shape(const char* utf8,
                    size_t utf8Bytes,
                    const char* utf8Start,
                    const char* utf8End,
                    const BiDiRunIterator* bidi,
                    const LanguageRunIterator* language,
                    const ScriptRunIterator* script,
                    const FontRunIterator* font) const;
};

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(new Impl) {
#if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        SkDebugf("SkLoadICU() failed!\n");
        return;
    }
#endif
    fImpl->fTypeface = tf ? std::move(tf) : SkTypeface::MakeDefault();
    fImpl->fHarfBuzzFont = create_hb_font(fImpl->fTypeface.get());
    if (!fImpl->fHarfBuzzFont) {
        SkDebugf("create_hb_font failed!\n");
    }
    fImpl->fBuffer.reset(hb_buffer_create());
    SkASSERT(fImpl->fBuffer);

    UErrorCode status = U_ZERO_ERROR;
    fImpl->fLineBreakIterator.reset(ubrk_open(UBRK_LINE, "th", nullptr, 0, &status));
    if (U_FAILURE(status)) {
        SkDebugf("Could not create line break iterator: %s", u_errorName(status));
        SK_ABORT("");
    }

    fImpl->fGraphemeBreakIterator.reset(ubrk_open(UBRK_CHARACTER, "th", nullptr, 0, &status));
    if (U_FAILURE(status)) {
        SkDebugf("Could not create grapheme break iterator: %s", u_errorName(status));
        SK_ABORT("");
    }

}

SkShaper::~SkShaper() {}

bool SkShaper::good() const {
    return fImpl->fBuffer &&
           fImpl->fLineBreakIterator &&
           fImpl->fGraphemeBreakIterator;
}

SkPoint SkShaper::shape(RunHandler* handler,
                        const SkFont& srcFont,
                        const char* utf8,
                        size_t utf8Bytes,
                        bool leftToRight,
                        SkPoint point,
                        SkScalar width) const
{
    SkASSERT(handler);
    sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
    UBiDiLevel defaultLevel = leftToRight ? UBIDI_DEFAULT_LTR : UBIDI_DEFAULT_RTL;

    RunIteratorQueue runSegmenter;

    SkTLazy<BiDiRunIterator> maybeBidi(BiDiRunIterator::Make(utf8, utf8Bytes, defaultLevel));
    BiDiRunIterator* bidi = maybeBidi.getMaybeNull();
    if (!bidi) {
        return point;
    }
    runSegmenter.insert(bidi);

    SkTLazy<LanguageRunIterator> maybeLanguage(LanguageRunIterator::Make(utf8, utf8Bytes));
    LanguageRunIterator* language = maybeLanguage.getMaybeNull();
    if (!language) {
        return point;
    }
    runSegmenter.insert(language);

    hb_unicode_funcs_t* hbUnicode = hb_buffer_get_unicode_funcs(fImpl->fBuffer.get());
    SkTLazy<ScriptRunIterator> maybeScript(ScriptRunIterator::Make(utf8, utf8Bytes, hbUnicode));
    ScriptRunIterator* script = maybeScript.getMaybeNull();
    if (!script) {
        return point;
    }
    runSegmenter.insert(script);

    SkTLazy<FontRunIterator> maybeFont(FontRunIterator::Make(utf8, utf8Bytes,
                                                             srcFont, std::move(fontMgr)));
    FontRunIterator* font = maybeFont.getMaybeNull();
    if (!font) {
        return point;
    }
    runSegmenter.insert(font);

    if (true) {
        return fImpl->shapeCorrect(handler, utf8, utf8Bytes, point, width,
                                   runSegmenter, bidi, language, script, font);
    } else {
        return fImpl->shapeOk(handler, utf8, utf8Bytes, point, width,
                              runSegmenter, bidi, language, script, font);
    }
}

SkPoint SkShaper::Impl::shapeCorrect(RunHandler* handler,
                                     const char* utf8,
                                     size_t utf8Bytes,
                                     SkPoint point,
                                     SkScalar width,
                                     RunIteratorQueue& runSegmenter,
                                     const BiDiRunIterator* bidi,
                                     const LanguageRunIterator* language,
                                     const ScriptRunIterator* script,
                                     const FontRunIterator* font) const
{
    ShapedLine line;
    SkPoint currentPoint = point;

    const char* utf8Start = nullptr;
    const char* utf8End = utf8;
    while (runSegmenter.advanceRuns()) {  // For each item
        utf8Start = utf8End;
        utf8End = runSegmenter.endOfCurrentRun();

        ShapedRun model(SkSpan<const char>(), SkFont(), 0, nullptr, 0);
        bool modelNeedsRegenerated = true;
        int modelOffset = 0;

        struct TextProps {
            int glyphLen = 0;
            SkVector advance = {0, 0};
        };
        // map from character position to [safe to break, glyph position, advance]
        std::unique_ptr<TextProps[]> modelText;
        int modelTextOffset = 0;
        SkVector modelTextAdvanceOffset = {0, 0};

        while (utf8Start < utf8End) {  // While there are still code points left in this item
            size_t utf8runLength = utf8End - utf8Start;
            if (modelNeedsRegenerated) {
                model = shape(utf8, utf8Bytes,
                              utf8Start, utf8End,
                              bidi, language, script, font);
                modelOffset = 0;

                SkVector advance = {0, 0};
                modelText.reset(new TextProps[utf8runLength + 1]());
                for (int i = 0; i < model.fNumGlyphs; ++i) {
                    SkASSERT(model.fGlyphs[i].fCluster < utf8runLength);
                    if (!model.fGlyphs[i].fUnsafeToBreak) {
                        modelText[model.fGlyphs[i].fCluster].glyphLen = i;
                        modelText[model.fGlyphs[i].fCluster].advance = advance;
                    }
                    advance += model.fGlyphs[i].fAdvance;
                }
                // Assume it is always safe to break after the end of an item
                modelText[utf8runLength].glyphLen = model.fNumGlyphs;
                modelText[utf8runLength].advance = model.fAdvance;
                modelTextOffset = 0;
                modelTextAdvanceOffset = {0, 0};
                modelNeedsRegenerated = false;
            }

            // TODO: break iterator per item, but just reset position if needed?
            // Maybe break iterator with model?
            UBreakIterator& breakIterator = *fLineBreakIterator;
            {
                UErrorCode status = U_ZERO_ERROR;
                UText utf8UText = UTEXT_INITIALIZER;
                utext_openUTF8(&utf8UText, utf8Start, utf8runLength, &status);
                std::unique_ptr<UText, SkFunctionWrapper<UText*, UText, utext_close>> autoClose(&utf8UText);
                if (U_FAILURE(status)) {
                    SkDebugf("Could not create utf8UText: %s", u_errorName(status));
                    return point;
                }
                ubrk_setUText(&breakIterator, &utf8UText, &status);
                if (U_FAILURE(status)) {
                    SkDebugf("Could not setText on break iterator: %s", u_errorName(status));
                    return point;
                }
            }

            ShapedRun best(SkSpan<const char>(), SkFont(), 0, nullptr, 0);
            best.fAdvance = { SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity };
            SkScalar widthLeft = width - line.fAdvance.fX;

            for (int32_t breakIteratorCurrent = ubrk_next(&breakIterator);
                 breakIteratorCurrent != UBRK_DONE;
                 breakIteratorCurrent = ubrk_next(&breakIterator))
            {
                // TODO: if past a safe to break, future safe to break will be at least as long

                // TODO: adjust breakIteratorCurrent by ignorable whitespace
                ShapedRun candidate = modelText[breakIteratorCurrent + modelTextOffset].glyphLen
                                    ? ShapedRun(SkSpan<const char>(utf8Start, breakIteratorCurrent),
                                                *font->currentFont(), bidi->currentLevel(),
                                                std::unique_ptr<ShapedGlyph[]>(),
                                                modelText[breakIteratorCurrent + modelTextOffset].glyphLen - modelOffset)
                                    : shape(utf8, utf8Bytes,
                                            utf8Start, utf8Start + breakIteratorCurrent,
                                            bidi, language, script, font);
                if (!candidate.fUtf8.data()) {
                    //report error
                    return point;
                }
                if (!candidate.fGlyphs) {
                    candidate.fAdvance = modelText[breakIteratorCurrent + modelTextOffset].advance - modelTextAdvanceOffset;
                }
                auto score = [widthLeft](const ShapedRun& run) -> SkScalar {
                    if (run.fAdvance.fX < widthLeft) {
                        if (run.fUtf8.data() == nullptr) {
                            return SK_ScalarNegativeInfinity;
                        } else {
                            return run.fUtf8.size();
                        }
                    } else {
                        return widthLeft - run.fAdvance.fX;
                    }
                };
                if (score(best) < score(candidate)) {
                    best = std::move(candidate);
                }
            }

            // If nothing fit (best score is negative) and the line is not empty
            if (width < line.fAdvance.fX + best.fAdvance.fX && !line.runs.empty()) {
                emit(line, handler, point, currentPoint);
                line.runs.reset();
                line.fAdvance = {0, 0};
            } else {
                if (!best.fGlyphs) {
                    best.fGlyphs.reset(new ShapedGlyph[best.fNumGlyphs]);
                    memcpy(best.fGlyphs.get(), model.fGlyphs.get() + modelOffset,
                           best.fNumGlyphs * sizeof(ShapedGlyph));
                    modelOffset += best.fNumGlyphs;
                    modelTextOffset += best.fUtf8.size();
                    modelTextAdvanceOffset += best.fAdvance;
                } else {
                    modelNeedsRegenerated = true;
                }
                utf8Start = best.fUtf8.end();
                line.fAdvance += best.fAdvance;
                line.runs.emplace_back(std::move(best));

                // If item broken, emit line (prevent remainder from accidentally fitting)
                if (utf8Start != utf8End) {
                    emit(line, handler, point, currentPoint);
                    line.runs.reset();
                    line.fAdvance = {0, 0};
                }
            }
        }
    }
    emit(line, handler, point, currentPoint);
    return currentPoint;
}

SkPoint SkShaper::Impl::shapeOk(RunHandler* handler,
                                const char* utf8,
                                size_t utf8Bytes,
                                SkPoint point,
                                SkScalar width,
                                RunIteratorQueue& runSegmenter,
                                const BiDiRunIterator* bidi,
                                const LanguageRunIterator* language,
                                const ScriptRunIterator* script,
                                const FontRunIterator* font) const
{
    SkTArray<ShapedRun> runs;
{
    UBreakIterator& lineBreakIterator = *fLineBreakIterator;
    UBreakIterator& graphemeBreakIterator = *fGraphemeBreakIterator;
    {
        UErrorCode status = U_ZERO_ERROR;
        UText utf8UText = UTEXT_INITIALIZER;
        utext_openUTF8(&utf8UText, utf8, utf8Bytes, &status);
        std::unique_ptr<UText, SkFunctionWrapper<UText*, UText, utext_close>> autoClose(&utf8UText);
        if (U_FAILURE(status)) {
            SkDebugf("Could not create utf8UText: %s", u_errorName(status));
            return point;
        }

        ubrk_setUText(&lineBreakIterator, &utf8UText, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Could not setText on line break iterator: %s", u_errorName(status));
            return point;
        }
        ubrk_setUText(&graphemeBreakIterator, &utf8UText, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Could not setText on grapheme break iterator: %s", u_errorName(status));
            return point;
        }
    }

    const char* utf8Start = nullptr;
    const char* utf8End = utf8;
    while (runSegmenter.advanceRuns()) {
        utf8Start = utf8End;
        utf8End = runSegmenter.endOfCurrentRun();

        runs.emplace_back(shape(utf8, utf8Bytes,
                                utf8Start, utf8End,
                                bidi, language, script, font));
        ShapedRun& run = runs.back();

        int32_t clusterOffset = utf8Start - utf8;
        uint32_t previousCluster = 0xFFFFFFFF;
        for (int i = 0; i < run.fNumGlyphs; ++i) {
            ShapedGlyph& glyph = run.fGlyphs[i];
            int32_t glyphCluster = glyph.fCluster + clusterOffset;

            int32_t lineBreakIteratorCurrent = ubrk_current(&lineBreakIterator);
            while (lineBreakIteratorCurrent != UBRK_DONE &&
                   lineBreakIteratorCurrent < glyphCluster)
            {
                lineBreakIteratorCurrent = ubrk_next(&lineBreakIterator);
            }
            glyph.fMayLineBreakBefore = glyph.fCluster != previousCluster &&
                                        lineBreakIteratorCurrent == glyphCluster;

            int32_t graphemeBreakIteratorCurrent = ubrk_current(&graphemeBreakIterator);
            while (graphemeBreakIteratorCurrent != UBRK_DONE &&
                   graphemeBreakIteratorCurrent < glyphCluster)
            {
                graphemeBreakIteratorCurrent = ubrk_next(&graphemeBreakIterator);
            }
            glyph.fGraphemeBreakBefore = glyph.fCluster != previousCluster &&
                                         graphemeBreakIteratorCurrent == glyphCluster;

            previousCluster = glyph.fCluster;
        }
    }
}

// Iterate over the glyphs in logical order to find potential line lengths.
{
    /** The position of the beginning of the line. */
    ShapedRunGlyphIterator beginning(runs);

    /** The position of the candidate line break. */
    ShapedRunGlyphIterator candidateLineBreak(runs);
    SkScalar candidateLineBreakWidth = 0;

    /** The position of the candidate grapheme break. */
    ShapedRunGlyphIterator candidateGraphemeBreak(runs);
    SkScalar candidateGraphemeBreakWidth = 0;

    /** The position of the current location. */
    ShapedRunGlyphIterator current(runs);
    SkScalar currentWidth = 0;
    while (ShapedGlyph* glyph = current.current()) {
        // 'Break' at graphemes until a line boundary, then only at line boundaries.
        // Only break at graphemes if no line boundary is valid.
        if (current != beginning) {
            if (glyph->fGraphemeBreakBefore || glyph->fMayLineBreakBefore) {
                // TODO: preserve line breaks <= grapheme breaks
                // and prevent line breaks inside graphemes
                candidateGraphemeBreak = current;
                candidateGraphemeBreakWidth = currentWidth;
                if (glyph->fMayLineBreakBefore) {
                    candidateLineBreak = current;
                    candidateLineBreakWidth = currentWidth;
                }
            }
        }

        SkScalar glyphWidth = glyph->fAdvance.fX;
        // Break when overwidth, the glyph has a visual representation, and some space is used.
        if (width < currentWidth + glyphWidth && glyph->fHasVisual && candidateGraphemeBreakWidth > 0){
            if (candidateLineBreak != beginning) {
                beginning = candidateLineBreak;
                currentWidth -= candidateLineBreakWidth;
                candidateGraphemeBreakWidth -= candidateLineBreakWidth;
                candidateLineBreakWidth = 0;
            } else if (candidateGraphemeBreak != beginning) {
                beginning = candidateGraphemeBreak;
                candidateLineBreak = beginning;
                currentWidth -= candidateGraphemeBreakWidth;
                candidateGraphemeBreakWidth = 0;
                candidateLineBreakWidth = 0;
            } else {
                SK_ABORT("");
            }

            if (width < currentWidth) {
                if (width < candidateGraphemeBreakWidth) {
                    candidateGraphemeBreak = candidateLineBreak;
                    candidateGraphemeBreakWidth = candidateLineBreakWidth;
                }
                current = candidateGraphemeBreak;
                currentWidth = candidateGraphemeBreakWidth;
            }

            glyph = beginning.current();
            if (glyph) {
                glyph->fMustLineBreakBefore = true;
            }

        } else {
            current.next();
            currentWidth += glyphWidth;
        }
    }
}

// Reorder the runs and glyphs per line and write them out.
    SkPoint currentPoint = point;
{
    ShapedRunGlyphIterator previousBreak(runs);
    ShapedRunGlyphIterator glyphIterator(runs);
    SkScalar maxAscent = 0;
    SkScalar maxDescent = 0;
    SkScalar maxLeading = 0;
    int previousRunIndex = -1;
    while (glyphIterator.current()) {
        int runIndex = glyphIterator.fRunIndex;
        int glyphIndex = glyphIterator.fGlyphIndex;
        ShapedGlyph* nextGlyph = glyphIterator.next();

        if (previousRunIndex != runIndex) {
            SkFontMetrics metrics;
            runs[runIndex].fFont.getMetrics(&metrics);
            maxAscent = SkTMin(maxAscent, metrics.fAscent);
            maxDescent = SkTMax(maxDescent, metrics.fDescent);
            maxLeading = SkTMax(maxLeading, metrics.fLeading);
            previousRunIndex = runIndex;
        }

        // Nothing can be written until the baseline is known.
        if (!(nextGlyph == nullptr || nextGlyph->fMustLineBreakBefore)) {
            continue;
        }

        currentPoint.fY -= maxAscent;

        int numRuns = runIndex - previousBreak.fRunIndex + 1;
        SkAutoSTMalloc<4, UBiDiLevel> runLevels(numRuns);
        for (int i = 0; i < numRuns; ++i) {
            runLevels[i] = runs[previousBreak.fRunIndex + i].fLevel;
        }
        SkAutoSTMalloc<4, int32_t> logicalFromVisual(numRuns);
        ubidi_reorderVisual(runLevels, numRuns, logicalFromVisual);

        // step through the runs in reverse visual order and the glyphs in reverse logical order
        // until a visible glyph is found and force them to the end of the visual line.

        for (int i = 0; i < numRuns; ++i) {
            int logicalIndex = previousBreak.fRunIndex + logicalFromVisual[i];

            int startGlyphIndex = (logicalIndex == previousBreak.fRunIndex)
                                ? previousBreak.fGlyphIndex
                                : 0;
            int endGlyphIndex = (logicalIndex == runIndex)
                              ? glyphIndex + 1
                              : runs[logicalIndex].fNumGlyphs;

            const auto& run = runs[logicalIndex];
            const RunHandler::RunInfo info = {
                run.fAdvance,
                maxAscent,
                maxDescent,
                maxLeading,
            };
            append(handler, info, run, startGlyphIndex, endGlyphIndex, &currentPoint);
        }

        handler->commitLine();

        currentPoint.fY += maxDescent + maxLeading;
        currentPoint.fX = point.fX;
        maxAscent = 0;
        maxDescent = 0;
        maxLeading = 0;
        previousRunIndex = -1;
        previousBreak = glyphIterator;
    }
}

    return currentPoint;
}


ShapedRun SkShaper::Impl::shape(const char* utf8,
                                const size_t utf8Bytes,
                                const char* utf8Start,
                                const char* utf8End,
                                const BiDiRunIterator* bidi,
                                const LanguageRunIterator* language,
                                const ScriptRunIterator* script,
                                const FontRunIterator* font) const
{
    ShapedRun run(SkSpan<const char>(), SkFont(), 0, nullptr, 0);

    hb_buffer_t* buffer = fBuffer.get();
    SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);
    hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
    hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

    // See 763e5466c0a03a7c27020e1e2598e488612529a7 for documentation.
    hb_buffer_set_flags(buffer, HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);

    // Add precontext.
    hb_buffer_add_utf8(buffer, utf8, utf8Start - utf8, utf8Start - utf8, 0);

    // Populate the hb_buffer directly with utf8 cluster indexes.
    const char* utf8Current = utf8Start;
    while (utf8Current < utf8End) {
        unsigned int cluster = utf8Current - utf8Start;
        hb_codepoint_t u = utf8_next(&utf8Current, utf8End);
        hb_buffer_add(buffer, u, cluster);
    }

    // Add postcontext.
    hb_buffer_add_utf8(buffer, utf8Current, utf8 + utf8Bytes - utf8Current, 0, 0);

    size_t utf8runLength = utf8End - utf8Start;
    if (!SkTFitsIn<int>(utf8runLength)) {
        SkDebugf("Shaping error: utf8 too long");
        return run;
    }
    hb_direction_t direction = is_LTR(bidi->currentLevel()) ? HB_DIRECTION_LTR:HB_DIRECTION_RTL;
    hb_buffer_set_direction(buffer, direction);
    hb_buffer_set_script(buffer, script->currentScript());
    hb_buffer_set_language(buffer, language->currentLanguage());
    hb_buffer_guess_segment_properties(buffer);
    // TODO: features
    if (!font->currentHBFont()) {
        return run;
    }
    hb_shape(font->currentHBFont(), buffer, nullptr, 0);
    unsigned len = hb_buffer_get_length(buffer);
    if (len == 0) {
        // TODO: this isn't an error, make it look different
        return run;
    }

    if (direction == HB_DIRECTION_RTL) {
        // Put the clusters back in logical order.
        // Note that the advances remain ltr.
        hb_buffer_reverse(buffer);
    }
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, nullptr);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer, nullptr);

    if (!SkTFitsIn<int>(len)) {
        SkDebugf("Shaping error: too many glyphs");
        return run;
    }

    run = ShapedRun(SkSpan<const char>(utf8Start, utf8runLength),
                    *font->currentFont(), bidi->currentLevel(),
                    std::unique_ptr<ShapedGlyph[]>(new ShapedGlyph[len]), len);
    int scaleX, scaleY;
    hb_font_get_scale(font->currentHBFont(), &scaleX, &scaleY);
    double textSizeY = run.fFont.getSize() / scaleY;
    double textSizeX = run.fFont.getSize() / scaleX * run.fFont.getScaleX();
    SkVector runAdvance = { 0, 0 };
    for (unsigned i = 0; i < len; i++) {
        ShapedGlyph& glyph = run.fGlyphs[i];
        glyph.fID = info[i].codepoint;
        glyph.fCluster = info[i].cluster;
        glyph.fOffset.fX = pos[i].x_offset * textSizeX;
        glyph.fOffset.fY = pos[i].y_offset * textSizeY;
        glyph.fAdvance.fX = pos[i].x_advance * textSizeX;
        glyph.fAdvance.fY = pos[i].y_advance * textSizeY;

        SkRect bounds;
        SkScalar advance;
        SkPaint p;
        run.fFont.getWidthsBounds(&glyph.fID, 1, &advance, &bounds, &p);
        glyph.fHasVisual = !bounds.isEmpty(); //!font->currentTypeface()->glyphBoundsAreZero(glyph.fID);
        glyph.fUnsafeToBreak = info[i].mask & HB_GLYPH_FLAG_UNSAFE_TO_BREAK;
        glyph.fMustLineBreakBefore = false;

        runAdvance += glyph.fAdvance;
    }
    run.fAdvance = runAdvance;

    return run;
}
