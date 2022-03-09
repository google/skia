/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/text/SkottieShaper.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkTextBlobPriv.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <cmath>
#include <string>
#include <tuple>
#include <vector>

using namespace skottie;

DEF_TEST(Skottie_OssFuzz8956, reporter) {
    static constexpr char json[] =
        "{\"v\":\" \",\"fr\":3,\"w\":4,\"h\":3,\"layers\":[{\"ty\": 1, \"sw\": 10, \"sh\": 10,"
            " \"sc\":\"#ffffff\", \"ks\":{\"o\":{\"a\": true, \"k\":"
            " [{\"t\": 0, \"s\": 0, \"e\": 1, \"i\": {\"x\":[]}}]}}}]}";

    SkMemoryStream stream(json, strlen(json));

    // Passes if parsing doesn't crash.
    auto animation = Animation::Make(&stream);
}

DEF_TEST(Skottie_Properties, reporter) {
    auto test_typeface = ToolUtils::create_portable_typeface();
    REPORTER_ASSERT(reporter, test_typeface);

    static const char json[] = R"({
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


    class TestPropertyObserver final : public PropertyObserver {
    public:
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
            fColors.push_back({SkString(node_name), lh()});
            fColorsWithFullKeypath.push_back({SkString(fCurrentNode.c_str()), lh()});
        }

        void onOpacityProperty(const char node_name[],
                const PropertyObserver::LazyHandle<OpacityPropertyHandle>& lh) override {
            fOpacities.push_back({SkString(node_name), lh()});
        }

        void onTextProperty(const char node_name[],
                            const PropertyObserver::LazyHandle<TextPropertyHandle>& lh) override {
            fTexts.push_back({SkString(node_name), lh()});
        }

        void onTransformProperty(const char node_name[],
                const PropertyObserver::LazyHandle<TransformPropertyHandle>& lh) override {
            fTransforms.push_back({SkString(node_name), lh()});
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
        std::vector<ColorInfo>     fColors;
        std::vector<OpacityInfo>   fOpacities;
        std::vector<TextInfo>      fTexts;
        std::vector<TransformInfo> fTransforms;
        std::string                fCurrentNode;
        std::vector<ColorInfo>     fColorsWithFullKeypath;
    };

    // Returns a single specified typeface for all requests.
    class FakeFontMgr : public SkFontMgr {
     public:
        FakeFontMgr(sk_sp<SkTypeface> test_font) : fTestFont(test_font) {}

        int onCountFamilies() const override { return 1; }
        void onGetFamilyName(int index, SkString* familyName) const override {}
        SkFontStyleSet* onCreateStyleSet(int index) const override { return nullptr; }
        SkFontStyleSet* onMatchFamily(const char familyName[]) const override { return nullptr; }
        SkTypeface* onMatchFamilyStyle(const char familyName[],
                                      const SkFontStyle& fontStyle) const override {
            return nullptr;
        }
        SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
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

    sk_sp<FakeFontMgr> test_font_manager = sk_make_sp<FakeFontMgr>(test_typeface);
    SkMemoryStream stream(json, strlen(json));
    auto observer = sk_make_sp<TestPropertyObserver>();

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
    REPORTER_ASSERT(reporter, texts[0].handle->get() == skottie::TextPropertyValue({
      test_typeface,
      SkString("inline_text"),
      100,
      0, 100,
      0,
      120,
      12,
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
      false,
      false
    }));
}

DEF_TEST(Skottie_Annotations, reporter) {
    static constexpr char json[] = R"({
                                     "v": "5.2.1",
                                     "w": 100,
                                     "h": 100,
                                     "fr": 10,
                                     "ip": 0,
                                     "op": 100,
                                     "layers": [
                                       {
                                         "ty": 1,
                                         "ind": 0,
                                         "ip": 0,
                                         "op": 1,
                                         "ks": {
                                           "o": { "a": 0, "k": 50 }
                                         },
                                         "sw": 100,
                                         "sh": 100,
                                         "sc": "#ffffff"
                                       }
                                     ],
                                     "markers": [
                                       {
                                           "cm": "marker_1",
                                           "dr": 25,
                                           "tm": 25
                                       },
                                       {
                                           "cm": "marker_2",
                                           "dr": 0,
                                           "tm": 75
                                       }
                                     ]
                                   })";

    class TestMarkerObserver final : public MarkerObserver {
    public:
        void onMarker(const char name[], float t0, float t1) override {
            fMarkers.push_back(std::make_tuple(name, t0, t1));
        }

        std::vector<std::tuple<std::string, float, float>> fMarkers;
    };

    SkMemoryStream stream(json, strlen(json));
    auto observer = sk_make_sp<TestMarkerObserver>();

    auto animation = skottie::Animation::Builder()
            .setMarkerObserver(observer)
            .make(&stream);

    REPORTER_ASSERT(reporter, animation);
    REPORTER_ASSERT(reporter, animation->duration() == 10);
    REPORTER_ASSERT(reporter, animation->inPoint()  == 0.0);
    REPORTER_ASSERT(reporter, animation->outPoint() == 100.0);

    REPORTER_ASSERT(reporter, observer->fMarkers.size() == 2ul);
    REPORTER_ASSERT(reporter, std::get<0>(observer->fMarkers[0]) == "marker_1");
    REPORTER_ASSERT(reporter, std::get<1>(observer->fMarkers[0]) == 0.25f);
    REPORTER_ASSERT(reporter, std::get<2>(observer->fMarkers[0]) == 0.50f);
    REPORTER_ASSERT(reporter, std::get<0>(observer->fMarkers[1]) == "marker_2");
    REPORTER_ASSERT(reporter, std::get<1>(observer->fMarkers[1]) == 0.75f);
    REPORTER_ASSERT(reporter, std::get<2>(observer->fMarkers[1]) == 0.75f);
}

