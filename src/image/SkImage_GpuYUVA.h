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

class GrTexture;
struct SkYUVASizeInfo;

// Wraps the 3 or 4 planes of a YUVA image for consumption by the GPU.
// Initially any direct rendering will be done by passing the individual planes to a shader.
// Once any method requests a flattened image (e.g., onReadPixels), the flattened RGB
// proxy will be stored and used for any future rendering.
class SkImage_GpuYUVA : public SkImage_GpuBase {
public:
    friend class GrYUVAImageTextureMaker;

    SkImage_GpuYUVA(sk_sp<GrContext>, int width, int height, uint32_t uniqueID, SkYUVColorSpace,
                    sk_sp<GrTextureProxy> proxies[], int numProxies, const SkYUVAIndex[4],
                    GrSurfaceOrigin, sk_sp<SkColorSpace>);
    ~SkImage_GpuYUVA() override;

    SkImageInfo onImageInfo() const override;

    GrTextureProxy* peekProxy() const override { return this->asTextureProxyRef().get(); }
    sk_sp<GrTextureProxy> asTextureProxyRef() const override;

    virtual bool onIsTextureBacked() const override { return SkToBool(fProxies[0].get()); }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>) const final;

    virtual bool isYUVA() const override { return true; }
    virtual bool asYUVATextureProxiesRef(sk_sp<GrTextureProxy> proxies[4],
                                         SkYUVAIndex yuvaIndices[4],
                                         SkYUVColorSpace* yuvColorSpace) const override {
        for (int i = 0; i < 4; ++i) {
            proxies[i] = fProxies[i];
            yuvaIndices[i] = fYUVAIndices[i];
        }
        *yuvColorSpace = fYUVColorSpace;
        return true;
    }

    bool setupMipmapsForPlanes() const;

    // Returns a ref-ed texture proxy with miplevels
    sk_sp<GrTextureProxy> asMippedTextureProxyRef() const;

    SkColorSpace* targetColorSpace() const { return fTargetColorSpace.get(); }

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
        @param yuvaSizes           width and height of promised gpu textures
        @param yuvaIndices         mapping from yuv plane index to texture representing that plane
        @param width               width of promised gpu texture
        @param height              height of promised gpu texture
        @param imageOrigin         one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param imageColorSpace     range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be released
        @param textureDoneProc     function called when we will no longer call textureFulfillProc
        @param textureContexts     per-texture state passed to textureFulfillProc,
                                   textureReleaseProc, and textureDoneProc
        @return                    created SkImage, or nullptr
     */
    static sk_sp<SkImage> MakePromiseYUVATexture(GrContext* context,
                                                 SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendFormat yuvaFormats[],
                                                 const SkISize yuvaSizes[],
                                                 const SkYUVAIndex yuvaIndices[4],
                                                 int width,
                                                 int height,
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace,
                                                 PromiseImageTextureFulfillProc textureFulfillProc,
                                                 PromiseImageTextureReleaseProc textureReleaseProc,
                                                 PromiseImageTextureDoneProc textureDoneProc,
                                                 PromiseImageTextureContext textureContexts[]);

private:
    SkImage_GpuYUVA(const SkImage_GpuYUVA* image, sk_sp<SkColorSpace>);

    // This array will usually only be sparsely populated.
    // The actual non-null fields are dictated by the 'fYUVAIndices' indices
    mutable sk_sp<GrTextureProxy>    fProxies[4];
    int                              fNumProxies;
    SkYUVAIndex                      fYUVAIndices[4];
    const SkYUVColorSpace            fYUVColorSpace;
    GrSurfaceOrigin                  fOrigin;
    const sk_sp<SkColorSpace>        fTargetColorSpace;

    // Repeated calls to onMakeColorSpace will result in a proliferation of unique IDs and
    // SkImage_GpuYUVA instances. Cache the result of the last successful onMakeColorSpace call.
    mutable sk_sp<SkColorSpace>      fOnMakeColorSpaceTarget;
    mutable sk_sp<SkImage>           fOnMakeColorSpaceResult;

    // This is only allocated when the image needs to be flattened rather than
    // using the separate YUVA planes. From thence forth we will only use the
    // the RGBProxy.
    mutable sk_sp<GrTextureProxy>    fRGBProxy;
    typedef SkImage_GpuBase INHERITED;
};

#endif
