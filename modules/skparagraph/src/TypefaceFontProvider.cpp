// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include <algorithm>
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkFontDescriptor.h"

namespace skia {
namespace textlayout {

int TypefaceFontProvider::onCountFamilies() const { return fRegisteredFamilies.count(); }

void TypefaceFontProvider::onGetFamilyName(int index, SkString* familyName) const {
    SkASSERT(index < fRegisteredFamilies.count());
    familyName->set(fFamilyNames[index]);
}

SkFontStyleSet* TypefaceFontProvider::onMatchFamily(const char familyName[]) const {
    auto found = fRegisteredFamilies.find(SkString(familyName));
    if (found) {
      return SkRef((*found).get());
    }
    return nullptr;
}

size_t TypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface) {
    if (typeface == nullptr) {
        return 0;
    }

    SkString familyName;
    typeface->getFamilyName(&familyName);

    return registerTypeface(std::move(typeface), std::move(familyName));
}

size_t TypefaceFontProvider::registerTypeface(sk_sp<SkTypeface> typeface, const SkString& familyName) {
    if (familyName.size() == 0) {
        return 0;
    }

    auto found = fRegisteredFamilies.find(familyName);
    if (found == nullptr) {
        found = fRegisteredFamilies.set(familyName, sk_make_sp<TypefaceFontStyleSet>(familyName));
        fFamilyNames.emplace_back(familyName);
    }

    (*found)->appendTypeface(std::move(typeface));

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
    if (typeface.get() != nullptr) {
        fStyles.emplace_back(std::move(typeface));
    }
}

}  // namespace textlayout
}  // namespace skia
