// Copyright 2019 Google LLC.
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

bool FontCollection::FamilyKey::operator==(const FontCollection::FamilyKey& other) const {
    return fFamilyNames == other.fFamilyNames && fFontStyle == other.fFontStyle;
}

size_t FontCollection::FamilyKey::Hasher::operator()(const FontCollection::FamilyKey& key) const {
    size_t hash = 0;
    for (const SkString& family : key.fFamilyNames) {
        hash ^= std::hash<std::string>()(family.c_str());
    }
    return hash ^
           std::hash<uint32_t>()(key.fFontStyle.weight()) ^
           std::hash<uint32_t>()(key.fFontStyle.slant());
}

FontCollection::FontCollection()
        : fEnableFontFallback(true)
        , fDefaultFamilyName(DEFAULT_FONT_FAMILY) { }

size_t FontCollection::getFontManagersCount() const { return this->getFontManagerOrder().size(); }

void FontCollection::setAssetFontManager(sk_sp<SkFontMgr> font_manager) {
    fAssetFontManager = font_manager;
}

void FontCollection::setDynamicFontManager(sk_sp<SkFontMgr> font_manager) {
    fDynamicFontManager = font_manager;
}

void FontCollection::setTestFontManager(sk_sp<SkFontMgr> font_manager) {
    fTestFontManager = font_manager;
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                                           const char defaultFamilyName[]) {
    fDefaultFontManager = std::move(fontManager);
    fDefaultFamilyName = defaultFamilyName;
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager) {
    fDefaultFontManager = fontManager;
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
    // Look inside the font collections cache first
    FamilyKey familyKey(familyNames, fontStyle);
    auto found = fTypefaces.find(familyKey);
    if (found) {
        return *found;
    }

    std::vector<sk_sp<SkTypeface>> typefaces;
    for (const SkString& familyName : familyNames) {
        sk_sp<SkTypeface> match = matchTypeface(familyName, fontStyle);
        if (match) {
            typefaces.emplace_back(std::move(match));
        }
    }

    if (typefaces.empty()) {
        sk_sp<SkTypeface> match = matchTypeface(fDefaultFamilyName, fontStyle);
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
sk_sp<SkTypeface> FontCollection::defaultFallback(SkUnichar unicode, SkFontStyle fontStyle, const SkString& locale) {

    for (const auto& manager : this->getFontManagerOrder()) {
        std::vector<const char*> bcp47;
        if (!locale.isEmpty()) {
            bcp47.push_back(locale.c_str());
        }
        sk_sp<SkTypeface> typeface(manager->matchFamilyStyleCharacter(
                0, fontStyle, bcp47.data(), bcp47.size(), unicode));
        if (typeface != nullptr) {
            return typeface;
        }
    }
    return nullptr;
}

sk_sp<SkTypeface> FontCollection::defaultFallback() {
    if (fDefaultFontManager == nullptr) {
        return nullptr;
    }
    auto result = fDefaultFontManager->matchFamilyStyle(fDefaultFamilyName.c_str(), SkFontStyle());
    return sk_ref_sp<SkTypeface>(result);
}


void FontCollection::disableFontFallback() { fEnableFontFallback = false; }
void FontCollection::enableFontFallback() { fEnableFontFallback = true; }

}  // namespace textlayout
}  // namespace skia
