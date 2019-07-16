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
#include "tools/Resources.h"

#include <stddef.h>

extern sk_sp<SkShader> SkRuntimeShaderMaker(SkString sksl, sk_sp<SkData> inputs,
                                            const SkMatrix* localMatrix, bool isOpaque);

const char* gProg = R"(
    layout(ctype=SkRect) in uniform half4 gColor;

    void main(float x, float y, inout half4 color) {
        color = half4(half(x)*(1.0/255), half(y)*(1.0/255), gColor.b, 1);
    }
)";

static sk_sp<SkShader> gShader;

class RuntimeShader : public skiagm::GM {
public:
    RuntimeShader() {
        fName.printf("runtime_shader");
    }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(512, 256);
    }

    void onOnceBeforeDraw() override {
        // use global to pass gl persistent cache test in dm
        if (!gShader) {
            SkMatrix localM;
            localM.setRotate(90, 128, 128);

            fData = SkData::MakeUninitialized(sizeof(SkColor4f));
            SkColor4f* c = (SkColor4f*)fData->writable_data();
            *c = {1, 0, 0, 1};
            gShader = SkRuntimeShaderMaker(SkString(gProg), fData, &localM, true);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setShader(gShader);
        canvas->drawRect({0, 0, 256, 256}, p);
    }
private:
    SkString fName;
    sk_sp<SkData> fData;

    typedef skiagm::GM INHERITED;
};
DEF_GM(return new RuntimeShader;)
