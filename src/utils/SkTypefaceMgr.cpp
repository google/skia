/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkTypefaceMgr.h"
#include "src/core/SkFontDescriptor.h"

class SkTypefaceStyleSet : public SkFontStyleSet {
    struct Rec {
        sk_sp<SkTypeface>   fTypeface;
    }
public:
    int count() override {
        return SkToInt(fList.size());
    }
    void getStyle(int index, SkFontStyle*, SkString* style) override {

    }
    SkTypeface* createTypeface(int index) = 0;
    SkTypeface* matchStyle(const SkFontStyle& pattern) = 0;

    static SkFontStyleSet* CreateEmpty();

protected:
    SkTypeface* matchStyleCSS3(const SkFontStyle& pattern);

private:
    using INHERITED = SkRefCnt;
};

SkTypefaceMgr::SkTypefaceMgr() {}
SkTypefaceMgr::~SkTypefaceMgr() {}

void SkTypefaceMgr::append(sk_sp<SkTypeface> tf) {
    fList.push_back(std::move(tf));
}

int SkTypefaceMgr::onCountFamilies() const {
    // TODO
    return SkToInt(fList.size());
}

void SkTypefaceMgr::onGetFamilyName(int index, SkString* familyName) const {
    fList[index]->getFamilyName(familyName);
}

SkFontStyleSet* SkTypefaceMgr::onCreateStyleSet(int index) const {
    return nullptr; // TODO
}

SkFontStyleSet* SkTypefaceMgr::onMatchFamily(const char familyName[]) const {
    for (const auto& tf : fList) {
        SkString name;
        tf->getFamilyName(&name);
        if (name.equals(familyName)) {
            return tf;
        }
    }
    return nullptr;
}

SkTypeface* SkTypefaceMgr::onMatchFamilyStyle(const char family[],
                                                 const SkFontStyle& style) const {
    // TODO
    return this->onMatchFamily(family);
}

SkTypeface* SkTypefaceMgr::onMatchFamilyStyleCharacter(const char familyName[],
                                                          const SkFontStyle& style,
                                                          const char* bcp47[], int bcp47Count,
                                                          SkUnichar uni) const {
    // TODO
    return nullptr;
}

// All of these are defined to fail by returning null

sk_sp<SkTypeface> SkTypefaceMgr::onMakeFromData(sk_sp<SkData>, int ttcIndex) const {
    return nullptr;
}

sk_sp<SkTypeface> SkTypefaceMgr::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                                          int ttcIndex) const {
    return nullptr;
}

sk_sp<SkTypeface> SkTypefaceMgr::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                                         const SkFontArguments&) const {
    return nullptr;
}

sk_sp<SkTypeface> SkTypefaceMgr::onMakeFromFontData(std::unique_ptr<SkFontData>) const {
    return nullptr;
}

sk_sp<SkTypeface> SkTypefaceMgr::onMakeFromFile(const char path[], int ttcIndex) const {
    return nullptr;
}

sk_sp<SkTypeface> SkTypefaceMgr::onLegacyMakeTypeface(const char family[], SkFontStyle) const {
    return nullptr;
}
