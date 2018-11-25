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
#include "SkMutex.h"

class GrSemaphore;

/*
 * This ImageGenerator is used to wrap a texture in one GrContext and can then be used as a source
 * in another GrContext. It holds onto a semaphore which the producing GrContext will signal and the
 * consuming GrContext will wait on before using the texture. Only one GrContext can ever be used
 * as a consumer (this is mostly because Vulkan can't allow multiple things to wait on the same
 * semaphore).
 *
 * In practice, this capability is used by clients to create backend-specific texture resources in
 * one thread (with, say, GrContext-A) and then ship them over to another GrContext (say,
 * GrContext-B) which will then use the texture as a source for draws. GrContext-A uses the
 * semaphore to notify GrContext-B when the shared texture is ready to use.
 */
class GrBackendTextureImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(sk_sp<GrTexture>, GrSurfaceOrigin,
                                                  sk_sp<GrSemaphore>,
                                                  SkAlphaType, sk_sp<SkColorSpace>);

    ~GrBackendTextureImageGenerator() override;

protected:
    // NOTE: We would like to validate that the owning context hasn't been abandoned, but we can't
    // do that safely (we might be on another thread). So assume everything is fine.
    bool onIsValid(GrContext*) const override { return true; }

#if SK_SUPPORT_GPU
    TexGenType onCanGenerateTexture() const override { return TexGenType::kCheap; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            SkTransferFunctionBehavior,
                                            bool willNeedMipMaps) override;
#endif

private:
    GrBackendTextureImageGenerator(const SkImageInfo& info, GrTexture*, GrSurfaceOrigin,
                                   uint32_t owningContextID, sk_sp<GrSemaphore>,
                                   const GrBackendTexture&);

    static void ReleaseRefHelper_TextureReleaseProc(void* ctx);

    class RefHelper : public SkNVRefCnt<RefHelper> {
    public:
        RefHelper(GrTexture* texture, uint32_t owningContextID)
            : fOriginalTexture(texture)
            , fOwningContextID(owningContextID)
            , fBorrowedTexture(nullptr)
            , fBorrowingContextReleaseProc(nullptr)
            , fBorrowingContextID(SK_InvalidGenID) {}

        ~RefHelper();

        GrTexture*          fOriginalTexture;
        uint32_t            fOwningContextID;

        // There is never a ref associated with this pointer. We rely on our atomic bookkeeping
        // with the context ID to know when this pointer is valid and safe to use. This lets us
        // avoid releasing a ref from another thread, or get into races during context shutdown.
        GrTexture*           fBorrowedTexture;
        // For the same reason as the fBorrowedTexture, there is no ref associated with this
        // pointer. The fBorrowingContextReleaseProc is used to make sure all uses of the wrapped
        // texture are finished on the borrowing context before we open this back up to other
        // contexts. In general a ref to this release proc is owned by all proxies and gpu uses of
        // the backend texture.
        GrReleaseProcHelper* fBorrowingContextReleaseProc;
        uint32_t             fBorrowingContextID;
    };

    RefHelper*           fRefHelper;
    // This Mutex is used to guard the borrowing of the texture to one GrContext at a time as well
    // as the creation of the fBorrowingContextReleaseProc. The latter happening if two threads with
    // the same consuming GrContext try to generate a texture at the same time.
    SkMutex              fBorrowingMutex;

    sk_sp<GrSemaphore>   fSemaphore;

    GrBackendTexture     fBackendTexture;
    GrPixelConfig        fConfig;
    GrSurfaceOrigin      fSurfaceOrigin;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
