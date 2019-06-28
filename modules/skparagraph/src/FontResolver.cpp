// Copyright 2019 Google LLC.

#include <queue>
#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skparagraph/src/FontResolver.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkUTF.h"
#include "third_party/externals/icu/source/common/unicode/normalizer2.h"

#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/DartTypes.h"

#include <hb.h>
#include "third_party/externals/harfbuzz/src/hb-icu.h"
#include "third_party/externals/harfbuzz/src/hb-ot.h"
#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/umachine.h>
#include <unicode/urename.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include "src/core/SkMakeUnique.h"
#include "src/core/SkTDPQueue.h"
#include "src/utils/SkUTF.h"

#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

#include <set>
#include "include/private/SkTHash.h"
#include <DartTypes.h>
#include <hb-ot-cmap-table.hh>
#include "modules/skshaper/include/SkShaper.h"
#include "include/core/SkStream.h"

namespace {
SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

bool is_not_base(SkUnichar codepoint) {
    return u_hasBinaryProperty(codepoint, UCHAR_DIACRITIC) ||
           u_hasBinaryProperty(codepoint, UCHAR_EXTENDER);
}

template <class T, void (*P)(T*)>
using resource = std::unique_ptr<T, SkFunctionWrapper<void, T, P>>;
using HBBlob = resource<hb_blob_t, hb_blob_destroy>;
using HBFace = resource<hb_face_t, hb_face_destroy>;
using HBFont = resource<hb_font_t, hb_font_destroy>;
using HBBuffer = resource<hb_buffer_t, hb_buffer_destroy>;
using ICUBiDi = resource<UBiDi, ubidi_close>;
using ICUBrk = resource<UBreakIterator, ubrk_close>;

hb_blob_t* get_cmap_only(hb_face_t* face, hb_tag_t tag, void* user_data) {

    if (tag != HB_OT_TAG_cmap) {
        return nullptr;
    }

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

hb_font_funcs_t* empty_get_font_funcs() {
    static hb_font_funcs_t* const funcs = []{
        // HarfBuzz will use the default (parent) implementation if they aren't set.
        hb_font_funcs_t* const funcs = hb_font_funcs_create();
        hb_font_funcs_make_immutable(funcs);
        return funcs;
    }();
    SkASSERT(funcs);
    return funcs;
}

hb_position_t skhb_position(SkScalar value) {
    // Treat HarfBuzz hb_position_t as 16.16 fixed-point.
    constexpr int kHbPosition1 = 1 << 16;
    return SkScalarRoundToInt(value * kHbPosition1);
}

constexpr bool is_LTR(UBiDiLevel level) {
    return (level & 1) == 0;
}
}  // namespace

// TODO: FontResolver and FontIterator have common functionality
namespace skia {
namespace textlayout {

// TODO: Cache virtual fonts
class VirtualFont {
    // HB Font with CMAP table only so it does not do "heavy" stuff like measuring and shaping
    // but allows to check font coverage
public:
    VirtualFont(const SkFont& font) : fFont(nullptr) {
        SkASSERT(font.getTypeface());
        int index;
        std::unique_ptr<SkStreamAsset> typefaceAsset = font.getTypeface()->openStream(&index);
        HBFace face;
        if (!typefaceAsset) {
            face.reset(hb_face_create_for_tables(
                get_cmap_only,
                reinterpret_cast<void*>(font.refTypeface().release()),
                [](void* user_data) {
                  SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data));
                }));
        } else {
            HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
            face.reset(hb_face_create(blob.get(), (unsigned)index));
        }
        SkASSERT(face);
        if (!face) {
            return;
        }
        hb_face_set_index(face.get(), (unsigned)index);
        hb_face_set_upem(face.get(), font.getTypeface()->getUnitsPerEm());

        HBFont otFont(hb_font_create(face.get()));
        SkASSERT(otFont);
        if (!otFont) {
            return;
        }
        hb_ot_font_set_funcs(otFont.get());
        int axis_count = font.getTypeface()->getVariationDesignPosition(nullptr, 0);
        if (axis_count > 0) {
            SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> axis_values(
                    axis_count);
            if (font.getTypeface()->getVariationDesignPosition(axis_values, axis_count) ==
                axis_count) {
                hb_font_set_variations(otFont.get(),
                                       reinterpret_cast<hb_variation_t*>(axis_values.get()),
                                       axis_count);
            }
        }

        // Creating a sub font means that non-available functions
        // are found from the parent.
        HBFont skFont(hb_font_create_sub_font(otFont.get()));
        hb_font_set_funcs(skFont.get(),
                          empty_get_font_funcs(),
                          reinterpret_cast<void*>(new SkFont(font)),
                          [](void* user_data) { delete reinterpret_cast<SkFont*>(user_data); });
        int scale = skhb_position(font.getSize());
        hb_font_set_scale(skFont.get(), scale, scale);
        fFont = std::move(skFont);
    }

