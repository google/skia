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
#pragma once

#include <memory>
#include <set>
#include <string>
#include "../private/SkTHash.h"
#include "SkFontMgr.h"
#include "SkRefCnt.h"
#include "SkTextStyle.h"
#include "SkFontMgr.h"

class SkFontCollection : public SkRefCnt {
 public:
  SkFontCollection();

  ~SkFontCollection();

  size_t GetFontManagersCount() const;

  void SetAssetFontManager(sk_sp<SkFontMgr> fontManager);
  void SetDynamicFontManager(sk_sp<SkFontMgr> fontManager);
  void SetTestFontManager(sk_sp<SkFontMgr> fontManager);

  SkTypeface* findTypeface(SkTextStyle& textStyle);

  void DisableFontFallback();

 private:

  sk_sp<SkTypeface> findByFamilyName(const std::string& familyName, SkFontStyle fontStyle);

  friend class ParagraphTester;

  struct FamilyKey {
    FamilyKey(const std::string& family, const std::string& loc, SkFontStyle style)
        : font_family(family), locale(loc), font_style(style) {}

    FamilyKey() {}

    std::string font_family;
    std::string locale;
    SkFontStyle font_style;

    bool operator==(const FamilyKey& other) const;

    struct Hasher {
      size_t operator()(const FamilyKey& key) const;
    };
  };

  bool _enableFontFallback;
  SkTHashMap<FamilyKey, sk_sp<SkTypeface>, FamilyKey::Hasher> _typefaces;
  sk_sp<SkFontMgr> _defaultFontManager;
  sk_sp<SkFontMgr> _assetFontManager;
  sk_sp<SkFontMgr> _dynamicFontManager;
  sk_sp<SkFontMgr> _testFontManager;
  std::vector<sk_sp<SkFontMgr>> GetFontManagerOrder() const;
};
