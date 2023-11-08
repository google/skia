/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_USE_DRAWING_MIPMAP_DOWNSAMPLER

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkRasterClip.h"
#include <memory>

namespace {

struct DrawDownSampler : SkMipmapDownSampler {
    SkPaint fPaint;

    DrawDownSampler() {
        // For efficiency, we want to be in src mode when building levels
        // to avoid any extra trying to "blend".
        fPaint.setBlendMode(SkBlendMode::kSrc);
    }

    void buildLevel(const SkPixmap& dst, const SkPixmap& src) override;
};

static SkSamplingOptions choose_options(const SkPixmap& dst, const SkPixmap& src) {
    // if we're a perfect 2x2 downscale, just use bilerp
    if (dst.width() * 2 == src.width() && dst.height() * 2 == src.height()) {
        return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }
    // general case -- might experiment with different cubic parameter values
    const auto cubic = SkCubicResampler::CatmullRom();
    return SkSamplingOptions(cubic);
}

void DrawDownSampler::buildLevel(const SkPixmap& dst, const SkPixmap& src) {
    const SkRasterClip rclip(dst.bounds());
    const SkMatrix mx = SkMatrix::Scale(SkIntToScalar(dst.width())  / src.width(),
                                        SkIntToScalar(dst.height()) / src.height());
    const auto sampling = choose_options(dst, src);

    SkDraw draw;
    draw.fDst = dst;
    draw.fCTM = &mx;
    draw.fRC = &rclip;

    SkBitmap bitmap;
    bitmap.installPixels(src.info(), const_cast<void*>(src.addr()), src.rowBytes());

    draw.drawBitmap(bitmap, SkMatrix::I(), nullptr, sampling, fPaint);
}

} // namespace

std::unique_ptr<SkMipmapDownSampler> SkMipmap::MakeDownSampler(const SkPixmap& root) {
    return std::make_unique<DrawDownSampler>();
}

#endif
