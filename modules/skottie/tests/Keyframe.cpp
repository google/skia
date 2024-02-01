/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/ExternalLayer.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "src/utils/SkJSON.h"
#include "tests/Test.h"

#include <cmath>

using namespace skottie;
using namespace skottie::internal;

namespace  {

template <typename T>
class MockProperty final : public AnimatablePropertyContainer {
public:
    explicit MockProperty(const char* jprop) {
        AnimationBuilder abuilder(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                  {100, 100}, 10, 1, 0);
        skjson::DOM json_dom(jprop, strlen(jprop));

        fDidBind = this->bind(abuilder, json_dom.root(), &fValue);
    }

    explicit operator bool() const { return fDidBind; }

    const T& operator()(float t) { this->seek(t); return fValue; }

private:
    void onSync() override {}

    T     fValue = T();
    bool  fDidBind;
};

}  // namespace

DEF_TEST(Skottie_Keyframe, reporter) {
    {
        MockProperty<ScalarValue> prop(R"({})");
        REPORTER_ASSERT(reporter, !prop);
    }
    {
        MockProperty<ScalarValue> prop(R"({ "a": 1, "k": [] })");
        REPORTER_ASSERT(reporter, !prop);
    }
    {
        // New style
        MockProperty<ScalarValue> prop(R"({
                                         "a": 1,
                                         "k": [
                                           { "t":  1, "s": 1 },
                                           { "t":  2, "s": 2 },
                                           { "t":  3, "s": 4 }
                                         ]
                                       })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, !prop.isStatic());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( -1), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(  0), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(  1), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(1.5), 1.5f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(  2), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(2.5), 3));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(  3), 4));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(  4), 4));
    }
    {
        // New style hold (hard stops)
        MockProperty<ScalarValue> prop(R"({
                                         "a": 1,
                                         "k": [
                                           { "t":  1, "s": 1, "h": true },
                                           { "t":  2, "s": 2, "h": true },
                                           { "t":  3, "s": 4, "h": true }
                                         ]
                                       })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, !prop.isStatic());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(0  ), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(1  ), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(1.5), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(std::nextafter(2.f, 0.f)), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(2  ), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(2.5), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(std::nextafter(3.f, 0.f)), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(3  ), 4));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(4  ), 4));
    }
    {
        // Legacy style
        MockProperty<ScalarValue> prop(R"({
                                         "a": 1,
                                         "k": [
                                           { "t":  1, "s": 1, "e": 2 },
                                           { "t":  2, "s": 2, "e": 4 },
                                           { "t":  3 }
                                         ]
                                       })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, !prop.isStatic());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(-1), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 0), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 1  ), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 1.5), 1.5f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 2  ), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 2.5), 3));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 3  ), 4));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop( 4  ), 4));
    }
    {
        // Legacy style hold (hard stops)
        MockProperty<ScalarValue> prop(R"({
                                         "a": 1,
                                         "k": [
                                           { "t":  1, "s": 1, "e": 2, "h": true },
                                           { "t":  2, "s": 2, "e": 4, "h": true },
                                           { "t":  3 }
                                         ]
                                       })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, !prop.isStatic());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(0  ), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(1  ), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(1.5), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(std::nextafter(2.f, 0.f)), 1));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(2  ), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(2.5), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(std::nextafter(3.f, 0.f)), 2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(3  ), 4));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(4  ), 4));
    }
    {
        // Static scalar prop (all equal keyframes, using float kf Value)
        MockProperty<ScalarValue> prop(R"({
                                         "a": 1,
                                         "k": [
                                           { "t":  1, "s": 42, "e": 42 },
                                           { "t":  2, "s": 42, "e": 42 },
                                           { "t":  3 }
                                         ]
                                       })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, prop.isStatic());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(0), 42));
    }
    {
        // Static vector prop (all equal keyframes, using uint32 kf Value)
        MockProperty<Vec2Value> prop(R"({
                                       "a": 1,
                                       "k": [
                                         { "t":  1, "s": [4,2], "e": [4,2] },
                                         { "t":  2, "s": [4,2], "e": [4,2] },
                                         { "t":  3 }
                                       ]
                                     })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, prop.isStatic());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(0).x, 4));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prop(0).y, 2));
    }
    {
        // Spatial interpolation [100,100]->[200,200], with supernormal easing:
        // https://cubic-bezier.com/#.5,-0.5,.5,1.5
        MockProperty<Vec2Value> prop(R"({
                                       "a": 1,
                                       "k": [
                                         { "t": 0, "s": [100,100],
                                           "o":{"x":[0.5], "y":[-0.5]}, "i":{"x":[0.5], "y":[1.5]},
                                          "to": [10,15], "ti": [-10,-5]
                                         },
                                         { "t": 1, "s": [200,200]
                                         }
                                       ]
                                     })");
        REPORTER_ASSERT(reporter, prop);
        REPORTER_ASSERT(reporter, !prop.isStatic());

        // Not linear.
        REPORTER_ASSERT(reporter, !SkScalarNearlyEqual(prop(0.5f).x, 150.f));
        REPORTER_ASSERT(reporter, !SkScalarNearlyEqual(prop(0.5f).y, 150.f));

        // Subnormal region triggers extrapolation.
        REPORTER_ASSERT(reporter, prop(0.15f).x < 100);
        REPORTER_ASSERT(reporter, prop(0.15f).y < 100);

        // Supernormal region triggers extrapolation.
        REPORTER_ASSERT(reporter, prop(0.85f).x > 200);
        REPORTER_ASSERT(reporter, prop(0.85f).y > 200);
    }
}
