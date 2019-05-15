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

#include "modules/skparagraph/include/SkFontCollection.h"

bool SkFontCollection::FamilyKey::operator==(
    const SkFontCollection::FamilyKey& other) const {

    return fFontFamily == other.fFontFamily &&
        fLocale == other.fLocale &&
        fFontStyle == other.fFontStyle;
}

size_t SkFontCollection::FamilyKey::Hasher::operator()(
    const SkFontCollection::FamilyKey& key) const {

    return std::hash<std::string>()(key.fFontFamily) ^
        std::hash<std::string>()(key.fLocale) ^
        std::hash<uint32_t>()(key.fFontStyle.weight()) ^
        std::hash<uint32_t>()(key.fFontStyle.slant());
}

SkFontCollection::SkFontCollection()
    : fEnableFontFallback(true)
    , fDefaultFontManager(SkFontMgr::RefDefault())
    , fDefaultFamilyName(DEFAULT_FONT_FAMILY) {
}

SkFontCollection::~SkFontCollection() = default;

size_t SkFontCollection::getFontManagersCount() const {

    return this->getFontManagerOrder().size();
}

void SkFontCollection::setAssetFontManager(sk_sp<SkFontMgr> font_manager) {

    fAssetFontManager = font_manager;
}

void SkFontCollection::setDynamicFontManager(sk_sp<SkFontMgr> font_manager) {

    fDynamicFontManager = font_manager;
}

void SkFontCollection::setTestFontManager(sk_sp<SkFontMgr> font_manager) {

    fTestFontManager = font_manager;
}

void SkFontCollection::setDefaultFontManager(
    sk_sp<SkFontMgr> fontManager,
    const std::string& defaultFamilyName) {
  fDefaultFontManager = fontManager;
  fDefaultFamilyName = defaultFamilyName;
}

// Return the available font managers in the order they should be queried.
std::vector<sk_sp<SkFontMgr>> SkFontCollection::getFontManagerOrder() const {

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

// TODO: locale
sk_sp<SkTypeface> SkFontCollection::findTypeface(const std::string& familyName, SkFontStyle fontStyle) {

  auto typeface = matchTypeface(familyName, fontStyle);

  if (typeface == nullptr && fEnableFontFallback) {
    return defaultFallback(familyName, fontStyle);
  }

  return typeface;
}

sk_sp<SkTypeface> SkFontCollection::matchTypeface(const std::string& familyName, SkFontStyle fontStyle) {

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
      fTypefaces.set(familyKey, typeface);
      return typeface;
    }
  }

  return nullptr;
}

sk_sp<SkTypeface> SkFontCollection::defaultFallback(const std::string& familyName, SkFontStyle fontStyle) {
  FamilyKey familyKey(familyName, "en", fontStyle);
  sk_sp<SkTypeface> typeface;
  typeface.reset(fDefaultFontManager->matchFamilyStyle(fDefaultFamilyName.c_str(), fontStyle));
  if (!familyName.empty()) {
    SkDebugf("Instead of %s (%d %s) ",
             familyName.c_str(),
             fontStyle.weight(),
             fontStyle.slant() == SkFontStyle::kUpright_Slant ? "normal" : "italic");
    SkDebugf("Using default %s (%d %s)\n",
             fDefaultFamilyName.c_str(),
             typeface->fontStyle().weight(),
             typeface->fontStyle().slant() == SkFontStyle::kUpright_Slant ? "normal" : "italic");
  }
  return typeface;
}

void SkFontCollection::disableFontFallback() {

    fEnableFontFallback = false;
}

