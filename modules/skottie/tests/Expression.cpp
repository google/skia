/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

using namespace skottie;

namespace {

class FakeScalarExpressionEvaluator : public ExpressionEvaluator<float> {
public:
    float evaluate(float t) override { return 7.0f; }
};

class FakeVectorExpressionEvaluator : public ExpressionEvaluator<std::vector<float>> {
public:
    std::vector<float> evaluate(float t) override { return {0.1f, 0.2f, 0.3f, 1.0f}; }
};

class FakeStringExpressionEvaluator : public ExpressionEvaluator<SkString> {
public:
    SkString evaluate(float t) override { return SkString("Hello, world!"); }
};

class FakeExpressionManager : public ExpressionManager {
public:
    sk_sp<ExpressionEvaluator<float>> createNumberExpressionEvaluator(
            const char expression[]) override {
        return sk_make_sp<FakeScalarExpressionEvaluator>();
    }

    sk_sp<ExpressionEvaluator<SkString>> createStringExpressionEvaluator(
            const char expression[]) override {
        return sk_make_sp<FakeStringExpressionEvaluator>();
    }

    sk_sp<ExpressionEvaluator<std::vector<float>>> createArrayExpressionEvaluator(
            const char expression[]) override {
        return sk_make_sp<FakeVectorExpressionEvaluator>();
    }
};

class FakePropertyObserver : public PropertyObserver {
public:
    void onOpacityProperty(const char node_name[],
                           const LazyHandle<OpacityPropertyHandle>& opacity_handle) override {
        opacity_handle_ = opacity_handle();
    }

    void onTransformProperty(const char node_name[],
                             const LazyHandle<TransformPropertyHandle>& transform_handle) override {
        transform_handle_ = transform_handle();
    }

    void onColorProperty(const char node_name[],
                         const LazyHandle<ColorPropertyHandle>& color_handle) override {
        color_handle_ = color_handle();
    }

    void onTextProperty(const char node_name[],
                        const LazyHandle<TextPropertyHandle>& text_handle) override {
        text_handle_ = text_handle();
    }

    std::unique_ptr<OpacityPropertyHandle> opacity_handle_;
    std::unique_ptr<TransformPropertyHandle> transform_handle_;
    std::unique_ptr<ColorPropertyHandle> color_handle_;
    std::unique_ptr<TextPropertyHandle> text_handle_;
};
}  // namespace

