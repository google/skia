/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TypefaceFontProvider.h"
#include <algorithm>
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"

namespace skia {
namespace textlayout {

int TypefaceFontProvider::onCountFamilies() const { return fRegisteredFamilies.size(); }

void TypefaceFontProvider::onGetFamilyName(int index, SkString* familyName) const {
    SkASSERT(index < fRegisteredFamilies.count());
    familyName->set(fRegisteredFamilies[index]->getFamilyName());
}

SkFontStyleSet* TypefaceFontProvider::onMatchFamily(const char familyName[]) const {
    for (auto& family : fRegisteredFamilies) {
        if (family->getFamilyName().equals(familyName)) {
            return SkRef(family.get());
        }
    }
    return nullptr;
}

void TypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface) {
    if (typeface == nullptr) {
        return;
    }

    SkString familyName;
    typeface->getFamilyName(&familyName);

    registerTypeface(std::move(typeface), familyName);
}

void TypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface, const SkString& alias) {
    if (alias.size() == 0) {
        return;
    }

    TypefaceFontStyleSet* found = nullptr;
    for (auto& family : fRegisteredFamilies) {
        if (family->getAlias().equals(alias)) {
            found = family.get();
            break;
        }
    }
    if (found == nullptr) {
        found = fRegisteredFamilies.emplace_back(sk_make_sp<TypefaceFontStyleSet>(alias)).get();
    }

    found->appendTypeface(std::move(typeface));
}

TypefaceFontStyleSet::TypefaceFontStyleSet(const SkString& familyName)
        : fFamilyName(familyName) {}

int TypefaceFontStyleSet::count() { return fStyles.size(); }

void TypefaceFontStyleSet::getStyle(int index, SkFontStyle* style, SkString* name) {
    SkASSERT(index < fStyles.count());
    if (style) {
        *style = fStyles[index]->fontStyle();
    }
    if (name) {
        *name = fFamilyName;
    }
}

SkTypeface* TypefaceFontStyleSet::createTypeface(int index) {
    SkASSERT(index < fStyles.count());
    return SkRef(fStyles[index].get());
}

SkTypeface* TypefaceFontStyleSet::matchStyle(const SkFontStyle& pattern) {
    return this->matchStyleCSS3(pattern);
}

void TypefaceFontStyleSet::appendTypeface(sk_sp<SkTypeface> typeface) {
    fStyles.emplace_back(std::move(typeface));
}

}  // namespace textlayout
}  // namespace skia
