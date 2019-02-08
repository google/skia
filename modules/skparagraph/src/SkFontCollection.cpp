/*
 * Copyright 2018 Google Inc.
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

#include "SkFontCollection.h"

#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "SkTextStyle.h"

// TODO: Extract the platform dependent part
#define DEFAULT_FONT_FAMILY "sans-serif"

bool SkFontCollection::FamilyKey::operator==(
    const SkFontCollection::FamilyKey& other) const {
  return font_family == other.font_family &&
         locale == other.locale &&
         font_style == other.font_style;
}

size_t SkFontCollection::FamilyKey::Hasher::operator()(
    const SkFontCollection::FamilyKey& key) const {
  return std::hash<std::string>()(key.font_family) ^
         std::hash<std::string>()(key.locale) ^
         std::hash<uint32_t>()(key.font_style.weight()) ^
         std::hash<uint32_t>()(key.font_style.slant());
}

SkFontCollection::SkFontCollection()
  : _enableFontFallback(true)
  ,  _defaultFontManager(SkFontMgr::RefDefault()) {
}

SkFontCollection::~SkFontCollection() {
}

size_t SkFontCollection::GetFontManagersCount() const {
  return GetFontManagerOrder().size();
}

void SkFontCollection::SetAssetFontManager(sk_sp<SkFontMgr> font_manager) {
  _assetFontManager = font_manager;
}

void SkFontCollection::SetDynamicFontManager(sk_sp<SkFontMgr> font_manager) {
  _dynamicFontManager = font_manager;
}

void SkFontCollection::SetTestFontManager(sk_sp<SkFontMgr> font_manager) {
  _testFontManager = font_manager;
}

// Return the available font managers in the order they should be queried.
std::vector<sk_sp<SkFontMgr>> SkFontCollection::GetFontManagerOrder() const {
  std::vector<sk_sp<SkFontMgr>> order;
  if (_dynamicFontManager)
    order.push_back(_dynamicFontManager);
  if (_assetFontManager)
    order.push_back(_assetFontManager);
  if (_testFontManager)
    order.push_back(_testFontManager);
  if (_defaultFontManager && _enableFontFallback)
    order.push_back(_defaultFontManager);
  return order;
}
/*
// TODO: default fallback
SkTypeface* SkFontCollection::findTypeface(SkTextStyle& textStyle) {

  SkDebugf("findTypeface %s\n", textStyle.getFontFamily().c_str());

  // Look inside the font collections cache first
  FamilyKey familyKey(textStyle.getFontFamily(), "en", textStyle.getFontStyle());
  auto found = _typefaces.find(familyKey);
  if (found != nullptr) {
    SkDebugf("Found in cache\n");
    textStyle.setTypeface(*found);
    return found->get();
  }

  sk_sp<SkTypeface> typeface;
  std::vector<SkFontStyleSet*> sets;
  for (auto manager : GetFontManagerOrder()) {
    // Cache the font collection for future queries
    auto set = manager->matchFamily(textStyle.getFontFamily().c_str());
    if (set != nullptr && set->count() > 0) {
      sets.emplace_back(set);
    }
  }

  if (sets.empty()) {
    SkDebugf("Font not found: %s\n", textStyle.getFontFamily().c_str());
    for (auto manager : GetFontManagerOrder()) {
      // Cache the font collection for future queries
      auto set = manager->matchFamily(DEFAULT_FONT_FAMILY);
      if (set != nullptr && set->count() > 0) {
        sets.emplace_back(set);
      }
    }
    if (!sets.empty()) {
      SkDebugf("Found in default: %s %d\n", DEFAULT_FONT_FAMILY, sets.size());
    }
  } else {
    SkDebugf("Found in managers: %s %d\n", textStyle.getFontFamily().c_str(), sets.size());
  }

  if (!sets.empty()) {
    for (auto set : sets) {
      for (int i = 0; i < set->count(); ++i) {
        sk_sp<SkTypeface>(set->createTypeface(i));
      }

      sk_sp <SkTypeface> match(set->matchStyle(textStyle.getFontStyle()));
      if (match != nullptr) {
        typeface = match;
        break;
      }
    }
  }

  if (typeface == nullptr) {
    SkDebugf("Match not found: %s | %s\n", textStyle.getFontFamily().c_str(), DEFAULT_FONT_FAMILY);
    return nullptr;
  }

  _typefaces.set(familyKey, typeface);
  textStyle.setTypeface(typeface);

  return typeface.get();
}
*/
/*
sk_sp<SkTypeface> SkFontCollection::findByFamilyName(const std::string& familyName, SkFontStyle fontStyle) {

  for (auto manager : GetFontManagerOrder()) {
    SkFontStyleSet* set = manager->matchFamily(familyName.c_str());
    if (set == nullptr || set->count() == 0) {
      continue;
    }

    for (int i = 0; i < set->count(); ++i) {
      sk_sp<SkTypeface>(set->createTypeface(i));
    }

    sk_sp <SkTypeface> match(set->matchStyle(fontStyle));
    if (match != nullptr) {
      return match;
    }
  }

  return nullptr;
}

SkTypeface* SkFontCollection::findTypeface(SkTextStyle& textStyle) {

  // Look inside the font collections cache first
  FamilyKey familyKey(textStyle.getFontFamily(), "en", textStyle.getFontStyle());
  auto found = _typefaces.find(familyKey);
  if (found != nullptr) {
    textStyle.setTypeface(*found);
    return found->get();
  }

  sk_sp<SkTypeface> typeface = findByFamilyName(textStyle.getFontFamily(), textStyle.getFontStyle());
  if (typeface == nullptr) {
    typeface = findByFamilyName(DEFAULT_FONT_FAMILY, textStyle.getFontStyle());
    if (typeface == nullptr) {
      return nullptr;
    } else {
      textStyle.setTypeface(typeface);
      return typeface.get();
    }
  }

  _typefaces.set(familyKey, typeface);
  textStyle.setTypeface(typeface);
  return typeface.get();
}
*/

SkTypeface* SkFontCollection::findTypeface(SkTextStyle& textStyle) {

  // Look inside the font collections cache first
  FamilyKey familyKey(textStyle.getFontFamily(), "en", textStyle.getFontStyle());
  auto found = _typefaces.find(familyKey);
  if (found != nullptr) {
    textStyle.setTypeface(*found);
    return SkRef(found->get());
  }

  sk_sp<SkTypeface> typeface = nullptr;
  size_t n = 0;
  for (auto manager : GetFontManagerOrder()) {
    ++n;
    SkFontStyleSet* set = manager->matchFamily(textStyle.getFontFamily().c_str());
    if (set == nullptr || set->count() == 0) {
      continue;
    }

    for (int i = 0; i < set->count(); ++i) {
      set->createTypeface(i);
    }

    sk_sp<SkTypeface> match(set->matchStyle(textStyle.getFontStyle()));
    if (match != nullptr) {
      typeface = std::move(match);
      break;
    }
  }

  if (typeface == nullptr) {
    typeface.reset(_defaultFontManager->matchFamilyStyle(DEFAULT_FONT_FAMILY, SkFontStyle()));
  } else {
    _typefaces.set(familyKey, typeface);
  }

  textStyle.setTypeface(typeface);

  return SkRef(typeface.get());
}


void SkFontCollection::DisableFontFallback() {
  _enableFontFallback = false;
}

