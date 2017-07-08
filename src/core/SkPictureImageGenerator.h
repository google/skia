/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureImageGenerator_DEFINED
#define SkPictureImageGenerator_DEFINED

#include "SkImageGenerator.h"
#include "SkTLazy.h"

class SkPictureImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(const SkISize&, sk_sp<SkPicture>, const SkMatrix*,
                                                  const SkPaint*, SkImage::BitDepth,
                                                  sk_sp<SkColorSpace>);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options& opts)
                     override;

#if SK_SUPPORT_GPU
    TexGenType onCanGenerateTexture() const override { return TexGenType::kExpensive; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            SkTransferFunctionBehavior) override;
#endif

private:
    SkPictureImageGenerator(const SkImageInfo& info, sk_sp<SkPicture>, const SkMatrix*,
                            const SkPaint*);

    sk_sp<SkPicture>    fPicture;
    SkMatrix            fMatrix;
    SkTLazy<SkPaint>    fPaint;

    typedef SkImageGenerator INHERITED;
};
#endif  // SkPictureImageGenerator_DEFINED
