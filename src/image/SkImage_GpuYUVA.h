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
class SkImage_GpuYUVA final : public SkImage_GpuBase {
public:
    friend class GrYUVAImageTextureMaker;

    SkImage_GpuYUVA(sk_sp<GrImageContext>,
                    uint32_t uniqueID,
                    GrYUVATextureProxies proxies,
                    sk_sp<SkColorSpace>);

    bool onHasMipmaps() const override;

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) override;

    bool onIsTextureBacked() const override {
        // We should have YUVA proxies or a RGBA proxy,but not both.
        SkASSERT(fYUVAProxies.isValid() != SkToBool(fRGBView));
        return true;
    }

    size_t onTextureSize() const override;

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    bool isYUVA() const override { return true; }

    bool setupMipmapsForPlanes(GrRecordingContext*) const;

#if GR_TEST_UTILS
    bool testingOnly_IsFlattened() const {
        // We should only have the flattened proxy or the planar proxies at one point in time.
        SkASSERT(SkToBool(fRGBView) != fYUVAProxies.isValid());
        return SkToBool(fRGBView.proxy());
    }
#endif

private:
    SkImage_GpuYUVA(sk_sp<GrImageContext>, const SkImage_GpuYUVA* image, sk_sp<SkColorSpace>);

    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         GrMipmapped,
                                                         GrImageTexGenPolicy) const override;

    bool flattenToRGB(GrRecordingContext*, GrMipmapped) const;

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
