/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontDescriptor.h"
#include "SkTestFontMgr.h"
#include "sk_tool_utils.h"

namespace {

static constexpr const char* kFamilyNames[] = {
    "Toy Liberation Sans",
    "Toy Liberation Serif",
    "Toy Liberation Mono",
};

class FontStyleSet final : public SkFontStyleSet {
public:
    explicit FontStyleSet(int familyIndex) {
        using sk_tool_utils::create_portable_typeface;
        const char* familyName = kFamilyNames[familyIndex];

        fTypefaces[0] = create_portable_typeface(familyName, SkFontStyle::Normal());
        fTypefaces[1] = create_portable_typeface(familyName, SkFontStyle::Bold());
        fTypefaces[2] = create_portable_typeface(familyName, SkFontStyle::Italic());
        fTypefaces[3] = create_portable_typeface(familyName, SkFontStyle::BoldItalic());
    }

    int count() override { return 4; }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
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

    SkTypeface* createTypeface(int index) override {
        return SkRef(fTypefaces[index].get());
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        return this->matchStyleCSS3(pattern);
    }

private:
    sk_sp<SkTypeface> fTypefaces[4];
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

class FontMgr final : public SkFontMgr {
public:
    FontMgr() {
        fFamilies[0] = sk_make_sp<FontStyleSet>(0);
        fFamilies[1] = sk_make_sp<FontStyleSet>(1);
        fFamilies[2] = sk_make_sp<FontStyleSet>(2);
    }

    int onCountFamilies() const override { return SK_ARRAY_COUNT(fFamilies); }

    void onGetFamilyName(int index, SkString* familyName) const override {
        *familyName = kFamilyNames[index];
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        return SkRef(fFamilies[index].get());
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (familyName) {
            if (strstr(familyName,  "ans")) { return this->createStyleSet(0); }
            if (strstr(familyName, "erif")) { return this->createStyleSet(1); }
            if (strstr(familyName,  "ono")) { return this->createStyleSet(2); }
        }
        return this->createStyleSet(0);
    }


    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> styleSet(this->matchFamily(familyName));
        return styleSet->matchStyle(style);
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override {
        (void)bcp47;
        (void)bcp47Count;
        (void)character;
        return this->matchFamilyStyle(familyName, style);
    }

    SkTypeface* onMatchFaceStyle(const SkTypeface* tf,
                                 const SkFontStyle& style) const override {
        SkString familyName;
        tf->getFamilyName(&familyName);
        return this->matchFamilyStyle(familyName.c_str(), style);
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                            int ttcIndex) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData>) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                           SkFontStyle style) const override {
        return sk_sp<SkTypeface>(this->matchFamilyStyle(familyName, style));
    }

private:
    sk_sp<FontStyleSet> fFamilies[3];
};
}

namespace sk_tool_utils {
sk_sp<SkFontMgr> MakePortableFontMgr() { return sk_make_sp<FontMgr>(); }
} // namespace sk_tool_utils
