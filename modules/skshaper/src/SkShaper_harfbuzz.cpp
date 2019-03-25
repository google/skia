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
#include "SkMakeUnique.h"
#include "SkMalloc.h"
#include "SkPoint.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkShaper.h"
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

hb_position_t skhb_position(SkScalar value) {
    // Treat HarfBuzz hb_position_t as 16.16 fixed-point.
    constexpr int kHbPosition1 = 1 << 16;
    return SkScalarRoundToInt(value * kHbPosition1);
}

hb_bool_t skhb_glyph(hb_font_t* hb_font,
                     void* font_data,
                     hb_codepoint_t unicode,
                     hb_codepoint_t variation_selector,
                     hb_codepoint_t* glyph,
                     void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    *glyph = font.unicharToGlyph(unicode);
    return *glyph != 0;
}

hb_bool_t skhb_nominal_glyph(hb_font_t* hb_font,
                             void* font_data,
                             hb_codepoint_t unicode,
                             hb_codepoint_t* glyph,
                             void* user_data) {
  return skhb_glyph(hb_font, font_data, unicode, 0, glyph, user_data);
}

unsigned skhb_nominal_glyphs(hb_font_t *hb_font, void *font_data,
                             unsigned int count,
                             const hb_codepoint_t *unicodes,
                             unsigned int unicode_stride,
                             hb_codepoint_t *glyphs,
                             unsigned int glyph_stride,
                             void *user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    // Batch call textToGlyphs since entry cost is not cheap.
    // Copy requred because textToGlyphs is dense and hb is strided.
    SkAutoSTMalloc<256, SkUnichar> unicode(count);
    for (unsigned i = 0; i < count; i++) {
        unicode[i] = *unicodes;
        unicodes = SkTAddOffset<const hb_codepoint_t>(unicodes, unicode_stride);
    }
    SkAutoSTMalloc<256, SkGlyphID> glyph(count);
    font.textToGlyphs(unicode.get(), count * sizeof(SkUnichar), kUTF32_SkTextEncoding,
                        glyph.get(), count);

    // Copy the results back to the sparse array.
    for (unsigned i = 0; i < count; i++) {
        *glyphs = glyph[i];
        glyphs = SkTAddOffset<hb_codepoint_t>(glyphs, glyph_stride);
    }
    // TODO: supposed to return index of first 0?
    return count;
}

hb_position_t skhb_glyph_h_advance(hb_font_t* hb_font,
                                   void* font_data,
                                   hb_codepoint_t codepoint,
                                   void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    SkScalar advance;
    SkGlyphID glyph = SkTo<SkGlyphID>(codepoint);

    font.getWidths(&glyph, 1, &advance);
    if (!font.isSubpixel()) {
        advance = SkScalarRoundToInt(advance);
    }
    return skhb_position(advance);
}

void skhb_glyph_h_advances(hb_font_t* hb_font,
                           void* font_data,
                           unsigned count,
                           const hb_codepoint_t* glyphs,
                           unsigned int glyph_stride,
                           hb_position_t* advances,
                           unsigned int advance_stride,
                           void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    // Batch call getWidths since entry cost is not cheap.
    // Copy requred because getWidths is dense and hb is strided.
    SkAutoSTMalloc<256, SkGlyphID> glyph(count);
    for (unsigned i = 0; i < count; i++) {
        glyph[i] = *glyphs;
        glyphs = SkTAddOffset<const hb_codepoint_t>(glyphs, glyph_stride);
    }
    SkAutoSTMalloc<256, SkScalar> advance(count);
    font.getWidths(glyph.get(), count, advance.get());

    if (!font.isSubpixel()) {
        for (unsigned i = 0; i < count; i++) {
            advance[i] = SkScalarRoundToInt(advance[i]);
        }
    }

    // Copy the results back to the sparse array.
    for (unsigned i = 0; i < count; i++) {
        *advances = skhb_position(advance[i]);
        advances = SkTAddOffset<hb_position_t>(advances, advance_stride);
    }
}

