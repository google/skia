/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"

sk_sp<SkBlender> stretch_colors_blender() {
    return SkRuntimeEffect::MakeForBlender(SkString(
        "half4 main(half4 src, half4 dst) { return ((dst.rgb - 0.25) * 16).rgb1; }"))
            .effect->makeBlender(nullptr);
}

DEF_SIMPLE_GM_CAN_FAIL(image_dither, canvas, errorMsg, 425, 110) {
    if (!canvas->getSurface()) {
        // This GM relies on a high-precision (F16) image to determine if image draws are dithered.
        // Serializing configs will tend to throw away that data (compressing to PNG), so they will
        // produce different results before/after serialization (and thus fail).
        *errorMsg = "Not supported in recording mode";
        return skiagm::DrawResult::kSkip;
    }

    // First, we make a non-dithered image with a shallow radial gradient. This will be our source:
    const SkColor colors[] = { 0xFF555555, 0xFF444444 };
    const SkPoint points[] = {{0, 0}, {100, 100}};
    sk_sp<SkShader> gradient = SkGradientShader::MakeLinear(
            points, colors, nullptr, std::size(colors), SkTileMode::kClamp);
    SkPaint gradientPaint;
    gradientPaint.setShader(gradient);

    sk_sp<SkSurface> surface = SkSurfaces::Raster(
            SkImageInfo::Make(100, 100, kRGBA_F16_SkColorType, kPremul_SkAlphaType));
    surface->getCanvas()->drawPaint(gradientPaint);
    sk_sp<SkImage> image = surface->makeImageSnapshot();

    // Now, we draw it three times:
    // 1) As-is (no dither), to ensure that our source image doesn't have any dithering included
    // 2) Using an image shader, with dithering enabled on the paint
    // 3) With drawImage, with dithering enabled on the paint
    //
    // We'd like #2 and #3 to both respect the dither flag, for consistency (b/320529640)
    canvas->translate(5, 5);

    canvas->drawImage(image, 0, 0);
    canvas->translate(105, 0);

    SkPaint imageShaderPaint;
    imageShaderPaint.setShader(image->makeShader(SkSamplingOptions{}));
    imageShaderPaint.setDither(true);
    canvas->drawRect({0, 0, 100, 100}, imageShaderPaint);
    canvas->translate(105, 0);

    SkPaint drawImagePaint;
    drawImagePaint.setDither(true);
    canvas->drawImage(image, 0, 0, SkSamplingOptions{}, &drawImagePaint);
    canvas->translate(105, 0);

    // Also draw the actual gradient with the dither flag, to see how it should look:
    gradientPaint.setDither(true);
    canvas->drawRect({0, 0, 100, 100}, gradientPaint);

    SkPaint colorStretchPaint;
    colorStretchPaint.setBlender(stretch_colors_blender());
    canvas->drawPaint(colorStretchPaint);

    return skiagm::DrawResult::kOk;
}
