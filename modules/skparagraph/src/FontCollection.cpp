// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/FontCollection.h"
#include "include/core/SkTypeface.h"

namespace {

    const SkString fDefaultFamilyName = SkString(DEFAULT_FONT_FAMILY);
    SkTArray<SkString> fFamilyNames;
    SkTHashMap<SkString, size_t> fFamilyNamesReverted;
    SkTArray<SkString> fLocales;

}
namespace skia {
namespace textlayout {

bool FontCollection::FamilyKey::operator==(const FontCollection::FamilyKey& other) const {
    return fFontFamily == other.fFontFamily && fLocale == other.fLocale &&
           fFontStyle == other.fFontStyle;
}

size_t FontCollection::FamilyKey::Hasher::operator()(const FontCollection::FamilyKey& key) const {
    return SkGoodHash()(key.fFontFamily) ^
           SkGoodHash()(key.fLocale) ^
           SkGoodHash()(key.fFontStyle.weight()) ^
           SkGoodHash()(key.fFontStyle.slant());
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

sk_sp<SkTypeface> FontCollection::matchTypeface(const SkString& familyName, SkFontStyle fontStyle) {
    // Look inside the font collections cache first
    FamilyKey familyKey(familyName, "en", fontStyle);
    auto found = fTypefaces.find(familyKey);
    if (found) {
        return *found;
    }

    sk_sp<SkTypeface> typeface = nullptr;
    for (const auto& manager : this->getFontManagerOrder()) {
        SkFontStyleSet* set = manager->matchFamily(familyName.c_str());
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

sk_sp<SkTypeface> FontCollection::matchDefaultTypeface(SkFontStyle fontStyle) {
    // Look inside the font collections cache first
    FamilyKey familyKey(fDefaultFamilyName, "en", fontStyle);
    auto found = fTypefaces.find(familyKey);
    if (found) {
        return *found;
    }

    sk_sp<SkTypeface> typeface = nullptr;
    for (const auto& manager : this->getFontManagerOrder()) {
        SkFontStyleSet* set = manager->matchFamily(fDefaultFamilyName.c_str());
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

    if (fDefaultFontManager == nullptr) {
        return nullptr;
    }
    auto result = fDefaultFontManager->matchFamilyStyle(fDefaultFamilyName.c_str(), fontStyle);
    return sk_ref_sp<SkTypeface>(result);
}

void FontCollection::disableFontFallback() { fEnableFontFallback = false; }

const SkString& FontCollection::getFontFamilyName(size_t index) {

    SkASSERT(index < fFamilyNames.size());
    return fFamilyNames[index];
}

bool FontCollection::getFontFamilyIndex(const SkString& name, size_t* index) {
    auto found = fFamilyNamesReverted.find(name);
    if (found == nullptr) {
        return false;
    }
    *index = *found;
    return true;
}

size_t FontCollection::addFontFamily(const SkString& familyName) {
    auto index = 0;
    auto found = fFamilyNamesReverted.find(familyName);
    if (found == nullptr) {
        index = fFamilyNames.size();
        fFamilyNames.emplace_back(familyName);
        fFamilyNamesReverted.set(familyName, index);
    } else {
        index = *found;
    }
    return index;
}

size_t FontCollection::addLocale(const SkString& locale) {
    auto index = fLocales.size();
    for (size_t i = 0; i < fLocales.size(); ++i) {
        if (fLocales[i] == locale) {
            index = i;
            break;
        }
    }
    if (index == fLocales.size()) {
        fLocales.emplace_back(locale);
    }
    return index;
}


}  // namespace textlayout
}  // namespace skia
