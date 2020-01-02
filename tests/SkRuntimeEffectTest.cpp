/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkRuntimeEffect.h"
#include "tests/Test.h"

DEF_TEST(SkRuntimeEffectInvalidInputs, r) {
    auto test = [r](const char* hdr, const char* expected) {
        SkString src = SkStringPrintf("%s void main(float x, float y, inout half4 color) {}", hdr);
        auto [effect, errorText] = SkRuntimeEffect::Make(src);
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r, errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected, errorText.c_str());
    };

    // Features that are only allowed in .fp files (key, in uniform, ctype, when, tracked).
    // Ensure that these fail, and the error messages contain the relevant keyword.
    test("layout(key) in bool Input;", "key");
    test("in uniform float Input;", "in uniform");
    test("layout(ctype=SkRect) float4 Input;", "ctype");
    test("in bool Flag; layout(when=Flag) uniform float Input;", "when");
    test("layout(tracked) uniform float Input;", "tracked");

    // Runtime SkSL supports a limited set of uniform types. No samplers, for example:
    test("uniform sampler2D s;", "sampler2D");

    // 'in' variables can't be arrays
    test("in int Input[2];", "array");
}
