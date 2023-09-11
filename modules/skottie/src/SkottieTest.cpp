/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "modules/skottie/include/Skottie.h"
#include "tests/Test.h"

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

            return SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10))->makeImageSnapshot();
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