DEF_TEST(Skottie_Expression, r) {
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
                 "ip": 0,
                 "op": 100,
                 "ty": 1,
                 "nm": "My Layer",
                 "sr": 1,
                 "ks": {
                   "o": {
                     "a": 0,
                     "k": 100,
                     "ix": 11,
                     "x": "fake; return value is specified by the FakeScalarExpressionEvaluator."
                   },
                    "r": {
                        "a": 0,
                        "k": 0,
                        "ix": 10
                    },
                    "p": {
                        "a": 0,
                        "k": [
                            50,
                            50,
                            0
                        ],
                        "ix": 2,
                        "l": 2
                    },
                    "a": {
                        "a": 0,
                        "k": [
                            50,
                            50,
                            0
                        ],
                        "ix": 1,
                        "l": 2,
                        "x": "fake; return value is specified by the FakeArrayExpressionEvaluator."
                    },
                    "s": {
                        "a": 0,
                        "k": [
                            100,
                            100,
                            100
                        ],
                        "ix": 6,
                        "l": 2
                    }
                },
                "ef": [
                {
                    "ty": 21,
                    "nm": "Fill",
                    "np": 9,
                    "mn": "ADBE Fill",
                    "ix": 1,
                    "en": 1,
                    "ef": [
                        {
                            "ty": 10,
                            "nm": "Fill Mask",
                            "mn": "ADBE Fill-0001",
                            "ix": 1,
                            "v": {
                                "a": 0,
                                "k": 0,
                                "ix": 1
                            }
                        },
                        {
                            "ty": 7,
                            "nm": "All Masks",
                            "mn": "ADBE Fill-0007",
                            "ix": 2,
                            "v": {
                                "a": 0,
                                "k": 0,
                                "ix": 2
                            }
                        },
                        {
                            "ty": 2,
                            "nm": "Color",
                            "mn": "ADBE Fill-0002",
                            "ix": 3,
                            "v": {
                                "a": 0,
                                "k": [
                                    1,
                                    0,
                                    0,
                                    1
                                ],
                                "ix": 3,
                                "x": "fake; return value is specified by the FakeArrayExpressionEvaluator."
                            }
                        },
                        {
                            "ty": 7,
                            "nm": "Invert",
                            "mn": "ADBE Fill-0006",
                            "ix": 4,
                            "v": {
                                "a": 0,
                                "k": 0,
                                "ix": 4
                            }
                        },
                        {
                            "ty": 0,
                            "nm": "Horizontal Feather",
                            "mn": "ADBE Fill-0003",
                            "ix": 5,
                            "v": {
                                "a": 0,
                                "k": 0,
                                "ix": 5
                            }
                        },
                        {
                            "ty": 0,
                            "nm": "Vertical Feather",
                            "mn": "ADBE Fill-0004",
                            "ix": 6,
                            "v": {
                                "a": 0,
                                "k": 0,
                                "ix": 6
                            }
                        },
                        {
                            "ty": 0,
                            "nm": "Opacity",
                            "mn": "ADBE Fill-0005",
                            "ix": 7,
                            "v": {
                                "a": 0,
                                "k": 1,
                                "ix": 7
                            }
                        }
                    ]
                }
                ],
                "ao": 0,
                "sw": 100,
                "sh": 100,
                "sc": "#000000",
                "st": 0,
                "bm": 0
               }
             ]
           })";

    SkMemoryStream stream(json, strlen(json));

    auto em = sk_make_sp<FakeExpressionManager>();
    auto observer = sk_make_sp<FakePropertyObserver>();

    auto anim = Animation::Builder()
        .setExpressionManager(em)
        .setFontManager(ToolUtils::TestFontMgr())
        .setPropertyObserver(observer)
        .make(&stream);

    REPORTER_ASSERT(r, anim);

    anim->seekFrameTime(0);

    REPORTER_ASSERT(r, SkScalarNearlyEqual(observer->opacity_handle_->get(), 7.0f));
    SkPoint anchor_point = observer->transform_handle_->get().fAnchorPoint;
    REPORTER_ASSERT(r, SkScalarNearlyEqual(anchor_point.fX, 0.1f));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(anchor_point.fY, 0.2f));
    REPORTER_ASSERT(r, (observer->color_handle_->get() == SkColor4f{0.1f, 0.2f, 0.3f, 1.0f}.toSkColor()));
}

DEF_TEST(Skottie_ExpressionText, r) {
    static constexpr char text_json[] =
    R"({
     "layers":[
        {
           "ty":5,
           "ks":{
              "a":{
                 "k":[
                    0,
                    0
                 ]
              },
              "p":{
                 "k":[
                    128,
                    144
                 ]
              },
              "o":{
                 "k":100
              }
           },
           "ind":0,
           "ip":0,
           "op":2,
           "nm":"TextLayer_0",
           "t":{
              "d":{
                 "k":[
                    {
                       "t":0,
                       "s":{
                          "f": "Helvetica",
                          "s":14,
                          "t":"will be replaced.",
                          "j":0,
                          "ps":[
                             0,
                             0
                          ],
                          "sz":[
                             384,
                             360
                          ],
                          "fc":[
                             0.95686274766921997,
                             0.37254902720451355,
                             0.25490197539329529,
                             1
                          ],
                          "lh":16
                       }
                    }
                 ],
                 "x": "fake; return value is specified by the FakeStringExpressionEvaluator."
              }
           }
        }
     ],
     "ip":0,
     "op":2,
     "fr":25,
     "w":1280,
     "h":720,
     "ddd":false,
     "v":"5.2.2",
     "nm":"skottie_animation",
     "fonts":{
        "list":[
           {
              "fName": "Helvetica",
              "fFamily":"external_font_family",
              "fStyle":"Regular"
           }
        ]
     }
  })";

    SkMemoryStream stream(text_json, strlen(text_json));

    auto em = sk_make_sp<FakeExpressionManager>();
    auto observer = sk_make_sp<FakePropertyObserver>();

    auto anim = Animation::Builder()
                        .setExpressionManager(em)
                        .setFontManager(ToolUtils::TestFontMgr())
                        .setPropertyObserver(observer)
                        .make(&stream);

    REPORTER_ASSERT(r, anim);

    anim->seekFrameTime(0);

    REPORTER_ASSERT(r, observer->text_handle_->get().fText == SkString("Hello, world!"));
}