// HarfBuzz callback to retrieve glyph extents, mainly used by HarfBuzz for
// fallback mark positioning, i.e. the situation when the font does not have
// mark anchors or other mark positioning rules, but instead HarfBuzz is
// supposed to heuristically place combining marks around base glyphs. HarfBuzz
// does this by measuring "ink boxes" of glyphs, and placing them according to
// Unicode mark classes. Above, below, centered or left or right, etc.
hb_bool_t skhb_glyph_extents(hb_font_t* hb_font,
                             void* font_data,
                             hb_codepoint_t codepoint,
                             hb_glyph_extents_t* extents,
                             void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    SkASSERT(codepoint < 0xFFFFu);
    SkASSERT(extents);

    SkRect sk_bounds;
    SkGlyphID glyph = codepoint;

    font.getWidths(&glyph, 1, nullptr, &sk_bounds);
    if (!font.isSubpixel()) {
        sk_bounds.set(sk_bounds.roundOut());
    }

    // Skia is y-down but HarfBuzz is y-up.
    extents->x_bearing = skhb_position(sk_bounds.fLeft);
    extents->y_bearing = skhb_position(-sk_bounds.fTop);
    extents->width = skhb_position(sk_bounds.width());
    extents->height = skhb_position(-sk_bounds.height());
    return true;
}

#define SK_HB_VERSION_CHECK(x, y, z) \
    (HB_VERSION_MAJOR >  (x)) || \
    (HB_VERSION_MAJOR == (x) && HB_VERSION_MINOR >  (y)) || \
    (HB_VERSION_MAJOR == (x) && HB_VERSION_MINOR == (y) && HB_VERSION_MICRO >= (z))

hb_font_funcs_t* skhb_get_font_funcs() {
    static hb_font_funcs_t* const funcs = []{
        // HarfBuzz will use the default (parent) implementation if they aren't set.
        hb_font_funcs_t* const funcs = hb_font_funcs_create();
        hb_font_funcs_set_variation_glyph_func(funcs, skhb_glyph, nullptr, nullptr);
        hb_font_funcs_set_nominal_glyph_func(funcs, skhb_nominal_glyph, nullptr, nullptr);
#if SK_HB_VERSION_CHECK(2, 0, 0)
        hb_font_funcs_set_nominal_glyphs_func(funcs, skhb_nominal_glyphs, nullptr, nullptr);
#else
        sk_ignore_unused_variable(skhb_nominal_glyphs);
#endif
        hb_font_funcs_set_glyph_h_advance_func(funcs, skhb_glyph_h_advance, nullptr, nullptr);
#if SK_HB_VERSION_CHECK(1, 8, 6)
        hb_font_funcs_set_glyph_h_advances_func(funcs, skhb_glyph_h_advances, nullptr, nullptr);
#else
        sk_ignore_unused_variable(skhb_glyph_h_advances);
#endif
        hb_font_funcs_set_glyph_extents_func(funcs, skhb_glyph_extents, nullptr, nullptr);
        hb_font_funcs_make_immutable(funcs);
        return funcs;
    }();
    SkASSERT(funcs);
    return funcs;
}

hb_blob_t* skhb_get_table(hb_face_t* face, hb_tag_t tag, void* user_data) {
    SkTypeface& typeface = *reinterpret_cast<SkTypeface*>(user_data);

    const size_t tableSize = typeface.getTableSize(tag);
    if (!tableSize) {
        return nullptr;
    }

    void* buffer = sk_malloc_throw(tableSize);
    if (!buffer) {
        return nullptr;
    }

    size_t actualSize = typeface.getTableData(tag, 0, tableSize, buffer);
    if (tableSize != actualSize) {
        sk_free(buffer);
        return nullptr;
    }

    return hb_blob_create(reinterpret_cast<char*>(buffer), tableSize,
                          HB_MEMORY_MODE_WRITABLE, buffer, sk_free);
}

