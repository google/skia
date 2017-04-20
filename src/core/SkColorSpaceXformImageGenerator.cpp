/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformImageGenerator.h"

std::unique_ptr<SkImageGenerator>
SkColorSpaceXformImageGenerator::Make(const SkPixmap& pmap, sk_sp<SkColorSpace> colorSpace,
                                      uint32_t uniqueId) {
    return std::unique_ptr<SkImageGenerator>(
            new SkColorSpaceXformImageGenerator(pmap, std::move(colorSpace), uniqueId));
}

SkColorSpaceXformImageGenerator::SkColorSpaceXformImageGenerator(const SkPixmap& pmap,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 uint32_t uniqueId)
    : INHERITED(pmap.info().makeColorSpace(colorSpace), uniqueId)
    , fPixmap(pmap)
    , fColorSpace(colorSpace) {
}

bool SkColorSpaceXformImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                  size_t rowBytes, SkPMColor ctable[],
                                                  int* ctableCount) {
    // TODO: Invoke SkColorSpaceXform
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromRasterWithColorSpaceXform(const SkPixmap& pmap,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    uint32_t uniqueId) {
    if (!colorSpace) {
        return nullptr;
    }

    return SkColorSpaceXformImageGenerator::Make(pmap, std::move(colorSpace), uniqueId);
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
