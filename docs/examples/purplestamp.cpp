// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(purplestamp, 256, 256, false, 0) {
// purplestamp

// Whether to use a separate bitmap (with a color type) at all.
bool drawWithRasterImage = true;

// This works with either kN32_SkColorType or kAlpha_8_SkColorType.
// Applies only if drawWithRasterImage is true.
SkColorType colorType = kN32_SkColorType;

void drawStamp(SkCanvas* canvas, int size) {
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(size, size), SkClipOp::kIntersect, true);

    canvas->clear(0x3F000000 /* translucent black */);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    canvas->drawRRect(
            SkRRect::MakeOval(SkRect::MakeXYWH(size / 4, size / 4, size / 2, size / 2)), paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    canvas->drawRect(SkRect::MakeWH(size, size), paint);

    canvas->restore();
}

sk_sp<SkImage> stampImage(int size) {
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(
            SkImageInfo::Make(size, size, colorType, kPremul_SkAlphaType));
    drawStamp(surface->getCanvas(), size);
    return surface->makeImageSnapshot();
}

void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColorFilter(SkColorFilters::Blend(0xFF7F00FF, SkBlendMode::kSrcIn));
    paint.setAntiAlias(true);

    SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kLinear);

    canvas->rotate(30);
    canvas->translate(60, 0);

    int stampSize = 200;
    if (drawWithRasterImage) {
        canvas->drawImage(stampImage(stampSize), 0, 0, sampling, &paint);
    } else {
        canvas->saveLayer(nullptr, &paint);
        drawStamp(canvas, stampSize);
        canvas->restore();
    }
}
}  // END FIDDLE
