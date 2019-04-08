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
#pragma once

#include <memory>
#include <set>
#include <string>
#include "../../include/private/SkTHash.h" // TODO: Figure out how to deal with it in Flutter engine
#include "SkFontMgr.h"
#include "SkRefCnt.h"
#include "SkTextStyle.h"
#include "SkFontMgr.h"

class SkFontCollection : public SkRefCnt {
  public:
    SkFontCollection();

    ~SkFontCollection();

    size_t getFontManagersCount() const;

    void setAssetFontManager(sk_sp<SkFontMgr> fontManager);
    void setDynamicFontManager(sk_sp<SkFontMgr> fontManager);
    void setTestFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const std::string& defaultFamilyName);

    sk_sp<SkTypeface> findTypeface(const std::string& familyName, SkFontStyle fontStyle);

    void disableFontFallback();

  private:

    sk_sp<SkTypeface>
    findByFamilyName(const std::string& familyName, SkFontStyle fontStyle);

    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const;

    struct FamilyKey {
        FamilyKey(const std::string& family,
            const std::string& loc,
            SkFontStyle style)
            : fFontFamily(family), fLocale(loc), fFontStyle(style) {}

        FamilyKey() {}

        std::string fFontFamily;
        std::string fLocale;
        SkFontStyle fFontStyle;

        bool operator==(const FamilyKey& other) const;

        struct Hasher {
            size_t operator()(const FamilyKey& key) const;
        };
    };

    bool fEnableFontFallback;
    SkTHashMap<FamilyKey, sk_sp<SkTypeface>, FamilyKey::Hasher> fTypefaces;
    sk_sp<SkFontMgr> fDefaultFontManager;
    sk_sp<SkFontMgr> fAssetFontManager;
    sk_sp<SkFontMgr> fDynamicFontManager;
    sk_sp<SkFontMgr> fTestFontManager;
    std::string fDefaultFamilyName;
};
