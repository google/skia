/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

using namespace skottie;

namespace {

TextPropertyValue make_text_prop(const char* str) {
    TextPropertyValue prop;

    prop.fTypeface = ToolUtils::DefaultPortableTypeface();
    prop.fText     = SkString(str);

    return prop;
}

class MockPropertyObserver final : public PropertyObserver {
public:
    explicit MockPropertyObserver(bool override_props = false) : fOverrideProps(override_props) {}

    struct ColorInfo {
        SkString                                      node_name;
        std::unique_ptr<skottie::ColorPropertyHandle> handle;
    };

    struct OpacityInfo {
        SkString                                        node_name;
        std::unique_ptr<skottie::OpacityPropertyHandle> handle;
    };

    struct TextInfo {
        SkString                                     node_name;
        std::unique_ptr<skottie::TextPropertyHandle> handle;
    };

    struct TransformInfo {
        SkString                                          node_name;
        std::unique_ptr<skottie::TransformPropertyHandle> handle;
    };

    void onColorProperty(const char node_name[],
            const PropertyObserver::LazyHandle<ColorPropertyHandle>& lh) override {
        auto prop_handle = lh();
        if (fOverrideProps) {
            static constexpr ColorPropertyValue kOverrideColor = 0xffffff00;
            prop_handle->set(kOverrideColor);
        }
        fColors.push_back({SkString(node_name), std::move(prop_handle)});
        fColorsWithFullKeypath.push_back({SkString(fCurrentNode.c_str()), lh()});
    }

    void onOpacityProperty(const char node_name[],
            const PropertyObserver::LazyHandle<OpacityPropertyHandle>& lh) override {
        auto prop_handle = lh();
        if (fOverrideProps) {
            static constexpr OpacityPropertyValue kOverrideOpacity = 0.75f;
            prop_handle->set(kOverrideOpacity);
        }
        fOpacities.push_back({SkString(node_name), std::move(prop_handle)});
    }

    void onTextProperty(const char node_name[],
                        const PropertyObserver::LazyHandle<TextPropertyHandle>& lh) override {
        auto prop_handle = lh();
        if (fOverrideProps) {
            static const TextPropertyValue kOverrideText = make_text_prop("foo");
            prop_handle->set(kOverrideText);
        }
        fTexts.push_back({SkString(node_name), std::move(prop_handle)});
    }

    void onTransformProperty(const char node_name[],
            const PropertyObserver::LazyHandle<TransformPropertyHandle>& lh) override {
        auto prop_handle = lh();
        if (fOverrideProps) {
            static constexpr TransformPropertyValue kOverrideTransform = {
                { 100, 100 },
                { 200, 200 },
                {   2,   2 },
                45,
                0,
                0,
            };
            prop_handle->set(kOverrideTransform);
        }
        fTransforms.push_back({SkString(node_name), std::move(prop_handle)});
    }

    void onEnterNode(const char node_name[], PropertyObserver::NodeType node_type) override {
        if (node_name == nullptr) {
            return;
        }
        fCurrentNode = fCurrentNode.empty() ? node_name : fCurrentNode + "." + node_name;
    }

    void onLeavingNode(const char node_name[], PropertyObserver::NodeType node_type) override {
        if (node_name == nullptr) {
            return;
        }
        auto length = strlen(node_name);
        fCurrentNode =
                fCurrentNode.length() > length
                        ? fCurrentNode.substr(0, fCurrentNode.length() - strlen(node_name) - 1)
                        : "";
    }

    const std::vector<ColorInfo>& colors() const { return fColors; }
    const std::vector<OpacityInfo>& opacities() const { return fOpacities; }
    const std::vector<TextInfo>& texts() const { return fTexts; }
    const std::vector<TransformInfo>& transforms() const { return fTransforms; }
    const std::vector<ColorInfo>& colorsWithFullKeypath() const {
        return fColorsWithFullKeypath;
    }

private:
    const bool                 fOverrideProps;
    std::vector<ColorInfo>     fColors;
    std::vector<OpacityInfo>   fOpacities;
    std::vector<TextInfo>      fTexts;
    std::vector<TransformInfo> fTransforms;
    std::string                fCurrentNode;
    std::vector<ColorInfo>     fColorsWithFullKeypath;
};

// Returns a single specified typeface for all requests.
class MockFontMgr : public SkFontMgr {
 public:
    MockFontMgr(sk_sp<SkTypeface> test_font) : fTestFont(std::move(test_font)) {}

