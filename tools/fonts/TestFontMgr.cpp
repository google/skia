/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/private/base/SkAssert.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkFontDescriptor.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/TestFontMgr.h"
#include "tools/fonts/TestTypeface.h"

#ifdef SK_XML
#include "tools/fonts/TestSVGTypeface.h"
#endif

#include <vector>

namespace {

class FontStyleSet final : public SkFontStyleSet {
public:
    FontStyleSet(const char* familyName) : fFamilyName(familyName) {}
    struct TypefaceEntry {
        TypefaceEntry(sk_sp<SkTypeface> typeface, SkFontStyle style, const char* styleName)
                : fTypeface(std::move(typeface)), fStyle(style), fStyleName(styleName) {}
        sk_sp<SkTypeface> fTypeface;
        SkFontStyle       fStyle;
        const char*       fStyleName;
    };

    int count() override { return fTypefaces.size(); }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (style) {
            *style = fTypefaces[index].fStyle;
        }
        if (name) {
            *name = fTypefaces[index].fStyleName;
        }
    }

    sk_sp<SkTypeface> createTypeface(int index) override {
        return fTypefaces[index].fTypeface;
    }

    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        return this->matchStyleCSS3(pattern);
    }

    SkString getFamilyName() { return fFamilyName; }

    std::vector<TypefaceEntry> fTypefaces;
    SkString                   fFamilyName;
};

class FontMgr final : public SkFontMgr {
public:
    FontMgr() {
        auto&& list = TestTypeface::Typefaces();
        for (auto&& family : list.families) {
            auto&& ss = fFamilies.emplace_back(sk_make_sp<FontStyleSet>(family.name));
            for (auto&& face : family.faces) {
                ss->fTypefaces.emplace_back(face.typeface, face.typeface->fontStyle(), face.name);
                if (face.isDefault) {
                    fDefaultFamily = ss;
                    fDefaultTypeface = face.typeface;
                }
            }
        }
        if (!fDefaultFamily) {
            SkASSERTF(false, "expected TestTypeface to return a default");
            fDefaultFamily = fFamilies[0];
            fDefaultTypeface = fDefaultFamily->fTypefaces[0].fTypeface;
        }

#if defined(SK_ENABLE_SVG)
        fFamilies.emplace_back(sk_make_sp<FontStyleSet>("Emoji"));
        fFamilies.back()->fTypefaces.emplace_back(
                TestSVGTypeface::Default(), SkFontStyle::Normal(), "Normal");

        fFamilies.emplace_back(sk_make_sp<FontStyleSet>("Planet"));
        fFamilies.back()->fTypefaces.emplace_back(
                TestSVGTypeface::Planets(), SkFontStyle::Normal(), "Normal");
#endif
    }

    int onCountFamilies() const override { return fFamilies.size(); }

    void onGetFamilyName(int index, SkString* familyName) const override {
        *familyName = fFamilies[index]->getFamilyName();
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        sk_sp<SkFontStyleSet> ref = fFamilies[index];
        return ref;
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (familyName) {
            if (strstr(familyName, "ono")) {
                return this->createStyleSet(0);
            }
            if (strstr(familyName, "ans")) {
                return this->createStyleSet(1);
            }
            if (strstr(familyName, "erif")) {
                return this->createStyleSet(2);
            }
#if defined(SK_ENABLE_SVG)
            if (strstr(familyName, "oji")) {
                return this->createStyleSet(6);
            }
            if (strstr(familyName, "Planet")) {
                return this->createStyleSet(7);
            }
#endif
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char         familyName[],
                                         const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> styleSet(this->matchFamily(familyName));
        return styleSet->matchStyle(style);
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char         familyName[],
                                                  const SkFontStyle& style,
                                                  const char*        bcp47[],
                                                  int                bcp47Count,
                                                  SkUnichar          character) const override {
        (void)bcp47;
        (void)bcp47Count;
        (void)character;
        return this->matchFamilyStyle(familyName, style);
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override { return nullptr; }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                            int ttcIndex) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char  familyName[],
                                           SkFontStyle style) const override {
        if (familyName == nullptr) {
            return sk_sp<SkTypeface>(fDefaultFamily->matchStyle(style));
        }
        sk_sp<SkTypeface> typeface = sk_sp<SkTypeface>(this->matchFamilyStyle(familyName, style));
        if (!typeface) {
            typeface = fDefaultTypeface;
        }
        return typeface;
    }

private:
    std::vector<sk_sp<FontStyleSet>> fFamilies;
    sk_sp<FontStyleSet>              fDefaultFamily;
    sk_sp<SkTypeface>                fDefaultTypeface;
};
}  // namespace

namespace ToolUtils {
sk_sp<SkFontMgr> MakePortableFontMgr() { return sk_make_sp<FontMgr>(); }
}  // namespace ToolUtils
