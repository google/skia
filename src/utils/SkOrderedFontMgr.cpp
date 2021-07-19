/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkOrderedFontMgr.h"
#include "src/core/SkFontDescriptor.h"

SkOrderedFontMgr::SkOrderedFontMgr() {}
SkOrderedFontMgr::~SkOrderedFontMgr() {}

void SkOrderedFontMgr::append(sk_sp<SkFontMgr> fm) {
    fList.push_back(std::move(fm));
}

int SkOrderedFontMgr::onCountFamilies() const {
    int count = 0;
    for (const auto& fm : fList) {
        count += fm->countFamilies();
    }
    return count;
}

void SkOrderedFontMgr::onGetFamilyName(int index, SkString* familyName) const {
    for (const auto& fm : fList) {
        const int count = fm->countFamilies();
        if (index < count) {
            return fm->getFamilyName(index, familyName);
        }
        index -= count;
    }
}

SkFontStyleSet* SkOrderedFontMgr::onCreateStyleSet(int index) const {
    for (const auto& fm : fList) {
        const int count = fm->countFamilies();
        if (index < count) {
            return fm->createStyleSet(index);
        }
        index -= count;
    }
    return nullptr;
}

SkFontStyleSet* SkOrderedFontMgr::onMatchFamily(const char familyName[]) const {
    for (const auto& fm : fList) {
        if (auto fs = fm->matchFamily(familyName)) {
            return fs;
        }
    }
    return nullptr;
}

SkTypeface* SkOrderedFontMgr::onMatchFamilyStyle(const char family[],
                                                 const SkFontStyle& style) const {
    for (const auto& fm : fList) {
        if (auto tf = fm->matchFamilyStyle(family, style)) {
            return tf;
        }
    }
    return nullptr;
}

SkTypeface* SkOrderedFontMgr::onMatchFamilyStyleCharacter(const char familyName[],
                                                          const SkFontStyle& style,
                                                          const char* bcp47[], int bcp47Count,
                                                          SkUnichar uni) const {
    for (const auto& fm : fList) {
        if (auto tf = fm->matchFamilyStyleCharacter(familyName, style, bcp47, bcp47Count, uni)) {
            return tf;
        }
    }
    return nullptr;
}

// All of these are defined to fail by returning null

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromData(sk_sp<SkData>, int ttcIndex) const {
    return nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                                          int ttcIndex) const {
    return nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                                         const SkFontArguments&) const {
    return nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromFile(const char path[], int ttcIndex) const {
    return nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onLegacyMakeTypeface(const char family[], SkFontStyle) const {
    return nullptr;
}