    bool shape(SkSpan<const char> fullText, SkSpan<const char> text, TextDirection textDirection, unsigned& len, hb_glyph_info_t*& info) {
        HBBuffer pbuffer(hb_buffer_create());
        if (!pbuffer) {
            SkDEBUGF("Could not create hb_buffer");
        }
        hb_buffer_t* buffer = pbuffer.get();
        SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);
        hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
        hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

        // See 763e5466c0a03a7c27020e1e2598e488612529a7 for documentation.
        hb_buffer_set_flags(buffer, HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);

        // Add precontext
        hb_buffer_add_utf8(buffer, fullText.begin(), fullText.size(), text.begin() - fullText.begin(), 0);

        // Populate the hb_buffer directly with utf8 cluster indexes.
        const char* utf8Current = text.begin();
        while (utf8Current < text.end()) {
            unsigned int cluster = utf8Current - fullText.begin();
            hb_codepoint_t u = utf8_next(&utf8Current, text.end());
            SkDebugf("codepoint @%d: %d\n", cluster, u);
            hb_buffer_add(buffer, u, cluster);
        }

        // Add postcontext
        hb_buffer_add_utf8(buffer, fullText.begin(), fullText.size(), text.end() - fullText.begin(), 0);

        auto bidi = SkShaper::MakeIcuBiDiRunIterator(
                text.begin(), text.size(),
                textDirection == TextDirection::kLtr ? (uint8_t)2 : (uint8_t)1);
        if (bidi == nullptr) {
            return false;
        }
        auto script = SkShaper::MakeHbIcuScriptRunIterator(text.begin(), text.size());
        auto language = SkShaper::MakeStdLanguageRunIterator(text.begin(), text.size());

        hb_direction_t direction =
                is_LTR(bidi->currentLevel()) ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;
        hb_buffer_set_direction(buffer, direction);
        hb_buffer_set_script(buffer, hb_script_from_iso15924_tag((hb_tag_t)script->currentScript()));
        hb_buffer_set_language(buffer, hb_language_from_string(language->currentLanguage(), -1));
        hb_buffer_guess_segment_properties(buffer);

        if (fFont == nullptr) {
            return false;
        }
        hb_shape(fFont.get(), buffer, nullptr, 0);
        len = hb_buffer_get_length(buffer);
        if (len == 0) {
            return false;
        }

        if (direction == HB_DIRECTION_RTL) {
            // Put the clusters back in logical order.
            // Note that the advances remain ltr.
            hb_buffer_reverse(buffer);
        }
        info = hb_buffer_get_glyph_infos(buffer, nullptr);

        return true;
    }

    hb_font_t* getFont() { return fFont.get(); }
private:
    HBFont fFont;
};

// TODO: Virtual font == Font coverage really
class FontCoverage {

public:
    ~FontCoverage() = default;

    FontCoverage(SkSpan<const char> fullText, SkSpan<const char> text)
    : fFullText(fullText) {
        fGaps.emplace(text);
#if defined(SK_USING_THIRD_PARTY_ICU)
        if (!SkLoadICU()) {
            SkDEBUGF("SkLoadICU() failed!\n");
        }
#endif
    }

