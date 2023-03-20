/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "include/private/base/SkTypeTraits.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkTDPQueue.h"
#include "src/base/SkUTF.h"
#include "src/core/SkLRUCache.h"

#include <hb.h>
#include <hb-ot.h>
#include <cstring>
#include <locale>
#include <memory>
#include <type_traits>
#include <utility>

using namespace skia_private;

// HB_FEATURE_GLOBAL_START and HB_FEATURE_GLOBAL_END were not added until HarfBuzz 2.0
// They would have always worked, they just hadn't been named yet.
#if !defined(HB_FEATURE_GLOBAL_START)
#  define HB_FEATURE_GLOBAL_START 0
#endif
#if !defined(HB_FEATURE_GLOBAL_END)
# define HB_FEATURE_GLOBAL_END ((unsigned int) -1)
#endif

namespace sknonstd {
template <> struct is_bitmask_enum<hb_buffer_flags_t> : std::true_type {};
}  // namespace sknonstd

namespace {
using HBBlob   = std::unique_ptr<hb_blob_t  , SkFunctionObject<hb_blob_destroy>  >;
using HBFace   = std::unique_ptr<hb_face_t  , SkFunctionObject<hb_face_destroy>  >;
using HBFont   = std::unique_ptr<hb_font_t  , SkFunctionObject<hb_font_destroy>  >;
using HBBuffer = std::unique_ptr<hb_buffer_t, SkFunctionObject<hb_buffer_destroy>>;

using SkUnicodeBidi = std::unique_ptr<SkBidiIterator>;
using SkUnicodeBreak = std::unique_ptr<SkBreakIterator>;

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
    AutoSTMalloc<256, SkUnichar> unicode(count);
    for (unsigned i = 0; i < count; i++) {
        unicode[i] = *unicodes;
        unicodes = SkTAddOffset<const hb_codepoint_t>(unicodes, unicode_stride);
    }
    AutoSTMalloc<256, SkGlyphID> glyph(count);
    font.textToGlyphs(unicode.get(), count * sizeof(SkUnichar), SkTextEncoding::kUTF32,
                        glyph.get(), count);

    // Copy the results back to the sparse array.
    unsigned int done;
    for (done = 0; done < count && glyph[done] != 0; done++) {
        *glyphs = glyph[done];
        glyphs = SkTAddOffset<hb_codepoint_t>(glyphs, glyph_stride);
    }
    // return 'done' to allow HarfBuzz to synthesize with NFC and spaces, return 'count' to avoid
    return done;
}

hb_position_t skhb_glyph_h_advance(hb_font_t* hb_font,
                                   void* font_data,
                                   hb_codepoint_t hbGlyph,
                                   void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    SkScalar advance;
    SkGlyphID skGlyph = SkTo<SkGlyphID>(hbGlyph);

    font.getWidths(&skGlyph, 1, &advance);
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
    AutoSTMalloc<256, SkGlyphID> glyph(count);
    for (unsigned i = 0; i < count; i++) {
        glyph[i] = *glyphs;
        glyphs = SkTAddOffset<const hb_codepoint_t>(glyphs, glyph_stride);
    }
    AutoSTMalloc<256, SkScalar> advance(count);
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
                             hb_codepoint_t hbGlyph,
                             hb_glyph_extents_t* extents,
                             void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);
    SkASSERT(extents);

    SkRect sk_bounds;
    SkGlyphID skGlyph = SkTo<SkGlyphID>(hbGlyph);

    font.getWidths(&skGlyph, 1, nullptr, &sk_bounds);
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

    auto data = typeface.copyTableData(tag);
    if (!data) {
        return nullptr;
    }
    SkData* rawData = data.release();
    return hb_blob_create(reinterpret_cast<char*>(rawData->writable_data()), rawData->size(),
                          HB_MEMORY_MODE_READONLY, rawData, [](void* ctx) {
                              SkSafeUnref(((SkData*)ctx));
                          });
}

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

SkDEBUGCODE(static hb_user_data_key_t gDataIdKey;)

HBFace create_hb_face(const SkTypeface& typeface) {
    int index = 0;
    std::unique_ptr<SkStreamAsset> typefaceAsset = typeface.openExistingStream(&index);
    HBFace face;
    if (typefaceAsset && typefaceAsset->getMemoryBase()) {
        HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
        // hb_face_create always succeeds. Check that the format is minimally recognized first.
        // hb_face_create_for_tables may still create a working hb_face.
        // See https://github.com/harfbuzz/harfbuzz/issues/248 .
        unsigned int num_hb_faces = hb_face_count(blob.get());
        if (0 < num_hb_faces && (unsigned)index < num_hb_faces) {
            face.reset(hb_face_create(blob.get(), (unsigned)index));
            // Check the number of glyphs as a basic sanitization step.
            if (face && hb_face_get_glyph_count(face.get()) == 0) {
                face.reset();
            }
        }
    }
    if (!face) {
        face.reset(hb_face_create_for_tables(
            skhb_get_table,
            const_cast<SkTypeface*>(SkRef(&typeface)),
            [](void* user_data){ SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data)); }));
        hb_face_set_index(face.get(), (unsigned)index);
    }
    SkASSERT(face);
    if (!face) {
        return nullptr;
    }
    hb_face_set_upem(face.get(), typeface.getUnitsPerEm());

    SkDEBUGCODE(
        hb_face_set_user_data(face.get(), &gDataIdKey, const_cast<SkTypeface*>(&typeface),
                              nullptr, false);
    )

    return face;
}

