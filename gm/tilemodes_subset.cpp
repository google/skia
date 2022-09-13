/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/shaders/SkImageShader.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {

constexpr SkTileMode kTileModes[] = {
        SkTileMode::kClamp,
        SkTileMode::kRepeat,
        SkTileMode::kMirror,
        SkTileMode::kDecal,
};

constexpr SkFilterMode kFilterModes[] = {
        SkFilterMode::kNearest,
        SkFilterMode::kLinear,
};

constexpr SkMipmapMode kMipmapModes[] = {
        SkMipmapMode::kNone,
        SkMipmapMode::kNearest,
        SkMipmapMode::kLinear,
};

constexpr int kTileSize = 100;
constexpr int kSubsetSize = kTileSize / 2;
constexpr int kPadding = 10;

constexpr SkRect kSrcSubRect = SkRect::MakeXYWH(1, 1, 2, 2);
constexpr SkRect kDstSubRect = SkRect::MakeXYWH(0, 0, kSubsetSize, kSubsetSize);
constexpr SkRect kDstRect = SkRect::MakeXYWH(0, 0, kTileSize, kTileSize);

sk_sp<SkImage> make_ringed_image(SkCanvas* canvas) {
    static constexpr SkColor4f kRingColor = SkColors::kRed,
                               kCheckColor1 = SkColors::kBlack,
                               kCheckColor2 = SkColors::kWhite;

    SkImageInfo info = SkImageInfo::Make(4, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(info, 0);

    bitmap.eraseColor(kRingColor);
    bitmap.erase(kCheckColor1, {1, 1, 2, 2});
    bitmap.erase(kCheckColor1, {2, 2, 3, 3});
    bitmap.erase(kCheckColor2, {2, 1, 3, 2});
    bitmap.erase(kCheckColor2, {1, 2, 2, 3});

    bitmap.setImmutable();
    return bitmap.asImage();
}

void draw_tile(int columnX,
               int rowY,
               SkCanvas* canvas,
               sk_sp<SkImage> image,
               const SkSamplingOptions& samplingOptions,
               SkTileMode tileMode) {
    canvas->save();
    canvas->translate(kPadding + columnX * (kTileSize + kPadding),
                      kPadding + rowY * (kTileSize + kPadding));

    SkPaint p;
    SkMatrix srcToDst = SkMatrix::RectToRect(kSrcSubRect, kDstSubRect);
    p.setShader(SkImageShader::MakeSubset(
            std::move(image), kSrcSubRect, tileMode, tileMode, samplingOptions, &srcToDst));
    canvas->drawRect(kDstRect, p);

    canvas->restore();
}

void draw_column(int columnX,
                 SkCanvas* canvas,
                 sk_sp<SkImage> image,
                 const SkSamplingOptions& samplingOptions) {
    int rowY = 0;
    for (SkTileMode tileMode : kTileModes) {
        draw_tile(columnX, rowY++, canvas, image, samplingOptions, tileMode);
    }
}

}  // anonymous namespace

namespace skiagm {

DEF_SIMPLE_GM_BG_CAN_FAIL(tilemode_subset, canvas, errorMsg, 780, 450, SK_ColorCYAN) {
    if (!canvas->recordingContext() &&  // Not Ganesh.
        !canvas->recorder()) {          // Not Graphite.
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    sk_sp<SkImage> image = make_ringed_image(canvas);
    image = ToolUtils::MakeTextureImage(canvas, std::move(image));

    int columnX = 0;
    for (SkMipmapMode mipmapMode : kMipmapModes) {
        for (SkFilterMode filterMode : kFilterModes) {
            draw_column(columnX++, canvas, image, SkSamplingOptions(filterMode, mipmapMode));
        }
    }
    draw_column(columnX++, canvas, image, SkSamplingOptions(SkCubicResampler::Mitchell()));

    return DrawResult::kOk;
}

}  // namespace skiagm