HBFont create_hb_font(const SkFont& font) {
    int index;
    std::unique_ptr<SkStreamAsset> typefaceAsset = font.getTypeface()->openStream(&index);
    HBFace face;
    if (!typefaceAsset) {
        face.reset(hb_face_create_for_tables(
            skhb_get_table,
            reinterpret_cast<void *>(font.refTypeface().release()),
            [](void* user_data){ SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data)); }));
    } else {
        HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
        face.reset(hb_face_create(blob.get(), (unsigned)index));
    }
    SkASSERT(face);
    if (!face) {
        return nullptr;
    }
    hb_face_set_index(face.get(), (unsigned)index);
    hb_face_set_upem(face.get(), font.getTypeface()->getUnitsPerEm());

    HBFont otFont(hb_font_create(face.get()));
    SkASSERT(otFont);
    if (!otFont) {
        return nullptr;
    }
    hb_ot_font_set_funcs(otFont.get());
    int axis_count = font.getTypeface()->getVariationDesignPosition(nullptr, 0);
    if (axis_count > 0) {
        SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> axis_values(axis_count);
        if (font.getTypeface()->getVariationDesignPosition(axis_values, axis_count) == axis_count) {
            hb_font_set_variations(otFont.get(),
                                   reinterpret_cast<hb_variation_t*>(axis_values.get()),
                                   axis_count);
        }
    }

    // Creating a sub font means that non-available functions
    // are found from the parent.
    HBFont skFont(hb_font_create_sub_font(otFont.get()));
    hb_font_set_funcs(skFont.get(), skhb_get_font_funcs(),
                      reinterpret_cast<void *>(new SkFont(font)),
                      [](void* user_data){ delete reinterpret_cast<SkFont*>(user_data); });
    int scale = skhb_position(font.getSize());
    hb_font_set_scale(skFont.get(), scale, scale);

    return skFont;
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
        HBFont hbFont = create_hb_font(font);
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
        if (fFont.unicharToGlyph(u)) {
            fCurrentFont = &fFont;
            fCurrentHBFont = fHBFont.get();
        // If the current fallback can handle this character, use it.
        } else if (fFallbackFont.getTypeface() && fFallbackFont.unicharToGlyph(u)) {
            fCurrentFont = &fFallbackFont;
            fCurrentHBFont = fFallbackHBFont.get();
        // If not, try to find a fallback typeface
        } else {
            sk_sp<SkTypeface> candidate(fFallbackMgr->matchFamilyStyleCharacter(
                nullptr, fFont.getTypeface()->fontStyle(), nullptr, 0, u));
            if (candidate) {
                fFallbackFont.setTypeface(std::move(candidate));
                fFallbackHBFont = create_hb_font(fFallbackFont);
                fCurrentFont = &fFallbackFont;
                fCurrentHBFont = fFallbackHBFont.get();
            } else {
                fCurrentFont = &fFont;
                fCurrentHBFont = fHBFont.get();
            }
        }

        while (fCurrent < fEnd) {
            const char* prev = fCurrent;
            u = utf8_next(&fCurrent, fEnd);

            // End run if not using initial typeface and initial typeface has this character.
            if (fCurrentFont->getTypeface() != fFont.getTypeface() && fFont.unicharToGlyph(u)) {
                fCurrent = prev;
                return;
            }

            // End run if current typeface does not have this character and some other font does.
            if (!fCurrentFont->unicharToGlyph(u)) {
                sk_sp<SkTypeface> candidate(fFallbackMgr->matchFamilyStyleCharacter(
                    nullptr, fFont.getTypeface()->fontStyle(), nullptr, 0, u));
                if (candidate) {
                    fCurrent = prev;
                    return;
                }
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
    ShapedRun(SkShaper::RunHandler::Range utf8Range, const SkFont& font, UBiDiLevel level,
              std::unique_ptr<ShapedGlyph[]> glyphs, size_t numGlyphs, SkVector advance = {0, 0})
        : fUtf8Range(utf8Range), fFont(font), fLevel(level)
        , fGlyphs(std::move(glyphs)), fNumGlyphs(numGlyphs), fAdvance(advance)
    {}

    SkShaper::RunHandler::Range fUtf8Range;
    SkFont fFont;
    UBiDiLevel fLevel;
    std::unique_ptr<ShapedGlyph[]> fGlyphs;
    size_t fNumGlyphs;
    SkVector fAdvance;
};
struct ShapedLine {
    SkTArray<ShapedRun> runs;
    SkVector fAdvance = { 0, 0 };
};

static constexpr bool is_LTR(UBiDiLevel level) {
    return (level & 1) == 0;
}

static void append(SkShaper::RunHandler* handler, const SkShaper::RunHandler::RunInfo& runInfo,
                   const ShapedRun& run, size_t startGlyphIndex, size_t endGlyphIndex,
                   SkPoint* p) {
    SkASSERT(startGlyphIndex <= endGlyphIndex);
    const size_t glyphLen = endGlyphIndex - startGlyphIndex;

    const auto buffer = handler->newRunBuffer(runInfo, run.fFont, glyphLen, run.fUtf8Range);
    SkASSERT(buffer.glyphs);
    SkASSERT(buffer.positions);

    for (size_t i = 0; i < glyphLen; i++) {
        // Glyphs are in logical order, but output ltr since PDF readers seem to expect that.
        const ShapedGlyph& glyph = run.fGlyphs[is_LTR(run.fLevel) ? startGlyphIndex + i
                                                                  : endGlyphIndex - 1 - i];
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
    size_t fGlyphIndex;
};

}  // namespace

class SkShaperHarfBuzz : public SkShaper {
public:
    SkShaperHarfBuzz();
    bool good() const;
private:
    HBBuffer fBuffer;
    ICUBrk fLineBreakIterator;
    ICUBrk fGraphemeBreakIterator;

    SkPoint shape(SkShaper::RunHandler* handler,
                  const SkFont& srcFont,
                  const char* utf8text,
                  size_t textBytes,
                  bool leftToRight,
                  SkPoint point,
                  SkScalar width) const override;

    SkPoint shapeCorrect(RunHandler* handler,
                         char const * const utf8,
                         size_t utf8Bytes,
                         SkPoint point,
                         SkScalar width,
                         RunIteratorQueue& runSegmenter,
                         const BiDiRunIterator* bidi,
                         const LanguageRunIterator* language,
                         const ScriptRunIterator* script,
                         const FontRunIterator* font) const;

    SkPoint shapeOk(SkShaper::RunHandler* handler,
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

std::unique_ptr<SkShaper> SkShaper::MakeHarfBuzz() {
    auto hb = skstd::make_unique<SkShaperHarfBuzz>();
    return hb->good() ? std::move(hb) : nullptr;
}

SkShaperHarfBuzz::SkShaperHarfBuzz() {
#if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        SkDebugf("SkLoadICU() failed!\n");
        return;
    }
#endif
    fBuffer.reset(hb_buffer_create());
    SkASSERT(fBuffer);

    UErrorCode status = U_ZERO_ERROR;
    fLineBreakIterator.reset(ubrk_open(UBRK_LINE, "th", nullptr, 0, &status));
    if (U_FAILURE(status)) {
        SkDebugf("Could not create line break iterator: %s", u_errorName(status));
        SK_ABORT("");
    }

    fGraphemeBreakIterator.reset(ubrk_open(UBRK_CHARACTER, "th", nullptr, 0, &status));
    if (U_FAILURE(status)) {
        SkDebugf("Could not create grapheme break iterator: %s", u_errorName(status));
        SK_ABORT("");
    }

}

bool SkShaperHarfBuzz::good() const {
    return fBuffer &&
           fLineBreakIterator &&
           fGraphemeBreakIterator;
}

SkPoint SkShaperHarfBuzz::shape(SkShaper::RunHandler* handler,
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

    hb_unicode_funcs_t* hbUnicode = hb_buffer_get_unicode_funcs(fBuffer.get());
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
        return shapeCorrect(handler, utf8, utf8Bytes, point, width,
                                   runSegmenter, bidi, language, script, font);
    } else {
        return shapeOk(handler, utf8, utf8Bytes, point, width,
                              runSegmenter, bidi, language, script, font);
    }
}

SkPoint SkShaperHarfBuzz::shapeCorrect(RunHandler* handler,
                                       char const * const utf8,
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

        ShapedRun model(RunHandler::Range(), SkFont(), 0, nullptr, 0);
        bool modelNeedsRegenerated = true;
        int modelGlyphOffset = 0;

        struct TextProps {
            int glyphLen = 0;
            SkVector advance = {0, 0};
        };
        // map from character position to [safe to break, glyph position, advance]
        std::unique_ptr<TextProps[]> modelText;
        int modelTextOffset = 0;
        SkVector modelAdvanceOffset = {0, 0};

        while (utf8Start < utf8End) {  // While there are still code points left in this item
            size_t utf8runLength = utf8End - utf8Start;
            if (modelNeedsRegenerated) {
                model = shape(utf8, utf8Bytes,
                              utf8Start, utf8End,
                              bidi, language, script, font);
                modelGlyphOffset = 0;

                SkVector advance = {0, 0};
                modelText.reset(new TextProps[utf8runLength + 1]());
                size_t modelStartCluster = utf8Start - utf8;
                for (size_t i = 0; i < model.fNumGlyphs; ++i) {
                    SkASSERT(modelStartCluster <= model.fGlyphs[i].fCluster);
                    SkASSERT(                     model.fGlyphs[i].fCluster < (size_t)(utf8End - utf8));
                    if (!model.fGlyphs[i].fUnsafeToBreak) {
                        modelText[model.fGlyphs[i].fCluster - modelStartCluster].glyphLen = i;
                        modelText[model.fGlyphs[i].fCluster - modelStartCluster].advance = advance;
                    }
                    advance += model.fGlyphs[i].fAdvance;
                }
                // Assume it is always safe to break after the end of an item
                modelText[utf8runLength].glyphLen = model.fNumGlyphs;
                modelText[utf8runLength].advance = model.fAdvance;
                modelTextOffset = 0;
                modelAdvanceOffset = {0, 0};
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

            ShapedRun best(RunHandler::Range(), SkFont(), 0, nullptr, 0,
                           { SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity });
            bool bestIsInvalid = true;
            bool bestUsesModelForGlyphs = false;
            SkScalar widthLeft = width - line.fAdvance.fX;

            for (int32_t breakIteratorCurrent = ubrk_next(&breakIterator);
                 breakIteratorCurrent != UBRK_DONE;
                 breakIteratorCurrent = ubrk_next(&breakIterator))
            {
                // TODO: if past a safe to break, future safe to break will be at least as long

                // TODO: adjust breakIteratorCurrent by ignorable whitespace
                bool candidateUsesModelForGlyphs = false;
                ShapedRun candidate = [&](const TextProps& props){
                    if (props.glyphLen) {
                        candidateUsesModelForGlyphs = true;
                        return ShapedRun(RunHandler::Range(utf8Start - utf8, breakIteratorCurrent),
                                         *font->currentFont(), bidi->currentLevel(),
                                         std::unique_ptr<ShapedGlyph[]>(),
                                         props.glyphLen - modelGlyphOffset,
                                         props.advance - modelAdvanceOffset);
                    } else {
                        return shape(utf8, utf8Bytes,
                                     utf8Start, utf8Start + breakIteratorCurrent,
                                     bidi, language, script, font);
                    }
                }(modelText[breakIteratorCurrent + modelTextOffset]);
                auto score = [widthLeft](const ShapedRun& run) -> SkScalar {
                    if (run.fAdvance.fX < widthLeft) {
                        return run.fUtf8Range.size();
                    } else {
                        return widthLeft - run.fAdvance.fX;
                    }
                };
                if (bestIsInvalid || score(best) < score(candidate)) {
                    best = std::move(candidate);
                    bestIsInvalid = false;
                    bestUsesModelForGlyphs = candidateUsesModelForGlyphs;
                }
            }

            // If nothing fit (best score is negative) and the line is not empty
            if (width < line.fAdvance.fX + best.fAdvance.fX && !line.runs.empty()) {
                emit(line, handler, point, currentPoint);
                line.runs.reset();
                line.fAdvance = {0, 0};
            } else {
                if (bestUsesModelForGlyphs) {
                    best.fGlyphs.reset(new ShapedGlyph[best.fNumGlyphs]);
                    memcpy(best.fGlyphs.get(), model.fGlyphs.get() + modelGlyphOffset,
                           best.fNumGlyphs * sizeof(ShapedGlyph));
                    modelGlyphOffset += best.fNumGlyphs;
                    modelTextOffset += best.fUtf8Range.size();
                    modelAdvanceOffset += best.fAdvance;
                } else {
                    modelNeedsRegenerated = true;
                }
                utf8Start += best.fUtf8Range.size();
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

SkPoint SkShaperHarfBuzz::shapeOk(RunHandler* handler,
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

        uint32_t previousCluster = 0xFFFFFFFF;
        for (size_t i = 0; i < run.fNumGlyphs; ++i) {
            ShapedGlyph& glyph = run.fGlyphs[i];
            int32_t glyphCluster = glyph.fCluster;

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
        size_t glyphIndex = glyphIterator.fGlyphIndex;
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

            size_t startGlyphIndex = (logicalIndex == previousBreak.fRunIndex)
                                   ? previousBreak.fGlyphIndex
                                   : 0;
            size_t endGlyphIndex = (logicalIndex == runIndex)
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


ShapedRun SkShaperHarfBuzz::shape(char const * const utf8,
                                  size_t const utf8Bytes,
                                  char const * const utf8Start,
                                  char const *  const utf8End,
                                  BiDiRunIterator const * const bidi,
                                  LanguageRunIterator const * const language,
                                  ScriptRunIterator const * const script,
                                  FontRunIterator const * const font) const
{
    size_t utf8runLength = utf8End - utf8Start;
    ShapedRun run(RunHandler::Range(utf8Start - utf8, utf8runLength),
                  *font->currentFont(), bidi->currentLevel(), nullptr, 0);

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
        unsigned int cluster = utf8Current - utf8;
        hb_codepoint_t u = utf8_next(&utf8Current, utf8End);
        hb_buffer_add(buffer, u, cluster);
    }

    // Add postcontext.
    hb_buffer_add_utf8(buffer, utf8Current, utf8 + utf8Bytes - utf8Current, 0, 0);

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
        return run;
    }

    if (direction == HB_DIRECTION_RTL) {
        // Put the clusters back in logical order.
        // Note that the advances remain ltr.
        hb_buffer_reverse(buffer);
    }
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, nullptr);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer, nullptr);

    run = ShapedRun(RunHandler::Range(utf8Start - utf8, utf8runLength),
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
#if SK_HB_VERSION_CHECK(1, 5, 0)
        glyph.fUnsafeToBreak = info[i].mask & HB_GLYPH_FLAG_UNSAFE_TO_BREAK;
#else
        glyph.fUnsafeToBreak = false;
#endif
        glyph.fMustLineBreakBefore = false;

        runAdvance += glyph.fAdvance;
    }
    run.fAdvance = runAdvance;

    return run;
}
