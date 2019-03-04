/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "Resources.h"
#include "SkColorFilterPriv.h"
#include "SkReadBuffer.h"
#include "effects/GrSkSLFP.h"

const char* SKSL_TEST_SRC = R"(
    in uniform float b;

    void main(inout half4 color) {
        color.rg = color.gr;
        color.b = half(b);
    }
)";

static void runtimeCpuFunc(float color[4], const void* context) {
    std::swap(color[0], color[1]);
    color[2] = *(float*) context;
}

DEF_SIMPLE_GPU_GM(runtimecolorfilter, context, rtc, canvas, 768, 256) {
    auto img = GetResourceAsImage("images/mandrill_256.png");
    canvas->drawImage(img, 0, 0, nullptr);

    float b = 0.75;
    sk_sp<SkData> data = SkData::MakeWithoutCopy(&b, sizeof(b));
    auto cf1 = SkRuntimeColorFilterFactory(SkString(SKSL_TEST_SRC), runtimeCpuFunc).make(data);
    SkPaint p;
    p.setColorFilter(cf1);
    canvas->drawImage(img, 256, 0, &p);

    static constexpr size_t kBufferSize = 512;
    char buffer[kBufferSize];
    SkBinaryWriteBuffer wb(buffer, kBufferSize);
    wb.writeFlattenable(cf1.get());
    SkReadBuffer rb(buffer, kBufferSize);
    auto cf2 = rb.readColorFilter();
    SkASSERT(cf2);
    p.setColorFilter(cf2);
    canvas->drawImage(img, 512, 0, &p);
}