    int onCountFamilies() const override { return 1; }
    void onGetFamilyName(int index, SkString* familyName) const override {}
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override { return nullptr; }
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontStyle) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                  const char* bcp47[], int bcp47Count,
                                                  SkUnichar character) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override {
        return fTestFont;
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                                int ttcIndex) const override {
        return fTestFont;
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                               const SkFontArguments&) const override {
        return fTestFont;
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return fTestFont;
    }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override {
        return fTestFont;
    }
 private:
    sk_sp<SkTypeface> fTestFont;
};

static const char gTestJson[] = R"({
                                 "v": "5.2.1",
                                 "w": 100,
                                 "h": 100,
                                 "fr": 1,
                                 "ip": 0,
                                 "op": 1,
                                 "fonts": {
                                   "list": [
                                     {
                                       "fName": "test_font",
                                       "fFamily": "test-family",
                                       "fStyle": "TestFontStyle"
                                     }
                                   ]
                                 },
                                 "layers": [
                                   {
                                     "ty": 4,
                                     "nm": "layer_0",
                                     "ind": 0,
                                     "ip": 0,
                                     "op": 1,
                                     "ks": {
                                       "o": { "a": 0, "k": 50 }
                                     },
                                     "ef": [{
                                       "ef": [
                                         {},
                                         {},
                                         { "v": { "a": 0, "k": [ 0, 1, 0 ] }},
                                         {},
                                         {},
                                         {},
                                         { "v": { "a": 0, "k": 1 }}
                                       ],
                                       "nm": "fill_effect_0",
                                       "mn": "ADBE Fill",
                                       "ty": 21
                                     }],
                                     "shapes": [
                                       {
                                         "ty": "el",
                                         "nm": "geometry_0",
                                         "p": { "a": 0, "k": [ 50, 50 ] },
                                         "s": { "a": 0, "k": [ 50, 50 ] }
                                       },
                                       {
                                         "ty": "fl",
                                         "nm": "fill_0",
                                         "c": { "a": 0, "k": [ 1, 0, 0] }
                                       },
                                       {
                                         "ty": "tr",
                                         "nm": "shape_transform_0",
                                         "o": { "a": 0, "k": 100 },
                                         "s": { "a": 0, "k": [ 50, 50 ] }
                                       }
                                     ]
                                   },
                                   {
                                     "ty": 5,
                                     "nm": "layer_1",
                                     "ip": 0,
                                     "op": 1,
                                     "ks": {
                                       "p": { "a": 0, "k": [25, 25] }
                                     },
                                     "t": {
                                       "d": {
                                         "k": [
                                            {
                                              "t": 0,
                                              "s": {
                                                "f": "test_font",
                                                "s": 100,
                                                "t": "inline_text",
                                                "lh": 120,
                                                "ls": 12
                                              }
                                            }
                                         ]
                                       }
                                     }
                                   }
                                 ]
                               })";

}  // anonymous namespace

