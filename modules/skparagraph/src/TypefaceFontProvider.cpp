// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/TypefaceFontProvider.h"
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

size_t TypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface) {
    if (typeface == nullptr) {
        return 0;
    }

    SkString familyName;
    typeface->getFamilyName(&familyName);

    return registerTypeface(std::move(typeface), familyName);
}

size_t TypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface, const SkString& familyName) {
    if (familyName.size() == 0) {
        return 0;
    }

    TypefaceFontStyleSet* found = nullptr;
    for (auto& family : fRegisteredFamilies) {
        if (family->getFamilyName().equals(familyName)) {
            found = family.get();
            break;
        }
    }
    if (found == nullptr) {
        found = fRegisteredFamilies.emplace_back(sk_make_sp<TypefaceFontStyleSet>(familyName)).get();
    }

    found->appendTypeface(std::move(typeface));
    return 1;
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
