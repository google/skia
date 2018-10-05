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

class GrContext;
class SkColorSpace;

class SkImage_GpuBase : public SkImage_Base {
public:
    SkImage_GpuBase(sk_sp<GrContext>, int width, int height, uint32_t uniqueID,
                    SkAlphaType, SkBudgeted, sk_sp<SkColorSpace>);
    ~SkImage_GpuBase() override;

    GrContext* context() const final { return fContext.get(); }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset) const final;

    bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                      int srcX, int srcY, CachingHint) const override;

    sk_sp<GrTextureProxy> asTextureProxyRef() const override {
        // we shouldn't end up calling this
        SkASSERT(false);
        return this->INHERITED::asTextureProxyRef();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerState&, SkColorSpace*,
                                            sk_sp<SkColorSpace>*,
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

    static bool ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                                       GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);

    typedef ReleaseContext TextureContext;
    typedef void(*TextureFulfillProc)(TextureContext textureContext, GrBackendTexture* outTexture);
    typedef void(*PromiseDoneProc)(TextureContext textureContext);

protected:
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

/**
 * This helper class manages the ref counting for the the ReleaseProc and DoneProc for promise
 * images. It holds a weak ref on the ReleaseProc (hard refs are owned by GrTextures). The weak ref
 * allows us to reuse an outstanding ReleaseProc (because we dropped our GrTexture but the GrTexture
 * isn't done on the GPU) without needing to call FulfillProc again. It also holds a hard ref on the
 * DoneProc. The idea is that after every flush we may call the ReleaseProc so that the client can
 * free up their GPU memory if they want to. The life time of the DoneProc matches that of any
 * outstanding ReleaseProc as well as the PromiseImageHelper. Thus we won't call the DoneProc until
 * all ReleaseProcs are finished and we are finished with the PromiseImageHelper (i.e. won't call
 * FulfillProc again).
 */
class SkPromiseImageHelper {
public:
    SkPromiseImageHelper()
        : fFulfillProc(nullptr)
        , fReleaseProc(nullptr)
        , fContext(nullptr)
        , fDoneHelper(nullptr) {
    }

    void set(SkImage_GpuBase::TextureFulfillProc fulfillProc,
             SkImage_GpuBase::TextureReleaseProc releaseProc,
             SkImage_GpuBase::PromiseDoneProc doneProc,
             SkImage_GpuBase::TextureContext context) {
        fFulfillProc = fulfillProc;
        fReleaseProc = releaseProc;
        fContext = context;
        fDoneHelper.reset(new GrReleaseProcHelper(doneProc, context));
    }

    SkPromiseImageHelper(SkImage_GpuBase::TextureFulfillProc fulfillProc,
                         SkImage_GpuBase::TextureReleaseProc releaseProc,
                         SkImage_GpuBase::PromiseDoneProc doneProc,
                         SkImage_GpuBase::TextureContext context)
        : fFulfillProc(fulfillProc)
        , fReleaseProc(releaseProc)
        , fContext(context)
        , fDoneHelper(new GrReleaseProcHelper(doneProc, context)) {
    }

    bool isValid() { return SkToBool(fDoneHelper); }

    void reset() {
        this->resetReleaseHelper();
        fDoneHelper.reset();
    }

    sk_sp<GrTexture> getTexture(GrResourceProvider* resourceProvider, GrPixelConfig config);

private:
    // Weak unrefs fReleaseHelper and sets it to null
    void resetReleaseHelper() {
        if (fReleaseHelper) {
            fReleaseHelper->weak_unref();
            fReleaseHelper = nullptr;
        }
    }

    SkImage_GpuBase::TextureFulfillProc fFulfillProc;
    SkImage_GpuBase::TextureReleaseProc fReleaseProc;
    SkImage_GpuBase::TextureContext     fContext;

    // We cache the GrBackendTexture so that if we deleted the GrTexture but the the release proc
    // has yet not been called (this can happen on Vulkan), then we can create a new texture without
    // needing to call the fulfill proc again.
    GrBackendTexture           fBackendTex;
    // The fReleaseHelper is used to track a weak ref on the release proc. This helps us make sure
    // we are always pairing fulfill and release proc calls correctly.
    SkPromiseReleaseProcHelper*  fReleaseHelper = nullptr;
    // We don't want to call the fDoneHelper until we are done with the PromiseImageHelper and all
    // ReleaseHelpers are finished. Thus we hold a hard ref here and we will pass a hard ref to each
    // fReleaseHelper we make.
    sk_sp<GrReleaseProcHelper> fDoneHelper;
};

#endif
