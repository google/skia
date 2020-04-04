/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkRTShader.h"
#include "tools/Resources.h"

#include <stddef.h>

static const char* RUNTIME_FUNCTIONS_SRC = R"(
    uniform half4 gColor;

    half scale(float x) {
        return half(x) / 255;
    }

    half4 blackAndWhite(half4 raw) {
        half value = raw.r * 0.22 + raw.g * 0.67 + raw.b * 0.11;
        return half4(half3(value), raw.a);
    }

    void main(float x, float y, inout half4 color) {
        color = blackAndWhite(half4(scale(x), scale(y), gColor.b, 1));
    }
)";

static sk_sp<SkShader> gShader;

class RuntimeFunctions : public skiagm::GM {
    sk_sp<SkData> fData;

    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("runtimefunctions"); }

    SkISize onISize() override { return {256, 256}; }

    void onOnceBeforeDraw() override {
        // use global to pass gl persistent cache test in dm
        if (!gShader) {
            SkMatrix localM;
            localM.setRotate(90, 128, 128);

            fData = SkData::MakeUninitialized(sizeof(SkColor4f));
            SkColor4f* c = (SkColor4f*)fData->writable_data();
            *c = {1, 0, 0, 1};
            gShader = SkRuntimeShaderFactory(SkString(RUNTIME_FUNCTIONS_SRC),
                                             true).make(fData, &localM);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setShader(gShader);
        canvas->drawRect({0, 0, 256, 256}, p);
    }
};
DEF_GM(return new RuntimeFunctions;)