HBFont create_typeface_hb_font(const SkTypeface& typeface) {
    HBFace face(create_hb_face(typeface));
    if (!face) {
        return nullptr;
    }

    HBFont otFont(hb_font_create(face.get()));
    SkASSERT(otFont);
    if (!otFont) {
        return nullptr;
    }
    hb_ot_font_set_funcs(otFont.get());
    int axis_count = typeface.getVariationDesignPosition(nullptr, 0);
    if (axis_count > 0) {
        AutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> axis_values(axis_count);
        if (typeface.getVariationDesignPosition(axis_values, axis_count) == axis_count) {
            hb_font_set_variations(otFont.get(),
                                   reinterpret_cast<hb_variation_t*>(axis_values.get()),
                                   axis_count);
        }
    }

    return otFont;
}

HBFont create_sub_hb_font(const SkFont& font, const HBFont& typefaceFont) {
    SkDEBUGCODE(
        hb_face_t* face = hb_font_get_face(typefaceFont.get());
        void* dataId = hb_face_get_user_data(face, &gDataIdKey);
        SkASSERT(dataId == font.getTypeface());
    )

    // Creating a sub font means that non-available functions
    // are found from the parent.
    HBFont skFont(hb_font_create_sub_font(typefaceFont.get()));
    hb_font_set_funcs(skFont.get(), skhb_get_font_funcs(),
                      reinterpret_cast<void *>(new SkFont(font)),
                      [](void* user_data){ delete reinterpret_cast<SkFont*>(user_data); });
    int scale = skhb_position(font.getSize());
    hb_font_set_scale(skFont.get(), scale, scale);

    return skFont;
}

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

class SkUnicodeBidiRunIterator final : public SkShaper::BiDiRunIterator {
public:
    SkUnicodeBidiRunIterator(const char* utf8, const char* end, SkUnicodeBidi bidi)
        : fBidi(std::move(bidi))
        , fEndOfCurrentRun(utf8)
        , fBegin(utf8)
        , fEnd(end)
        , fUTF16LogicalPosition(0)
        , fLevel(SkBidiIterator::kLTR)
    {}

    void consume() override {
        SkASSERT(fUTF16LogicalPosition < fBidi->getLength());
        int32_t endPosition = fBidi->getLength();
        fLevel = fBidi->getLevelAt(fUTF16LogicalPosition);
        SkUnichar u = utf8_next(&fEndOfCurrentRun, fEnd);
        fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        SkBidiIterator::Level level;
        while (fUTF16LogicalPosition < endPosition) {
            level = fBidi->getLevelAt(fUTF16LogicalPosition);
            if (level != fLevel) {
                break;
            }
            u = utf8_next(&fEndOfCurrentRun, fEnd);

            fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        }
    }
    size_t endOfCurrentRun() const override {
        return fEndOfCurrentRun - fBegin;
    }
    bool atEnd() const override {
        return fUTF16LogicalPosition == fBidi->getLength();
    }
    SkBidiIterator::Level currentLevel() const override {
        return fLevel;
    }
private:
    SkUnicodeBidi fBidi;
    char const * fEndOfCurrentRun;
    char const * const fBegin;
    char const * const fEnd;
    int32_t fUTF16LogicalPosition;
    SkBidiIterator::Level fLevel;
};

class SkUnicodeHbScriptRunIterator final: public SkShaper::ScriptRunIterator {
public:
    SkUnicodeHbScriptRunIterator(const char* utf8,
                                 size_t utf8Bytes,
                                 hb_script_t defaultScript)
            : fCurrent(utf8)
            , fBegin(utf8)
            , fEnd(fCurrent + utf8Bytes)
            , fCurrentScript(defaultScript) {}
    hb_script_t hb_script_for_unichar(SkUnichar u) {
         return hb_unicode_script(hb_unicode_funcs_get_default(), u);
    }
    void consume() override {
        SkASSERT(fCurrent < fEnd);
        SkUnichar u = utf8_next(&fCurrent, fEnd);
        fCurrentScript = hb_script_for_unichar(u);
        while (fCurrent < fEnd) {
            const char* prev = fCurrent;
            u = utf8_next(&fCurrent, fEnd);
            const hb_script_t script = hb_script_for_unichar(u);
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
    size_t endOfCurrentRun() const override {
        return fCurrent - fBegin;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    SkFourByteTag currentScript() const override {
        return SkSetFourByteTag(HB_UNTAG(fCurrentScript));
    }
private:
    char const * fCurrent;
    char const * const fBegin;
    char const * const fEnd;
    hb_script_t fCurrentScript;
};

class RunIteratorQueue {
public:
    void insert(SkShaper::RunIterator* runIterator, int priority) {
        fEntries.insert({runIterator, priority});
    }

    bool advanceRuns() {
        const SkShaper::RunIterator* leastRun = fEntries.peek().runIterator;
        if (leastRun->atEnd()) {
            SkASSERT(this->allRunsAreAtEnd());
            return false;
        }
        const size_t leastEnd = leastRun->endOfCurrentRun();
        SkShaper::RunIterator* currentRun = nullptr;
        SkDEBUGCODE(size_t previousEndOfCurrentRun);
        while ((currentRun = fEntries.peek().runIterator)->endOfCurrentRun() <= leastEnd) {
            int priority = fEntries.peek().priority;
            fEntries.pop();
            SkDEBUGCODE(previousEndOfCurrentRun = currentRun->endOfCurrentRun());
            currentRun->consume();
            SkASSERT(previousEndOfCurrentRun < currentRun->endOfCurrentRun());
            fEntries.insert({currentRun, priority});
        }
        return true;
    }

    size_t endOfCurrentRun() const {
        return fEntries.peek().runIterator->endOfCurrentRun();
    }

private:
    bool allRunsAreAtEnd() const {
        for (int i = 0; i < fEntries.count(); ++i) {
            if (!fEntries.at(i).runIterator->atEnd()) {
                return false;
            }
        }
        return true;
    }

    struct Entry {
        SkShaper::RunIterator* runIterator;
        int priority;
    };
    static bool CompareEntry(Entry const& a, Entry const& b) {
        size_t aEnd = a.runIterator->endOfCurrentRun();
        size_t bEnd = b.runIterator->endOfCurrentRun();
        return aEnd  < bEnd || (aEnd == bEnd && a.priority < b.priority);
    }
    SkTDPQueue<Entry, CompareEntry> fEntries;
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
    ShapedRun(SkShaper::RunHandler::Range utf8Range, const SkFont& font, SkBidiIterator::Level level,
              std::unique_ptr<ShapedGlyph[]> glyphs, size_t numGlyphs, SkVector advance = {0, 0})
        : fUtf8Range(utf8Range), fFont(font), fLevel(level)
        , fGlyphs(std::move(glyphs)), fNumGlyphs(numGlyphs), fAdvance(advance)
    {}

    SkShaper::RunHandler::Range fUtf8Range;
    SkFont fFont;
    SkBidiIterator::Level fLevel;
    std::unique_ptr<ShapedGlyph[]> fGlyphs;
    size_t fNumGlyphs;
    SkVector fAdvance;

    static_assert(::sk_is_trivially_relocatable<decltype(fUtf8Range)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fFont)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fLevel)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fGlyphs)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fAdvance)>::value);

    using sk_is_trivially_relocatable = std::true_type;
};
struct ShapedLine {
    SkTArray<ShapedRun> runs;
    SkVector fAdvance = { 0, 0 };
};

