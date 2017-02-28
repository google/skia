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

private:
    SkCrossContextImageData(sk_sp<SkImage> image) : fImage(std::move(image)) {
        SkASSERT(!fImage->isTextureBacked());
    }

#if SK_SUPPORT_GPU
    SkCrossContextImageData(const GrBackendTextureDesc& desc,
                            std::unique_ptr<GrExternalTextureData> textureData,
                            SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace)
            : fAlphaType(alphaType)
            , fColorSpace(std::move(colorSpace))
            , fDesc(desc)
            , fTextureData(std::move(textureData)) {
        // Point our texture desc at our copy of the backend information
        fDesc.fTextureHandle = fTextureData->getBackendObject();
    }
#endif

    // For non-GPU backed images
    sk_sp<SkImage> fImage;

#if SK_SUPPORT_GPU
    // GPU-backed images store some generic information (needed to reconstruct the SkImage),
    // and some backend-specific info (to reconstruct the texture).
    SkAlphaType fAlphaType;
    sk_sp<SkColorSpace> fColorSpace;
    GrBackendTextureDesc fDesc;
    std::unique_ptr<GrExternalTextureData> fTextureData;
#endif

    friend class SkImage;
};

#endif
