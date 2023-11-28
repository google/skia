// Copyright 2019 Google LLC.
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skshaper/include/SkShaper.h"

namespace {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    const char* kColorEmojiFontMac = "Apple Color Emoji";
#else
    const char* kColorEmojiLocale = "und-Zsye";
#endif
}
namespace skia {
namespace textlayout {

bool FontCollection::FamilyKey::operator==(const FontCollection::FamilyKey& other) const {
    return fFamilyNames == other.fFamilyNames &&
           fFontStyle == other.fFontStyle &&
           fFontArguments == other.fFontArguments;
}

size_t FontCollection::FamilyKey::Hasher::operator()(const FontCollection::FamilyKey& key) const {
    size_t hash = 0;
    for (const SkString& family : key.fFamilyNames) {
        hash ^= std::hash<std::string>()(family.c_str());
    }
    return hash ^
           std::hash<uint32_t>()(key.fFontStyle.weight()) ^
           std::hash<uint32_t>()(key.fFontStyle.slant()) ^
           std::hash<std::optional<FontArguments>>()(key.fFontArguments);
}

FontCollection::FontCollection()
        : fEnableFontFallback(true)
        , fDefaultFamilyNames({SkString(DEFAULT_FONT_FAMILY)}) { }

size_t FontCollection::getFontManagersCount() const { return this->getFontManagerOrder().size(); }

void FontCollection::setAssetFontManager(sk_sp<SkFontMgr> font_manager) {
    fAssetFontManager = std::move(font_manager);
}

void FontCollection::setDynamicFontManager(sk_sp<SkFontMgr> font_manager) {
    fDynamicFontManager = std::move(font_manager);
}

void FontCollection::setTestFontManager(sk_sp<SkFontMgr> font_manager) {
    fTestFontManager = std::move(font_manager);
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                                           const char defaultFamilyName[]) {
    fDefaultFontManager = std::move(fontManager);
    fDefaultFamilyNames.emplace_back(defaultFamilyName);
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                                           const std::vector<SkString>& defaultFamilyNames) {
    fDefaultFontManager = std::move(fontManager);
    fDefaultFamilyNames = defaultFamilyNames;
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager) {
    fDefaultFontManager = std::move(fontManager);
}

// Return the available font managers in the order they should be queried.
std::vector<sk_sp<SkFontMgr>> FontCollection::getFontManagerOrder() const {
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

std::vector<sk_sp<SkTypeface>> FontCollection::findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle) {
    return findTypefaces(familyNames, fontStyle, std::nullopt);
}

std::vector<sk_sp<SkTypeface>> FontCollection::findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle, const std::optional<FontArguments>& fontArgs) {
    // Look inside the font collections cache first
    FamilyKey familyKey(familyNames, fontStyle, fontArgs);
    auto found = fTypefaces.find(familyKey);
    if (found) {
        return *found;
    }

    std::vector<sk_sp<SkTypeface>> typefaces;
    for (const SkString& familyName : familyNames) {
        sk_sp<SkTypeface> match = matchTypeface(familyName, fontStyle);
        if (match && fontArgs) {
            match = fontArgs->CloneTypeface(match);
        }
        if (match) {
            typefaces.emplace_back(std::move(match));
        }
    }

    if (typefaces.empty()) {
        sk_sp<SkTypeface> match;
        for (const SkString& familyName : fDefaultFamilyNames) {
            match = matchTypeface(familyName, fontStyle);
            if (match) {
                break;
            }
        }
        if (!match) {
            for (const auto& manager : this->getFontManagerOrder()) {
                match = manager->legacyMakeTypeface(nullptr, fontStyle);
                if (match) {
                    break;
                }
            }
        }
        if (match) {
            typefaces.emplace_back(std::move(match));
        }
    }

    fTypefaces.set(familyKey, typefaces);
    return typefaces;
}

sk_sp<SkTypeface> FontCollection::matchTypeface(const SkString& familyName, SkFontStyle fontStyle) {
    for (const auto& manager : this->getFontManagerOrder()) {
        sk_sp<SkFontStyleSet> set(manager->matchFamily(familyName.c_str()));
        if (!set || set->count() == 0) {
            continue;
        }

        sk_sp<SkTypeface> match(set->matchStyle(fontStyle));
        if (match) {
            return match;
        }
    }

    return nullptr;
}

// Find ANY font in available font managers that resolves the unicode codepoint
sk_sp<SkTypeface> FontCollection::defaultFallback(SkUnichar unicode,
                                                  SkFontStyle fontStyle,
                                                  const SkString& locale) {

    for (const auto& manager : this->getFontManagerOrder()) {
        std::vector<const char*> bcp47;
        if (!locale.isEmpty()) {
            bcp47.push_back(locale.c_str());
        }
        sk_sp<SkTypeface> typeface(manager->matchFamilyStyleCharacter(
            nullptr, fontStyle, bcp47.data(), bcp47.size(), unicode));

        if (typeface != nullptr) {
            return typeface;
        }
    }
    return nullptr;
}

// Find ANY font in available font managers that resolves this emojiStart
sk_sp<SkTypeface> FontCollection::defaultEmojiFallback(SkUnichar emojiStart,
                                                       SkFontStyle fontStyle,
                                                       const SkString& locale) {

    for (const auto& manager : this->getFontManagerOrder()) {
        std::vector<const char*> bcp47;
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        sk_sp<SkTypeface> emojiTypeface =
            fDefaultFontManager->matchFamilyStyle(kColorEmojiFontMac, SkFontStyle());
        if (emojiTypeface != nullptr) {
            return emojiTypeface;
        }
#else
          bcp47.push_back(kColorEmojiLocale);
#endif
        if (!locale.isEmpty()) {
            bcp47.push_back(locale.c_str());
        }

        // Not really ideal since the first codepoint may not be the best one
        // but we start from a good colored emoji at least
        sk_sp<SkTypeface> typeface(manager->matchFamilyStyleCharacter(
            nullptr, fontStyle, bcp47.data(), bcp47.size(), emojiStart));
        if (typeface != nullptr) {
            // ... and stop as soon as we find something in hope it will work for all of them
            return typeface;
        }
    }
    return nullptr;
}

sk_sp<SkTypeface> FontCollection::defaultFallback() {
    if (fDefaultFontManager == nullptr) {
        return nullptr;
    }
    for (const SkString& familyName : fDefaultFamilyNames) {
        sk_sp<SkTypeface> match = fDefaultFontManager->matchFamilyStyle(familyName.c_str(),
                                                                        SkFontStyle());
        if (match) {
            return match;
        }
    }
    return nullptr;
}

void FontCollection::disableFontFallback() { fEnableFontFallback = false; }
void FontCollection::enableFontFallback() { fEnableFontFallback = true; }

void FontCollection::clearCaches() {
    fParagraphCache.reset();
    fTypefaces.reset();
    SkShaper::PurgeCaches();
}

}  // namespace textlayout
}  // namespace skia
