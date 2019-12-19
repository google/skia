/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRuntimeEffect.h"
#include "tests/Test.h"

DEF_TEST(SkRuntimeEffectInvalidPrograms, r) {
    auto test = [r](const char* src, const char* expected) {
        auto [effect, errorText] = SkRuntimeEffect::Make(SkString(src));
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r, errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected, errorText.c_str());
    };

    // Features that are only allowed in .fp files (key, in uniform, ctype, when, tracked).
    // Ensure that these fail, and the error messages contain the relevant keyword.
    test("layout(key) in bool Input;"
         "void main(float x, float y, inout half4 color) {}",
         "key");

    test("in uniform float Input;"
         "void main(float x, float y, inout half4 color) {}",
         "in uniform");

    test("layout(ctype=SkRect) float4 Input;"
         "void main(float x, float y, inout half4 color) {}",
         "ctype");

    test("in bool Flag; layout(when=Flag) uniform float Input;"
         "void main(float x, float y, inout half4 color) {}",
         "when");

    test("layout(tracked) uniform float Input;"
         "void main(float x, float y, inout half4 color) {}",
         "tracked");
}
