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

    enum class Id {
        kDefault,  // Images created with this generator have their own unique id
        kCloneSrc, // Images created with this generator share an id with the |src| SkBitmap
    };
    std::unique_ptr<SkImageGenerator> Make(const SkBitmap& src,
            sk_sp<SkColorSpace> dstColorSpace, Id id);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options& opts) override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&,
                                            const SkIPoint&) override;
#endif

private:
    SkBitmap            fBitmap;
    sk_sp<SkColorSpace> fDstColorSpace;

    friend class SkImageGenerator;

    typedef SkImageGenerator INHERITED;
};

#endif // SkColorSpaceXformImageGenerator_DEFINED