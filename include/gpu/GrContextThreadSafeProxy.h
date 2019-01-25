/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextThreadSafeProxy_DEFINED
#define GrContextThreadSafeProxy_DEFINED

#include "GrContext_Base.h"

class GrContextThreadSafeProxyPriv;
class SkSurfaceCharacterization;
struct SkYUVAIndex;

/**
 * Can be used to perform actions related to the generating GrContext in a thread safe manner. The
 * proxy does not access the 3D API (e.g. OpenGL) that backs the generating GrContext.
 */
class SK_API GrContextThreadSafeProxy : public GrContext_Base {
public:
    ~GrContextThreadSafeProxy() override;

    sk_sp<GrContextThreadSafeProxy> threadSafeProxy() override;
#if 0
    {
        SkASSERT(!INHERITED::threadSafeProxy());   // we don't want a ref counting loop
        return sk_ref_sp<GrContextThreadSafeProxy>(this);
    }
#endif

    /**
     *  Create a surface characterization for a DDL that will be replayed into the GrContext
     *  that created this proxy. On failure the resulting characterization will be invalid (i.e.,
     *  "!c.isValid()").
     *
     *  @param cacheMaxResourceBytes The max resource bytes limit that will be in effect when the
     *                               DDL created with this characterization is replayed.
     *                               Note: the contract here is that the DDL will be created as
     *                               if it had a full 'cacheMaxResourceBytes' to use. If replayed
     *                               into a GrContext that already has locked GPU memory, the
     *                               replay can exceed the budget. To rephrase, all resource
     *                               allocation decisions are made at record time and at playback
     *                               time the budget limits will be ignored.
     *  @param ii                    The image info specifying properties of the SkSurface that
     *                               the DDL created with this characterization will be replayed
     *                               into.
     *                               Note: Ganesh doesn't make use of the SkImageInfo's alphaType
     *  @param backendFormat         Information about the format of the GPU surface that will
     *                               back the SkSurface upon replay
     *  @param sampleCount           The sample count of the SkSurface that the DDL created with
     *                               this characterization will be replayed into
     *  @param origin                The origin of the SkSurface that the DDL created with this
     *                               characterization will be replayed into
     *  @param surfaceProps          The surface properties of the SkSurface that the DDL created
     *                               with this characterization will be replayed into
     *  @param isMipMapped           Will the surface the DDL will be replayed into have space
     *                               allocated for mipmaps?
     *  @param willUseGLFBO0         Will the surface the DDL will be replayed into be backed by GL
     *                               FBO 0. This flag is only valid if using an GL backend.
     */
    SkSurfaceCharacterization createCharacterization(
                                  size_t cacheMaxResourceBytes,
                                  const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                  int sampleCount, GrSurfaceOrigin origin,
                                  const SkSurfaceProps& surfaceProps,
                                  bool isMipMapped, bool willUseGLFBO0 = false);

    // Matches the defines in SkImage_GpuBase.h
    typedef void* TextureContext;
    typedef void (*TextureReleaseProc)(TextureContext textureContext);
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

        We call the promiseDoneProc when we will no longer call the textureFulfillProc again. We
        pass in the textureContext as a parameter to the promiseDoneProc. We also guarantee that
        there will be no outstanding textureReleaseProcs that still need to be called when we call
        the textureDoneProc. Thus when the textureDoneProc gets called the client is able to cleanup
        all GPU objects and meta data needed for the textureFulfill call.

        This call is only valid if the SkDeferredDisplayListRecorder is backed by a gpu context.

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
    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
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

    /**
        This entry point operates the same as 'makePromiseTexture' except that its
        textureFulfillProc can be called up to four times to fetch the required YUVA
        planes (passing a different textureContext to each call). So, if the 'yuvaIndices'
        indicate that only the first two backend textures are used, 'textureFulfillProc' will
        be called with the first two 'textureContexts'.
     */
    sk_sp<SkImage> makeYUVAPromiseTexture(SkYUVColorSpace yuvColorSpace,
                                          const GrBackendFormat yuvaFormats[],
                                          const SkISize yuvaSizes[],
                                          const SkYUVAIndex yuvaIndices[4],
                                          int imageWidth,
                                          int imageHeight,
                                          GrSurfaceOrigin imageOrigin,
                                          sk_sp<SkColorSpace> imageColorSpace,
                                          TextureFulfillProc textureFulfillProc,
                                          TextureReleaseProc textureReleaseProc,
                                          PromiseDoneProc promiseDoneProc,
                                          TextureContext textureContexts[]);

    bool operator==(const GrContextThreadSafeProxy& that) const {
        // Each GrContext should only ever have a single thread-safe proxy.
        SkASSERT((this == &that) == (this->uniqueID() == that.uniqueID()));
        return this == &that;
    }

    bool operator!=(const GrContextThreadSafeProxy& that) const { return !(*this == that); }

    // Provides access to functions that aren't part of the public API.
    GrContextThreadSafeProxyPriv priv();
    const GrContextThreadSafeProxyPriv priv() const;

    static sk_sp<GrContextThreadSafeProxy> Make(GrBackendApi backend,
                                                uint32_t uniqueID,
                                                sk_sp<const GrCaps> caps,
                                                const GrContextOptions& options,
                                                sk_sp<GrSkSLFPFactoryCache> cache);
#if 0
    {
        sk_sp<GrContextThreadSafeProxy> context(new GrContextThreadSafeProxy(backend, options, uniqueID));

        if (!context->initWeakest(std::move(caps), nullptr, std::move(cache))) {
            return nullptr;
        }

        return context;
    }
#endif

private:
    GrContextThreadSafeProxy(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

#if 0
    // DDL TODO: need to add unit tests for backend & maybe options
    GrContextThreadSafeProxy(sk_sp<const GrCaps> caps,
                             uint32_t uniqueID,
                             GrBackendApi backend,
                             const GrContextOptions& options,
                             sk_sp<GrSkSLFPFactoryCache> cache);
#endif

    sk_sp<GrSkSLFPFactoryCache> fFPFactoryCache;

    friend class GrContextThreadSafeProxyPriv;

    typedef GrContext_Base INHERITED;
};

#endif
