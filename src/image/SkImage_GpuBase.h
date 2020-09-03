/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuBase_DEFINED
#define SkImage_GpuBase_DEFINED

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkYUVAIndex.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/GrTypesPriv.h"
#include "src/image/SkImage_Base.h"

class GrColorSpaceXform;
class GrContext;
class GrDirectContext;
class GrRenderTargetContext;
class SkColorSpace;

class SkImage_GpuBase : public SkImage_Base {
public:
    GrContext* context() const final { return fContext.get(); }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset, GrDirectContext*) const final;

    bool onReadPixels(GrDirectContext *dContext,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRB,
                      int srcX,
                      int srcY,
                      CachingHint) const override;

    GrSurfaceProxyView refView(GrRecordingContext*, GrMipmapped) const final;

    GrSurfaceProxyView refPinnedView(GrRecordingContext* context, uint32_t* uniqueID) const final {
        *uniqueID = this->uniqueID();
        SkASSERT(this->view(context));
        return *this->view(context);
    }

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    GrTexture* getTexture() const;

    bool onIsValid(GrRecordingContext*) const final;

#if GR_TEST_UTILS
    void resetContext(sk_sp<GrContext> newContext);
#endif

    static bool ValidateBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                       GrColorType grCT, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
    static bool ValidateCompressedBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                                 SkAlphaType);
    static bool MakeTempTextureProxies(GrRecordingContext*, const GrBackendTexture yuvaTextures[],
                                       int numTextures, const SkYUVAIndex [4],
                                       GrSurfaceOrigin imageOrigin,
                                       GrSurfaceProxyView tempViews[4],
                                       sk_sp<GrRefCntedCallback> releaseHelper);

    static SkAlphaType GetAlphaTypeFromYUVAIndices(const SkYUVAIndex yuvaIndices[4]) {
        return -1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex ? kPremul_SkAlphaType
                                                               : kOpaque_SkAlphaType;
    }

    using PromiseImageTextureContext = SkDeferredDisplayListRecorder::PromiseImageTextureContext;
    using PromiseImageTextureFulfillProc =
            SkDeferredDisplayListRecorder::PromiseImageTextureFulfillProc;
    using PromiseImageTextureReleaseProc =
            SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc;
    using PromiseImageTextureDoneProc = SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc;

protected:
    SkImage_GpuBase(sk_sp<GrContext>, SkISize size, uint32_t uniqueID, SkColorType, SkAlphaType,
                    sk_sp<SkColorSpace>);

    using PromiseImageApiVersion = SkDeferredDisplayListRecorder::PromiseImageApiVersion;
    // Helper for making a lazy proxy for a promise image. The PromiseDoneProc we be called,
    // if not null, immediately if this function fails. Othwerwise, it is installed in the
    // proxy along with the TextureFulfillProc and TextureReleaseProc. PromiseDoneProc must not
    // be null.
    static sk_sp<GrTextureProxy> MakePromiseImageLazyProxy(
            GrRecordingContext*, int width, int height, GrBackendFormat, GrMipmapped,
            PromiseImageTextureFulfillProc, PromiseImageTextureReleaseProc,
            PromiseImageTextureDoneProc, PromiseImageTextureContext, PromiseImageApiVersion);

    static bool RenderYUVAToRGBA(const GrCaps&, GrRenderTargetContext*,
                                 const SkRect&, SkYUVColorSpace,
                                 sk_sp<GrColorSpaceXform>,
                                 GrSurfaceProxyView [4],
                                 const SkYUVAIndex [4]);

    // TODO: Migrate this to something much weaker, such as GrContextThreadSafeProxy.
    sk_sp<GrContext> fContext;

private:
    using INHERITED = SkImage_Base;
};

#endif
