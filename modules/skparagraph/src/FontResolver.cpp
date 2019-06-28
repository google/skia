// Copyright 2019 Google LLC.

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

class VirtualFont {
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

    hb_font_t* getFont() { return fFont.get(); }
private:
    HBFont fFont;
};

class FontCoverage {

public:
    ~FontCoverage() = default;

    explicit FontCoverage(SkSpan<const char> fullText)
    : fFullText(fullText)
    , fUnresolved(fullText.size()) {
    }

    bool initialize() {

#if defined(SK_USING_THIRD_PARTY_ICU)
        if (!SkLoadICU()) {
            SkDEBUGF("SkLoadICU() failed!\n");
            return false;
        }
#endif
        return true;
    }

    void scanGaps(SkSpan<const char> text, std::function<void(SkSpan<const char> text)> visit) {
        // Detect all not covered part of the text and scan them
        const char* start = text.begin();
        for (auto& ch : text) {
            auto font = fFontMapping.find(&ch);
            if (font == nullptr) {
                continue;
            }
            if (font->getTypeface() != nullptr) {
                if (start != nullptr) {
                    if (start != &ch) {
                        visit(SkSpan<const char>(start, &ch - start));
                    }
                    start = nullptr;
                }
            } else {
                if (start == nullptr) {
                    start = &ch;
                }
            }
        }
        if (start != nullptr) {
            visit(SkSpan<const char>(start, text.end() - start));
        }
    }

    size_t cover(SkSpan<const char> text, const SkFont& font, TextDirection textDirection) {

        HBBuffer pbuffer(hb_buffer_create());
        if (!pbuffer) {
            SkDEBUGF("Could not create hb_buffer");
            return false;
        }
        hb_buffer_t* buffer = pbuffer.get();
        SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);
        hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
        hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

        // See 763e5466c0a03a7c27020e1e2598e488612529a7 for documentation.
        hb_buffer_set_flags(buffer, HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);

        // Add precontext
        hb_buffer_add_utf8(buffer, fFullText.begin(), fFullText.size(), text.begin() - fFullText.begin(), 0);

        // Populate the hb_buffer directly with utf8 cluster indexes.
        const char* utf8Current = text.begin();
        while (utf8Current < text.end()) {
            unsigned int cluster = utf8Current - fFullText.begin();
            hb_codepoint_t u = utf8_next(&utf8Current, text.end());
            SkDebugf("codepoint @%d: %d\n", cluster, u);
            hb_buffer_add(buffer, u, cluster);
        }

        // Add postcontext
        hb_buffer_add_utf8(buffer, fFullText.begin(), fFullText.size(), text.end() - fFullText.begin(), 0);

        auto bidi = SkShaper::MakeIcuBiDiRunIterator(
                text.begin(), text.size(),
                textDirection == TextDirection::kLtr ? (uint8_t)2 : (uint8_t)1);
        if (bidi == nullptr) {
            return 0;
        }
        auto script = SkShaper::MakeHbIcuScriptRunIterator(text.begin(), text.size());
        auto language = SkShaper::MakeStdLanguageRunIterator(text.begin(), text.size());

        hb_direction_t direction =
                is_LTR(bidi->currentLevel()) ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;
        hb_buffer_set_direction(buffer, direction);
        hb_buffer_set_script(buffer, hb_script_from_iso15924_tag((hb_tag_t)script->currentScript()));
        hb_buffer_set_language(buffer, hb_language_from_string(language->currentLanguage(), -1));
        hb_buffer_guess_segment_properties(buffer);

        VirtualFont virtualFont(font);
        if (virtualFont.getFont() == nullptr) {
            return 0;
        }
        hb_shape(virtualFont.getFont(), buffer, nullptr, 0);
        unsigned len = hb_buffer_get_length(buffer);
        if (len == 0) {
            return 0;
        }

