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

bool FontResolver::isEmpty() {
    return fFontIterator == fFontSwitches.end();
}

void FontResolver::getFirstFont(SkFont* font, SkScalar* height) {
    *font = fFirstResolvedFont.fFont;
    *height = fFirstResolvedFont.fHeight;
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
    fUnresolvedCodepoints.push_back_n(fUnresolvedIndexes.size());
    fUnresolved = fCodepoints.size();

    // Walk through all available fonts to resolve the block
    auto wasUnresolved = fUnresolved;
    for (auto& fontFamily : style.getFontFamilies()) {
        auto typeface = fFontCollection->matchTypeface(fontFamily.c_str(), style.getFontStyle(), style.getLocale());
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

    if (fUnresolved != wasUnresolved || allWhitespaces()) {
        addResolvedWhitespacesToMapping();
        wasUnresolved = fUnresolved;
    }

    if (fUnresolved > 0) {
        // Check the default font
        auto typeface =
                fFontCollection->matchDefaultTypeface(style.getFontStyle(), style.getLocale());
        if (typeface != nullptr) {
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            resolveAllCharactersByFont(font);
        }
        if (fUnresolved != wasUnresolved || allWhitespaces()) {
            addResolvedWhitespacesToMapping();
            wasUnresolved = fUnresolved;
        }
    }

    if (fUnresolved > 0 && fFontCollection->fontFallbackEnabled()) {
        while (fUnresolved > 0 && !allWhitespaces()) {
            auto unicode = firstUnresolved();
            auto typeface = fFontCollection->defaultFallback(unicode, style.getFontStyle(), style.getLocale());
            if (typeface == nullptr) {
                break;
            }

            SkString name;
            typeface->getFamilyName(&name);
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            auto newResolved = resolveAllCharactersByFont(font);
            if (newResolved == 0) {
                // Not a single unicode character was resolved
                break;
            }
        }
        if (fUnresolved != wasUnresolved || allWhitespaces()) {
            addResolvedWhitespacesToMapping();
            wasUnresolved = fUnresolved;
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
                fUnresolvedIndexes[stillUnresolved] = w;
                fUnresolvedCodepoints[stillUnresolved] = fCodepoints[w];
                ++stillUnresolved;
            }
        } else {
            fFontMapping.set(fCharacters[resolved.start] - fText.begin(), font);
        }
    };

    // Try to resolve all the unresolved unicode points
    SkString name;
    font.fFont.getTypeface()->getFamilyName(&name);
    for (size_t i = 0; i < glyphs.size(); ++i) {
        auto glyph = glyphs[i];
        auto index = fUnresolvedIndexes[i];
        auto codepoint = fCodepoints[index];

        if (u_hasBinaryProperty(codepoint, UCHAR_BIDI_CONTROL)) {
            // Skip control characters - they don't have to be resolved
        } else if (glyph == 0) {
            processRuns();

            resolved = SkRange<size_t>(0, 0);
            whitespaces = SkRange<size_t>(0, 0);

            fUnresolvedIndexes[stillUnresolved] = index;
            fUnresolvedCodepoints[stillUnresolved] = codepoint;
            ++stillUnresolved;
            continue;
        }

        if (index == resolved.end) {
            ++resolved.end;
        } else {
            processRuns();
            resolved = SkRange<size_t>(index, index + 1);
        }
        if (u_isUWhiteSpace(codepoint)) {
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
    return wasUnresolved - stillUnresolved;
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

FontDescr FontResolver::makeFont(sk_sp<SkTypeface> typeface, SkScalar size, SkScalar height) {
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
    return fUnresolvedCodepoints[0];
}

void FontResolver::setLastResortFont() {
    TextStyle foundStyle;
    sk_sp<SkTypeface> typeface = nullptr;
    for (auto& style : fStyles) {
        for (auto& fontFamily : style.fStyle.getFontFamilies()) {
            typeface = fFontCollection->matchTypeface(fontFamily.c_str(), style.fStyle.getFontStyle(), style.fStyle.getLocale());
            if (typeface.get() != nullptr) {
                foundStyle = style.fStyle;
                break;
            }
        }
        if (typeface != nullptr) {
          break;
        }
    }
    if (typeface == nullptr) {
        for (auto& fontFamily : fDefaultStyle.getFontFamilies()) {
            typeface = fFontCollection->matchTypeface(fontFamily.c_str(), fDefaultStyle.getFontStyle(), fDefaultStyle.getLocale());
            if (typeface.get() != nullptr) {
                foundStyle = fDefaultStyle;
                break;
            }
        }
    }

    if (typeface == nullptr) {
        foundStyle = fStyles.empty() ? fDefaultStyle : fStyles.front().fStyle;
        typeface = fFontCollection->defaultFallback(0, foundStyle.getFontStyle(), foundStyle.getLocale());
    }

    if (typeface == nullptr) {
        typeface = fFontCollection->defaultFallback();
    }

    fFirstResolvedFont = makeFont(typeface, foundStyle.getFontSize(), foundStyle.getHeight());
    fFirstResolvedFont.fStart = 0;
}

void FontResolver::findAllFontsForAllStyledBlocks(ParagraphImpl* master) {
    fFontCollection = master->fontCollection();
    fStyles = master->styles();
    fText = master->text();
    fDefaultStyle = master->paragraphStyle().getTextStyle();

    if (fText.empty()) {
        setLastResortFont();
        return;
    }

    Block combinedBlock;
    for (auto& block : fStyles) {
        SkASSERT(combinedBlock.fRange.width() == 0 ||
                 combinedBlock.fRange.end == block.fRange.start);

        if (!combinedBlock.fRange.empty()) {
            if (block.fStyle.matchOneAttribute(StyleType::kFont, combinedBlock.fStyle)) {
                combinedBlock.add(block.fRange);
                continue;
            }
            // Resolve all characters in the block for this style
            this->findAllFontsForStyledBlock(combinedBlock.fStyle, combinedBlock.fRange);
        }

        if (block.fStyle.isPlaceholder()) {
            fFontMapping.set(block.fRange.start, FontDescr());
            combinedBlock.fRange = EMPTY_RANGE;
        } else {
            combinedBlock.fRange = block.fRange;
            combinedBlock.fStyle = block.fStyle;
        }
    }

    this->findAllFontsForStyledBlock(combinedBlock.fStyle, combinedBlock.fRange);

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
            continue;
        }

        if (prev->fFont.getTypeface() != nullptr) {
            fFontSwitches.emplace_back(*prev);
        }
        prev = found;
        prev->fStart = &ch - fText.begin();
    }

    if (prev != nullptr) {
        if (prev->fFont.getTypeface() != nullptr) {
            fFontSwitches.emplace_back(*prev);
        }
    }

    if (fFontSwitches.empty()) {
        setLastResortFont();
        if (fFirstResolvedFont.fFont.getTypeface() != nullptr) {
            fFontSwitches.emplace_back(fFirstResolvedFont);
        }
    }

    fFontIterator = fFontSwitches.begin();
}
}  // namespace textlayout
}  // namespace skia
