/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkCrossContextImageData_DEFINED
#define SkCrossContextImageData_DEFINED

#include "SkImage.h"

#if SK_SUPPORT_GPU
#include "GrBackendSurface.h"
#include "GrExternalTextureData.h"
#endif

class SK_API SkCrossContextImageData : SkNoncopyable {
public:
    /**
     *  Decodes and uploads the encoded data to a texture using the supplied GrContext, then
     *  returns an instance of SkCrossContextImageData that can be used to transport that texture
     *  to a different GrContext, across thread boundaries. The GrContext used here, and the one
     *  used to reconstruct the texture-backed image later must be in the same GL share group,
     *  or otherwise be able to share resources. After calling this, you *must* construct exactly
     *  one SkImage from the returned value, using SkImage::MakeFromCrossContextImageData.
     *
     *  The texture will be decoded and uploaded to be suitable for use with surfaces that have the
     *  supplied destination color space. The color space of the texture itself will be determined
     *  from the encoded data.
     */
    static std::unique_ptr<SkCrossContextImageData> MakeFromEncoded(
        GrContext*, sk_sp<SkData>, SkColorSpace* dstColorSpace);

    virtual ~SkCrossContextImageData() {}

protected:
    SkCrossContextImageData() {}

private:
    virtual sk_sp<SkImage> makeImage(GrContext*) = 0;

    friend class SkImage;
};

class SkCCIDImage : public SkCrossContextImageData {
public:
    ~SkCCIDImage() override {}

private:
    SkCCIDImage(sk_sp<SkImage> image) : fImage(std::move(image)) {
        SkASSERT(!fImage->isTextureBacked());
    }

    sk_sp<SkImage> makeImage(GrContext*) override {
        return fImage;
    }

    sk_sp<SkImage> fImage;

    friend class SkCrossContextImageData;
    friend class SkImage;
};

#if SK_SUPPORT_GPU
class SkCCIDBackendTexture : public SkCrossContextImageData {
public:
    ~SkCCIDBackendTexture() override {}

private:
    SkCCIDBackendTexture(const GrBackendTexture& backendTex,
                         GrSurfaceOrigin origin,
                         std::unique_ptr<GrExternalTextureData> textureData,
                         SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace)
            : fAlphaType(alphaType)
            , fColorSpace(std::move(colorSpace))
            , fBackendTex(backendTex)
            , fOrigin(origin)
            , fTextureData(std::move(textureData)) {}

    sk_sp<SkImage> makeImage(GrContext*) override;

    // GPU-backed images store some generic information (needed to reconstruct the SkImage),
    // and some backend-specific info (to reconstruct the texture).
    SkAlphaType fAlphaType;
    sk_sp<SkColorSpace> fColorSpace;
    GrBackendTexture fBackendTex;
    GrSurfaceOrigin fOrigin;
    std::unique_ptr<GrExternalTextureData> fTextureData;

    friend class SkCrossContextImageData;
};
#endif

#endif
