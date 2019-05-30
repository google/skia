/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "modules/skparagraph/include/FontCollection.h"
#include <string>

namespace skia {
namespace textlayout {

bool FontCollection::FamilyKey::operator==(const FontCollection::FamilyKey& other) const {
    return fFontFamily == other.fFontFamily && fLocale == other.fLocale &&
           fFontStyle == other.fFontStyle;
}

size_t FontCollection::FamilyKey::Hasher::operator()(const FontCollection::FamilyKey& key) const {
    return std::hash<std::string>()(key.fFontFamily.c_str()) ^
           std::hash<std::string>()(key.fLocale.c_str()) ^
           std::hash<uint32_t>()(key.fFontStyle.weight()) ^
           std::hash<uint32_t>()(key.fFontStyle.slant());
}

FontCollection::FontCollection()
        : fEnableFontFallback(true)
        , fDefaultFontManager(SkFontMgr::RefDefault())
        , fDefaultFamilyName(DEFAULT_FONT_FAMILY) {}

FontCollection::~FontCollection() = default;

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
    fDefaultFontManager = fontManager;
    fDefaultFamilyName = defaultFamilyName;
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

sk_sp<SkTypeface> FontCollection::matchTypeface(const char familyName[], SkFontStyle fontStyle) {
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
            fTypefaces.set(familyKey, typeface);
            return typeface;
        }
    }

    return nullptr;
}

sk_sp<SkTypeface> FontCollection::defaultFallback(SkUnichar unicode, SkFontStyle fontStyle) {
    auto candidate = fDefaultFontManager->matchFamilyStyleCharacter(
            fDefaultFamilyName.c_str(), fontStyle, nullptr, 0, unicode);
    if (candidate == nullptr) {
        candidate = fDefaultFontManager->matchFamilyStyle(fDefaultFamilyName.c_str(), fontStyle);
    }
    return sk_ref_sp<SkTypeface>(candidate);
}

void FontCollection::disableFontFallback() { fEnableFontFallback = false; }

}  // namespace textlayout
}  // namespace skia