constexpr bool is_LTR(SkBidiIterator::Level level) {
    return (level & 1) == 0;
}

void append(SkShaper::RunHandler* handler, const SkShaper::RunHandler::RunInfo& runInfo,
                   const ShapedRun& run, size_t startGlyphIndex, size_t endGlyphIndex) {
    SkASSERT(startGlyphIndex <= endGlyphIndex);
    const size_t glyphLen = endGlyphIndex - startGlyphIndex;

    const auto buffer = handler->runBuffer(runInfo);
    SkASSERT(buffer.glyphs);
    SkASSERT(buffer.positions);

    SkVector advance = {0,0};
    for (size_t i = 0; i < glyphLen; i++) {
        // Glyphs are in logical order, but output ltr since PDF readers seem to expect that.
        const ShapedGlyph& glyph = run.fGlyphs[is_LTR(run.fLevel) ? startGlyphIndex + i
                                                                  : endGlyphIndex - 1 - i];
        buffer.glyphs[i] = glyph.fID;
        if (buffer.offsets) {
            buffer.positions[i] = advance + buffer.point;
            buffer.offsets[i] = glyph.fOffset;
        } else {
            buffer.positions[i] = advance + buffer.point + glyph.fOffset;
        }
        if (buffer.clusters) {
            buffer.clusters[i] = glyph.fCluster;
        }
        advance += glyph.fAdvance;
    }
    handler->commitRunBuffer(runInfo);
}

void emit(SkUnicode* unicode, const ShapedLine& line, SkShaper::RunHandler* handler) {
    // Reorder the runs and glyphs per line and write them out.
    handler->beginLine();

    int numRuns = line.runs.size();
    AutoSTMalloc<4, SkBidiIterator::Level> runLevels(numRuns);
    for (int i = 0; i < numRuns; ++i) {
        runLevels[i] = line.runs[i].fLevel;
    }
    AutoSTMalloc<4, int32_t> logicalFromVisual(numRuns);
    unicode->reorderVisual(runLevels, numRuns, logicalFromVisual);

    for (int i = 0; i < numRuns; ++i) {
        int logicalIndex = logicalFromVisual[i];

        const auto& run = line.runs[logicalIndex];
        const SkShaper::RunHandler::RunInfo info = {
            run.fFont,
            run.fLevel,
            run.fAdvance,
            run.fNumGlyphs,
            run.fUtf8Range
        };
        handler->runInfo(info);
    }
    handler->commitRunInfo();
    for (int i = 0; i < numRuns; ++i) {
        int logicalIndex = logicalFromVisual[i];

        const auto& run = line.runs[logicalIndex];
        const SkShaper::RunHandler::RunInfo info = {
            run.fFont,
            run.fLevel,
            run.fAdvance,
            run.fNumGlyphs,
            run.fUtf8Range
        };
        append(handler, info, run, 0, run.fNumGlyphs);
    }

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
        SkASSERT(fRunIndex < runs.size());
        SkASSERT(fGlyphIndex < runs[fRunIndex].fNumGlyphs);

        ++fGlyphIndex;
        if (fGlyphIndex == runs[fRunIndex].fNumGlyphs) {
            fGlyphIndex = 0;
            ++fRunIndex;
            if (fRunIndex >= runs.size()) {
                return nullptr;
            }
        }
        return &runs[fRunIndex].fGlyphs[fGlyphIndex];
    }

    ShapedGlyph* current() {
        const SkTArray<ShapedRun>& runs = *fRuns;
        if (fRunIndex >= runs.size()) {
            return nullptr;
        }
        return &runs[fRunIndex].fGlyphs[fGlyphIndex];
    }

    const SkTArray<ShapedRun>* fRuns;
    int fRunIndex;
    size_t fGlyphIndex;
};

class ShaperHarfBuzz : public SkShaper {
public:
    ShaperHarfBuzz(std::unique_ptr<SkUnicode>,
                   SkUnicodeBreak line,
                   SkUnicodeBreak grapheme,
                   HBBuffer,
                   sk_sp<SkFontMgr>);

protected:
    std::unique_ptr<SkUnicode> fUnicode;
    SkUnicodeBreak fLineBreakIterator;
    SkUnicodeBreak fGraphemeBreakIterator;

