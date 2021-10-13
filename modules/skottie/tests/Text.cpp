/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unordered_map>

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

using namespace skottie;

namespace {

class RecordMatchFamilyStyleSkFontMgr : public SkFontMgr {
public:
    const SkFontStyle* styleRequestedWhenMatchingFamily(const char* family) const {
        auto s = fStyleRequestedWhenMatchingFamily.find(family);
        return s != fStyleRequestedWhenMatchingFamily.end() ? &s->second : nullptr;
    }

private:
    int onCountFamilies() const override { return 0; }
    void onGetFamilyName(int index, SkString* familyName) const override {}
    SkFontStyleSet* onCreateStyleSet(int index) const override { return nullptr; }

    SkFontStyleSet* onMatchFamily(const char[]) const override { return nullptr; }

    SkTypeface* onMatchFamilyStyle(const char family[], const SkFontStyle& style) const override {
        SkASSERT(fStyleRequestedWhenMatchingFamily.find(family) ==
                 fStyleRequestedWhenMatchingFamily.end());
        fStyleRequestedWhenMatchingFamily[family] = style;
        return nullptr;
    }
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override {
        return nullptr;
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
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override {
        return nullptr;
    }

    mutable std::unordered_map<std::string, SkFontStyle> fStyleRequestedWhenMatchingFamily;
};

} // namespace

// This test relies on Skottie internals/implementation details, and may need to
// be updated in the future, if Skottie font resolution changes.
DEF_TEST(Skottie_Text_Style, r) {
    static constexpr char json[] =
        R"({
             "v": "5.2.1",
             "w": 100,
             "h": 100,
             "fr": 10,
             "ip": 0,
             "op": 100,
             "fonts": {
               "list": [
                 { "fName"  : "f1", "fFamily": "f1", "fStyle" : "Regular"   },
                 { "fName"  : "f2", "fFamily": "f2", "fStyle" : "Medium"    },
                 { "fName"  : "f3", "fFamily": "f3", "fStyle" : "Bold"      },
                 { "fName"  : "f4", "fFamily": "f4", "fStyle" : "Light"     },
                 { "fName"  : "f5", "fFamily": "f5", "fStyle" : "Extra"     },
                 { "fName"  : "f6", "fFamily": "f6", "fStyle" : "ExtraBold" },

                 { "fName"  : "f7" , "fFamily": "f7" , "fStyle" : "Regular Italic"    },
                 { "fName"  : "f8" , "fFamily": "f8" , "fStyle" : "Medium Italic"     },
                 { "fName"  : "f9" , "fFamily": "f9" , "fStyle" : "Bold Italic"       },
                 { "fName"  : "f10", "fFamily": "f10", "fStyle" : "Light Oblique"     },
                 { "fName"  : "f11", "fFamily": "f11", "fStyle" : "Extra Oblique"     },
                 { "fName"  : "f12", "fFamily": "f12", "fStyle" : "Extrabold Oblique" },

                 { "fName"  : "f13", "fFamily": "f13", "fStyle" : "Italic"  },
                 { "fName"  : "f14", "fFamily": "f14", "fStyle" : "Oblique" },
                 { "fName"  : "f15", "fFamily": "f15", "fStyle" : ""        }
               ]
             }
           })";

    SkMemoryStream stream(json, strlen(json));
    auto fmgr = sk_make_sp<RecordMatchFamilyStyleSkFontMgr>();

    auto anim = Animation::Builder()
                    .setFontManager(fmgr)
                    .make(&stream);

    REPORTER_ASSERT(r, anim);

    static constexpr struct {
        const char*         family;
        SkFontStyle::Weight weight;
        SkFontStyle::Slant  slant;
    } expected[] = {
        { "f1" , SkFontStyle::kNormal_Weight   , SkFontStyle::kUpright_Slant },
        { "f2" , SkFontStyle::kMedium_Weight   , SkFontStyle::kUpright_Slant },
        { "f3" , SkFontStyle::kBold_Weight     , SkFontStyle::kUpright_Slant },
        { "f4" , SkFontStyle::kLight_Weight    , SkFontStyle::kUpright_Slant },
        { "f5" , SkFontStyle::kExtraBold_Weight, SkFontStyle::kUpright_Slant },
        { "f6" , SkFontStyle::kExtraBold_Weight, SkFontStyle::kUpright_Slant },

        { "f7" , SkFontStyle::kNormal_Weight   , SkFontStyle::kItalic_Slant  },
        { "f8" , SkFontStyle::kMedium_Weight   , SkFontStyle::kItalic_Slant  },
        { "f9" , SkFontStyle::kBold_Weight     , SkFontStyle::kItalic_Slant  },
        { "f10", SkFontStyle::kLight_Weight    , SkFontStyle::kOblique_Slant },
        { "f11", SkFontStyle::kExtraBold_Weight, SkFontStyle::kOblique_Slant },
        { "f12", SkFontStyle::kExtraBold_Weight, SkFontStyle::kOblique_Slant },

        { "f13", SkFontStyle::kNormal_Weight   , SkFontStyle::kItalic_Slant  },
        { "f14", SkFontStyle::kNormal_Weight   , SkFontStyle::kOblique_Slant },
        { "f15", SkFontStyle::kNormal_Weight   , SkFontStyle::kUpright_Slant },
    };

    for (const auto& exp : expected) {
        const auto* style = fmgr->styleRequestedWhenMatchingFamily(exp.family);
        REPORTER_ASSERT(r, style);
        REPORTER_ASSERT(r, style->weight() == exp.weight);
        REPORTER_ASSERT(r, style->slant () == exp.slant );
    }
}

