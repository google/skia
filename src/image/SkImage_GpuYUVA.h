/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuYUVA_DEFINED
#define SkImage_GpuYUVA_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_GpuBase.h"

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

    GrSemaphoresSubmitted onFlush(GrContext*, const GrFlushInfo&) override;

    // This returns the single backing proxy if the YUV channels have already been flattened but
    // nullptr if they have not.
    GrTextureProxy* peekProxy() const override;
    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*) const override;

    virtual bool onIsTextureBacked() const override { return fProxies[0] || fRGBProxy; }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                SkColorType, sk_sp<SkColorSpace>) const final;

    virtual bool isYUVA() const override { return true; }

    bool setupMipmapsForPlanes(GrRecordingContext*) const;

    // Returns a ref-ed texture proxy with miplevels
    sk_sp<GrTextureProxy> asMippedTextureProxyRef(GrRecordingContext*) const;

#if GR_TEST_UTILS
    bool testingOnly_IsFlattened() const {
        // We should only have the flattened proxy or the planar proxies at one point in time.
        SkASSERT(SkToBool(fRGBProxy) != SkToBool(fProxies[0]));
        return SkToBool(fRGBProxy);
    }
#endif

    /**
     * This is the implementation of SkDeferredDisplayListRecorder::makeYUVAPromiseTexture.
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
                                                 PromiseImageTextureContext textureContexts[],
                                                 PromiseImageApiVersion);

private:
    SkImage_GpuYUVA(const SkImage_GpuYUVA* image, sk_sp<SkColorSpace>);

    // This array will usually only be sparsely populated.
    // The actual non-null fields are dictated by the 'fYUVAIndices' indices
    mutable sk_sp<GrTextureProxy>    fProxies[4];
    int                              fNumProxies;
    SkYUVAIndex                      fYUVAIndices[4];
    const SkYUVColorSpace            fYUVColorSpace;
    GrSurfaceOrigin                  fOrigin;
    // If this is non-null then the planar data should be converted from fFromColorSpace to
    // this->colorSpace(). Otherwise we assume the planar data (post YUV->RGB conversion) is already
    // in this->colorSpace().
    const sk_sp<SkColorSpace> fFromColorSpace;

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
