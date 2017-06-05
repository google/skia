/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformImageGenerator.h"

std::unique_ptr<SkImageGenerator> SkColorSpaceXformImageGenerator::Make(
        const SkBitmap& src, sk_sp<SkColorSpace> dstColorSpace, Id id) {
    if (!dstColorSpace) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkColorSpaceXformImageGenerator(src, std::move(dstColorSpace), id));
}

SkColorSpaceXformImageGenerator::SkColorSpaceXformImageGenerator(const SkPixmap& srcPixmap,
                                                                 sk_sp<SkColorSpace> dstColorSpace,
                                                                 Id id)
    : INHERITED(bitmap.info().makeColorSpace(dstColorSpace),
                )
    , fPixmap(srcPixmap)
    , fColorSpace(dstColorSpace) {
}

bool SkColorSpaceXformImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                  size_t rowBytes, SkPMColor ctable[],
                                                  int* ctableCount) {
    Options opts;
    opts.fColorTable = ctable;
    opts.fColorTableCount = ctableCount;
    opts.fBehavior = SkTransferFunctionBehavior::kIgnore;
    return this->onGetPixels(info, pixels, rowBytes, opts);
}

bool SkColorSpaceXformImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                  size_t rowBytes, const Options& opts) {
    // TODO: Invoke SkColorSpaceXform
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTextureProxy.h"

sk_sp<GrTextureProxy> SkColorSpaceXformImageGenerator::onGenerateTexture(GrContext* ctx,
                                                                         const SkImageInfo& info,
                                                                         const SkIPoint& origin) {
    SkASSERT(ctx);

    // TODO: Upload pixmap to temporary texture, draw (with color space xform) to secondary
    // surface, return that surface as a texture proxy.
    return nullptr;
}
#endif