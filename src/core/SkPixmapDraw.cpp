/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file contains implementations of SkPixmap methods which require the CPU backend.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "src/shaders/SkImageShader.h"

#include <utility>

struct SkSamplingOptions;

bool SkPixmap::scalePixels(const SkPixmap& actualDst, const SkSamplingOptions& sampling) const {
    // We may need to tweak how we interpret these just a little below, so we make copies.
    SkPixmap src = *this,
             dst = actualDst;

    // Can't do anthing with empty src or dst
    if (src.width() <= 0 || src.height() <= 0 ||
        dst.width() <= 0 || dst.height() <= 0) {
        return false;
    }

    // no scaling involved?
    if (src.width() == dst.width() && src.height() == dst.height()) {
        return src.readPixels(dst);
    }

    // If src and dst are both unpremul, we'll fake the source out to appear as if premul,
    // and mark the destination as opaque.  This odd combination allows us to scale unpremul
    // pixels without ever premultiplying them (perhaps losing information in the color channels).
    // This is an idiosyncratic feature of scalePixels(), and is tested by scalepixels_unpremul GM.
    bool clampAsIfUnpremul = false;
    if (src.alphaType() == kUnpremul_SkAlphaType &&
        dst.alphaType() == kUnpremul_SkAlphaType) {
        src.reset(src.info().makeAlphaType(kPremul_SkAlphaType), src.addr(), src.rowBytes());
        dst.reset(dst.info().makeAlphaType(kOpaque_SkAlphaType), dst.addr(), dst.rowBytes());

        // We'll need to tell the image shader to clamp to [0,1] instead of the
        // usual [0,a] when using a bicubic scaling (kHigh_SkFilterQuality).
        clampAsIfUnpremul = true;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(src)) {
        return false;
    }
    bitmap.setImmutable();        // Don't copy when we create an image.

    SkMatrix scale = SkMatrix::RectToRect(SkRect::Make(src.bounds()), SkRect::Make(dst.bounds()));

    sk_sp<SkShader> shader = SkImageShader::Make(bitmap.asImage(),
                                                 SkTileMode::kClamp,
                                                 SkTileMode::kClamp,
                                                 sampling,
                                                 &scale,
                                                 clampAsIfUnpremul);

    sk_sp<SkSurface> surface =
            SkSurfaces::WrapPixels(dst.info(), dst.writable_addr(), dst.rowBytes());
    if (!shader || !surface) {
        return false;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setShader(std::move(shader));
    surface->getCanvas()->drawPaint(paint);
    return true;
}
