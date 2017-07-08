/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformImageGenerator_DEFINED
#define SkColorSpaceXformImageGenerator_DEFINED

#include "SkImageGenerator.h"
#include "SkImagePriv.h"

class SkColorSpaceXformImageGenerator : public SkImageGenerator {
public:

    static std::unique_ptr<SkImageGenerator> Make(
            const SkBitmap& src, sk_sp<SkColorSpace> dst, SkCopyPixelsMode);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options& opts) override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            SkTransferFunctionBehavior) override;
    TexGenType onCanGenerateTexture() const override {
        return TexGenType::kExpensive;
    }
#endif

private:
    SkBitmap            fSrc;
    sk_sp<SkColorSpace> fDst;

    static std::unique_ptr<SkImageGenerator> Make(
            const SkBitmap& src, sk_sp<SkColorSpace> dst, SkCopyPixelsMode, uint32_t id);
    SkColorSpaceXformImageGenerator(const SkBitmap& src, sk_sp<SkColorSpace> dst, uint32_t id);

    friend sk_sp<SkImage> SkMakeImageInColorSpace(const SkBitmap&, sk_sp<SkColorSpace>, uint32_t,
                                                  SkCopyPixelsMode);
    typedef SkImageGenerator INHERITED;
};

#endif // SkColorSpaceXformImageGenerator_DEFINED
