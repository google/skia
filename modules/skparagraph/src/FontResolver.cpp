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

namespace {
SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}
}  // namespace

// TODO: FontResolver and FontIterator have common functionality
namespace skia {
namespace textlayout {

bool FontResolver::findNext(const char* codepoint, SkFont* font, SkScalar* height) {

    SkASSERT(fFontIterator != nullptr);
    TextIndex index = codepoint - fText.begin();
    while (fFontIterator != fFontSwitches.end() && fFontIterator->fStart <= index) {
        if (fFontIterator->fStart == index) {
            *font = fFontIterator->fFont;
            *height = fFontIterator->fHeight;
            return true;
        }
        ++fFontIterator;
    }
    return false;
}

void FontResolver::findAllFontsForStyledBlock(const TextStyle& style, TextRange textRange) {
    fCodepoints.reset();
    fCharacters.reset();
    fUnresolvedIndexes.reset();
    fUnresolvedCodepoints.reset();

    // Extract all unicode codepoints
    const char* end = fText.begin() + textRange.end;
    const char* current = fText.begin() + textRange.start;
    while (current != end) {
        fCharacters.emplace_back(current);
        fCodepoints.emplace_back(utf8_next(&current, end));
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
        resolveAllCharactersByFont(font);
        if (fUnresolved == 0) {
            break;
        }
    }

    if (fUnresolved > 0) {
        auto typeface = fFontCollection->matchDefaultTypeface(style.getFontStyle());
        if (typeface.get() != nullptr) {
            // Resolve all unresolved characters
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            resolveAllCharactersByFont(font);
        }
    }

    addResolvedWhitespacesToMapping();

    if (fUnresolved > 0 && fFontCollection->fontFallbackEnabled()) {
        while (fUnresolved > 0) {
            auto unicode = firstUnresolved();
            auto typeface = fFontCollection->defaultFallback(unicode, style.getFontStyle(), style.getLocale());
            if (typeface == nullptr) {
                break;
            }
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            if (!resolveAllCharactersByFont(font)) {
                // Not a single unicode character was resolved
                break;
            }
            SkString name;
            typeface->getFamilyName(&name);
            SkDebugf("Default font fallback resolution: %s\n", name.c_str());
        }
    }

    // In case something still unresolved
    if (fResolvedFonts.count() == 0) {
        auto result = fFontCollection->defaultFallback(firstUnresolved(), style.getFontStyle(), style.getLocale());
        if (result == nullptr) {
            SkDebugf("No fallback!!!\n");
            return;
        }
        makeFont(result,
                 style.getFontSize(),
                 style.getHeight());
        if (fFirstResolvedFont.fFont.getTypeface() != nullptr) {
            SkString name;
            fFirstResolvedFont.fFont.getTypeface()->getFamilyName(&name);
            SkDebugf("Urgent font resolution: %s\n", name.c_str());
        } else {
            SkDebugf("No font!!!\n");
        }
    }
}

size_t FontResolver::resolveAllCharactersByFont(const FontDescr& font) {
    // Consolidate all unresolved unicodes in one array to make a batch call
    SkTArray<SkGlyphID> glyphs(fUnresolved);
    glyphs.push_back_n(fUnresolved, SkGlyphID(0));
    font.fFont.getTypeface()->unicharsToGlyphs(
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
            //SkDebugf("Resolved %d @%d\n", font.fFont.getTypeface()->uniqueID(), resolved.start);
            fFontMapping.set(fCharacters[resolved.start] - fText.begin(), font);
        }
    };

    // Try to resolve all the unresolved unicode points
    for (size_t i = 0; i < glyphs.size(); ++i) {
        auto glyph = glyphs[i];
        auto index = fUnresolvedIndexes[i];

        if (glyph == 0) {
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
            fFontMapping.set(fCharacters[index] - fText.begin(), *found);
            ++resolvedWhitespaces;
        }
    }
    fUnresolved -= resolvedWhitespaces;
}

FontDescr FontResolver::makeFont(sk_sp<SkTypeface> typeface,
                                               SkScalar size,
                                               SkScalar height) {
    SkFont font(typeface, size);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setHinting(SkFontHinting::kSlight);
    font.setSubpixel(true);
    FontDescr descr(font, height);

    const FontDescr* foundFont = fResolvedFonts.find(descr);
    if (foundFont == nullptr) {
        if (fResolvedFonts.count() == 0) {
            fFirstResolvedFont = descr;
        }
        fResolvedFonts.add(descr);
    }
    return descr;
}

SkUnichar FontResolver::firstUnresolved() {
    if (fUnresolved == 0) return 0;

    bool firstTry = fUnresolved == fCodepoints.size();
    auto index = firstTry ? 0 : fUnresolvedIndexes[0];
    return fCodepoints[index];
}

void FontResolver::findAllFontsForAllStyledBlocks(ParagraphImpl* master) {
    fFontCollection = master->fontCollection();
    fStyles = master->styles();
    fText = master->text();
    fTextRange = TextRange(0, fText.size());

    Block combined;
    for (auto& block : fStyles) {
        SkASSERT(combined.fRange.empty() ||
                 combined.fRange.end == block.fRange.start);

        if (!combined.fRange.empty() &&
                block.fStyle.matchOneAttribute(StyleType::kFont, combined.fStyle)) {
            combined.add(block.fRange);
            continue;
        }

        if (!combined.fRange.empty()) {
            this->findAllFontsForStyledBlock(combined.fStyle, combined.fRange);
        }

        combined = block;
    }
    this->findAllFontsForStyledBlock(combined.fStyle, combined.fRange);


    fFontSwitches.reset();
    FontDescr* prev = nullptr;
    for (auto& ch : fText) {
        if (fFontSwitches.count() == fFontMapping.count()) {
            // Checked all
            break;
        }
        auto found = fFontMapping.find(&ch - fText.begin());
        if (found == nullptr) {
            // Insignificant character
            continue;
        }
        if (prev == nullptr) {
            prev = found;
            prev->fStart = 0;
        }

        if (*prev == *found) {
            // Same font
            continue;
        }
        fFontSwitches.emplace_back(*prev);

        prev = found;
        prev->fStart = &ch - fText.begin();
    }

    if (prev == nullptr) {
        fFirstResolvedFont.fStart = 0;
        prev = &fFirstResolvedFont;
    }
    fFontSwitches.emplace_back(*prev);
    fFontIterator = fFontSwitches.begin();
}
}  // namespace textlayout
}  // namespace skia