        if (direction == HB_DIRECTION_RTL) {
            // Put the clusters back in logical order.
            // Note that the advances remain ltr.
            hb_buffer_reverse(buffer);
        }
        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, nullptr);

        size_t notCovered = 0;
        size_t covered = 0;
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
                    fFontMapping.set(text.begin() + unresolvedClusters.front(), SkFont());
                    notCovered += unresolvedClusters.size();
                    unresolvedClusters.clear();
                }

                if (resolvedClusters.empty()) {
                    // Start counting resolved glyphs
                    resolvedClusters.emplace_back(cluster);
                } else {
                    // Continue counting resolved glyphs
                }
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
                        // 1. codepoint is a diactitical mark
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
                        fFontMapping.set(fFullText.begin() + resolvedClusters.front(), font);
                        covered += resolvedClusters.size();
                        resolvedClusters.clear();
                    }
                    fFontMapping.set(fFullText.begin() + cluster, SkFont());
                } else {
                    // We already added null font to the mapping
                }

                SkASSERT(resolvedClusters.empty());
            }
        }

        if (!resolvedClusters.empty()) {
            fFontMapping.set(fFullText.begin() + resolvedClusters.front(), font);
            covered += resolvedClusters.size();
        } else if (!unresolvedClusters.empty()) {
            fFontMapping.set(fFullText.begin() + unresolvedClusters.front(), font);
            notCovered += unresolvedClusters.size();
        }

        fFirstResolved = fFullText.end();
        if (fFontMapping.count() == 0) {
            fUnresolved = fFullText.size();
            SkDebugf("\nResolution map is empty\n");
            return 0;
        }

        // Recalculate the unresolved count
        fUnresolved = 0;
        const char* unresolved = nullptr;
        for (auto& ch : fFullText) {
            auto font = fFontMapping.find(&ch);
            if (font != nullptr) {
                if (font->getTypeface() != nullptr) {
                    if (fFirstResolved == fFullText.end()) {
                        fFirstResolved = &ch;
                    }
                    SkString name;
                    font->getTypeface()->getFamilyName(&name);
                    SkDebugf("Resolved from %d: %s\n", &ch - fFullText.begin(), name.c_str());
                    if (unresolved != nullptr) {
                        fUnresolved += &ch - unresolved;
                        unresolved = nullptr;
                    }
                } else {
                    SkDebugf("Unresolved from %d\n", &ch - fFullText.begin());
                    if (unresolved == nullptr) {
                        unresolved = &ch;
                    }
                }
            } else {
                // Continue with the current
            }
        }
        if (unresolved != nullptr) {
            fUnresolved += fFullText.end() - unresolved;
        }

        return covered;
    }

    bool find(const char* ch, SkFont* font) {
        auto found = fFontMapping.find(ch);
        if (found == nullptr) {
            return false;
        }
        *font = *found;
        return true;
    }

    size_t unresolved() { return fUnresolved; }

    const char* firstResolved() const { return fFirstResolved; }

private:
    SkSpan<const char> fFullText;
    SkTHashMap<const char*, SkFont> fFontMapping;
    size_t fUnresolved;
    const char* fFirstResolved;
};

FontResolver::FontResolver(sk_sp<FontCollection> fontCollection, SkSpan<const char> fullText)
        : fFontCollection(fontCollection)
        , fFullText(fullText)
        , fFontCoverage(new FontCoverage(fullText)) {

    fFontCoverage->initialize();
}

bool FontResolver::findFirst(const char* codepoint, SkFont* font, SkScalar* height) {
    return fFontCoverage->find(codepoint, font);
}

bool FontResolver::findNext(const char* codepoint, SkFont* font, SkScalar* height) {
    return fFontCoverage->find(codepoint, font);
}

const char* FontResolver::firstResolvedCharacter() const {
    return fFontCoverage->firstResolved();
}

SkFont FontResolver::firstResolvedFont() const {
    return fFirstResolvedFont.first;
}

