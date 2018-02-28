/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImage_chrome_DEFINED
#define SkImage_chrome_DEFINED

#include "GrTypes.h"
#include "SkColorSpace.h"
#include "SkRefCnt.h"
#include "SkImage.h"
#include "SkSize.h"
#include "SkYUVImageInfo.h"

class SkSurface;
class GrBackendTexture;
class GrContext;

#ifndef SK_SUPPORT_LEGACY_YUV_COLORSPACE

class SK_API SkYUVImage {
public:
    /**
     *  Create a new image by copying the pixels from the specified y, u, v textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromYUVTexturesCopy(GrContext* context,
                                                  SkYUVColorSpace yuvColorSpace,
                                                  const GrBackendObject yuvTextureHandles[3],
                                                  const SkISize yuvSizes[3],
                                                  GrSurfaceOrigin surfaceOrigin,
                                                  sk_sp<SkColorSpace> colorSpace = nullptr);

    /**
     *  Create a new image by copying the pixels from the specified y and uv textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromNV12TexturesCopy(GrContext* context,
                                                   SkYUVColorSpace yuvColorSpace,
                                                   const GrBackendObject nv12TextureHandles[2],
                                                   const SkISize nv12Sizes[2],
                                                   GrSurfaceOrigin surfaceOrigin,
                                                   sk_sp<SkColorSpace> colorSpace = nullptr);

    /**
     *  Create a new image by copying the pixels from the specified y, u, v textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromYUVTexturesCopy(GrContext* context,
                                                  SkYUVColorSpace yuvColorSpace,
                                                  const GrBackendTexture yuvTextureHandles[3],
                                                  const SkISize yuvSizes[3],
                                                  GrSurfaceOrigin surfaceOrigin,
                                                  sk_sp<SkColorSpace> colorSpace = nullptr);

    /**
     *  Create a new image by copying the pixels from the specified y and uv textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromNV12TexturesCopy(GrContext* context,
                                                   SkYUVColorSpace yuvColorSpace,
                                                   const GrBackendTexture nv12TextureHandles[2],
                                                   const SkISize nv12Sizes[2],
                                                   GrSurfaceOrigin surfaceOrigin,
                                                   sk_sp<SkColorSpace> colorSpace = nullptr);
};

#endif

#endif
