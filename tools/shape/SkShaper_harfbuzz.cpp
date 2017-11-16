/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <hb-ot.h>
#include <unicode/brkiter.h>
#include <unicode/locid.h>
#include <unicode/stringpiece.h>
#include <unicode/ubidi.h>
#include <unicode/uchriter.h>
#include <unicode/unistr.h>
#include <unicode/uscript.h>

#include "SkFontMgr.h"
#include "SkShaper.h"
#include "SkStream.h"
#include "SkTDPQueue.h"
#include "SkTLazy.h"
#include "SkTemplates.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkUtils.h"

namespace {
template <class T, void(*P)(T*)> using resource = std::unique_ptr<T, SkFunctionWrapper<void, T, P>>;
using HBBlob   = resource<hb_blob_t  , hb_blob_destroy  >;
using HBFace   = resource<hb_face_t  , hb_face_destroy  >;
using HBFont   = resource<hb_font_t  , hb_font_destroy  >;
using HBBuffer = resource<hb_buffer_t, hb_buffer_destroy>;
using ICUBiDi  = resource<UBiDi      , ubidi_close      >;

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

static const int FONT_SIZE_SCALE = 512;
HBFont create_hb_font(SkTypeface* tf) {
    int index;
    HBBlob blob(stream_to_blob(std::unique_ptr<SkStreamAsset>(tf->openStream(&index))));
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
    hb_font_set_scale(font.get(), FONT_SIZE_SCALE, FONT_SIZE_SCALE);
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

class RunIterator {
public:
    virtual ~RunIterator() {}
    virtual void consume() = 0;
    // These iterators are UChar based because the bidi one is.
    // If we could do bidi in utf-8, these would all be utf-8.
    virtual const UChar* endOfCurrentRun() const = 0;
    virtual bool atEnd() const = 0;
    bool operator<(const RunIterator& that) const {
        return this->endOfCurrentRun() < that.endOfCurrentRun();
    }
};

class BiDiRunIterator : public RunIterator {
public:
    static SkTLazy<BiDiRunIterator> Make(const icu::UnicodeString& utf16, UBiDiLevel initialBidi) {
        UErrorCode status = U_ZERO_ERROR;
        SkTLazy<BiDiRunIterator> ret;
        ICUBiDi bidi(ubidi_openSized(utf16.length(), 0, &status));
        if (U_FAILURE(status)) {
            SkDebugf("Bidi error: %s", u_errorName(status));
            return ret;
        }
        SkASSERT(bidi);

        ubidi_setPara(bidi.get(), utf16.getBuffer(), utf16.length(), initialBidi, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Bidi error: %s", u_errorName(status));
            return ret;
        }

        int32_t runCount = ubidi_countRuns(bidi.get(), &status);
        if (U_FAILURE(status)) {
            SkDebugf("Bidi error: %s", u_errorName(status));
            return ret;
        }

        ret.init(std::move(bidi), runCount);
        return ret;
    }
    BiDiRunIterator(ICUBiDi bidi, int32_t runCount)
        : fBidi(std::move(bidi))
        , fRunCount(runCount)
        , fRun(0)
        , fEndOfCurrentRun(ubidi_getText(fBidi.get()))
        , fDirection(HB_DIRECTION_LTR)
    {}
    void consume() override {
        SkASSERT(fRun < fRunCount);
        int32_t start;
        int32_t length;
        UBiDiDirection direction = ubidi_getVisualRun(fBidi.get(), fRun, &start, &length);
        ++fRun;
        fDirection = direction ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
        fEndOfCurrentRun = ubidi_getText(fBidi.get()) + start + length;
    }
    const UChar* endOfCurrentRun() const override {
        return fEndOfCurrentRun;
    }
    bool atEnd() const override {
        return fRun == fRunCount;
    }

    hb_direction_t currentDirection() const {
        return fDirection;
    }
private:
    ICUBiDi fBidi;
    int32_t fRunCount;
    int32_t fRun;
    const UChar* fEndOfCurrentRun;
    hb_direction_t fDirection;
};

class ScriptRunIterator : public RunIterator {
public:
    static SkTLazy<ScriptRunIterator> Make(const icu::UnicodeString& utf16,
                                           hb_unicode_funcs_t* hbUnicode)
    {
        SkTLazy<ScriptRunIterator> ret;
        ret.init(utf16, hbUnicode);
        return ret;
    }
    ScriptRunIterator(const icu::UnicodeString& utf16, hb_unicode_funcs_t* hbUnicode)
        : fCurrent(utf16.getBuffer()), fEnd(fCurrent + utf16.length())
        , fHBUnicode(hbUnicode)
        , fCurrentScript(HB_SCRIPT_UNKNOWN)
    {}
    void consume() override {
        SkASSERT(fCurrent < fEnd);
        SkUnichar u = SkUTF16_NextUnichar(&fCurrent);
        fCurrentScript = hb_unicode_script(fHBUnicode, u);
        while (fCurrent < fEnd) {
            const UChar* prev = fCurrent;
            u = SkUTF16_NextUnichar(&fCurrent);
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
    const UChar* endOfCurrentRun() const override {
        return fCurrent;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    hb_script_t currentScript() const {
        return fCurrentScript;
    }
private:
    const UChar* fCurrent;
    const UChar* fEnd;
    hb_unicode_funcs_t* fHBUnicode;
    hb_script_t fCurrentScript;
};

class FontRunIterator : public RunIterator {
public:
    static SkTLazy<FontRunIterator> Make(const icu::UnicodeString& utf16,
                                         sk_sp<SkTypeface> typeface,
                                         hb_font_t* hbFace,
                                         sk_sp<SkFontMgr> fallbackMgr)
    {
        SkTLazy<FontRunIterator> ret;
        ret.init(utf16, std::move(typeface), hbFace, std::move(fallbackMgr));
        return ret;
    }
    FontRunIterator(const icu::UnicodeString& utf16, sk_sp<SkTypeface> typeface, hb_font_t* hbFace,
                    sk_sp<SkFontMgr> fallbackMgr)
        : fCurrent(utf16.getBuffer()), fEnd(fCurrent + utf16.length())
        , fFallbackMgr(std::move(fallbackMgr))
        , fHBFont(hbFace), fTypeface(std::move(typeface))
        , fFallbackHBFont(nullptr), fFallbackTypeface(nullptr)
        , fCurrentHBFont(fHBFont), fCurrentTypeface(fTypeface.get())
    {}
    void consume() override {
        SkASSERT(fCurrent < fEnd);
        SkUnichar u = SkUTF16_NextUnichar(&fCurrent);
        // If the starting typeface can handle this character, use it.
        if (fTypeface->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1)) {
            fFallbackTypeface.reset();
        // If not, try to find a fallback typeface
        } else {
            fFallbackTypeface.reset(fFallbackMgr->matchFamilyStyleCharacter(
                nullptr, fTypeface->fontStyle(), nullptr, 0, u));
        }

        if (fFallbackTypeface) {
            fFallbackHBFont = create_hb_font(fFallbackTypeface.get());
            fCurrentTypeface = fFallbackTypeface.get();
            fCurrentHBFont = fFallbackHBFont.get();
        } else {
            fFallbackHBFont.reset();
            fCurrentTypeface = fTypeface.get();
            fCurrentHBFont = fHBFont;
        }

        while (fCurrent < fEnd) {
            const UChar* prev = fCurrent;
            u = SkUTF16_NextUnichar(&fCurrent);

            // If using a fallback and the initial typeface has this character, stop fallback.
            if (fFallbackTypeface &&
                fTypeface->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1))
            {
                fCurrent = prev;
                return;
            }
            // If the current typeface cannot handle this character, stop using it.
            if (!fCurrentTypeface->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1)) {
                fCurrent = prev;
                return;
            }
        }
    }
    const UChar* endOfCurrentRun() const override {
        return fCurrent;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    SkTypeface* currentTypeface() const {
        return fCurrentTypeface;
    }
    hb_font_t* currentHBFont() const {
        return fCurrentHBFont;
    }
private:
    const UChar* fCurrent;
    const UChar* fEnd;
    sk_sp<SkFontMgr> fFallbackMgr;
    hb_font_t* fHBFont;
    sk_sp<SkTypeface> fTypeface;
    HBFont fFallbackHBFont;
    sk_sp<SkTypeface> fFallbackTypeface;
    hb_font_t* fCurrentHBFont;
    SkTypeface* fCurrentTypeface;
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
        const UChar* leastEnd = leastRun->endOfCurrentRun();
        RunIterator* currentRun = nullptr;
        SkDEBUGCODE(const UChar* previousEndOfCurrentRun);
        while ((currentRun = fRunIterators.peek())->endOfCurrentRun() <= leastEnd) {
            fRunIterators.pop();
            SkDEBUGCODE(previousEndOfCurrentRun = currentRun->endOfCurrentRun());
            currentRun->consume();
            SkASSERT(previousEndOfCurrentRun < currentRun->endOfCurrentRun());
            fRunIterators.insert(currentRun);
        }
        return true;
    }

    const UChar* endOfCurrentRun() const {
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
    bool fLineBreakBefore;
    bool fMustLineBreakBefore;
    bool fHasVisual;
};
struct ShapedRun {
    ShapedRun(const char* utf8Start, const char* utf8End, int numGlyphs, const SkPaint& paint,
              hb_direction_t direction, std::unique_ptr<ShapedGlyph[]> glyphs)
        : fUtf8Start(utf8Start), fUtf8End(utf8End), fNumGlyphs(numGlyphs), fPaint(paint)
        , fDirection(direction), fGlyphs(std::move(glyphs))
    {}

    const char* fUtf8Start;
    const char* fUtf8End;
    int fNumGlyphs;
    SkPaint fPaint;
    hb_direction_t fDirection;
    std::unique_ptr<ShapedGlyph[]> fGlyphs;
};

static void append(SkTextBlobBuilder* b, const ShapedRun& run, int start, int end, SkPoint* p) {
    unsigned len = end - start;
    auto runBuffer = b->allocRunTextPos(run.fPaint, len, run.fUtf8End - run.fUtf8Start, SkString());
    memcpy(runBuffer.utf8text, run.fUtf8Start, run.fUtf8End - run.fUtf8Start);

    for (unsigned i = 0; i < len; i++) {
        const ShapedGlyph& glyph = run.fGlyphs[i + start];
        runBuffer.glyphs[i] = glyph.fID;
        runBuffer.clusters[i] = glyph.fCluster;
        reinterpret_cast<SkPoint*>(runBuffer.pos)[i] =
                SkPoint::Make(SkDoubleToScalar(p->fX + glyph.fOffset.fX),
                              SkDoubleToScalar(p->fY - glyph.fOffset.fY));
        p->fX += glyph.fAdvance.fX;
        p->fY += glyph.fAdvance.fY;
    }
}

struct ShapedRunGlyphIterator {
    ShapedRunGlyphIterator(const SkTArray<ShapedRun>& origRuns)
        : fRuns(&origRuns), fRunIndex(0), fGlyphIndex(0)
    {
        const SkTArray<ShapedRun>& runs = *fRuns;
        if (runs.count() > fRunIndex && runs[fRunIndex].fDirection == HB_DIRECTION_RTL) {
            fGlyphIndex = runs[fRunIndex].fNumGlyphs - 1;
        }
    }

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

        if (runs[fRunIndex].fDirection == HB_DIRECTION_LTR) {
            if (fGlyphIndex < runs[fRunIndex].fNumGlyphs - 1) {
                ++fGlyphIndex;
            } else if (fRunIndex >= runs.count() - 1) {
                ++fRunIndex;
                return nullptr;
            } else {
                ++fRunIndex;
                if (runs[fRunIndex].fDirection == HB_DIRECTION_RTL) {
                    fGlyphIndex = runs[fRunIndex].fNumGlyphs - 1;
                }
            }
        } else {
            if (fGlyphIndex > 0) {
                --fGlyphIndex;
            } else if (fRunIndex >= runs.count() - 1) {
                ++fRunIndex;
                return nullptr;
            } else {
                ++fRunIndex;
                if (runs[fRunIndex].fDirection == HB_DIRECTION_RTL) {
                    fGlyphIndex = runs[fRunIndex].fNumGlyphs - 1;
                }
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
    std::unique_ptr<icu::BreakIterator> fBreakIterator;
};

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(new Impl) {
    fImpl->fTypeface = tf ? std::move(tf) : SkTypeface::MakeDefault();
    fImpl->fHarfBuzzFont = create_hb_font(fImpl->fTypeface.get());
    SkASSERT(fImpl->fHarfBuzzFont);
    fImpl->fBuffer.reset(hb_buffer_create());
    SkASSERT(fImpl->fBuffer);

    icu::Locale thai("th");
    UErrorCode status = U_ZERO_ERROR;
    fImpl->fBreakIterator.reset(icu::BreakIterator::createLineInstance(thai, status));
    if (U_FAILURE(status)) {
        SkDebugf("Could not create break iterator: %s", u_errorName(status));
        SK_ABORT("");
    }
}

SkShaper::~SkShaper() {}

bool SkShaper::good() const { return fImpl->fHarfBuzzFont != nullptr; }

SkScalar SkShaper::shape(SkTextBlobBuilder* builder,
                         const SkPaint& srcPaint,
                         const char* utf8text,
                         size_t textBytes,
                         bool leftToRight,
                         SkPoint point,
                         SkScalar width) const {
    sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
    SkASSERT(builder);
    UBiDiLevel bidiLevel = leftToRight ? UBIDI_DEFAULT_LTR : UBIDI_DEFAULT_RTL;
    //hb_script_t script = ...
    double x = point.x();
    //double y = point.y();
    SkPoint currentPoint = point;

    // This 'shape' function only accepts utf8.
    // ubidi only accepts utf16 (though internally it basically works on utf32 chars).
    // We want an ubidi_setPara(UBiDi*, UText*, UBiDiLevel, UBiDiLevel*, UErrorCode*);
    // Internally, harfbuzz is all utf32, but always makes a copy.

    if (!SkTFitsIn<int32_t>(textBytes)) {
        SkDebugf("Bidi error: text too long");
        return (SkScalar)x;
    }
    icu::UnicodeString utf16 = icu::UnicodeString::fromUTF8(icu::StringPiece(utf8text, textBytes));

    RunIteratorQueue runSegmenter;

    SkTLazy<BiDiRunIterator> maybeBidi(BiDiRunIterator::Make(utf16, bidiLevel));
    BiDiRunIterator* bidi = maybeBidi.getMaybeNull();
    if (!bidi) {
        return (SkScalar)x;
    }
    runSegmenter.insert(bidi);

    hb_unicode_funcs_t* hbUnicode = hb_buffer_get_unicode_funcs(fImpl->fBuffer.get());
    SkTLazy<ScriptRunIterator> maybeScript(ScriptRunIterator::Make(utf16, hbUnicode));
    ScriptRunIterator* script = maybeScript.getMaybeNull();
    if (!script) {
        return (SkScalar)x;
    }
    runSegmenter.insert(script);

    SkTLazy<FontRunIterator> maybeFont(FontRunIterator::Make(utf16,
                                                             fImpl->fTypeface,
                                                             fImpl->fHarfBuzzFont.get(),
                                                             std::move(fontMgr)));
    FontRunIterator* font = maybeFont.getMaybeNull();
    if (!font) {
        return (SkScalar)x;
    }
    runSegmenter.insert(font);

    UErrorCode status = U_ZERO_ERROR;
    UText utf8UText = UTEXT_INITIALIZER;
    utext_openUTF8(&utf8UText, utf8text, textBytes, &status);
    if (U_FAILURE(status)) {
        SkDebugf("Could not create utf8UText: %s", u_errorName(status));
        return (SkScalar)x;
    }
    icu::BreakIterator& breakIterator = *fImpl->fBreakIterator;
    breakIterator.setText(&utf8UText, status);
    if (U_FAILURE(status)) {
        SkDebugf("Could not setText on break iterator: %s", u_errorName(status));
        return (SkScalar)x;
    }

    SkTArray<ShapedRun> runs;

{
    const UChar* utf16Start = nullptr;
    const char* utf8Start = nullptr;
    const UChar* utf16End = utf16.getBuffer();
    const char* utf8End = utf8text;
    while (runSegmenter.advanceRuns()) {
        utf16Start = utf16End;
        utf8Start = utf8End;
        utf16End = runSegmenter.endOfCurrentRun();

        hb_buffer_t* buffer = fImpl->fBuffer.get();
        SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);
        hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
        hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

        // If the hb_buffer is created with utf16, clusters will point to utf16 indexes,
        // but the SkTextBlob can only take utf8 and utf8 cluster indexes.
        // So populate the hb_buffer directly with utf8 cluster indexes.
        const char* utf8Current = utf8Start;
        const UChar* utf16Current = utf16Start;
        while (utf16Current < utf16End) {
            hb_codepoint_t u = SkUTF16_NextUnichar(&utf16Current);
            hb_buffer_add(buffer, u, utf8Current - utf8Start);
            SkUTF8_NextUnichar(&utf8Current);
        }

        utf8End = utf8Current;
        size_t utf8runLength = utf8End - utf8Start;
        if (!SkTFitsIn<int>(utf8runLength)) {
            SkDebugf("Shaping error: utf8 too long");
            return (SkScalar)x;
        }
        hb_buffer_set_script(buffer, script->currentScript());
        hb_buffer_set_direction(buffer, bidi->currentDirection());
        // TODO: language
        hb_buffer_guess_segment_properties(buffer);
        // TODO: features
        hb_shape(font->currentHBFont(), buffer, nullptr, 0);
        unsigned len = hb_buffer_get_length(buffer);
        if (len == 0) {
            continue;
        }

        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, nullptr);
        hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer, nullptr);

        if (!SkTFitsIn<int>(len)) {
            SkDebugf("Shaping error: too many glyphs");
            return (SkScalar)x;
        }

        SkPaint paint(srcPaint);
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        paint.setTypeface(sk_ref_sp(font->currentTypeface()));
        ShapedRun& run = runs.emplace_back(utf8Start, utf8End, len, paint, bidi->currentDirection(),
                                           std::unique_ptr<ShapedGlyph[]>(new ShapedGlyph[len]));
        double textSizeY = run.fPaint.getTextSize() / (double)FONT_SIZE_SCALE;
        double textSizeX = textSizeY * run.fPaint.getTextScaleX();
        for (unsigned i = 0; i < len; i++) {
            ShapedGlyph& glyph = run.fGlyphs[i];
            glyph.fID = info[i].codepoint;
            glyph.fCluster = info[i].cluster;
            glyph.fOffset.fX = pos[i].x_offset * textSizeX;
            glyph.fOffset.fY = pos[i].y_offset * textSizeY;
            glyph.fAdvance.fX = pos[i].x_advance * textSizeX;
            glyph.fAdvance.fY = pos[i].y_advance * textSizeY;
            glyph.fHasVisual = true; //!font->currentTypeface()->glyphBoundsAreZero(glyph.fID);
            //info->mask safe_to_break;
            glyph.fMustLineBreakBefore = false;
        }

        int clusterOffset = utf8Start - utf8text;
        if (run.fDirection == HB_DIRECTION_LTR) {
            uint32_t previousCluster = 0xFFFFFFFF;
            for (unsigned i = 0; i < len; i++) {
                ShapedGlyph& glyph = run.fGlyphs[i];
                glyph.fLineBreakBefore = glyph.fCluster != previousCluster &&
                                         breakIterator.isBoundary(glyph.fCluster + clusterOffset);
                previousCluster = glyph.fCluster;
            }
        } else {
            uint32_t previousCluster = 0xFFFFFFFF;
            for (unsigned i = len; i --> 0;) {
                ShapedGlyph& glyph = run.fGlyphs[i];
                glyph.fLineBreakBefore = glyph.fCluster != previousCluster &&
                                         breakIterator.isBoundary(glyph.fCluster + clusterOffset);
                previousCluster = glyph.fCluster;
            }

        }
    }
}

{
    SkScalar widthSoFar = 0;
    bool previousBreakValid = false; // Set when previousBreak is set to a valid candidate break.
    bool canAddBreakNow = false; // Disallow line breaks before the first glyph of a run.
    ShapedRunGlyphIterator previousBreak(runs);
    ShapedRunGlyphIterator glyphIterator(runs);
    while (ShapedGlyph* glyph = glyphIterator.current()) {
        if (canAddBreakNow && glyph->fLineBreakBefore) {
            previousBreakValid = true;
            previousBreak = glyphIterator;
        }
        SkScalar glyphWidth = glyph->fAdvance.fX;
        if (widthSoFar + glyphWidth < width) {
            widthSoFar += glyphWidth;
            glyphIterator.next();
            canAddBreakNow = true;
            continue;
        }

        if (widthSoFar == 0) {
            // Adding just this glyph is too much, just break with this glyph
            glyphIterator.next();
            previousBreak = glyphIterator;
        } else if (!previousBreakValid) {
            // No break opprotunity found yet, just break without this glyph
            previousBreak = glyphIterator;
        }
        glyphIterator = previousBreak;
        glyph = glyphIterator.current();
        if (glyph) {
            glyph->fMustLineBreakBefore = true;
        }
        previousBreakValid = false;
        widthSoFar = 0;
        canAddBreakNow = false;
    }
}

{
    ShapedRunGlyphIterator previousBreak(runs);
    ShapedRunGlyphIterator glyphIterator(runs);
    SkScalar maxAscent = 0;
    SkScalar maxDescent = 0;
    SkScalar maxLeading = 0;
    int previousRunIndex = -1;
    while (ShapedGlyph* glyph = glyphIterator.current()) {
        int runIndex = glyphIterator.fRunIndex;
        ShapedGlyph* nextGlyph = glyphIterator.next();

        if (previousRunIndex != runIndex) {
            SkPaint::FontMetrics metrics;
            runs[runIndex].fPaint.getFontMetrics(&metrics);
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
        ShapedRunGlyphIterator drawGlyphIterator(previousBreak);
        while (drawGlyphIterator != glyphIterator) {
            int runIndex = drawGlyphIterator.fRunIndex;
            int glyphIndex = drawGlyphIterator.fGlyphIndex;

            drawGlyphIterator.next();
            int nextRunIndex = drawGlyphIterator.fRunIndex;

            // No need to write anything unless
            // * at the end of the line
            // * at the end of a ltr run
            // * at the end of a rtl run and the next run is ltr
            if (!(drawGlyphIterator == glyphIterator ||
                 (runIndex != nextRunIndex && (
                   (runs[runIndex].fDirection == HB_DIRECTION_LTR) ||
                   (runs[runIndex].fDirection == HB_DIRECTION_RTL &&
                    runs[nextRunIndex].fDirection == HB_DIRECTION_LTR)
                 ))
            )) {
                continue;
            }

            int startGlyphIndex;
            int endGlyphIndex;
            if (runs[runIndex].fDirection == HB_DIRECTION_LTR) {
                startGlyphIndex = previousBreak.fGlyphIndex;
                endGlyphIndex = glyphIndex + 1;
                append(builder, runs[runIndex], startGlyphIndex, endGlyphIndex, &currentPoint);
            } else {
                while (runIndex >= previousBreak.fRunIndex) {
                    if (runIndex == glyphIterator.fRunIndex) {
                        startGlyphIndex = glyphIndex;
                    } else {
                        startGlyphIndex = 0;
                    }
                    if (runIndex == previousBreak.fRunIndex) {
                        endGlyphIndex = previousBreak.fGlyphIndex + 1;
                    } else {
                        endGlyphIndex = runs[runIndex].fNumGlyphs;
                    }
                    append(builder, runs[runIndex], startGlyphIndex, endGlyphIndex, &currentPoint);
                    --runIndex;
                }
            }
            previousBreak = drawGlyphIterator;
        }
        currentPoint.fY += maxDescent + maxLeading;
        currentPoint.fX = point.fX;
        maxAscent = 0;
        maxDescent = 0;
        maxLeading = 0;
        previousRunIndex = -1;
        previousBreak = glyphIterator;
    }
}

    return (SkScalar)x;
}
