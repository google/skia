// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/FontCollectionImpl.h"
#include <unicode/brkiter.h>
#include <string>
#include "src/utils/SkUTF.h"
#include "src/core/SkMakeUnique.h"

namespace {
SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}
}  // namespace

namespace skia {
namespace textlayout {

std::unique_ptr<FontCollection> FontCollection::make() {
    return skstd::make_unique<FontCollectionImpl>();
}

bool FontCollectionImpl::FamilyKey::operator==(const FontCollectionImpl::FamilyKey& other) const {
    return fFontFamily == other.fFontFamily && fLocale == other.fLocale &&
           fFontStyle == other.fFontStyle;
}

size_t FontCollectionImpl::FamilyKey::Hasher::operator()(
        const FontCollectionImpl::FamilyKey& key) const {
    return std::hash<std::string>()(key.fFontFamily.c_str()) ^
           std::hash<std::string>()(key.fLocale.c_str()) ^
           std::hash<uint32_t>()(key.fFontStyle.weight()) ^
           std::hash<uint32_t>()(key.fFontStyle.slant());
}

FontCollectionImpl::FontCollectionImpl()
        : fEnableFontFallback(true)
        , fDefaultFontManager(SkFontMgr::RefDefault())
        , fDefaultFamilyName(DEFAULT_FONT_FAMILY)
        , fHintingOn(false) {}

// Return the available font managers in the order they should be queried.
std::vector<sk_sp<SkFontMgr>> FontCollectionImpl::getFontManagerOrder() const {
    std::vector<sk_sp<SkFontMgr>> order;
    if (fDynamicFontManager) {
        order.push_back(fDynamicFontManager);
    }
    if (fAssetFontManager) {
        order.push_back(fAssetFontManager);
    }
    if (fTestFontManager) {
        order.push_back(fTestFontManager);
    }
    if (fDefaultFontManager && fEnableFontFallback) {
        order.push_back(fDefaultFontManager);
    }
    return order;
}

sk_sp<SkTypeface> FontCollectionImpl::matchTypeface(const char familyName[],
                                                    SkFontStyle fontStyle) {
    // Look inside the font collections cache first
    FamilyKey familyKey(familyName, "en", fontStyle);
    auto found = fTypefaces.find(familyKey);
    if (found) {
        return *found;
    }

    sk_sp<SkTypeface> typeface = nullptr;
    for (const auto& manager : this->getFontManagerOrder()) {
        SkFontStyleSet* set = manager->matchFamily(familyName);
        if (nullptr == set || set->count() == 0) {
            continue;
        }

        for (int i = 0; i < set->count(); ++i) {
            set->createTypeface(i);
        }

        sk_sp<SkTypeface> match(set->matchStyle(fontStyle));
        if (match) {
            typeface = std::move(match);
            return typeface;
        }
    }

    return nullptr;
}

sk_sp<SkTypeface> FontCollectionImpl::defaultFallback(SkUnichar unicode, SkFontStyle fontStyle) {
    for (const auto& manager : this->getFontManagerOrder()) {
        std::vector<const char*> bcp47;
        sk_sp<SkTypeface> typeface(manager->matchFamilyStyleCharacter(0, fontStyle, bcp47.data(),
                                                                      bcp47.size(), unicode));
        if (typeface != nullptr) {
            return typeface;
        }
    }

    auto result = fDefaultFontManager->matchFamilyStyle(fDefaultFamilyName.c_str(), fontStyle);
    return sk_ref_sp<SkTypeface>(result);
}

void FontCollectionImpl::resolveFonts(const TextStyle& style, SkSpan<const char> text) {
    fCodepoints.reset();
    fCharacters.reset();
    fUnresolvedIndexes.reset();
    fUnresolvedCodepoints.reset();

    // Extract all unicode codepoints
    const char* current = text.begin();
    while (current != text.end()) {
        fCharacters.emplace_back(current);
        fCodepoints.emplace_back(utf8_next(&current, text.end()));
        fUnresolvedIndexes.emplace_back(fUnresolvedIndexes.size());
    }
    fUnresolved = fCodepoints.size();

    // Walk through all available fonts to resolve the block
    for (auto& fontFamily : style.getFontFamilies()) {
        auto typeface = this->matchTypeface(fontFamily.c_str(), style.getFontStyle());
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
        auto typeface = this->matchTypeface(this->fDefaultFamilyName.c_str(), style.getFontStyle());
        if (typeface.get() != nullptr) {
            // Resolve all unresolved characters
            auto font = makeFont(typeface, style.getFontSize(), style.getHeight());
            resolveAllCharactersByFont(font);
        }
    }

    addResolvedWhitespacesToMapping();

    if (fUnresolved > 0 && this->fontFallbackEnabled()) {
        while (fUnresolved > 0) {
            auto unicode = firstUnresolved();
            auto typeface = this->defaultFallback(unicode, style.getFontStyle());
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
        makeFont(this->defaultFallback(firstUnresolved(), style.getFontStyle()),
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

std::pair<SkFont, SkScalar>* FontCollectionImpl::findFontForCodepoint(const char* ch, bool first) {
    auto found = fFontMapping.find(ch);
    if (found == nullptr && first) {
        // Resolve the first character with the first found font
        found = fFontMapping.set(ch, this->fFirstResolvedFont);
    }
    return found;
}

size_t FontCollectionImpl::resolveAllCharactersByFont(std::pair<SkFont, SkScalar> font) {
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

void FontCollectionImpl::addResolvedWhitespacesToMapping() {
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

std::pair<SkFont, SkScalar> FontCollectionImpl::makeFont(sk_sp<SkTypeface> typeface,
                                                         SkScalar size,
                                                         SkScalar height) {
    SkFont font(typeface, size);
    font.setEdging(SkFont::Edging::kAntiAlias);
    if (!fHintingOn) {
        font.setHinting(SkFontHinting::kSlight);
        font.setSubpixel(true);
    }
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
