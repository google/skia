/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMFontMgr.h"
#include "SkFontDescriptor.h"
#include "sk_tool_utils.h"

namespace DM {

    static constexpr const char* kFamilyNames[] = {
        "Toy Liberation Sans",
        "Toy Liberation Serif",
        "Toy Liberation Mono",
    };

    FontStyleSet::FontStyleSet(int familyIndex) : fFamilyName(kFamilyNames[familyIndex]) {}

    // Each font family has 4 styles: Normal, Bold, Italic, BoldItalic.
    int FontStyleSet::count() { return 4; }
    void FontStyleSet::getStyle(int index, SkFontStyle* style, SkString* name) {
        switch (index) {
            default:
            case  0: if (style) { *style = SkFontStyle::Normal(); }
                     if (name)  { *name = "Normal"; }
                     break;
            case  1: if (style) { *style = SkFontStyle::Bold(); }
                     if (name)  { *name = "Bold"; }
                     break;
            case  2: if (style) { *style = SkFontStyle::Italic(); }
                     if (name)  { *name = "Italic"; }
                     break;
            case  3: if (style) { *style = SkFontStyle::BoldItalic(); }
                     if (name)  { *name = "BoldItalic"; }
                     break;
        }
    }

    SkTypeface* FontStyleSet::matchStyle(const SkFontStyle& style) {
        return this->matchStyleCSS3(style);
    }

    SkTypeface* FontStyleSet::createTypeface(int index) {
        SkFontStyle style;
        this->getStyle(index, &style, nullptr);

        return sk_tool_utils::create_portable_typeface(fFamilyName, style).release();
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    int FontMgr::onCountFamilies() const { return SK_ARRAY_COUNT(kFamilyNames); }

    void FontMgr::onGetFamilyName(int index, SkString* familyName) const {
        *familyName = kFamilyNames[index];
    }

    SkFontStyleSet* FontMgr::onCreateStyleSet(int index) const {
        return new FontStyleSet(index);
    }

    SkFontStyleSet* FontMgr::onMatchFamily(const char familyName[]) const {
        if (familyName) {
            if (strstr(familyName,  "ans")) { return this->createStyleSet(0); }
            if (strstr(familyName, "erif")) { return this->createStyleSet(1); }
            if (strstr(familyName,  "ono")) { return this->createStyleSet(2); }
        }
        return this->createStyleSet(0);
    }

    SkTypeface* FontMgr::onMatchFamilyStyle(const char familyName[],
                                            const SkFontStyle& style) const {
        sk_sp<SkFontStyleSet> styleSet(this->matchFamily(familyName));
        return styleSet->matchStyle(style);
    }

    SkTypeface* FontMgr::onMatchFamilyStyleCharacter(const char familyName[],
                                                     const SkFontStyle& style,
                                                     const char* bcp47[],
                                                     int bcp47Count,
                                                     SkUnichar character) const {
        (void)bcp47;
        (void)bcp47Count;
        (void)character;
        return this->matchFamilyStyle(familyName, style);
    }

    SkTypeface* FontMgr::onMatchFaceStyle(const SkTypeface* tf,
                                          const SkFontStyle& style) const {
        SkString familyName;
        tf->getFamilyName(&familyName);
        return this->matchFamilyStyle(familyName.c_str(), style);
    }

    sk_sp<SkTypeface> FontMgr::onMakeFromData(sk_sp<SkData>, int ttcIndex) const {
        return sk_sp<SkTypeface>(this->matchFamilyStyle("Sans", SkFontStyle::Normal()));
    }

    sk_sp<SkTypeface> FontMgr::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                                     int ttcIndex) const {
        return sk_sp<SkTypeface>(this->matchFamilyStyle("Sans", SkFontStyle::Normal()));
    }

    sk_sp<SkTypeface> FontMgr::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                                    const SkFontArguments&) const {
        return sk_sp<SkTypeface>(this->matchFamilyStyle("Sans", SkFontStyle::Normal()));
    }

    sk_sp<SkTypeface> FontMgr::onMakeFromFontData(std::unique_ptr<SkFontData>) const {
        return sk_sp<SkTypeface>(this->matchFamilyStyle("Sans", SkFontStyle::Normal()));
    }

    sk_sp<SkTypeface> FontMgr::onMakeFromFile(const char path[], int ttcIndex) const {
        return sk_sp<SkTypeface>(this->matchFamilyStyle("Sans", SkFontStyle::Normal()));
    }

    sk_sp<SkTypeface> FontMgr::onLegacyMakeTypeface(const char familyName[],
                                                    SkFontStyle style) const {
        return sk_sp<SkTypeface>(this->matchFamilyStyle(familyName, style));
    }

} // namespace DM