DEF_TEST(Skottie_Props, reporter) {
    auto test_typeface = ToolUtils::DefaultPortableTypeface();
    REPORTER_ASSERT(reporter, test_typeface);
    auto test_font_manager = sk_make_sp<MockFontMgr>(test_typeface);

    SkMemoryStream stream(gTestJson, strlen(gTestJson));
    auto observer = sk_make_sp<MockPropertyObserver>();

    auto animation = skottie::Animation::Builder()
            .setPropertyObserver(observer)
            .setFontManager(test_font_manager)
            .make(&stream);

    REPORTER_ASSERT(reporter, animation);

    const auto& colors = observer->colors();
    REPORTER_ASSERT(reporter, colors.size() == 2);
    REPORTER_ASSERT(reporter, colors[0].node_name.equals("fill_0"));
    REPORTER_ASSERT(reporter, colors[0].handle->get() == 0xffff0000);
    REPORTER_ASSERT(reporter, colors[1].node_name.equals("fill_effect_0"));
    REPORTER_ASSERT(reporter, colors[1].handle->get() == 0xff00ff00);

    const auto& colorsWithFullKeypath = observer->colorsWithFullKeypath();
    REPORTER_ASSERT(reporter, colorsWithFullKeypath.size() == 2);
    REPORTER_ASSERT(reporter, colorsWithFullKeypath[0].node_name.equals("layer_0.fill_0"));
    REPORTER_ASSERT(reporter, colorsWithFullKeypath[0].handle->get() == 0xffff0000);
    REPORTER_ASSERT(reporter, colorsWithFullKeypath[1].node_name.equals("layer_0.fill_effect_0"));
    REPORTER_ASSERT(reporter, colorsWithFullKeypath[1].handle->get() == 0xff00ff00);

    const auto& opacities = observer->opacities();
    REPORTER_ASSERT(reporter, opacities.size() == 3);
    REPORTER_ASSERT(reporter, opacities[0].node_name.equals("shape_transform_0"));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(opacities[0].handle->get(), 100));
    REPORTER_ASSERT(reporter, opacities[1].node_name.equals("layer_0"));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(opacities[1].handle->get(), 50));

    const auto& transforms = observer->transforms();
    REPORTER_ASSERT(reporter, transforms.size() == 3);
    REPORTER_ASSERT(reporter, transforms[0].node_name.equals("layer_0"));
    REPORTER_ASSERT(reporter, transforms[0].handle->get() == skottie::TransformPropertyValue({
        SkPoint::Make(0, 0),
        SkPoint::Make(0, 0),
        SkVector::Make(100, 100),
        0,
        0,
        0
    }));
    REPORTER_ASSERT(reporter, transforms[1].node_name.equals("layer_1"));
    REPORTER_ASSERT(reporter, transforms[1].handle->get() == skottie::TransformPropertyValue({
        SkPoint::Make(0, 0),
        SkPoint::Make(25, 25),
        SkVector::Make(100, 100),
        0,
        0,
        0
    }));
    REPORTER_ASSERT(reporter, transforms[2].node_name.equals("shape_transform_0"));
    REPORTER_ASSERT(reporter, transforms[2].handle->get() == skottie::TransformPropertyValue({
        SkPoint::Make(0, 0),
        SkPoint::Make(0, 0),
        SkVector::Make(50, 50),
        0,
        0,
        0
    }));

    const auto& texts = observer->texts();
    REPORTER_ASSERT(reporter, texts.size() == 1);
    REPORTER_ASSERT(reporter, texts[0].node_name.equals("layer_1"));
    skottie::TextPropertyValue text_prop({
      test_typeface,
      SkString("inline_text"),
      100,
      0, 100,
      0,
      120,
      12,
      0,
      0,
      SkTextUtils::kLeft_Align,
      Shaper::VAlign::kTopBaseline,
      Shaper::ResizePolicy::kNone,
      Shaper::LinebreakPolicy::kExplicit,
      Shaper::Direction::kLTR,
      Shaper::Capitalization::kNone,
      SkRect::MakeEmpty(),
      SK_ColorTRANSPARENT,
      SK_ColorTRANSPARENT,
      TextPaintOrder::kFillStroke,
      SkPaint::Join::kDefault_Join,
      false,
      false,
      nullptr,
      SkString()
    });
    REPORTER_ASSERT(reporter, texts[0].handle->get() == text_prop);
    text_prop.fLocale = "custom_lc";
    texts[0].handle->set(text_prop);
    REPORTER_ASSERT(reporter, texts[0].handle->get() == text_prop);
}

DEF_TEST(Skottie_Props_Revalidation, reporter) {
    auto test_typeface = ToolUtils::DefaultPortableTypeface();
    REPORTER_ASSERT(reporter, test_typeface);
    auto test_font_manager = sk_make_sp<MockFontMgr>(test_typeface);

    SkMemoryStream stream(gTestJson, strlen(gTestJson));
    auto observer = sk_make_sp<MockPropertyObserver>(true);

    auto animation = skottie::Animation::Builder()
            .setPropertyObserver(observer)
            .setFontManager(test_font_manager)
            .make(&stream);

    REPORTER_ASSERT(reporter, animation);

    SkPictureRecorder recorder;
    // Rendering without seek() should not trigger revalidation asserts.
    animation->render(recorder.beginRecording(100, 100));

    // Mutate all props.
    for (const auto& c : observer->colors()) {
        c.handle->set(SK_ColorGRAY);
    }
    for (const auto& o : observer->opacities()) {
        o.handle->set(0.25f);
    }
    for (const auto& t : observer->texts()) {
        t.handle->set(make_text_prop("bar"));
    }
    for (const auto& t : observer->transforms()) {
        t.handle->set({
            { 1000, 1000 },
            { 2000, 2000 },
            {   20,   20 },
            5,
            0,
            0,
        });
    }

    // Rendering without seek() after property mutation should not trigger revalidation asserts.
    animation->render(recorder.beginRecording(100, 100));
}

