/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "Resources.h"
#include "SkColorFilter.h"
#include "effects/GrSkSLFP.h"

GR_FP_SRC_STRING SKSL_TEST_SRC = R"(
    in float b;

    void main(inout half4 color) {
        color.rg = color.gr;
        color.b = half(b);
    }
)";

static void runtimeCpuFunc(float color[4], const void* context) {
    std::swap(color[0], color[1]);
    color[2] = *(float*) context;
}

DEF_SIMPLE_GM(runtimecolorfilter, canvas, 512, 256) {
    auto img = GetResourceAsImage("images/mandrill_256.png");

    static int testIndex = GrSkSLFP::NewIndex();
    float b = 0.75;
    auto cf = SkColorFilter::MakeRuntime(testIndex, SKSL_TEST_SRC, &b, sizeof(float), runtimeCpuFunc);

    SkPaint p;
    p.setColorFilter(cf);
    canvas->drawImage(img, 0, 0, nullptr);
    canvas->drawImage(img, 256, 0, &p);
}
