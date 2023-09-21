/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkColorFilterPriv.h"

static sk_sp<SkShader> color_shader(SkColor4f color) {
    // Why not use SkShaders::Color? We want a shader that inhibits any paint optimization by any
    // backend. The CPU backend will ask the shader portion of the pipeline if it's constant.
    // If so, that portion of the pipeline is executed to get the color, and the color filter is
    // directly applied to the result. The only way to have the color filter run as part of the
    // full CPU pipeline is to have a shader that returns false for isConstant:
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bmp.eraseColor(color);
    return bmp.makeShader(SkFilterMode::kNearest);
}

static sk_sp<SkShader> paint_color_shader() {
    // This will return the paint color (unless it's a child of a runtime effect)
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::MakeA8(1, 1));
    bmp.eraseColor(SkColors::kWhite);
    return bmp.makeShader(SkFilterMode::kNearest);
}

static sk_sp<SkShader> raw_shader(SkColor4f color) {
    return SkRuntimeEffect::MakeForShader(SkString("uniform half4 c;"
                                                   "half4 main(float2 xy) { return c; }"))
            .effect->makeShader(SkData::MakeWithCopy(&color, sizeof(SkColor4f)), {});
}

static sk_sp<SkShader> managed_shader(SkColor4f color) {
    return SkRuntimeEffect::MakeForShader(SkString("layout(color) uniform half4 c;"
                                                   "half4 main(float2 xy) { return c; }"))
            .effect->makeShader(SkData::MakeWithCopy(&color, sizeof(SkColor4f)), {});
}

static sk_sp<SkShader> gradient_shader() {
    const SkPoint pts[] = {{0, 0}, {40, 40}};
    const SkColor4f colors[] = {SkColors::kRed, SkColors::kGreen};
    return SkGradientShader::MakeLinear(pts, colors, nullptr, nullptr, 2, SkTileMode::kClamp);
}

static sk_sp<SkColorFilter> raw_cf(SkColor4f color) {
    return SkRuntimeEffect::MakeForColorFilter(SkString("uniform half4 c;"
                                                        "half4 main(half4 color) { return c; }"))
            .effect->makeColorFilter(SkData::MakeWithCopy(&color, sizeof(SkColor4f)));
}

static sk_sp<SkColorFilter> managed_cf(SkColor4f color) {
    return SkRuntimeEffect::MakeForColorFilter(SkString("layout(color) uniform half4 c;"
                                                        "half4 main(half4 color) { return c; }"))
            .effect->makeColorFilter(SkData::MakeWithCopy(&color, sizeof(SkColor4f)));
}

static sk_sp<SkColorFilter> indirect_cf(SkColor4f color) {
    SkRuntimeEffect::ChildPtr children[] = {color_shader(color)};
    return SkRuntimeEffect::MakeForColorFilter(
                   SkString("uniform shader s;"
                            "half4 main(half4 color) { return s.eval(float2(0)); }"))
            .effect->makeColorFilter(nullptr, children);
}

static sk_sp<SkColorFilter> mode_cf(SkColor4f color) {
    return SkColorFilters::Blend(color, nullptr, SkBlendMode::kSrc);
}

static sk_sp<SkColorFilter> spin(sk_sp<SkColorFilter> cf) {
    return cf->makeWithWorkingColorSpace(SkColorSpace::MakeSRGB()->makeColorSpin());
}

static sk_sp<SkShader> spin(sk_sp<SkShader> shader) {
    return shader->makeWithWorkingColorSpace(SkColorSpace::MakeSRGB()->makeColorSpin());
}

static sk_sp<SkShader> linear(sk_sp<SkShader> shader) {
    return shader->makeWithWorkingColorSpace(SkColorSpace::MakeSRGBLinear());
}

DEF_SIMPLE_GM_CAN_FAIL(workingspace, canvas, errorMsg, 200, 350) {
    if (!canvas->getSurface()) {
        // The only backend that really fails is DDL (because the color filter fails to evaluate on
        // the CPU when we do paint optimization). We can't identify DDL separate from other
        // recording backends, so skip this GM for all of them:
        *errorMsg = "Not supported in recording/DDL mode";
        return skiagm::DrawResult::kSkip;
    }

    // This GM checks that changing the working color space of a color filter does the right thing.
    // We create a variety of color filters that are sensitive to the working space, and test them
    // with both fixed-input color (so that paint optimization can apply the filter on the CPU),
    // and with a shader input (so they're forced to evaluate in the drawing pipeline).
    //
    // In all cases, the tests are designed to draw green if implemented correctly. Any other color
    // (red or blue, most likely) is an error.
    //
    // The bottom row is the exception - it draws red-to-green gradients. The first two should be
    // "ugly" (via brown). The last one should be "nice" (via yellow).

    canvas->translate(5, 5);
    canvas->save();

    auto cell = [&](sk_sp<SkShader> shader,
                    sk_sp<SkColorFilter> colorFilter,
                    SkColor4f paintColor = SkColors::kBlack) {
        SkPaint paint;
        paint.setColor(paintColor);
        paint.setShader(shader);
        paint.setColorFilter(colorFilter);
        canvas->drawRect({0, 0, 40, 40}, paint);
        canvas->translate(50, 0);
    };

    auto nextRow = [&]() {
        canvas->restore();
        canvas->translate(0, 50);
        canvas->save();
    };

    auto blackShader = color_shader(SkColors::kBlack);

    cell(nullptr, raw_cf(SkColors::kGreen));
    cell(nullptr, managed_cf(SkColors::kGreen));
    cell(nullptr, indirect_cf(SkColors::kGreen));
    cell(nullptr, mode_cf(SkColors::kGreen));

    nextRow();

    cell(blackShader, raw_cf(SkColors::kGreen));
    cell(blackShader, managed_cf(SkColors::kGreen));
    cell(blackShader, indirect_cf(SkColors::kGreen));
    cell(blackShader, mode_cf(SkColors::kGreen));

    nextRow();

    cell(nullptr, spin(raw_cf(SkColors::kRed)));  // Un-managed red turns into green
    cell(nullptr, spin(managed_cf(SkColors::kGreen)));
    cell(nullptr, spin(indirect_cf(SkColors::kGreen)));
    cell(nullptr, spin(mode_cf(SkColors::kGreen)));

    nextRow();

    cell(blackShader, spin(raw_cf(SkColors::kRed)));  // Un-managed red turns into green
    cell(blackShader, spin(managed_cf(SkColors::kGreen)));
    cell(blackShader, spin(indirect_cf(SkColors::kGreen)));
    cell(blackShader, spin(mode_cf(SkColors::kGreen)));

    nextRow();

    cell(raw_shader(SkColors::kGreen), nullptr);
    cell(managed_shader(SkColors::kGreen), nullptr);
    cell(color_shader(SkColors::kGreen), nullptr);
    cell(paint_color_shader(), nullptr, SkColors::kGreen);

    nextRow();

    cell(spin(raw_shader(SkColors::kRed)), nullptr);  // Un-managed red turns into green
    cell(spin(managed_shader(SkColors::kGreen)), nullptr);
    cell(spin(color_shader(SkColors::kGreen)), nullptr);
    cell(spin(paint_color_shader()), nullptr, SkColors::kGreen);

    nextRow();

    cell(gradient_shader(), nullptr);          // Red to green, via ugly brown
    cell(spin(gradient_shader()), nullptr);    // Same (spin doesn't change anything)
    cell(linear(gradient_shader()), nullptr);  // Red to green, via bright yellow

    return skiagm::DrawResult::kOk;
}