static SkRect ComputeBlobBounds(const sk_sp<SkTextBlob>& blob) {
    auto bounds = SkRect::MakeEmpty();

    if (!blob) {
        return bounds;
    }

    SkAutoSTArray<16, SkRect> glyphBounds;

    for (SkTextBlobRunIterator it(blob.get()); !it.done(); it.next()) {
        glyphBounds.reset(SkToInt(it.glyphCount()));
        it.font().getBounds(it.glyphs(), it.glyphCount(), glyphBounds.get(), nullptr);

        SkASSERT(it.positioning() == SkTextBlobRunIterator::kFull_Positioning);
        for (uint32_t i = 0; i < it.glyphCount(); ++i) {
            bounds.join(glyphBounds[i].makeOffset(it.pos()[i * 2    ],
                                                  it.pos()[i * 2 + 1]));
        }
    }

    return bounds;
}

static SkRect ComputeShapeResultBounds(const skottie::Shaper::Result& res) {
    auto bounds = SkRect::MakeEmpty();

    for (const auto& fragment : res.fFragments) {
        bounds.join(ComputeBlobBounds(fragment.fBlob).makeOffset(fragment.fPos.x(),
                                                                 fragment.fPos.y()));
    }

    return bounds;
}

DEF_TEST(Skottie_Shaper_HAlign, reporter) {
    auto typeface = SkTypeface::MakeDefault();
    REPORTER_ASSERT(reporter, typeface);

    static constexpr struct {
        SkScalar text_size,
                 tolerance;
    } kTestSizes[] = {
        // These gross tolerances are required for the test to pass on NativeFonts bots.
        // Might be worth investigating why we need so much slack.
        {  5, 2.0f },
        { 10, 2.0f },
        { 15, 2.4f },
        { 25, 4.4f },
    };

    static constexpr struct {
        SkTextUtils::Align align;
        SkScalar           l_selector,
                           r_selector;
    } kTestAligns[] = {
        { SkTextUtils::  kLeft_Align, 0.0f, 1.0f },
        { SkTextUtils::kCenter_Align, 0.5f, 0.5f },
        { SkTextUtils:: kRight_Align, 1.0f, 0.0f },
    };

    const SkString text("Foo, bar.\rBaz.");
    const SkPoint  text_point = SkPoint::Make(100, 100);

    for (const auto& tsize : kTestSizes) {
        for (const auto& talign : kTestAligns) {
            const skottie::Shaper::TextDesc desc = {
                typeface,
                tsize.text_size,
                0, tsize.text_size,
                tsize.text_size,
                0,
                0,
                talign.align,
                Shaper::VAlign::kTopBaseline,
                Shaper::ResizePolicy::kNone,
                Shaper::LinebreakPolicy::kExplicit,
                Shaper::Direction::kLTR,
                Shaper::Capitalization::kNone,
                Shaper::Flags::kNone
            };

            const auto shape_result = Shaper::Shape(text, desc, text_point,
                                                    SkFontMgr::RefDefault());
            REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
            REPORTER_ASSERT(reporter, shape_result.fFragments[0].fBlob);

            const auto shape_bounds = ComputeShapeResultBounds(shape_result);
            REPORTER_ASSERT(reporter, !shape_bounds.isEmpty());

            const auto expected_l = text_point.x() - shape_bounds.width() * talign.l_selector;
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.left() - expected_l) < tsize.tolerance,
                            "%f %f %f %f %d", shape_bounds.left(), expected_l, tsize.tolerance,
                                              tsize.text_size, talign.align);

            const auto expected_r = text_point.x() + shape_bounds.width() * talign.r_selector;
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.right() - expected_r) < tsize.tolerance,
                            "%f %f %f %f %d", shape_bounds.right(), expected_r, tsize.tolerance,
                                              tsize.text_size, talign.align);

        }
    }
}