    ShapedRun shape(const char* utf8, size_t utf8Bytes,
                    const char* utf8Start,
                    const char* utf8End,
                    const BiDiRunIterator&,
                    const LanguageRunIterator&,
                    const ScriptRunIterator&,
                    const FontRunIterator&,
                    const Feature*, size_t featuresSize) const;
private:
    const sk_sp<SkFontMgr> fFontMgr;
    HBBuffer               fBuffer;
    hb_language_t          fUndefinedLanguage;

    void shape(const char* utf8, size_t utf8Bytes,
               const SkFont&,
               bool leftToRight,
               SkScalar width,
               RunHandler*) const override;

    void shape(const char* utf8Text, size_t textBytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               SkScalar width,
               RunHandler*) const override;

    void shape(const char* utf8Text, size_t textBytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               const Feature*, size_t featuresSize,
               SkScalar width,
               RunHandler*) const override;

    virtual void wrap(char const * const utf8, size_t utf8Bytes,
                      const BiDiRunIterator&,
                      const LanguageRunIterator&,
                      const ScriptRunIterator&,
                      const FontRunIterator&,
                      RunIteratorQueue& runSegmenter,
                      const Feature*, size_t featuresSize,
                      SkScalar width,
                      RunHandler*) const = 0;
};

class ShaperDrivenWrapper : public ShaperHarfBuzz {
public:
    using ShaperHarfBuzz::ShaperHarfBuzz;
private:
    void wrap(char const * const utf8, size_t utf8Bytes,
              const BiDiRunIterator&,
              const LanguageRunIterator&,
              const ScriptRunIterator&,
              const FontRunIterator&,
              RunIteratorQueue& runSegmenter,
              const Feature*, size_t featuresSize,
              SkScalar width,
              RunHandler*) const override;
};

class ShapeThenWrap : public ShaperHarfBuzz {
public:
    using ShaperHarfBuzz::ShaperHarfBuzz;
private:
    void wrap(char const * const utf8, size_t utf8Bytes,
              const BiDiRunIterator&,
              const LanguageRunIterator&,
              const ScriptRunIterator&,
              const FontRunIterator&,
              RunIteratorQueue& runSegmenter,
              const Feature*, size_t featuresSize,
              SkScalar width,
              RunHandler*) const override;
};

class ShapeDontWrapOrReorder : public ShaperHarfBuzz {
public:
    using ShaperHarfBuzz::ShaperHarfBuzz;
private:
    void wrap(char const * const utf8, size_t utf8Bytes,
              const BiDiRunIterator&,
              const LanguageRunIterator&,
              const ScriptRunIterator&,
              const FontRunIterator&,
              RunIteratorQueue& runSegmenter,
              const Feature*, size_t featuresSize,
              SkScalar width,
              RunHandler*) const override;
};

static std::unique_ptr<SkShaper> MakeHarfBuzz(sk_sp<SkFontMgr> fontmgr, bool correct) {
    HBBuffer buffer(hb_buffer_create());
    if (!buffer) {
        SkDEBUGF("Could not create hb_buffer");
        return nullptr;
    }

    auto unicode = SkUnicode::Make();
    if (!unicode) {
        return nullptr;
    }

    const auto lname = std::locale().name();
    auto lineIter = unicode->makeBreakIterator(lname.c_str(), SkUnicode::BreakType::kLines);
    if (!lineIter) {
        return nullptr;
    }
    auto graphIter = unicode->makeBreakIterator(lname.c_str(), SkUnicode::BreakType::kGraphemes);
    if (!graphIter) {
        return nullptr;
    }

    if (correct) {
        return std::make_unique<ShaperDrivenWrapper>(std::move(unicode),
            std::move(lineIter), std::move(graphIter), std::move(buffer), std::move(fontmgr));
    } else {
        return std::make_unique<ShapeThenWrap>(std::move(unicode),
            std::move(lineIter), std::move(graphIter), std::move(buffer), std::move(fontmgr));
    }
}

ShaperHarfBuzz::ShaperHarfBuzz(std::unique_ptr<SkUnicode> unicode,
    SkUnicodeBreak lineIter, SkUnicodeBreak graphIter, HBBuffer buffer, sk_sp<SkFontMgr> fontmgr)
    : fUnicode(std::move(unicode))
    , fLineBreakIterator(std::move(lineIter))
    , fGraphemeBreakIterator(std::move(graphIter))
    , fFontMgr(std::move(fontmgr))
    , fBuffer(std::move(buffer))
    , fUndefinedLanguage(hb_language_from_string("und", -1))
{ }

void ShaperHarfBuzz::shape(const char* utf8, size_t utf8Bytes,
                           const SkFont& srcFont,
                           bool leftToRight,
                           SkScalar width,
                           RunHandler* handler) const
{
    SkBidiIterator::Level defaultLevel = leftToRight ? SkBidiIterator::kLTR : SkBidiIterator::kRTL;
    std::unique_ptr<BiDiRunIterator> bidi(MakeSkUnicodeBidiRunIterator(fUnicode.get(),
                                                                       utf8,
                                                                       utf8Bytes,
                                                                       defaultLevel));

    if (!bidi) {
        return;
    }

    std::unique_ptr<LanguageRunIterator> language(MakeStdLanguageRunIterator(utf8, utf8Bytes));
    if (!language) {
        return;
    }

    std::unique_ptr<ScriptRunIterator> script(MakeSkUnicodeHbScriptRunIterator(utf8, utf8Bytes));
    if (!script) {
        return;
    }

    std::unique_ptr<FontRunIterator> font(
                MakeFontMgrRunIterator(utf8, utf8Bytes, srcFont,
                                       fFontMgr ? fFontMgr : SkFontMgr::RefDefault()));
    if (!font) {
        return;
    }

    this->shape(utf8, utf8Bytes, *font, *bidi, *script, *language, width, handler);
}

