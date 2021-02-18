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
    uniform half4 gColor;

    half scale(float x) {
        return x / 255;
    }

    half4 blackAndWhite(half4 raw) {
        half value = raw.r * 0.22 + raw.g * 0.67 + raw.b * 0.11;
        return half4(value.xxx, raw.a);
    }

    half4 main(float2 p) {
        return blackAndWhite(half4(scale(p.x), scale(p.y), gColor.b, 1));
    }
)";

class RuntimeFunctions : public skiagm::GM {
    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("runtimefunctions"); }

    SkISize onISize() override { return {256, 256}; }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkRuntimeEffect> gEffect =
                SkRuntimeEffect::Make(SkString(RUNTIME_FUNCTIONS_SRC)).effect;
        SkASSERT(gEffect);

        SkMatrix localM;
        localM.setRotate(90, 128, 128);

        SkColor4f inputColor = { 1, 0, 0, 1 };
        auto shader = gEffect->makeShader(SkData::MakeWithCopy(&inputColor, sizeof(inputColor)),
                                          nullptr, 0, &localM, true);
        SkPaint p;
        p.setShader(std::move(shader));
        canvas->drawRect({0, 0, 256, 256}, p);
    }
};

DEF_GM(return new RuntimeFunctions;)
