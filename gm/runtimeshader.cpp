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
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkRuntimeEffect.h"

const char* gProg = R"(
    uniform half4 gColor;

    void main(float x, float y, inout half4 color) {
        color = half4(half(x)*(1.0/255), half(y)*(1.0/255), gColor.b, 1);
    }
)";

class RuntimeShader : public skiagm::GM {
    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("runtime_shader"); }

    SkISize onISize() override { return {512, 256}; }

    void onDraw(SkCanvas* canvas) override {
        // static to pass gl persistent cache test in dm
        static sk_sp<SkRuntimeEffect> gEffect = std::get<0>(SkRuntimeEffect::Make(SkString(gProg)));
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
DEF_GM(return new RuntimeShader;)
