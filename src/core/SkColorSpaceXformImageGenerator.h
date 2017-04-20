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
public:
    static std::unique_ptr<SkImageGenerator> Make(const SkPixmap&, sk_sp<SkColorSpace>,
                                                  uint32_t uniqueId);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&,
                                            const SkIPoint&) override;
#endif

private:
    SkColorSpaceXformImageGenerator(const SkPixmap& pmap, sk_sp<SkColorSpace> colorSpace,
                                    uint32_t uniqueId);

    SkPixmap fPixmap;
    sk_sp<SkColorSpace> fColorSpace;

    typedef SkImageGenerator INHERITED;
};
#endif  // SkColorSpaceXformImageGenerator_DEFINED
