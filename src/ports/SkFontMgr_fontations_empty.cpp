/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontMgr_Fontations.h"
#include "src/ports/SkTypeface_fontations_priv.h"

#include <memory>
#include <utility>

class SkData;
class SkString;


namespace {

/**
 *  SkFontMgr_Fontations_Empty
 *
 *  A SkFontMgr with an empty list of fonts, meant as a basic tool for font instantiation from
 *  data in testing, see TestFontMgr in FontToolUtils.
 */
class SkFontMgr_Fontations_Empty : public SkFontMgr {
public:
    SkFontMgr_Fontations_Empty() = default;

protected:
    int onCountFamilies() const override { return 0; }
    void onGetFamilyName(int index, SkString* familyName) const override {}
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        return SkFontStyleSet::CreateEmpty();
    }
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        return SkFontStyleSet::CreateEmpty();
    }
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontStyle) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle&,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return SkTypeface_Fontations::MakeFromStream(std::move(stream), args);
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                           SkFontStyle style) const override {
        return nullptr;
    }
};

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_Fontations_Empty() {
    return sk_make_sp<SkFontMgr_Fontations_Empty>();
}
