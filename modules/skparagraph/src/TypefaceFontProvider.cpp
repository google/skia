// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include <algorithm>
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkFontDescriptor.h"

namespace skia {
namespace textlayout {

int TypefaceFontProvider::onCountFamilies() const { return fRegisteredFamilies.size(); }

void TypefaceFontProvider::onGetFamilyName(int index, SkString* familyName) const {
    SkASSERT((unsigned)index < fRegisteredFamilies.size());
    familyName->set(fFamilyNames[index]);
}

sk_sp<SkFontStyleSet> TypefaceFontProvider::onMatchFamily(const char familyName[]) const {
    auto found = fRegisteredFamilies.find(familyName);
    if (found != fRegisteredFamilies.end()) {
        return found->second;
    } else {
        return nullptr;
    }
}

sk_sp<SkFontStyleSet> TypefaceFontProvider::onCreateStyleSet(int index) const {
    SkASSERT((unsigned)index < fRegisteredFamilies.size());
    auto found = fRegisteredFamilies.find(fFamilyNames[index]);
    if (found != fRegisteredFamilies.end()) {
        return found->second;
    } else {
        return nullptr;
    }
}

sk_sp<SkTypeface> TypefaceFontProvider::onMatchFamilyStyle(const char familyName[], const SkFontStyle& pattern) const {
    sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
    if (sset) {
        return sset->matchStyle(pattern);
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

    auto fname(familyName.c_str());
    auto found = fRegisteredFamilies.find(fname);
    if (found == fRegisteredFamilies.end()) {
        auto val = fRegisteredFamilies[fname] = sk_make_sp<TypefaceFontStyleSet>(familyName);
        fFamilyNames.emplace_back(fname);
        val->appendTypeface(std::move(typeface));
    } else {
        found->second->appendTypeface(std::move(typeface));
    }

    return 1;
}

sk_sp<SkTypeface> TypefaceFontProvider::onLegacyMakeTypeface(const char familyName[],
                                                             SkFontStyle style) const {
    if (familyName) {
        sk_sp<SkTypeface> matchedByFamily = this->matchFamilyStyle(familyName, style);
        if (matchedByFamily) {
            return matchedByFamily;
        }
    }
    if (this->countFamilies() == 0) {
        return nullptr;
    }
    sk_sp<SkFontStyleSet> defaultFamily = this->createStyleSet(0);
    if (!defaultFamily) {
        return nullptr;
    }
    return defaultFamily->matchStyle(style);
}

TypefaceFontStyleSet::TypefaceFontStyleSet(const SkString& familyName)
        : fFamilyName(familyName) {}

int TypefaceFontStyleSet::count() { return fStyles.size(); }

void TypefaceFontStyleSet::getStyle(int index, SkFontStyle* style, SkString* name) {
    SkASSERT(index < fStyles.size());
    if (style) {
        *style = fStyles[index]->fontStyle();
    }
    if (name) {
        *name = fFamilyName;
    }
}

sk_sp<SkTypeface> TypefaceFontStyleSet::createTypeface(int index) {
    SkASSERT(index < fStyles.size());
    return fStyles[index];
}

sk_sp<SkTypeface> TypefaceFontStyleSet::matchStyle(const SkFontStyle& pattern) {
    return this->matchStyleCSS3(pattern);
}

void TypefaceFontStyleSet::appendTypeface(sk_sp<SkTypeface> typeface) {
    if (typeface.get() != nullptr) {
        fStyles.emplace_back(std::move(typeface));
    }
}

}  // namespace textlayout
}  // namespace skia
