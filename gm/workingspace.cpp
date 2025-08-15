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
#include "src/shaders/SkWorkingColorSpaceShader.h"

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
                                                   "half4 main(float2 xy) {"
                                                       "return half4(c.rgb*c.a, c.a);"
                                                   "}"))
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

// When color conversion and alpha type is handled correctly from all input types (e.g. image,
// color, and runtime effect), this should produce a 2x3 grid of squares in the same green color.
//
// * For CPU/Ganesh, the bottom row (all workInUnpremul cases) render incorrectly because all inputs
//   still produce premul values and the wrapped shader is assumed to produce a premul value.
// * For Graphite, solid color and image inputs correctly convert to unpremul alpha but other shader
//   types produce premul values w/o any extra conversion to unpremul. Only the bottom-middle cell
//   renders incorrectly.
DEF_SIMPLE_GM(workingspace_input_output, canvas, 256, 256) {
    // unpremul, sRGB input color to the runtime shader that is then wrapped in a workingspace
    SkColor4f inputColor = {0.2f, 0.4f, 0.7f, 0.5f};

    // These should produce the same value, barring colortype encoding precision
    sk_sp<SkShader> childColor = SkShaders::Color(inputColor, nullptr);
    sk_sp<SkShader> childUniform = managed_shader(inputColor);
    sk_sp<SkShader> childImage = color_shader(inputColor);

    // The input working space will be the linear space with the current surface's gamut
    sk_sp<SkColorSpace> inputCS = canvas->imageInfo().refColorSpace();
    if (inputCS) {
        inputCS = inputCS->makeLinearGamma();
    } else {
        inputCS = SkColorSpace::MakeSRGBLinear();
    }

    // The output space will be a colorspin of the input gamut with a 2.2 gamma
    sk_sp<SkColorSpace> outputCS = inputCS->makeColorSpin();
    skcms_Matrix3x3 outputGamut;
    outputCS->toXYZD50(&outputGamut);
    outputCS = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, outputGamut);

    SkRect rect = {0.f, 0.f, 32.f, 32.f};
    const float padding = 4.f;

    canvas->translate(padding, padding);
    for (bool workInUnpremul : {false, true}) {
        const char* manualUnpremul = workInUnpremul ? "false" : "true";
        SkString sksl = SkStringPrintf(
                "uniform shader child;"
                "half4 main(float2 xy) {"
                    "half4 inRGBA = child.eval(xy);" // This is in inputCS w/ inputAlpha type
                    "if (%s) {" // do manual unpremul if not `workInUnpremul`
                        "inRGBA.rgb /= inRGBA.a + 0.00001;"
                    "}"
                    // the "effect" to apply in the linear unpremul working space
                    "half4 scaled = half4(saturate(inRGBA.rgb * 1.5), inRGBA.a);"
                    // manual color spin and 2.2 gamma to inline the conversion from
                    // inputCS to outputCS.
                    "half4 outRGBA = half4(pow(scaled.bgr, half3(2.2)), scaled.a);"
                    "if (%s) {" // do manual premul if not `workInUnpremul`
                        "outRGBA.rgb *= outRGBA.a;"
                    "}"
                    "return outRGBA;"
                "}",
                manualUnpremul, manualUnpremul);

        auto rte = SkRuntimeEffect::MakeForShader(sksl);
        SkASSERTF(rte.effect, "Failed to create runtime effect: %s", rte.errorText.c_str());

        canvas->save();
        for (auto&& child : {childColor, childUniform, childImage}) {
            SkRuntimeShaderBuilder builder{rte.effect};
            builder.child("child") = child;
            sk_sp<SkShader> rteShader = builder.makeShader();

            SkPaint p;
            p.setAntiAlias(true);
            // The option that takes a workInUnpremul parameter isn't public yet
            p.setShader(SkWorkingColorSpaceShader::Make(std::move(rteShader),
                                                        inputCS,
                                                        outputCS,
                                                        workInUnpremul));
            canvas->drawRect(rect, p);
            canvas->translate(rect.width() + padding, 0.f);
        }
        canvas->restore();
        canvas->translate(0.f, rect.height() + padding);
    }
}
