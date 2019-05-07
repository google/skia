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
#include <utility>

class GrContext;
class GrRenderTargetContext;

const char* SKSL_TEST_SRC = R"(
    layout(ctype=float) in uniform half b;

    void main(inout half4 color) {
        color.a = color.r*0.3 + color.g*0.6 + color.b*0.1;
        color.r = 0;
        color.g = 0;
        color.b = 0;
    }
)";

static void runtimeCpuFunc(float color[4], const void* context) {
    color[3] = color[0]*0.3 + color[1]*0.6 + color[2]*0.1;
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
}

DEF_SIMPLE_GPU_GM(runtimecolorfilter, context, rtc, canvas, 768, 256) {
    auto img = GetResourceAsImage("images/mandrill_256.png");
    canvas->drawImage(img, 0, 0, nullptr);

    float b = 0.75;
    sk_sp<SkData> data = SkData::MakeWithCopy(&b, sizeof(b));
    static SkRuntimeColorFilterFactory fact = SkRuntimeColorFilterFactory(SkString(SKSL_TEST_SRC),
                                                                          runtimeCpuFunc);
    auto cf1 = fact.make(data);
    SkPaint p;
    p.setColorFilter(cf1);
    canvas->drawImage(img, 256, 0, &p);

    static constexpr size_t kBufferSize = 512;
    char buffer[kBufferSize];
    SkBinaryWriteBuffer wb(buffer, kBufferSize);
    wb.writeFlattenable(cf1.get());
    SkReadBuffer rb(buffer, kBufferSize);
    auto cf2 = rb.readColorFilter();
    if (cf2) {
        p.setColorFilter(cf2);
        canvas->drawImage(img, 512, 0, &p);
    }
}

DEF_SIMPLE_GM(runtimecolorfilter_interpreted, canvas, 768, 256) {
    auto img = GetResourceAsImage("images/mandrill_256.png");
    canvas->drawImage(img, 0, 0, nullptr);

    float b = 0.75;
    sk_sp<SkData> data = SkData::MakeWithCopy(&b, sizeof(b));
    static SkRuntimeColorFilterFactory fact = SkRuntimeColorFilterFactory(SkString(SKSL_TEST_SRC),
                                                                          nullptr);
    auto cf1 = fact.make(data);
    SkPaint p;
    p.setColorFilter(cf1);
    canvas->drawImage(img, 256, 0, &p);

    static constexpr size_t kBufferSize = 512;
    char buffer[kBufferSize];
    SkBinaryWriteBuffer wb(buffer, kBufferSize);
    wb.writeFlattenable(cf1.get());
    SkReadBuffer rb(buffer, kBufferSize);
    auto cf2 = rb.readColorFilter();
    if (cf2) {
        p.setColorFilter(cf2);
        canvas->drawImage(img, 512, 0, &p);
    }
}

// These need to be static for some dm caching tests in DM...
static SkRuntimeColorFilterFactory gInterp =
    SkRuntimeColorFilterFactory(SkString(SKSL_TEST_SRC), nullptr);
static SkRuntimeColorFilterFactory gCpuProc =
    SkRuntimeColorFilterFactory(SkString(SKSL_TEST_SRC), runtimeCpuFunc);

class RuntimeCF : public skiagm::GM {
public:
    RuntimeCF(bool useCpuProc) : fFact(useCpuProc ? gCpuProc : gInterp) {
        fName.printf("runtime_cf_interp_%d", !useCpuProc);
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
        fImg = GetResourceAsImage("images/mandrill_256.png")->makeRasterImage();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fImg, 0, 0, nullptr);

        float b = 0.75;
        sk_sp<SkData> data = SkData::MakeWithCopy(&b, sizeof(b));
        auto cf1 = fFact.make(data);
        SkPaint p;
        p.setColorFilter(cf1);
        canvas->drawImage(fImg, 256, 0, &p);
    }
private:
    sk_sp<SkImage> fImg;
    SkRuntimeColorFilterFactory fFact;
    SkString fName;

    typedef skiagm::GM INHERITED;
};
DEF_GM(return new RuntimeCF(false);)
//DEF_GM(return new RuntimeCF(true);)
