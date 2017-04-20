/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorSpaceXformImageGenerator_DEFINED
#define SkColorSpaceXformImageGenerator_DEFINED

#include "SkImageGenerator.h"

class SkColorSpaceXformImageGenerator : public SkImageGenerator {
protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     SkPMColor ctable[], int* ctableCount) override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options& opts) override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&,
                                            const SkIPoint&) override;
#endif

private:
    SkColorSpaceXformImageGenerator(const SkPixmap& srcPixmap, sk_sp<SkColorSpace> dstColorSpace,
                                    uint32_t uniqueId);

    SkPixmap fPixmap;
    sk_sp<SkColorSpace> fColorSpace;

    friend class SkImageGenerator;

    typedef SkImageGenerator INHERITED;
};
#endif  // SkColorSpaceXformImageGenerator_DEFINED
