/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "SkGr.h"
#include "SkImagePriv.h"
#include "SkImage_GpuBase.h"

class GrTexture;

class SkBitmap;
struct SkYUVAIndex;

class SkImage_Gpu : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrContext>, uint32_t uniqueID, SkAlphaType, sk_sp<GrTextureProxy>,
                sk_sp<SkColorSpace>, SkBudgeted);
    ~SkImage_Gpu() override;

    SkImageInfo onImageInfo() const override;

    GrTextureProxy* peekProxy() const override {
        return fProxy.get();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef() const override {
        return fProxy;
    }

    sk_sp<SkColorSpace> refColorSpace() { return fColorSpace; }

    /**
        Create a new SkImage that is very similar to an SkImage created by MakeFromTexture. The main
        difference is that the client doesn't have the backend texture on the gpu yet but they know
        all the properties of the texture. So instead of passing in a GrBackendTexture the client
        supplies a GrBackendFormat, width, height, and GrMipMapped state.

        When we actually send the draw calls to the GPU, we will call the textureFulfillProc and
        the client will return a GrBackendTexture to us. The properties of the GrBackendTexture must
        match those set during the SkImage creation, and it must have a valid backend gpu texture.
        The gpu texture supplied by the client must stay valid until we call the textureReleaseProc.

        When we are done with the texture returned by the textureFulfillProc we will call the
        textureReleaseProc passing in the textureContext. This is a signal to the client that they
        are free to delete the underlying gpu texture. If future draws also use the same promise
        image we will call the textureFulfillProc again if we've already called the
        textureReleaseProc. We will always call textureFulfillProc and textureReleaseProc in pairs.
        In other words we will never call textureFulfillProc or textureReleaseProc multiple times
        for the same textureContext before calling the other.

        We we call the promiseDoneProc when we will no longer call the textureFulfillProc again. We
        also guarantee that there will be no outstanding textureReleaseProcs that still need to be
        called when we call the textureDoneProc. Thus when the textureDoneProc gets called the
        client is able to cleanup all GPU objects and meta data needed for the textureFulfill call.

        @param context             Gpu context
        @param backendFormat       format of promised gpu texture
        @param width               width of promised gpu texture
        @param height              height of promised gpu texture
        @param mipMapped           mip mapped state of promised gpu texture
        @param origin              one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param colorType           one of: kUnknown_SkColorType, kAlpha_8_SkColorType,
                                   kRGB_565_SkColorType, kARGB_4444_SkColorType,
                                   kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                   kGray_8_SkColorType, kRGBA_F16_SkColorType
        @param alphaType           one of: kUnknown_SkAlphaType, kOpaque_SkAlphaType,
                                   kPremul_SkAlphaType, kUnpremul_SkAlphaType
        @param colorSpace          range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be released
        @param promiseDoneProc     function called when we will no longer call textureFulfillProc
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @return                    created SkImage, or nullptr
     */
    static sk_sp<SkImage> MakePromiseTexture(GrContext* context,
                                             const GrBackendFormat& backendFormat,
                                             int width,
                                             int height,
                                             GrMipMapped mipMapped,
                                             GrSurfaceOrigin origin,
                                             SkColorType colorType,
                                             SkAlphaType alphaType,
                                             sk_sp<SkColorSpace> colorSpace,
                                             TextureFulfillProc textureFulfillProc,
                                             TextureReleaseProc textureReleaseProc,
                                             PromiseDoneProc promiseDoneProc,
                                             TextureContext textureContext);

    static sk_sp<SkImage> MakePromiseYUVATexture(GrContext* context,
                                                 SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendFormat yuvaFormats[],
                                                 const SkYUVAIndex yuvaIndices[4],
                                                 int imageWidth,
                                                 int imageHeight,
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace,
                                                 TextureFulfillProc textureFulfillProc,
                                                 TextureReleaseProc textureReleaseProc,
                                                 PromiseDoneProc promiseDoneProc,
                                                 TextureContext textureContexts[]);

    void resetContext(sk_sp<GrContext> newContext) {
        SkASSERT(fContext->uniqueID() == newContext->uniqueID());
        fContext = newContext;
    }

    static sk_sp<SkImage> ConvertYUVATexturesToRGB(
            GrContext*, SkYUVColorSpace yuvColorSpace, const GrBackendTexture yuvaTextures[],
            const SkYUVAIndex yuvaIndices[4], SkISize imageSize, GrSurfaceOrigin imageOrigin,
            SkBudgeted, GrRenderTargetContext*);

private:
    sk_sp<GrTextureProxy> fProxy;

    typedef SkImage_GpuBase INHERITED;
};

#endif
