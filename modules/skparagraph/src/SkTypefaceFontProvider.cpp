/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypefaceFontProvider.h"
#include <algorithm>

#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"

int SkTypefaceFontProvider::onCountFamilies() const { return fRegisteredFamilies.size(); }

void SkTypefaceFontProvider::onGetFamilyName(int index, SkString* familyName) const {
    SkASSERT(index < fRegisteredFamilies.count());
    familyName->set(fRegisteredFamilies[index]->getFamilyName());
}

SkFontStyleSet* SkTypefaceFontProvider::onMatchFamily(const char familyName[]) const {
    for (auto& family : fRegisteredFamilies) {
        if (family->getFamilyName().equals(familyName)) {
            return SkRef(family.get());
        }
    }
    return nullptr;
}

void SkTypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface) {
    if (typeface == nullptr) {
        return;
    }

    SkString familyName;
    typeface->getFamilyName(&familyName);

    registerTypeface(std::move(typeface), familyName);
}

void SkTypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface, const SkString& alias) {
    if (alias.size() == 0) {
        return;
    }

    SkTypefaceFontStyleSet* found = nullptr;
    for (auto& family : fRegisteredFamilies) {
        if (family->getAlias().equals(alias)) {
            found = family.get();
            break;
        }
    }
    if (found == nullptr) {
        found = fRegisteredFamilies.emplace_back(sk_make_sp<SkTypefaceFontStyleSet>(alias)).get();
    }

    found->appendTypeface(std::move(typeface));
}

SkTypefaceFontStyleSet::SkTypefaceFontStyleSet(const SkString& familyName)
        : fFamilyName(familyName) {}

int SkTypefaceFontStyleSet::count() { return fStyles.size(); }

void SkTypefaceFontStyleSet::getStyle(int index, SkFontStyle* style, SkString* name) {
    SkASSERT(index < fStyles.count());
    if (style) {
        *style = fStyles[index]->fontStyle();
    }
    if (name) {
        *name = fFamilyName;
    }
}

SkTypeface* SkTypefaceFontStyleSet::createTypeface(int index) {
    SkASSERT(index < fStyles.count());
    return SkRef(fStyles[index].get());
}

SkTypeface* SkTypefaceFontStyleSet::matchStyle(const SkFontStyle& pattern) {
    return this->matchStyleCSS3(pattern);
}

void SkTypefaceFontStyleSet::appendTypeface(sk_sp<SkTypeface> typeface) {
    fStyles.emplace_back(std::move(typeface));
}
