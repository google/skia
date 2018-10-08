/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuYUVA_DEFINED
#define SkImage_GpuYUVA_DEFINED

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SkCachedData.h"
#include "SkImage_GpuBase.h"
#include "SkYUVAIndex.h"

class GrTexture;

// Wraps the 3 or 4 planes of a YUVA image for consumption by the GPU.
// Initially any direct rendering will be done by passing the individual planes to a shader.
// Once any method requests a flattened image (e.g., onReadPixels), the flattened RGB
// proxy will be stored and used for any future rendering.
class SkImage_GpuYUVA : public SkImage_GpuBase {
public:
    SkImage_GpuYUVA(sk_sp<GrContext>, uint32_t uniqueID, SkYUVColorSpace,
                    sk_sp<GrTextureProxy> proxies[], const SkYUVAIndex yuvaIndices[4], SkISize size,
                    GrSurfaceOrigin, sk_sp<SkColorSpace>, SkBudgeted);
    ~SkImage_GpuYUVA() override;

    SkImageInfo onImageInfo() const override;

    GrTextureProxy* peekProxy() const override { return this->asTextureProxyRef().get(); }
    sk_sp<GrTextureProxy> asTextureProxyRef() const override;

    /**
        Create a new SkImage_GpuYUVA that's very similar to SkImage created by MakeFromYUVATextures.
        The main difference is that the client doesn't have the backend textures on the gpu yet but
        they know all the properties of the texture. So instead of passing in GrBackendTextures the
        client supplies GrBackendFormats and the image size.

        When we actually send the draw calls to the GPU, we will call the textureFulfillProc and
        the client will return the GrBackendTextures to us. The properties of the GrBackendTextures
        must match those set during the SkImage creation, and it must have valid backend gpu
        textures. The gpu textures supplied by the client must stay valid until we call the
        textureReleaseProc.

        When we are done with the texture returned by the textureFulfillProc we will call the
        textureReleaseProc passing in the textureContext. This is a signal to the client that they
        are free to delete the underlying gpu textures. If future draws also use the same promise
        image we will call the textureFulfillProc again if we've already called the
        textureReleaseProc. We will always call textureFulfillProc and textureReleaseProc in pairs.
        In other words we will never call textureFulfillProc or textureReleaseProc multiple times
        for the same textureContext before calling the other.

        We call the promiseDoneProc when we will no longer call the textureFulfillProc again. We
        also guarantee that there will be no outstanding textureReleaseProcs that still need to be
        called when we call the textureDoneProc. Thus when the textureDoneProc gets called the
        client is able to cleanup all GPU objects and meta data needed for the textureFulfill call.

        @param context             Gpu context
        @param yuvColorSpace       color range of expected YUV pixels
        @param yuvaFormats         formats of promised gpu textures for each YUVA plane
        @param yuvaIndices         mapping from yuv plane index to texture representing that plane
        @param imageSize           width and height of promised gpu texture
        @param imageOrigin         one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param imageColorSpace     range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be released
        @param promiseDoneProc     function called when we will no longer call textureFulfillProc
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @return                    created SkImage, or nullptr
     */
    static sk_sp<SkImage> MakePromiseYUVATexture(GrContext* context,
                                                 SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendFormat yuvaFormats[],
                                                 const SkYUVAIndex yuvaIndices[4],
                                                 SkISize imageSize,
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace,
                                                 TextureFulfillProc textureFulfillProc,
                                                 TextureReleaseProc textureReleaseProc,
                                                 PromiseDoneProc promiseDoneProc,
                                                 TextureContext textureContexts[]);

    static sk_sp<SkImage> MakeFromYUVATextures(GrContext* context,
                                               SkYUVColorSpace yuvColorSpace,
                                               const GrBackendTexture yuvaTextures[],
                                               SkYUVAIndex yuvaIndices[4],
                                               SkISize imageSize,
                                               GrSurfaceOrigin imageOrigin,
                                               sk_sp<SkColorSpace> imageColorSpace);

private:
    // This array will usually only be sparsely populated.
    // The actual non-null fields are dictated by the 'fYUVAIndices' indices
    sk_sp<GrTextureProxy>            fProxies[4];
    SkYUVAIndex                      fYUVAIndices[4];
    // This is only allocated when the image needs to be flattened rather than
    // using the separate YUVA planes. From thence forth we will only use the
    // the RGBProxy.
    sk_sp<GrTextureProxy>            fRGBProxy;
    const SkYUVColorSpace            fYUVColorSpace;
    GrSurfaceOrigin                  fOrigin;

    typedef SkImage_GpuBase INHERITED;
};

#endif
