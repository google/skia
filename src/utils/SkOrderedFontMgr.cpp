/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkOrderedFontMgr.h"

SkOrderedFontMgr::SkOrderedFontMgr() {}
SkOrderedFontMgr::~SkOrderedFontMgr() {}

void SkOrderedFontMgr::append(sk_sp<SkFontMgr> fm) {
    fList.push_back(std::move(fm));
}

int SkOrderedFontMgr::onCountFamilies() {
    int count = 0;
    for (const auto& fm : fList) {
        count += fm->countFamilies();
    }
    return count;
}
    
void SkOrderedFontMgr::onGetFamilyName(int index, SkString* familyName) {
    for (const auto& fm : fList) {
        const int count = fm->countFamilies();
        if (index < count) {
            return fm->getFamilyName(index, familyName);
        }
        index -= count;
    }
}
    
SkFontStyleSet* SkOrderedFontMgr::onCreateStyleSet(int index) {
    for (const auto& fm : fList) {
        const int count = fm->countFamilies();
        if (index < count) {
            return fm->createStyleSet(index);
        }
        index -= count;
    }
    return nullptr;
}

SkFontStyleSet* SkOrderedFontMgr::onMatchFamily(const char familyName[]) {
    for (const auto& fm : fList) {
        if (auto fs = fm->matchFamily(familyName)) {
            return fs;
        }
        return nullptr;
    }
}

SkTypeface* SkOrderedFontMgr::onMatchFamilyStyle(const char family[], const SkFontStyle& style) {
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
                                                          SkUnichar uni) {
    for (const auto& fm : fList) {
        if (auto tf = fm->matchFamilyStyleCharacter(familyName, style, bcp47, bcp47Count, uni)) {
            return tf;
        }
    }
    return nullptr;
}

SkTypeface* SkOrderedFontMgr::onMatchFaceStyle(const SkTypeface* face, const SkFontStyle& style) {
    for (const auto& fm : fList) {
        if (auto tf = fm->matchFaceStyle(face, style)) {
            return tf;
        }
    }
    return nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromData(sk_sp<SkData> data, int ttcIndex) {
    return fList.size() ? fList[0]->makeFromData(data, ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                                          int ttcIndex) {
    return fList.size() ? fList[0]->makeFromStreamIndex(std::move(stream), ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                         const SkFontArguments& args) {
    return fList.size() ? fList[0]->makeFromStreamArgs(std::move(stream), ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromFontData(std::unique_ptr<SkFontData> data) {
    return fList.size() ? fList[0]->makeFromFontData(std::move(data)) : nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onMakeFromFile(const char path[], int ttcIndex) {
    return fList.size() ? fList[0]->makeFromFile(path, ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkOrderedFontMgr::onLegacyMakeTypeface(const char family[], SkFontStyle style) {
    return fList.size() ? fList[0]->legacyMakeTypeface(family, style) : nullptr;
}
