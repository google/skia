/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuBase_DEFINED
#define SkImage_GpuBase_DEFINED

#include "GrBackendSurface.h"
#include "GrTypesPriv.h"
#include "SkImage_Base.h"
#include "SkYUVAIndex.h"

class GrContext;
class SkColorSpace;

class SkImage_GpuBase : public SkImage_Base {
public:
    SkImage_GpuBase(sk_sp<GrContext>, int width, int height, uint32_t uniqueID,
                    SkAlphaType, SkBudgeted, sk_sp<SkColorSpace>);
    ~SkImage_GpuBase() override;

    GrContext* context() const final { return fContext.get(); }

    bool getROPixels(SkBitmap*, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset) const final;

    bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                      int srcX, int srcY, CachingHint) const override;

    sk_sp<GrTextureProxy> asTextureProxyRef() const override {
        // we shouldn't end up calling this
        SkASSERT(false);
        return this->INHERITED::asTextureProxyRef();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerState&,
                                            SkScalar scaleAdjust[2]) const final;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const final {
        *uniqueID = this->uniqueID();
        return this->asTextureProxyRef();
    }

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    GrTexture* onGetTexture() const final;

    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>) const final;

    bool onIsValid(GrContext*) const final;

#if GR_TEST_UTILS
    void resetContext(sk_sp<GrContext> newContext) {
        SkASSERT(fContext->uniqueID() == newContext->uniqueID());
        fContext = newContext;
    }
#endif

    static bool ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                                       GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
    static bool MakeTempTextureProxies(GrContext* ctx, const GrBackendTexture yuvaTextures[],
                                       int numTextures, const SkYUVAIndex [4],
                                       GrSurfaceOrigin imageOrigin,
                                       sk_sp<GrTextureProxy> tempTextureProxies[4]);

    static SkAlphaType GetAlphaTypeFromYUVAIndices(const SkYUVAIndex yuvaIndices[4]) {
        return -1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex ? kPremul_SkAlphaType
                                                               : kOpaque_SkAlphaType;
    }

    typedef ReleaseContext TextureContext;
    typedef void(*TextureFulfillProc)(TextureContext textureContext, GrBackendTexture* outTexture);
    typedef void(*PromiseDoneProc)(TextureContext textureContext);

protected:
    // Helper for making a lazy proxy for a promise image. The PromiseDoneProc we be called,
    // if not null, immediately if this function fails. Othwerwise, it is installed in the
    // proxy along with the TextureFulfillProc and TextureReleaseProc. PromiseDoneProc must not
    // be null.
    static sk_sp<GrTextureProxy> MakePromiseImageLazyProxy(
            GrContext*, int width, int height, GrSurfaceOrigin, GrPixelConfig, GrBackendFormat,
            GrMipMapped, SkImage_GpuBase::TextureFulfillProc, SkImage_GpuBase::TextureReleaseProc,
            SkImage_GpuBase::PromiseDoneProc, SkImage_GpuBase::TextureContext);

    static bool RenderYUVAToRGBA(GrContext* ctx, GrRenderTargetContext* renderTargetContext,
                                 const SkRect& rect, SkYUVColorSpace yuvColorSpace,
                                 const sk_sp<GrTextureProxy> proxies[4],
                                 const SkYUVAIndex yuvaIndices[4]);

    sk_sp<GrContext>      fContext;
    const SkAlphaType     fAlphaType;  // alpha type for final image
    const SkBudgeted      fBudgeted;
    sk_sp<SkColorSpace>   fColorSpace; // color space for final image

private:
    typedef SkImage_Base INHERITED;
};


/**
 * This helper holds the normal hard ref for the Release proc as well as a hard ref on the DoneProc.
 * Thus when a GrTexture is being released, it will unref both the ReleaseProc and DoneProc.
 */
class SkPromiseReleaseProcHelper : public GrReleaseProcHelper {
public:
    SkPromiseReleaseProcHelper(SkImage_GpuBase::TextureReleaseProc releaseProc,
                               SkImage_GpuBase::TextureContext context,
                               sk_sp<GrReleaseProcHelper> doneHelper)
        : INHERITED(releaseProc, context)
        , fDoneProcHelper(std::move(doneHelper)) {
    }

    void weak_dispose() const override {
        // Call the inherited weak_dispose first so that we call the ReleaseProc before the DoneProc
        // if we hold the last ref to the DoneProc.
        INHERITED::weak_dispose();
        fDoneProcHelper.reset();
    }

private:
    mutable sk_sp<GrReleaseProcHelper> fDoneProcHelper;

    typedef GrReleaseProcHelper INHERITED;
};

#endif
