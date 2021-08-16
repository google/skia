/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "tests/Test.h"
#include <iostream>

using namespace skottie;

namespace {

class FakeScalarExpressionEvaluator : public ExpressionEvaluator<float> {
public:
    float evaluate(float t) override {
        return 7.0f;
    }
};

class FakeVectorExpressionEvaluator : public ExpressionEvaluator<std::vector<float>> {
public:
    std::vector<float> evaluate(float t) override {
        return {1.0f, 2.0f};
    }
};

class FakeExpressionManager : public ExpressionManager {
public:
    sk_sp<ExpressionEvaluator<float>> createNumberExpressionEvaluator(
        const char expression[]) override {
        return sk_make_sp<FakeScalarExpressionEvaluator>();
    }

    sk_sp<ExpressionEvaluator<SkString>> createStringExpressionEvaluator(
        const char expression[]) override {
        return nullptr;
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
            opacity_handle_.reset(opacity_handle().release());
        }

        void onTransformProperty(const char node_name[],
                                 const LazyHandle<TransformPropertyHandle>& transform_handle) override {
            transform_handle_.reset(transform_handle().release());
        }

    std::unique_ptr<OpacityPropertyHandle> opacity_handle_;
    std::unique_ptr<TransformPropertyHandle> transform_handle_;
};
} // namespace

DEF_TEST(Skottie_ScalarExpression, r) {
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
                "ao": 0,
                "sw": 100,
                "sh": 100,
                "sc": "#51251d",
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
        .setPropertyObserver(observer)
        .make(&stream);

    REPORTER_ASSERT(r, anim);

    anim->seekFrameTime(0);

    REPORTER_ASSERT(r, SkScalarNearlyEqual(observer->opacity_handle_->get(), 7.0f));
    SkPoint anchor_point = observer->transform_handle_->get().fAnchorPoint;
    REPORTER_ASSERT(r, SkScalarNearlyEqual(anchor_point.fX, 1.0f));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(anchor_point.fY, 2.0f));
}