DEF_TEST(Skottie_Shaper_VAlign, reporter) {
    auto typeface = SkTypeface::MakeDefault();
    REPORTER_ASSERT(reporter, typeface);

    static constexpr struct {
        SkScalar text_size,
                 tolerance;
    } kTestSizes[] = {
        // These gross tolerances are required for the test to pass on NativeFonts bots.
        // Might be worth investigating why we need so much slack.
        {  5, 2.0f },
        { 10, 4.0f },
        { 15, 5.5f },
        { 25, 8.0f },
    };

    struct {
        skottie::Shaper::VAlign align;
        SkScalar                topFactor;
    } kTestAligns[] = {
        { skottie::Shaper::VAlign::kVisualTop   , 0.0f },
        { skottie::Shaper::VAlign::kVisualCenter, 0.5f },
        // TODO: any way to test kTopBaseline?
    };

    const SkString text("Foo, bar.\rBaz.");
    const auto text_box = SkRect::MakeXYWH(100, 100, 1000, 1000); // large-enough to avoid breaks.


    for (const auto& tsize : kTestSizes) {
        for (const auto& talign : kTestAligns) {
            const skottie::Shaper::TextDesc desc = {
                typeface,
                tsize.text_size,
                0, tsize.text_size,
                tsize.text_size,
                0,
                0,
                SkTextUtils::Align::kCenter_Align,
                talign.align,
                Shaper::ResizePolicy::kNone,
                Shaper::LinebreakPolicy::kParagraph,
                Shaper::Direction::kLTR,
                Shaper::Capitalization::kNone,
                Shaper::Flags::kNone
            };

            const auto shape_result = Shaper::Shape(text, desc, text_box, SkFontMgr::RefDefault());
            REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
            REPORTER_ASSERT(reporter, shape_result.fFragments[0].fBlob);

            const auto shape_bounds = ComputeShapeResultBounds(shape_result);
            REPORTER_ASSERT(reporter, !shape_bounds.isEmpty());

            const auto v_diff = text_box.height() - shape_bounds.height();

            const auto expected_t = text_box.top() + v_diff * talign.topFactor;
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.top() - expected_t) < tsize.tolerance,
                            "%f %f %f %f %d", shape_bounds.top(), expected_t, tsize.tolerance,
                                              tsize.text_size, SkToU32(talign.align));

            const auto expected_b = text_box.bottom() - v_diff * (1 - talign.topFactor);
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.bottom() - expected_b) < tsize.tolerance,
                            "%f %f %f %f %d", shape_bounds.bottom(), expected_b, tsize.tolerance,
                                              tsize.text_size, SkToU32(talign.align));
        }
    }
}

DEF_TEST(Skottie_Shaper_FragmentGlyphs, reporter) {
    skottie::Shaper::TextDesc desc = {
        SkTypeface::MakeDefault(),
        18,
        0, 18,
        18,
         0,
         0,
        SkTextUtils::Align::kCenter_Align,
        Shaper::VAlign::kTop,
        Shaper::ResizePolicy::kNone,
        Shaper::LinebreakPolicy::kParagraph,
        Shaper::Direction::kLTR,
        Shaper::Capitalization::kNone,
        Shaper::Flags::kNone
    };

    const SkString text("Foo bar baz");
    const auto text_box = SkRect::MakeWH(100, 100);

    {
        const auto shape_result = Shaper::Shape(text, desc, text_box, SkFontMgr::RefDefault());
        // Default/consolidated mode => single blob result.
        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
        REPORTER_ASSERT(reporter, shape_result.fFragments[0].fBlob);
    }

    {
        desc.fFlags = Shaper::Flags::kFragmentGlyphs;
        const auto shape_result = skottie::Shaper::Shape(text, desc, text_box,
                                                         SkFontMgr::RefDefault());
        // Fragmented mode => one blob per glyph.
        const size_t expectedSize = text.size();
        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == expectedSize);
        for (size_t i = 0; i < expectedSize; ++i) {
            REPORTER_ASSERT(reporter, shape_result.fFragments[i].fBlob);
        }
    }
}

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && !defined(SK_BUILD_FOR_WIN)

