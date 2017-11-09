/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMFontMgr_DEFINED
#define DMFontMgr_DEFINED

#include "SkFontMgr.h"

// An SkFontMgr that always uses sk_tool_utils::create_portable_typeface().

namespace DM {

    class FontStyleSet final : public SkFontStyleSet {
    public:
        explicit FontStyleSet(int familyIndex);

        int count() override;
        void getStyle(int index, SkFontStyle* style, SkString* name) override;
        SkTypeface* createTypeface(int index) override;
        SkTypeface* matchStyle(const SkFontStyle& pattern) override;

    private:
        sk_sp<SkTypeface> fTypefaces[4];
    };

    class FontMgr final : public SkFontMgr {
    public:
        FontMgr();

        int onCountFamilies() const override;
        void onGetFamilyName(int index, SkString* familyName) const override;

        SkFontStyleSet* onCreateStyleSet(int index) const override;
        SkFontStyleSet* onMatchFamily(const char familyName[]) const override;

        SkTypeface* onMatchFamilyStyle(const char familyName[],
                                       const SkFontStyle&) const override;
        SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                const SkFontStyle&,
                                                const char* bcp47[], int bcp47Count,
                                                SkUnichar character) const override;
        SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                     const SkFontStyle&) const override;

        sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override;
        sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                                int ttcIndex) const override;
        sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                               const SkFontArguments&) const override;
        sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData>) const override;
        sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;

        sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override;

    private:
        sk_sp<FontStyleSet> fFamilies[3];
    };

}  // namespace DM

#endif//DMFontMgr_DEFINED
