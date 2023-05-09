/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkPixmapUtils.h"

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"

#include <utility>

static bool draw_orientation(const SkPixmap& dst, const SkPixmap& src, SkEncodedOrigin origin) {
    auto surf = SkSurfaces::WrapPixels(dst.info(), dst.writable_addr(), dst.rowBytes());
    if (!surf) {
        return false;
    }

    SkBitmap bm;
    bm.installPixels(src);

    SkMatrix m = SkEncodedOriginToMatrix(origin, dst.width(), dst.height());

    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);
    surf->getCanvas()->concat(m);
    surf->getCanvas()->drawImage(SkImages::RasterFromBitmap(bm), 0, 0, SkSamplingOptions(), &p);
    return true;
}

namespace SkPixmapUtils {

bool Orient(const SkPixmap& dst, const SkPixmap& src, SkEncodedOrigin origin) {
    if (src.colorType() != dst.colorType()) {
        return false;
    }
    // note: we just ignore alphaType and colorSpace for this transformation

    int w = src.width();
    int h = src.height();
    if (SkEncodedOriginSwapsWidthHeight(origin)) {
        using std::swap;
        swap(w, h);
    }
    if (dst.width() != w || dst.height() != h) {
        return false;
    }
    if (w == 0 || h == 0) {
        return true;
    }

    // check for aliasing to self
    if (src.addr() == dst.addr()) {
        return kTopLeft_SkEncodedOrigin == origin;
    }
    return draw_orientation(dst, src, origin);
}

SkImageInfo SwapWidthHeight(const SkImageInfo& info) {
    return info.makeWH(info.height(), info.width());
}

}