DEF_TEST(Skottie_Shaper_ExplicitFontMgr, reporter) {
    class CountingFontMgr : public SkFontMgr {
    public:
        size_t fallbackCount() const { return fFallbackCount; }

    protected:
        int onCountFamilies() const override { return 0; }
        void onGetFamilyName(int index, SkString* familyName) const override {
            SkDEBUGFAIL("onGetFamilyName called with bad index");
        }
        SkFontStyleSet* onCreateStyleSet(int index) const override {
            SkDEBUGFAIL("onCreateStyleSet called with bad index");
            return nullptr;
        }
        SkFontStyleSet* onMatchFamily(const char[]) const override {
            return SkFontStyleSet::CreateEmpty();
        }

        SkTypeface* onMatchFamilyStyle(const char[], const SkFontStyle&) const override {
            return nullptr;
        }
        SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                const SkFontStyle& style,
                                                const char* bcp47[],
                                                int bcp47Count,
                                                SkUnichar character) const override {
            fFallbackCount++;
            return nullptr;
        }

        sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                               const SkFontArguments&) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMakeFromFile(const char[], int) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onLegacyMakeTypeface(const char [], SkFontStyle) const override {
            return nullptr;
        }
    private:
        mutable size_t fFallbackCount = 0;
    };

    auto fontmgr = sk_make_sp<CountingFontMgr>();

    skottie::Shaper::TextDesc desc = {
        ToolUtils::create_portable_typeface(),
        18,
        0, 18,
        18,
         0,
         0,
        SkTextUtils::Align::kCenter_Align,
        Shaper::VAlign::kTop,
        Shaper::ResizePolicy::kNone,
        Shaper::LinebreakPolicy::kParagraph,
        Shaper::Direction::kLTR,
        Shaper::Capitalization::kNone,
        Shaper::Flags::kNone
    };

    const auto text_box = SkRect::MakeWH(100, 100);

    {
        const auto shape_result = Shaper::Shape(SkString("foo bar"), desc, text_box, fontmgr);

        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
        REPORTER_ASSERT(reporter, shape_result.fFragments[0].fBlob);
        REPORTER_ASSERT(reporter, fontmgr->fallbackCount() == 0ul);
        REPORTER_ASSERT(reporter, shape_result.fMissingGlyphCount == 0);
    }

    {
        // An unassigned codepoint should trigger fallback.
        const auto shape_result = skottie::Shaper::Shape(SkString("foo\U000DFFFFbar"),
                                                         desc, text_box, fontmgr);

        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
        REPORTER_ASSERT(reporter, shape_result.fFragments[0].fBlob);
        REPORTER_ASSERT(reporter, fontmgr->fallbackCount() == 1ul);
        REPORTER_ASSERT(reporter, shape_result.fMissingGlyphCount == 1ul);
    }
}

#endif

