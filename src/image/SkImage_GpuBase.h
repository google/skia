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
#include "include/gpu/GrContext.h"
#include "include/private/GrTypesPriv.h"
#include "src/image/SkImage_Base.h"

class GrColorSpaceXform;
class SkColorSpace;

class SkImage_GpuBase : public SkImage_Base {
public:
    GrContext* context() const final { return fContext.get(); }

    bool getROPixels(SkBitmap*, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(GrRecordingContext*, const SkIRect& subset) const final;

    bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                      int srcX, int srcY, CachingHint) const override;

    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext* context) const override {
        // we shouldn't end up calling this
        SkASSERT(false);
        return this->INHERITED::asTextureProxyRef(context);
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*, const GrSamplerState&,
                                            SkScalar scaleAdjust[2]) const final;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(GrRecordingContext* context,
                                                uint32_t* uniqueID) const final {
        *uniqueID = this->uniqueID();
        return this->asTextureProxyRef(context);
    }

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    GrTexture* onGetTexture() const final;

    bool onIsValid(GrContext*) const final;

#if GR_TEST_UTILS
    void resetContext(sk_sp<GrContext> newContext);
#endif

    static bool ValidateBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                       GrColorType grCT, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
    static bool MakeTempTextureProxies(GrContext* ctx, const GrBackendTexture yuvaTextures[],
                                       int numTextures, const SkYUVAIndex [4],
                                       GrSurfaceOrigin imageOrigin,
                                       sk_sp<GrTextureProxy> tempTextureProxies[4]);

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
    SkImage_GpuBase(sk_sp<GrContext>, int width, int height, uint32_t uniqueID, SkColorType,
                    SkAlphaType, sk_sp<SkColorSpace>);

    using PromiseImageApiVersion = SkDeferredDisplayListRecorder::PromiseImageApiVersion;
    // Helper for making a lazy proxy for a promise image. The PromiseDoneProc we be called,
    // if not null, immediately if this function fails. Othwerwise, it is installed in the
    // proxy along with the TextureFulfillProc and TextureReleaseProc. PromiseDoneProc must not
    // be null.
    static sk_sp<GrTextureProxy> MakePromiseImageLazyProxy(
            GrContext*, int width, int height, GrSurfaceOrigin, GrColorType, GrBackendFormat,
            GrMipMapped, PromiseImageTextureFulfillProc, PromiseImageTextureReleaseProc,
            PromiseImageTextureDoneProc, PromiseImageTextureContext, PromiseImageApiVersion);

    static bool RenderYUVAToRGBA(GrContext* ctx, GrRenderTargetContext* renderTargetContext,
                                 const SkRect& rect, SkYUVColorSpace yuvColorSpace,
                                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                                 const sk_sp<GrTextureProxy> proxies[4],
                                 const SkYUVAIndex yuvaIndices[4]);

    sk_sp<GrContext> fContext;

private:
    typedef SkImage_Base INHERITED;
};

#endif