void ShaperHarfBuzz::shape(const char* utf8, size_t utf8Bytes,
                           FontRunIterator& font,
                           BiDiRunIterator& bidi,
                           ScriptRunIterator& script,
                           LanguageRunIterator& language,
                           SkScalar width,
                           RunHandler* handler) const
{
    this->shape(utf8, utf8Bytes, font, bidi, script, language, nullptr, 0, width, handler);
}

void ShaperHarfBuzz::shape(const char* utf8, size_t utf8Bytes,
                           FontRunIterator& font,
                           BiDiRunIterator& bidi,
                           ScriptRunIterator& script,
                           LanguageRunIterator& language,
                           const Feature* features, size_t featuresSize,
                           SkScalar width,
                           RunHandler* handler) const
{
    SkASSERT(handler);
    RunIteratorQueue runSegmenter;
    runSegmenter.insert(&font,     3); // The font iterator is always run last in case of tie.
    runSegmenter.insert(&bidi,     2);
    runSegmenter.insert(&script,   1);
    runSegmenter.insert(&language, 0);

    this->wrap(utf8, utf8Bytes, bidi, language, script, font, runSegmenter,
               features, featuresSize, width, handler);
}

void ShaperDrivenWrapper::wrap(char const * const utf8, size_t utf8Bytes,
                               const BiDiRunIterator& bidi,
                               const LanguageRunIterator& language,
                               const ScriptRunIterator& script,
                               const FontRunIterator& font,
                               RunIteratorQueue& runSegmenter,
                               const Feature* features, size_t featuresSize,
                               SkScalar width,
                               RunHandler* handler) const
{
    ShapedLine line;

    const char* utf8Start = nullptr;
    const char* utf8End = utf8;
    while (runSegmenter.advanceRuns()) {  // For each item
        utf8Start = utf8End;
        utf8End = utf8 + runSegmenter.endOfCurrentRun();

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
                              bidi, language, script, font,
                              features, featuresSize);
                modelGlyphOffset = 0;

                SkVector advance = {0, 0};
                modelText = std::make_unique<TextProps[]>(utf8runLength + 1);
                size_t modelStartCluster = utf8Start - utf8;
                size_t previousCluster = 0;
                for (size_t i = 0; i < model.fNumGlyphs; ++i) {
                    SkASSERT(modelStartCluster <= model.fGlyphs[i].fCluster);
                    SkASSERT(                     model.fGlyphs[i].fCluster < (size_t)(utf8End - utf8));
                    if (!model.fGlyphs[i].fUnsafeToBreak) {
                        // Store up to the first glyph in the cluster.
                        size_t currentCluster = model.fGlyphs[i].fCluster - modelStartCluster;
                        if (previousCluster != currentCluster) {
                            previousCluster  = currentCluster;
                            modelText[currentCluster].glyphLen = i;
                            modelText[currentCluster].advance = advance;
                        }
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
            if (!fLineBreakIterator->setText(utf8Start, utf8runLength)) {
                return;
            }
            SkBreakIterator& breakIterator = *fLineBreakIterator;

            ShapedRun best(RunHandler::Range(), SkFont(), 0, nullptr, 0,
                           { SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity });
            bool bestIsInvalid = true;
            bool bestUsesModelForGlyphs = false;
            SkScalar widthLeft = width - line.fAdvance.fX;

            for (int32_t breakIteratorCurrent = breakIterator.next();
                 !breakIterator.isDone();
                 breakIteratorCurrent = breakIterator.next())
            {
                // TODO: if past a safe to break, future safe to break will be at least as long

                // TODO: adjust breakIteratorCurrent by ignorable whitespace
                bool candidateUsesModelForGlyphs = false;
                ShapedRun candidate = [&](const TextProps& props){
                    if (props.glyphLen) {
                        candidateUsesModelForGlyphs = true;
                        return ShapedRun(RunHandler::Range(utf8Start - utf8, breakIteratorCurrent),
                                         font.currentFont(), bidi.currentLevel(),
                                         std::unique_ptr<ShapedGlyph[]>(),
                                         props.glyphLen - modelGlyphOffset,
                                         props.advance - modelAdvanceOffset);
                    } else {
                        return shape(utf8, utf8Bytes,
                                     utf8Start, utf8Start + breakIteratorCurrent,
                                     bidi, language, script, font,
                                     features, featuresSize);
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
                emit(fUnicode.get(), line, handler);
                line.runs.clear();
                line.fAdvance = {0, 0};
            } else {
                if (bestUsesModelForGlyphs) {
                    best.fGlyphs = std::make_unique<ShapedGlyph[]>(best.fNumGlyphs);
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
                    emit(fUnicode.get(), line, handler);
                    line.runs.clear();
                    line.fAdvance = {0, 0};
                }
            }
        }
    }
    emit(fUnicode.get(), line, handler);
}

void ShapeThenWrap::wrap(char const * const utf8, size_t utf8Bytes,
                         const BiDiRunIterator& bidi,
                         const LanguageRunIterator& language,
                         const ScriptRunIterator& script,
                         const FontRunIterator& font,
                         RunIteratorQueue& runSegmenter,
                         const Feature* features, size_t featuresSize,
                         SkScalar width,
                         RunHandler* handler) const
{
    SkTArray<ShapedRun> runs;
{
    if (!fLineBreakIterator->setText(utf8, utf8Bytes)) {
        return;
    }
    if (!fGraphemeBreakIterator->setText(utf8, utf8Bytes)) {
        return;
    }

    SkBreakIterator& lineBreakIterator = *fLineBreakIterator;
    SkBreakIterator& graphemeBreakIterator = *fGraphemeBreakIterator;
    const char* utf8Start = nullptr;
    const char* utf8End = utf8;
    while (runSegmenter.advanceRuns()) {
        utf8Start = utf8End;
        utf8End = utf8 + runSegmenter.endOfCurrentRun();

        runs.emplace_back(shape(utf8, utf8Bytes,
                                utf8Start, utf8End,
                                bidi, language, script, font,
                                features, featuresSize));
        ShapedRun& run = runs.back();

        uint32_t previousCluster = 0xFFFFFFFF;
        for (size_t i = 0; i < run.fNumGlyphs; ++i) {
            ShapedGlyph& glyph = run.fGlyphs[i];
            int32_t glyphCluster = glyph.fCluster;

            int32_t lineBreakIteratorCurrent = lineBreakIterator.current();
            while (!lineBreakIterator.isDone() && lineBreakIteratorCurrent < glyphCluster)
            {
                lineBreakIteratorCurrent = lineBreakIterator.next();
            }
            glyph.fMayLineBreakBefore = glyph.fCluster != previousCluster &&
                                        lineBreakIteratorCurrent == glyphCluster;

            int32_t graphemeBreakIteratorCurrent = graphemeBreakIterator.current();
            while (!graphemeBreakIterator.isDone() && graphemeBreakIteratorCurrent < glyphCluster)
            {
                graphemeBreakIteratorCurrent = graphemeBreakIterator.next();
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
{
    ShapedRunGlyphIterator previousBreak(runs);
    ShapedRunGlyphIterator glyphIterator(runs);
    int previousRunIndex = -1;
    while (glyphIterator.current()) {
        const ShapedRunGlyphIterator current = glyphIterator;
        ShapedGlyph* nextGlyph = glyphIterator.next();

        if (previousRunIndex != current.fRunIndex) {
            SkFontMetrics metrics;
            runs[current.fRunIndex].fFont.getMetrics(&metrics);
            previousRunIndex = current.fRunIndex;
        }

        // Nothing can be written until the baseline is known.
        if (!(nextGlyph == nullptr || nextGlyph->fMustLineBreakBefore)) {
            continue;
        }

        int numRuns = current.fRunIndex - previousBreak.fRunIndex + 1;
        AutoSTMalloc<4, SkBidiIterator::Level> runLevels(numRuns);
        for (int i = 0; i < numRuns; ++i) {
            runLevels[i] = runs[previousBreak.fRunIndex + i].fLevel;
        }
        AutoSTMalloc<4, int32_t> logicalFromVisual(numRuns);
        fUnicode->reorderVisual(runLevels, numRuns, logicalFromVisual);

        // step through the runs in reverse visual order and the glyphs in reverse logical order
        // until a visible glyph is found and force them to the end of the visual line.

        handler->beginLine();

        struct SubRun { const ShapedRun& run; size_t startGlyphIndex; size_t endGlyphIndex; };
        auto makeSubRun = [&runs, &previousBreak, &current, &logicalFromVisual](size_t visualIndex){
            int logicalIndex = previousBreak.fRunIndex + logicalFromVisual[visualIndex];
            const auto& run = runs[logicalIndex];
            size_t startGlyphIndex = (logicalIndex == previousBreak.fRunIndex)
                                   ? previousBreak.fGlyphIndex
                                   : 0;
            size_t endGlyphIndex = (logicalIndex == current.fRunIndex)
                                 ? current.fGlyphIndex + 1
                                 : run.fNumGlyphs;
            return SubRun{ run, startGlyphIndex, endGlyphIndex };
        };
        auto makeRunInfo = [](const SubRun& sub) {
            uint32_t startUtf8 = sub.run.fGlyphs[sub.startGlyphIndex].fCluster;
            uint32_t endUtf8 = (sub.endGlyphIndex < sub.run.fNumGlyphs)
                             ? sub.run.fGlyphs[sub.endGlyphIndex].fCluster
                             : sub.run.fUtf8Range.end();

            SkVector advance = SkVector::Make(0, 0);
            for (size_t i = sub.startGlyphIndex; i < sub.endGlyphIndex; ++i) {
                advance += sub.run.fGlyphs[i].fAdvance;
            }

            return RunHandler::RunInfo{
                sub.run.fFont,
                sub.run.fLevel,
                advance,
                sub.endGlyphIndex - sub.startGlyphIndex,
                RunHandler::Range(startUtf8, endUtf8 - startUtf8)
            };
        };

        for (int i = 0; i < numRuns; ++i) {
            handler->runInfo(makeRunInfo(makeSubRun(i)));
        }
        handler->commitRunInfo();
        for (int i = 0; i < numRuns; ++i) {
            SubRun sub = makeSubRun(i);
            append(handler, makeRunInfo(sub), sub.run, sub.startGlyphIndex, sub.endGlyphIndex);
        }

        handler->commitLine();

        previousRunIndex = -1;
        previousBreak = glyphIterator;
    }
}
}

void ShapeDontWrapOrReorder::wrap(char const * const utf8, size_t utf8Bytes,
                                  const BiDiRunIterator& bidi,
                                  const LanguageRunIterator& language,
                                  const ScriptRunIterator& script,
                                  const FontRunIterator& font,
                                  RunIteratorQueue& runSegmenter,
                                  const Feature* features, size_t featuresSize,
                                  SkScalar width,
                                  RunHandler* handler) const
{
    sk_ignore_unused_variable(width);
    SkTArray<ShapedRun> runs;

    const char* utf8Start = nullptr;
    const char* utf8End = utf8;
    while (runSegmenter.advanceRuns()) {
        utf8Start = utf8End;
        utf8End = utf8 + runSegmenter.endOfCurrentRun();

        runs.emplace_back(shape(utf8, utf8Bytes,
                                utf8Start, utf8End,
                                bidi, language, script, font,
                                features, featuresSize));
    }

    handler->beginLine();
    for (const auto& run : runs) {
        const RunHandler::RunInfo info = {
            run.fFont,
            run.fLevel,
            run.fAdvance,
            run.fNumGlyphs,
            run.fUtf8Range
        };
        handler->runInfo(info);
    }
    handler->commitRunInfo();
    for (const auto& run : runs) {
        const RunHandler::RunInfo info = {
            run.fFont,
            run.fLevel,
            run.fAdvance,
            run.fNumGlyphs,
            run.fUtf8Range
        };
        append(handler, info, run, 0, run.fNumGlyphs);
    }
    handler->commitLine();
}

class HBLockedFaceCache {
public:
    HBLockedFaceCache(SkLRUCache<SkTypefaceID, HBFont>& lruCache, SkMutex& mutex)
        : fLRUCache(lruCache), fMutex(mutex)
    {
        fMutex.acquire();
    }
    HBLockedFaceCache(const HBLockedFaceCache&) = delete;
    HBLockedFaceCache& operator=(const HBLockedFaceCache&) = delete;
    HBLockedFaceCache& operator=(HBLockedFaceCache&&) = delete;

    ~HBLockedFaceCache() {
        fMutex.release();
    }

    HBFont* find(SkTypefaceID fontId) {
        return fLRUCache.find(fontId);
    }
    HBFont* insert(SkTypefaceID fontId, HBFont hbFont) {
        return fLRUCache.insert(fontId, std::move(hbFont));
    }
    void reset() {
        fLRUCache.reset();
    }
private:
    SkLRUCache<SkTypefaceID, HBFont>& fLRUCache;
    SkMutex& fMutex;
};
static HBLockedFaceCache get_hbFace_cache() {
    static SkMutex gHBFaceCacheMutex;
    static SkLRUCache<SkTypefaceID, HBFont> gHBFaceCache(100);
    return HBLockedFaceCache(gHBFaceCache, gHBFaceCacheMutex);
}

ShapedRun ShaperHarfBuzz::shape(char const * const utf8,
                                  size_t const utf8Bytes,
                                  char const * const utf8Start,
                                  char const * const utf8End,
                                  const BiDiRunIterator& bidi,
                                  const LanguageRunIterator& language,
                                  const ScriptRunIterator& script,
                                  const FontRunIterator& font,
                                  Feature const * const features, size_t const featuresSize) const
{
    size_t utf8runLength = utf8End - utf8Start;
    ShapedRun run(RunHandler::Range(utf8Start - utf8, utf8runLength),
                  font.currentFont(), bidi.currentLevel(), nullptr, 0);

    hb_buffer_t* buffer = fBuffer.get();
    SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);
    hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
    hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

    // Documentation for HB_BUFFER_FLAG_BOT/EOT at 763e5466c0a03a7c27020e1e2598e488612529a7.
    // Currently BOT forces a dotted circle when first codepoint is a mark; EOT has no effect.
    // Avoid adding dotted circle, re-evaluate if BOT/EOT change. See https://skbug.com/9618.
    // hb_buffer_set_flags(buffer, HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);

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

    hb_direction_t direction = is_LTR(bidi.currentLevel()) ? HB_DIRECTION_LTR:HB_DIRECTION_RTL;
    hb_buffer_set_direction(buffer, direction);
    hb_buffer_set_script(buffer, hb_script_from_iso15924_tag((hb_tag_t)script.currentScript()));
    // Buffers with HB_LANGUAGE_INVALID race since hb_language_get_default is not thread safe.
    // The user must provide a language, but may provide data hb_language_from_string cannot use.
    // Use "und" for the undefined language in this case (RFC5646 4.1 5).
    hb_language_t hbLanguage = hb_language_from_string(language.currentLanguage(), -1);
    if (hbLanguage == HB_LANGUAGE_INVALID) {
        hbLanguage = fUndefinedLanguage;
    }
    hb_buffer_set_language(buffer, hbLanguage);
    hb_buffer_guess_segment_properties(buffer);

    // TODO: better cache HBFace (data) / hbfont (typeface)
    // An HBFace is expensive (it sanitizes the bits).
    // An HBFont is fairly inexpensive.
    // An HBFace is actually tied to the data, not the typeface.
    // The size of 100 here is completely arbitrary and used to match libtxt.
    HBFont hbFont;
    {
        HBLockedFaceCache cache = get_hbFace_cache();
        SkTypefaceID dataId = font.currentFont().getTypeface()->uniqueID();
        HBFont* typefaceFontCached = cache.find(dataId);
        if (!typefaceFontCached) {
            HBFont typefaceFont(create_typeface_hb_font(*font.currentFont().getTypeface()));
            typefaceFontCached = cache.insert(dataId, std::move(typefaceFont));
        }
        hbFont = create_sub_hb_font(font.currentFont(), *typefaceFontCached);
    }
    if (!hbFont) {
        return run;
    }

    SkSTArray<32, hb_feature_t> hbFeatures;
    for (const auto& feature : SkSpan(features, featuresSize)) {
        if (feature.end < SkTo<size_t>(utf8Start - utf8) ||
                          SkTo<size_t>(utf8End   - utf8)  <= feature.start)
        {
            continue;
        }
        if (feature.start <= SkTo<size_t>(utf8Start - utf8) &&
                             SkTo<size_t>(utf8End   - utf8) <= feature.end)
        {
            hbFeatures.push_back({ (hb_tag_t)feature.tag, feature.value,
                                   HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END});
        } else {
            hbFeatures.push_back({ (hb_tag_t)feature.tag, feature.value,
                                   SkTo<unsigned>(feature.start), SkTo<unsigned>(feature.end)});
        }
    }

    hb_shape(hbFont.get(), buffer, hbFeatures.data(), hbFeatures.size());
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
                    font.currentFont(), bidi.currentLevel(),
                    std::unique_ptr<ShapedGlyph[]>(new ShapedGlyph[len]), len);

    // Undo skhb_position with (1.0/(1<<16)) and scale as needed.
    AutoSTArray<32, SkGlyphID> glyphIDs(len);
    for (unsigned i = 0; i < len; i++) {
        glyphIDs[i] = info[i].codepoint;
    }
    AutoSTArray<32, SkRect> glyphBounds(len);
    SkPaint p;
    run.fFont.getBounds(glyphIDs.get(), len, glyphBounds.get(), &p);

    double SkScalarFromHBPosX = +(1.52587890625e-5) * run.fFont.getScaleX();
    double SkScalarFromHBPosY = -(1.52587890625e-5);  // HarfBuzz y-up, Skia y-down
    SkVector runAdvance = { 0, 0 };
    for (unsigned i = 0; i < len; i++) {
        ShapedGlyph& glyph = run.fGlyphs[i];
        glyph.fID = info[i].codepoint;
        glyph.fCluster = info[i].cluster;
        glyph.fOffset.fX = pos[i].x_offset * SkScalarFromHBPosX;
        glyph.fOffset.fY = pos[i].y_offset * SkScalarFromHBPosY;
        glyph.fAdvance.fX = pos[i].x_advance * SkScalarFromHBPosX;
        glyph.fAdvance.fY = pos[i].y_advance * SkScalarFromHBPosY;

        glyph.fHasVisual = !glyphBounds[i].isEmpty(); //!font->currentTypeface()->glyphBoundsAreZero(glyph.fID);
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

}  // namespace

std::unique_ptr<SkShaper::BiDiRunIterator>
SkShaper::MakeIcuBiDiRunIterator(const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) {
    auto unicode = SkUnicode::Make();
    if (!unicode) {
        return nullptr;
    }
    return SkShaper::MakeSkUnicodeBidiRunIterator(unicode.get(),
                                                  utf8,
                                                  utf8Bytes,
                                                  bidiLevel);
}

std::unique_ptr<SkShaper::BiDiRunIterator>
SkShaper::MakeSkUnicodeBidiRunIterator(SkUnicode* unicode, const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) {
    // ubidi only accepts utf16 (though internally it basically works on utf32 chars).
    // We want an ubidi_setPara(UBiDi*, UText*, UBiDiLevel, UBiDiLevel*, UErrorCode*);
    if (!SkTFitsIn<int32_t>(utf8Bytes)) {
        SkDEBUGF("Bidi error: text too long");
        return nullptr;
    }

    int32_t utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Bytes);
    if (utf16Units < 0) {
        SkDEBUGF("Invalid utf8 input\n");
        return nullptr;
    }

    std::unique_ptr<uint16_t[]> utf16(new uint16_t[utf16Units]);
    (void)SkUTF::UTF8ToUTF16(utf16.get(), utf16Units, utf8, utf8Bytes);

    auto bidiDir = (bidiLevel % 2 == 0) ? SkBidiIterator::kLTR : SkBidiIterator::kRTL;
    SkUnicodeBidi bidi = unicode->makeBidiIterator(utf16.get(), utf16Units, bidiDir);
    if (!bidi) {
        SkDEBUGF("Bidi error\n");
        return nullptr;
    }

    return std::make_unique<SkUnicodeBidiRunIterator>(utf8, utf8 + utf8Bytes, std::move(bidi));
}

std::unique_ptr<SkShaper::ScriptRunIterator>
SkShaper::MakeHbIcuScriptRunIterator(const char* utf8, size_t utf8Bytes) {
    return SkShaper::MakeSkUnicodeHbScriptRunIterator(utf8, utf8Bytes);
}

std::unique_ptr<SkShaper::ScriptRunIterator>
SkShaper::MakeSkUnicodeHbScriptRunIterator(const char* utf8, size_t utf8Bytes) {
    return std::make_unique<SkUnicodeHbScriptRunIterator>(utf8, utf8Bytes, HB_SCRIPT_UNKNOWN);
}

std::unique_ptr<SkShaper::ScriptRunIterator> SkShaper::MakeSkUnicodeHbScriptRunIterator(
        const char* utf8, size_t utf8Bytes, SkFourByteTag script) {
    return std::make_unique<SkUnicodeHbScriptRunIterator>(
            utf8, utf8Bytes, hb_script_from_iso15924_tag((hb_tag_t)script));
}

std::unique_ptr<SkShaper> SkShaper::MakeShaperDrivenWrapper(sk_sp<SkFontMgr> fontmgr) {
    return MakeHarfBuzz(std::move(fontmgr), true);
}
std::unique_ptr<SkShaper> SkShaper::MakeShapeThenWrap(sk_sp<SkFontMgr> fontmgr) {
    return MakeHarfBuzz(std::move(fontmgr), false);
}
std::unique_ptr<SkShaper> SkShaper::MakeShapeDontWrapOrReorder(std::unique_ptr<SkUnicode> unicode,
                                                               sk_sp<SkFontMgr> fontmgr) {
    HBBuffer buffer(hb_buffer_create());
    if (!buffer) {
        SkDEBUGF("Could not create hb_buffer");
        return nullptr;
    }

    if (!unicode) {
        return nullptr;
    }

    return std::make_unique<ShapeDontWrapOrReorder>
        (std::move(unicode), nullptr, nullptr, std::move(buffer), std::move(fontmgr));
}

void SkShaper::PurgeHarfBuzzCache() {
    HBLockedFaceCache cache = get_hbFace_cache();
    cache.reset();
}
