/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBackendTextureImageGenerator_DEFINED
#define GrBackendTextureImageGenerator_DEFINED

#include "SkImageGenerator.h"

#include "GrBackendSurface.h"
#include "SkAtomics.h"

class GrSemaphore;

class GrBackendTextureImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(sk_sp<GrTexture>, sk_sp<GrSemaphore>,
                                                  SkAlphaType, sk_sp<SkColorSpace>);

    ~GrBackendTextureImageGenerator() override;

protected:
    // NOTE: We would like to validate that the owning context hasn't been abandoned, but we can't
    // do that safely (we might be on another thread). So assume everything is fine.
    bool onIsValid(GrContext*) const override { return true; }

#if SK_SUPPORT_GPU
    TexGenType onCanGenerateTexture() const override { return TexGenType::kCheap; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            SkTransferFunctionBehavior) override;
#endif

private:
    GrBackendTextureImageGenerator(const SkImageInfo& info, GrTexture*,
                                   uint32_t owningContextID, sk_sp<GrSemaphore>,
                                   const GrBackendTexture&);

    static void ReleaseRefHelper_TextureReleaseProc(void* ctx);

    class RefHelper : public SkNVRefCnt<RefHelper> {
    public:
        RefHelper(GrTexture* texture, uint32_t owningContextID)
            : fOriginalTexture(texture)
            , fOwningContextID(owningContextID)
            , fBorrowedTexture(nullptr)
            , fBorrowingContextID(SK_InvalidGenID) { }

        ~RefHelper();

        GrTexture* fOriginalTexture;
        uint32_t fOwningContextID;

        // There is never a ref associated with this pointer. We rely on our atomic bookkeeping
        // with the context ID to know when this pointer is valid and safe to use. This lets us
        // avoid releasing a ref from another thread, or get into races during context shutdown.
        GrTexture* fBorrowedTexture;
        SkAtomic<uint32_t> fBorrowingContextID;
    };

    RefHelper* fRefHelper;

    sk_sp<GrSemaphore> fSemaphore;
    uint32_t fLastBorrowingContextID;

    GrBackendTexture fBackendTexture;
    GrSurfaceOrigin fSurfaceOrigin;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