void FontResolver::findAllFontsForStyledBlock(const TextStyle& style, SkSpan<const char> text) {

    // Walk through all available fonts to resolve the block
    for (auto& fontFamily : style.getFontFamilies()) {
        auto typeface = fFontCollection->matchTypeface(fontFamily.c_str(), style.getFontStyle());
        if (typeface.get() == nullptr) {
            continue;
        }

        // Resolve all unresolved characters
        // TODO: text direction
        auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
        fFontCoverage->scanGaps(text, [&](SkSpan<const char> gap) {
            SkDebugf("\nGAP: [%d:%d)\n", gap.begin() - fFullText.begin(), gap.end() - fFullText.begin());
            fFontCoverage->cover(gap, font.first, TextDirection::kLtr);
        });

        if (fFontCoverage->unresolved() == 0) {
            break;
        }
    }

    // In case something still unresolved
    if (fFontCoverage->unresolved() > 0 && fFontCollection->fontFallbackEnabled()) {
        auto font = makeFont(fFontCollection->defaultFallback(0,
                             style.getFontStyle()),
                             style.getFontSize(), style.getHeight());
        fFontCoverage->cover(text, font.first, TextDirection::kLtr);
    }
}

/*
void FontResolver::findAllFontsForStyledBlock1(const TextStyle& style, SkSpan<const char> text) {
    fCodepoints.reset();
    fCharacters.reset();
    fUnresolvedIndexes.reset();
    fUnresolvedCodepoints.reset();

    // Extract all unicode codepoints
    const char* current = text.begin();
    while (current != text.end()) {
        fCharacters.emplace_back(current);
        auto codepoint = utf8_next(&current, text.end());
        fCodepoints.emplace_back(codepoint);
        fUnresolvedIndexes.emplace_back(fUnresolvedIndexes.size());
    }
    fUnresolved = fCodepoints.size();

    // Walk through all available fonts to resolve the block
    for (auto& fontFamily : style.getFontFamilies()) {
        auto typeface = fFontCollection->matchTypeface(fontFamily.c_str(), style.getFontStyle());
        if (typeface.get() == nullptr) {
            continue;
        }

        // Resolve all unresolved characters
        auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
        if (fFontCoverage.cover(text, font.first, TextDirection::kLtr)) {
            break;
        }

        resolveAllCharactersByFont(font);

        if (fUnresolved == 0) {
            break;
        }
    }

    if (fUnresolved > 0 && fFontCollection->fontFallbackEnabled()) {
        auto typeface = fFontCollection->matchDefaultTypeface(style.getFontStyle());
        if (typeface.get() != nullptr) {
            // Resolve all unresolved characters
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            resolveAllCharactersByFont(font);
        }
    }

    addResolvedWhitespacesToMapping();

    if (fUnresolved > 0 && fFontCollection->fontFallbackEnabled()) {
        size_t tryIndex = 0;
        while (fUnresolved > 0 && tryIndex < fUnresolved) {
            auto unicode = fUnresolvedIndexes[tryIndex];
            auto typeface = fFontCollection->defaultFallback(unicode, style.getFontStyle());
            if (typeface == nullptr) {
                break;
            }
            auto wasUnresolved = fUnresolved;
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            if (!resolveAllCharactersByFont(font)) {
                // Not a single unicode character was resolved
                ++tryIndex;
            } else {
                if (wasUnresolved == fUnresolved) {
                    break;
                }
            }
        }
    }

    // In case something still unresolved
    if (fResolvedFonts.count() == 0) {
        makeFont(fFontCollection->defaultFallback(firstUnresolved(),
                                                  style.getFontStyle()),
                 style.getFontSize(), style.getHeight());
        if (fFirstResolvedFont.first.getTypeface() != nullptr) {
            SkString name;
            fFirstResolvedFont.first.getTypeface()->getFamilyName(&name);
            SkDebugf("Urgent font resolution: %s\n", name.c_str());
        } else {
            SkDebugf("No font!!!\n");
        }
    }
}
*/

