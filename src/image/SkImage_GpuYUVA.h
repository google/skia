/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuYUVA_DEFINED
#define SkImage_GpuYUVA_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkCachedData.h"
#include "src/gpu/GrYUVATextureProxies.h"
#include "src/image/SkImage_GpuBase.h"

class GrDirectContext;
class GrRecordingContext;
class GrTexture;

// Wraps the 1 to 4 planes of a YUVA image for consumption by the GPU.
// Initially any direct rendering will be done by passing the individual planes to a shader.
// Once any method requests a flattened image (e.g., onReadPixels), the flattened RGB
// proxy will be stored and used for any future rendering.
class SkImage_GpuYUVA : public SkImage_GpuBase {
public:
    friend class GrYUVAImageTextureMaker;

    SkImage_GpuYUVA(sk_sp<GrImageContext>,
                    uint32_t uniqueID,
                    GrYUVATextureProxies proxies,
                    sk_sp<SkColorSpace>);

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) override;

    // This returns the single backing proxy if the YUV channels have already been flattened but
    // nullptr if they have not.
    GrTextureProxy* peekProxy() const override;

    const GrSurfaceProxyView* view(GrRecordingContext* context) const override;

    bool onIsTextureBacked() const override {
        // We should have YUVA proxies or a RGBA proxy,but not both.
        SkASSERT(fYUVAProxies.isValid() != SkToBool(fRGBView));
        return true;
    }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    bool isYUVA() const override { return true; }

    bool setupMipmapsForPlanes(GrRecordingContext*) const;

    // Returns a ref-ed texture proxy view with miplevels
    GrSurfaceProxyView refMippedView(GrRecordingContext*) const;

#if GR_TEST_UTILS
    bool testingOnly_IsFlattened() const {
        // We should only have the flattened proxy or the planar proxies at one point in time.
        SkASSERT(SkToBool(fRGBView) != fYUVAProxies.isValid());
        return SkToBool(fRGBView.proxy());
    }
#endif

    /**
     * This is the implementation of SkDeferredDisplayListRecorder::makeYUVAPromiseTexture.
     */
    static sk_sp<SkImage> MakePromiseYUVATexture(GrRecordingContext*,
                                                 const GrYUVABackendTextureInfo&,
                                                 sk_sp<SkColorSpace> imageColorSpace,
                                                 PromiseImageTextureFulfillProc textureFulfillProc,
                                                 PromiseImageTextureReleaseProc textureReleaseProc,
                                                 PromiseImageTextureContext textureContexts[]);

private:
    SkImage_GpuYUVA(sk_sp<GrImageContext>, const SkImage_GpuYUVA* image, sk_sp<SkColorSpace>);

    void flattenToRGB(GrRecordingContext*) const;

    mutable GrYUVATextureProxies     fYUVAProxies;

    // This is only allocated when the image needs to be flattened rather than
    // using the separate YUVA planes. From thence forth we will only use the
    // the RGBView.
    mutable GrSurfaceProxyView       fRGBView;

    // If this is non-null then the planar data should be converted from fFromColorSpace to
    // this->colorSpace(). Otherwise we assume the planar data (post YUV->RGB conversion) is already
    // in this->colorSpace().
    const sk_sp<SkColorSpace>        fFromColorSpace;

    // Repeated calls to onMakeColorSpace will result in a proliferation of unique IDs and
    // SkImage_GpuYUVA instances. Cache the result of the last successful onMakeColorSpace call.
    mutable sk_sp<SkColorSpace>      fOnMakeColorSpaceTarget;
    mutable sk_sp<SkImage>           fOnMakeColorSpaceResult;

    using INHERITED = SkImage_GpuBase;
};

#endif