DEF_TEST(Skottie_Text_LayoutError, r) {
    // Text node properties:
    //   - scale to fit
    //   - box width: 100
    //   - min font size: 70
    //   - string: Foo Bar Baz
    //
    // Layout should fail with these unsatisfiable constraints.
    static constexpr char json[] =
        R"({
             "v": "5.2.1",
             "w": 100,
             "h": 100,
             "fr": 10,
             "ip": 0,
             "op": 100,
             "fonts": {
               "list": [{
                 "fFamily": "Arial",
                 "fName": "Arial",
                 "fStyle": "Bold"
               }]
             },
             "layers": [{
               "ty": 5,
               "t": {
                 "d": {
                   "k": [{
                     "t": 0,
                     "s": {
                       "f": "Arial",
                       "t": "Foo Bar Baz",
                       "s": 24,
                       "fc": [1,1,1,1],
                       "lh": 70,
                       "ps": [0, 0],
                       "sz": [100, 100],
                       "mf": 70,
                       "rs": 1
                     }
                   }]
                 }
               }
             }]
           })";

    class Logger final : public skottie::Logger {
    public:
        const std::vector<SkString>& errors() const { return fErrors; }

    private:
        void log(Level lvl, const char message[], const char* = nullptr) override {
            if (lvl == Level::kError) {
                fErrors.emplace_back(message);
            }
        }

        std::vector<SkString> fErrors;
    };

    class PortableRP final : public skresources::ResourceProvider {
    private:
        sk_sp<SkTypeface> loadTypeface(const char[], const char[]) const override {
            return ToolUtils::create_portable_typeface("Serif", SkFontStyle());
        }
    };

    SkMemoryStream stream(json, strlen(json));
    auto logger = sk_make_sp<Logger>();

    auto anim = Animation::Builder()
                    .setLogger(logger)
                    .setResourceProvider(sk_make_sp<PortableRP>())
                    .make(&stream);

    REPORTER_ASSERT(r, anim);
    REPORTER_ASSERT(r, logger->errors().size() == 1);
    REPORTER_ASSERT(r, logger->errors()[0].startsWith("Text layout failed"));
}
