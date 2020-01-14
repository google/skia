/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/AnimatableProperty.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "src/utils/SkJSON.h"

#include <string.h>

namespace  {

template <typename T>
class TestProperty final {
public:
    TestProperty(skiatest::Reporter* reporter, const char* json)
        : fDom(json, strlen(json))
        , fBuilder(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                   SkSize::MakeEmpty(), 0, 0, 0)
        , fProp(fBuilder, fDom.root()) {}

    const skottie::internal::AnimatableProperty<T>* operator ->() const { return &fProp; }

    const T& operator()(float t) const { return fProp(t); }

private:
    skjson::DOM                              fDom;
    skottie::internal::AnimationBuilder      fBuilder;
    skottie::internal::AnimatableProperty<T> fProp;
};

} // namespace

DEF_TEST(Skottie_ScalarProperty, reporter) {
    {
        // Invalid/default scalar.
        TestProperty<skottie::ScalarValue> prop(reporter, R"({})");
        REPORTER_ASSERT(reporter, prop->isStatic());
        REPORTER_ASSERT(reporter, prop(0) == 0);
    }
    {
        // Static scalar.
        TestProperty<skottie::ScalarValue> prop(reporter, R"({
                                                               "a": 0,
                                                               "k": 42
                                                             })");
        REPORTER_ASSERT(reporter, prop->isStatic());
        REPORTER_ASSERT(reporter, prop(0) == 42);
    }
    {
        // Static old-style scalar.
        TestProperty<skottie::ScalarValue> prop(reporter, R"({
                                                               "k": 42
                                                             })");
        REPORTER_ASSERT(reporter, prop->isStatic());
        REPORTER_ASSERT(reporter, prop(0) == 42);
    }
    {
        // Keyframed constant scalar.
        TestProperty<skottie::ScalarValue> prop(reporter, R"({
                                                               "a": 1,
                                                               "k": [
                                                                 { "t": 10, "s": 42},
                                                                 { "t": 20, "s": 42},
                                                                 { "t": 30 }
                                                               ]
                                                             })");
        REPORTER_ASSERT(reporter, prop->isStatic());
        REPORTER_ASSERT(reporter, prop(0) == 42);
    }
    {
        // Keyframed scalar.
        TestProperty<skottie::ScalarValue> prop(reporter, R"({
                                                               "a": 1,
                                                               "k": [
                                                                 { "t": 10, "s":  50},
                                                                 { "t": 20, "s": 100},
                                                                 { "t": 30, "s": 100},
                                                                 { "t": 40, "s":  75},
                                                                 { "t": 50}
                                                               ]
                                                             })");
        REPORTER_ASSERT(reporter, !prop->isStatic());
        REPORTER_ASSERT(reporter, prop(-10) ==  50);
        REPORTER_ASSERT(reporter, prop( 10) ==  50);
        REPORTER_ASSERT(reporter, prop( 20) == 100);
        REPORTER_ASSERT(reporter, prop( 30) == 100);
        REPORTER_ASSERT(reporter, prop( 40) ==  75);
        REPORTER_ASSERT(reporter, prop(100) ==  75);
    }
    {
        // Keyframed (hold) scalar.
        TestProperty<skottie::ScalarValue> prop(reporter, R"({
                                                               "a": 1,
                                                               "k": [
                                                                 { "t": 10, "s":  50, "h": 1},
                                                                 { "t": 20, "s": 100, "h": 1}
                                                               ]
                                                             })");
        REPORTER_ASSERT(reporter, !prop->isStatic());
        REPORTER_ASSERT(reporter, prop(-10) ==  50);
        REPORTER_ASSERT(reporter, prop( 10) ==  50);
        REPORTER_ASSERT(reporter, prop( 19) ==  50);
        REPORTER_ASSERT(reporter, prop( 21) == 100);
        REPORTER_ASSERT(reporter, prop(100) == 100);
    }
}