    void cover(const SkFont& font,
               TextDirection textDirection,
               std::function<void(const char* ch)> addCoverage) {

        auto text = fGaps.front();
        fGaps.pop();

        VirtualFont virtualFont(font);
        hb_glyph_info_t* info;
        unsigned len;
        if (!virtualFont.shape(fFullText, text, textDirection, len, info)) {
            return;
        }

        std::vector<uint32_t> resolvedClusters;
        std::vector<uint32_t> unresolvedClusters;
        for (unsigned i = 0; i < len; i++) {
            auto glyph = info[i].codepoint;
            auto cluster = info[i].cluster;
            SkDebugf("glyph @%d: %d\n", cluster, glyph);

            if (glyph != 0) {
                if (!unresolvedClusters.empty()) {
                    if (unresolvedClusters.back() == cluster) {
                        // Last cluster is not fully resolved; let's consider this glyph as unresolved
                        SkASSERT(resolvedClusters.empty());
                        continue;
                    }
                    fGaps.emplace(text.begin() + unresolvedClusters.front(), unresolvedClusters.size());
                    unresolvedClusters.clear();
                }

                resolvedClusters.emplace_back(cluster);
                SkASSERT(unresolvedClusters.empty());
            } else {
                // We found unresolved glyph
                if (!resolvedClusters.empty()) {

                    if (resolvedClusters.back() == cluster) {
                        // Last cluster is not fully resolved; let's drop it
                        cluster = resolvedClusters.back();
                        resolvedClusters.pop_back();
                    } else {
                        // TODO: check if the unresolved glyph is a diactitical mark and must be attached to the previous cluster
                        // 1. codepoint is a diacritical mark
                        // 2. it attached to the previous resolved cluster (could be few of them in a row)
                        // Then you should remove the cluster from resolved
                        const char* start = text.begin() + cluster;
                        SkUnichar codepoint = utf8_next(&start, text.end());
                        if (is_not_base(codepoint)) {
                            // Last cluster is not fully resolved; let's drop it
                            cluster = resolvedClusters.back();
                            resolvedClusters.pop_back();
                        }
                    }
                    if (!resolvedClusters.empty()) {
                        addCoverage(fFullText.begin() + resolvedClusters.front());
                        resolvedClusters.clear();
                    }
                } else {
                    // We already added null font to the mapping
                }

                unresolvedClusters.emplace_back(cluster);
                SkASSERT(resolvedClusters.empty());
            }
        }

        if (!resolvedClusters.empty()) {
            SkASSERT(unresolvedClusters.empty());
            addCoverage(fFullText.begin() + resolvedClusters.front());
        } else if (!unresolvedClusters.empty()) {
            SkASSERT(resolvedClusters.empty());
            fGaps.emplace(text.begin() + unresolvedClusters.front(), unresolvedClusters.size());
        }
    }

    bool noGaps() { return fGaps.empty(); }

    void addFirstGap(SkSpan<const char> text) {
        SkASSERT(fGaps.empty());
        fGaps.emplace(text);
    }

    void clearGaps() {
        while (!fGaps.empty()) {
            fGaps.pop();
        }
    }

    bool find(const char* ch, SkFont* font) {
        auto found = fFontMapping.find(ch);
        if (found == nullptr) {
            return false;
        }
        *font = *found;
        return true;
    }

private:

    std::queue<SkSpan<const char>> fGaps;
    SkSpan<const char> fFullText;
    SkTHashMap<const char*, SkFont> fFontMapping;
};

FontResolver::FontResolver(sk_sp<FontCollection> fontCollection, SkSpan<const char> fullText)
        : fFontCollection(fontCollection)
        , fFullText(fullText) {
}

bool FontResolver::findFirst(const char* codepoint, SkFont* font, SkScalar* height) {
    auto result = fFontMapping.find(codepoint);
    if (result == nullptr) {
        return false;
    }

    *font = result->first;
    *height = result->second;
    return true;
}

bool FontResolver::findNext(const char* codepoint, SkFont* font, SkScalar* height) {
    auto result = fFontMapping.find(codepoint);
    if (result == nullptr) {
        return false;
    }

    *font = result->first;
    *height = result->second;
    return true;
}

void FontResolver::findAllFontsForStyledBlock(const TextStyle& style, SkSpan<const char> text) {

    // Walk through all available fonts to resolve the block
    FontCoverage fontCoverage(fFullText, text);

    for (auto& fontFamily : style.getFontFamilies()) {
        if (fontCoverage.noGaps()) {
            break;
        }

        auto typeface = fFontCollection->matchTypeface(fontFamily.c_str(), style.getFontStyle());
        if (typeface.get() == nullptr) {
            continue;
        }

        // Resolve all unresolved characters
        // TODO: text direction
        auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
        fontCoverage.cover(font.first,  TextDirection::kLtr,
                           [this, font](const char* ch) {
                             fFontMapping.set(ch, font);
                           });
    }

    if (fontCoverage.noGaps()) {
        return;
    }

    // In case something still unresolved
    if (fFontCollection->fontFallbackEnabled()) {
        // TODO: Make better fallback
        auto font = makeFont(fFontCollection->defaultFallback(0,
                             style.getFontStyle()),
                             style.getFontSize(), style.getHeight());
        fontCoverage.cover(font.first,  TextDirection::kLtr,
                           [this, font](const char* ch) {
                             fFontMapping.set(ch, font);
                           });
    }

    if (fontCoverage.noGaps()) {
        return;
    }

    // We still have gaps and no fallback; this is the time for last resort font!
    // TODO: last resort font
}

std::pair<SkFont, SkScalar> FontResolver::makeFont(sk_sp<SkTypeface> typeface,
                                                   SkScalar size,
                                                   SkScalar height) {
    SkFont font(typeface, size);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setHinting(SkFontHinting::kSlight);
    font.setSubpixel(true);
    auto pair = std::make_pair(font, height);

    auto foundFont = fResolvedFonts.find(pair);
    if (foundFont == nullptr) {
        fResolvedFonts.add(pair);
    }

    return pair;
}

}  // namespace textlayout
}  // namespace skia
