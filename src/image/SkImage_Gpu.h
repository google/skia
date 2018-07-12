/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "GrClip.h"
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "SkAtomics.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkSurface.h"

class GrTexture;

class SkImage_Gpu : public SkImage_Base {
public:
    SkImage_Gpu(sk_sp<GrContext>, uint32_t uniqueID, SkAlphaType, sk_sp<GrTextureProxy>,
                sk_sp<SkColorSpace>, SkBudgeted);
    ~SkImage_Gpu() override;

    SkImageInfo onImageInfo() const override;
    SkColorType onColorType() const override;
    SkAlphaType onAlphaType() const override { return fAlphaType; }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    GrContext* context() const override { return fContext.get(); }
    GrTextureProxy* peekProxy() const override {
        return fProxy.get();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef() const override {
        return fProxy;
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerState&, SkColorSpace*,
                                            sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const override;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const override {
        *uniqueID = this->uniqueID();
        return fProxy;
    }

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const override;

    GrTexture* onGetTexture() const override;

    bool onReadPixels(const SkImageInfo&, void* dstPixels, size_t dstRowBytes,
                      int srcX, int srcY, CachingHint) const override;

    sk_sp<SkColorSpace> refColorSpace() { return fColorSpace; }

    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>, SkColorType) const override;

    typedef ReleaseContext TextureContext;
    typedef void (*TextureFulfillProc)(TextureContext textureContext, GrBackendTexture* outTexture);
    typedef void (*PromiseDoneProc)(TextureContext textureContext);

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

    /** Implementation of MakeFromYUVTexturesCopy and MakeFromNV12TexturesCopy */
    static sk_sp<SkImage> MakeFromYUVATexturesCopyImpl(GrContext* ctx,
                                                       SkYUVColorSpace colorSpace,
                                                       const GrBackendTexture yuvaTextures[],
                                                       SkYUVAIndex yuvaIndices[4],
                                                       SkISize size,
                                                       GrSurfaceOrigin origin,
                                                       sk_sp<SkColorSpace> imageColorSpace);

    bool onIsValid(GrContext*) const override;

    void resetContext(sk_sp<GrContext> newContext) {
        SkASSERT(fContext->uniqueID() == newContext->uniqueID());
        fContext = newContext;
    }

private:
    sk_sp<GrContext>       fContext;
    sk_sp<GrTextureProxy>  fProxy;
    const SkAlphaType      fAlphaType;
    const SkBudgeted       fBudgeted;
    sk_sp<SkColorSpace>    fColorSpace;
    mutable SkAtomic<bool> fAddedRasterVersionToCache;


    typedef SkImage_Base INHERITED;
};

#endif
