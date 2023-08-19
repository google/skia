/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"

static const char* RUNTIME_FUNCTIONS_SRC = R"(
// Source: @notargs https://twitter.com/notargs/status/1250468645030858753
uniform half4 iResolution;
const float iTime = 0;

float f(vec3 p) {
    p.z -= iTime * 10.;
    float a = p.z * .1;
    p.xy *= mat2(cos(a), sin(a), -sin(a), cos(a));
    return .1 - length(cos(p.xy) + sin(p.yz));
}

half4 main(vec2 fragcoord) {
    vec3 d = .5 - fragcoord.xy1 / iResolution.y;
    vec3 p=vec3(0);
    for (int i = 0; i < 32; i++) {
      p += f(p) * d;
    }
    return ((sin(p) + vec3(2, 5, 9)) / length(p)).xyz1;
}
)";

class RuntimeFunctions : public skiagm::GM {
    bool runAsBench() const override { return true; }

    SkString getName() const override { return SkString("runtimefunctions"); }

    SkISize getISize() override { return {256, 256}; }

    void onDraw(SkCanvas* canvas) override {
        SkRuntimeEffect::Result result =
                SkRuntimeEffect::MakeForShader(SkString(RUNTIME_FUNCTIONS_SRC));
        SkASSERTF(result.effect, "%s", result.errorText.c_str());

        SkMatrix localM;
        localM.setRotate(90, 128, 128);

        SkV4 iResolution = { 255, 255, 0, 0 };
        auto shader = result.effect->makeShader(
                SkData::MakeWithCopy(&iResolution, sizeof(iResolution)), nullptr, 0, &localM);
        SkPaint p;
        p.setShader(std::move(shader));
        canvas->drawRect({0, 0, 256, 256}, p);
    }
};

DEF_GM(return new RuntimeFunctions;)