size_t FontResolver::resolveAllCharactersByFont(std::pair<SkFont, SkScalar> font) {
    // Consolidate all unresolved unicodes in one array to make a batch call
    SkTArray<SkGlyphID> glyphs(fUnresolved);
    glyphs.push_back_n(fUnresolved, SkGlyphID(0));
    font.first.getTypeface()->unicharsToGlyphs(
            fUnresolved == fCodepoints.size() ? fCodepoints.data() : fUnresolvedCodepoints.data(),
            fUnresolved, glyphs.data());

    SkRange<size_t> resolved(0, 0);
    SkRange<size_t> whitespaces(0, 0);
    size_t stillUnresolved = 0;

    auto processRuns = [&]() {
        if (resolved.width() == 0) {
            return;
        }

        if (resolved.width() == whitespaces.width()) {
            // The entire run is just whitespaces;
            // Remember the font and mark whitespaces back unresolved
            // to calculate its mapping for the other fonts
            for (auto w = whitespaces.start; w != whitespaces.end; ++w) {
                if (fWhitespaces.find(w) == nullptr) {
                    fWhitespaces.set(w, font);
                }
                fUnresolvedIndexes[stillUnresolved++] = w;
                fUnresolvedCodepoints.emplace_back(fCodepoints[w]);
            }
        } else {
            fFontMapping.set(fCharacters[resolved.start], font);
        }
    };

    // Try to resolve all the unresolved unicode points
    SkUnichar* lastBase = nullptr;
    for (size_t i = 0; i < glyphs.size(); ++i) {
        auto index = fUnresolvedIndexes[i];
        auto codepoint = fCodepoints[index];
        if (glyphs[i] != 0) {
            if (is_not_base(codepoint)) {
                if (lastBase != nullptr) {
                    // The base character is resolved in this font, so we can keep this one
                } else {
                    // The base was not resolved, at least, not in this font
                    glyphs[i] = 0;
                }
            }
        }

        if (glyphs[i] == 0) {
            if (!is_not_base(codepoint)) {
                lastBase = &fCodepoints[index];
            }

            processRuns();

            resolved = SkRange<size_t>(0, 0);
            whitespaces = SkRange<size_t>(0, 0);

            fUnresolvedIndexes[stillUnresolved++] = index;
            fUnresolvedCodepoints.emplace_back(fCodepoints[index]);
            continue;
        }

        if (index == resolved.end) {
            ++resolved.end;
        } else {
            processRuns();
            resolved = SkRange<size_t>(index, index + 1);
        }
        if (u_isUWhiteSpace(fCodepoints[index])) {
            if (index == whitespaces.end) {
                ++whitespaces.end;
            } else {
                whitespaces = SkRange<size_t>(index, index + 1);
            }
        } else {
            whitespaces = SkRange<size_t>(0, 0);
        }
    }

    // One last time to take care of the tail run
    processRuns();

    size_t wasUnresolved = fUnresolved;
    fUnresolved = stillUnresolved;
    return fUnresolved < wasUnresolved;
}

void FontResolver::addResolvedWhitespacesToMapping() {
    size_t resolvedWhitespaces = 0;
    for (size_t i = 0; i < fUnresolved; ++i) {
        auto index = fUnresolvedIndexes[i];
        auto found = fWhitespaces.find(index);
        if (found != nullptr) {
            fFontMapping.set(fCharacters[index], *found);
            ++resolvedWhitespaces;
        }
    }
    fUnresolved -= resolvedWhitespaces;
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
        if (fResolvedFonts.count() == 0) {
            fFirstResolvedFont = pair;
        }
        fResolvedFonts.add(pair);
    }

    return pair;
}

}  // namespace textlayout
}  // namespace skia