DEF_TEST(Skottie_Image_Loading, reporter) {
    class TestResourceProvider final : public skresources::ResourceProvider {
    public:
        TestResourceProvider(sk_sp<skresources::ImageAsset> single_asset,
                             sk_sp<skresources::ImageAsset>  multi_asset)
            : fSingleFrameAsset(std::move(single_asset))
            , fMultiFrameAsset (std::move( multi_asset)) {}

    private:
        sk_sp<ImageAsset> loadImageAsset(const char path[],
                                         const char name[],
                                         const char id[]) const override {
            return strcmp(id, "single_frame")
                    ? fMultiFrameAsset
                    : fSingleFrameAsset;
        }

        const sk_sp<skresources::ImageAsset> fSingleFrameAsset,
                                             fMultiFrameAsset;
    };

    auto make_animation = [&reporter] (sk_sp<skresources::ImageAsset> single_asset,
                                       sk_sp<skresources::ImageAsset>  multi_asset,
                                       bool deferred_image_loading) {
        static constexpr char json[] = R"({
                                         "v": "5.2.1",
                                         "w": 100,
                                         "h": 100,
                                         "fr": 10,
                                         "ip": 0,
                                         "op": 100,
                                         "assets": [
                                           {
                                             "id": "single_frame",
                                             "p" : "single_frame.png",
                                             "u" : "images/",
                                             "w" : 500,
                                             "h" : 500
                                           },
                                           {
                                             "id": "multi_frame",
                                             "p" : "multi_frame.png",
                                             "u" : "images/",
                                             "w" : 500,
                                             "h" : 500
                                           }
                                         ],
                                         "layers": [
                                           {
                                             "ty": 2,
                                             "refId": "single_frame",
                                             "ind": 0,
                                             "ip": 0,
                                             "op": 100,
                                             "ks": {}
                                           },
                                           {
                                             "ty": 2,
                                             "refId": "multi_frame",
                                             "ind": 1,
                                             "ip": 0,
                                             "op": 100,
                                             "ks": {}
                                           }
                                         ]
                                       })";

        SkMemoryStream stream(json, strlen(json));

        const auto flags = deferred_image_loading
            ? static_cast<uint32_t>(skottie::Animation::Builder::kDeferImageLoading)
            : 0;
        auto animation =
            skottie::Animation::Builder(flags)
                .setResourceProvider(sk_make_sp<TestResourceProvider>(std::move(single_asset),
                                                                      std::move( multi_asset)))
                .make(&stream);

        REPORTER_ASSERT(reporter, animation);

        return  animation;
    };

    class TestAsset final : public skresources::ImageAsset {
    public:
        explicit TestAsset(bool multi_frame) : fMultiFrame(multi_frame) {}

        const std::vector<float>& requestedFrames() const { return fRequestedFrames; }

    private:
        bool isMultiFrame() override { return fMultiFrame; }

        sk_sp<SkImage> getFrame(float t) override {
            fRequestedFrames.push_back(t);

            return SkSurface::MakeRasterN32Premul(10, 10)->makeImageSnapshot();
        }

        const bool fMultiFrame;

        std::vector<float> fRequestedFrames;
    };

    {
        auto single_asset = sk_make_sp<TestAsset>(false),
              multi_asset = sk_make_sp<TestAsset>(true);

        // Default image loading: single-frame images are loaded upfront, multi-frame images are
        // loaded on-demand.
        auto animation = make_animation(single_asset, multi_asset, false);

        REPORTER_ASSERT(reporter, single_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter,  multi_asset->requestedFrames().size() == 0);
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(single_asset->requestedFrames()[0]));

        animation->seekFrameTime(1);
        REPORTER_ASSERT(reporter, single_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter,  multi_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(multi_asset->requestedFrames()[0], 1));

        animation->seekFrameTime(2);
        REPORTER_ASSERT(reporter, single_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter,  multi_asset->requestedFrames().size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(multi_asset->requestedFrames()[1], 2));
    }

    {
        auto single_asset = sk_make_sp<TestAsset>(false),
              multi_asset = sk_make_sp<TestAsset>(true);

        // Deferred image loading: both single-frame and multi-frame images are loaded on-demand.
        auto animation = make_animation(single_asset, multi_asset, true);

        REPORTER_ASSERT(reporter, single_asset->requestedFrames().size() == 0);
        REPORTER_ASSERT(reporter,  multi_asset->requestedFrames().size() == 0);

        animation->seekFrameTime(1);
        REPORTER_ASSERT(reporter, single_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter,  multi_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(single_asset->requestedFrames()[0], 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual (multi_asset->requestedFrames()[0], 1));

        animation->seekFrameTime(2);
        REPORTER_ASSERT(reporter, single_asset->requestedFrames().size() == 1);
        REPORTER_ASSERT(reporter,  multi_asset->requestedFrames().size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(multi_asset->requestedFrames()[1], 2));
    }
}

DEF_TEST(Skottie_Layer_NoType, r) {
    static constexpr char json[] =
        R"({
             "v": "5.2.1",
             "w": 100,
             "h": 100,
             "fr": 10,
             "ip": 0,
             "op": 100,
             "layers": [
               {
                 "ind": 0,
                 "ip": 0,
                 "op": 100,
                 "ks": {}
               }
             ]
           })";

    SkMemoryStream stream(json, strlen(json));
    auto anim = Animation::Make(&stream);

    // passes if we don't crash
    REPORTER_ASSERT(r, anim);
}
