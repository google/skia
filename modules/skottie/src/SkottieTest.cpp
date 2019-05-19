/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/text/SkottieShaper.h"

#include "tests/Test.h"

#include <cmath>
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
    static constexpr char json[] = R"({
                                     "v": "5.2.1",
                                     "w": 100,
                                     "h": 100,
                                     "fr": 1,
                                     "ip": 0,
                                     "op": 1,
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
                                       }
                                     ]
                                   })";

    class TestPropertyObserver final : public PropertyObserver {
    public:
        struct ColorInfo {
            SkString node_name;
            SkColor  color;
        };

        struct OpacityInfo {
            SkString node_name;
            float    opacity;
        };

        struct TransformInfo {
            SkString                        node_name;
            skottie::TransformPropertyValue transform;
        };

        void onColorProperty(const char node_name[],
                const PropertyObserver::LazyHandle<ColorPropertyHandle>& lh) override {
            fColors.push_back({SkString(node_name), lh()->get()});
        }

        void onOpacityProperty(const char node_name[],
                const PropertyObserver::LazyHandle<OpacityPropertyHandle>& lh) override {
            fOpacities.push_back({SkString(node_name), lh()->get()});
        }

        void onTransformProperty(const char node_name[],
                const PropertyObserver::LazyHandle<TransformPropertyHandle>& lh) override {
            fTransforms.push_back({SkString(node_name), lh()->get()});
        }

        const std::vector<ColorInfo>& colors() const { return fColors; }
        const std::vector<OpacityInfo>& opacities() const { return fOpacities; }
        const std::vector<TransformInfo>& transforms() const { return fTransforms; }

    private:
        std::vector<ColorInfo>     fColors;
        std::vector<OpacityInfo>   fOpacities;
        std::vector<TransformInfo> fTransforms;
    };

    SkMemoryStream stream(json, strlen(json));
    auto observer = sk_make_sp<TestPropertyObserver>();

    auto animation = skottie::Animation::Builder()
            .setPropertyObserver(observer)
            .make(&stream);

    REPORTER_ASSERT(reporter, animation);

    const auto& colors = observer->colors();
    REPORTER_ASSERT(reporter, colors.size() == 1);
    REPORTER_ASSERT(reporter, colors[0].node_name.equals("fill_0"));
    REPORTER_ASSERT(reporter, colors[0].color == 0xffff0000);

    const auto& opacities = observer->opacities();
    REPORTER_ASSERT(reporter, opacities.size() == 2);
    REPORTER_ASSERT(reporter, opacities[0].node_name.equals("shape_transform_0"));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(opacities[0].opacity, 100));
    REPORTER_ASSERT(reporter, opacities[1].node_name.equals("layer_0"));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(opacities[1].opacity, 50));

    const auto& transforms = observer->transforms();
    REPORTER_ASSERT(reporter, transforms.size() == 2);
    REPORTER_ASSERT(reporter, transforms[0].node_name.equals("shape_transform_0"));
    REPORTER_ASSERT(reporter, transforms[0].transform == skottie::TransformPropertyValue({
        SkPoint::Make(0, 0),
        SkPoint::Make(0, 0),
        SkVector::Make(50, 50),
        0,
        0,
        0
    }));
    REPORTER_ASSERT(reporter, transforms[1].node_name.equals("layer_0"));
    REPORTER_ASSERT(reporter, transforms[1].transform == skottie::TransformPropertyValue({
        SkPoint::Make(0, 0),
        SkPoint::Make(0, 0),
        SkVector::Make(100, 100),
        0,
        0,
        0
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

    REPORTER_ASSERT(reporter, observer->fMarkers.size() == 2ul);
    REPORTER_ASSERT(reporter, std::get<0>(observer->fMarkers[0]) == "marker_1");
    REPORTER_ASSERT(reporter, std::get<1>(observer->fMarkers[0]) == 0.25f);
    REPORTER_ASSERT(reporter, std::get<2>(observer->fMarkers[0]) == 0.50f);
    REPORTER_ASSERT(reporter, std::get<0>(observer->fMarkers[1]) == "marker_2");
    REPORTER_ASSERT(reporter, std::get<1>(observer->fMarkers[1]) == 0.75f);
    REPORTER_ASSERT(reporter, std::get<2>(observer->fMarkers[1]) == 0.75f);
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
                tsize.text_size,
                talign.align,
                skottie::Shaper::VAlign::kTopBaseline,
            };

            const auto shape_result = skottie::Shaper::Shape(text, desc, text_point);
            REPORTER_ASSERT(reporter, shape_result.fBlob);

            const auto shape_bounds = shape_result.computeBounds();
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
        { 10, 2.0f },
        { 15, 2.4f },
        { 25, 4.4f },
    };

    struct {
        skottie::Shaper::VAlign align;
        SkScalar                topFactor;
    } kTestAligns[] = {
        { skottie::Shaper::VAlign::kTop   , 0.0f },
        { skottie::Shaper::VAlign::kCenter, 0.5f },
        // TODO: any way to test kTopBaseline?
    };

    const SkString text("Foo, bar.\rBaz.");
    const auto text_box = SkRect::MakeXYWH(100, 100, 1000, 1000); // large-enough to avoid breaks.


    for (const auto& tsize : kTestSizes) {
        for (const auto& talign : kTestAligns) {
            const skottie::Shaper::TextDesc desc = {
                typeface,
                tsize.text_size,
                tsize.text_size,
                SkTextUtils::Align::kCenter_Align,
                talign.align,
            };

            const auto shape_result = skottie::Shaper::Shape(text, desc, text_box);
            REPORTER_ASSERT(reporter, shape_result.fBlob);

            const auto shape_bounds = shape_result.computeBounds();
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
