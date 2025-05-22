/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBackendTextureImageGenerator_DEFINED
#define GrBackendTextureImageGenerator_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkMutex.h"
#include "include/private/gpu/ganesh/GrTextureGenerator.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

#include <memory>

class GrRecordingContext;
class GrSemaphore;
class GrTexture;
class SkColorInfo;
class SkColorSpace;
class SkRecorder;
enum GrSurfaceOrigin : int;
enum SkAlphaType : int;
enum SkColorType : int;
enum class GrImageTexGenPolicy : int;
struct SkImageInfo;

namespace skgpu {
class RefCntedCallback;
enum class Mipmapped : bool;
}  // namespace skgpu

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
class GrBackendTextureImageGenerator final : public GrTextureGenerator {
public:
    static std::unique_ptr<GrTextureGenerator> Make(const sk_sp<GrTexture>&, GrSurfaceOrigin,
                                                    std::unique_ptr<GrSemaphore>, SkColorType,
                                                    SkAlphaType, sk_sp<SkColorSpace>);

    ~GrBackendTextureImageGenerator() override;

protected:
    bool onIsValid(GrRecordingContext*) const override;
    bool onIsValid(SkRecorder*) const override;
    bool onIsProtected() const override;

    GrSurfaceProxyView onGenerateTexture(GrRecordingContext*,
                                         const SkImageInfo&,
                                         skgpu::Mipmapped mipmapped,
                                         GrImageTexGenPolicy) override;

private:
    GrBackendTextureImageGenerator(const SkColorInfo&,
                                   const sk_sp<GrTexture>&,
                                   GrSurfaceOrigin,
                                   GrDirectContext::DirectContextID owningContextID,
                                   std::unique_ptr<GrSemaphore>);

    static void ReleaseRefHelper_TextureReleaseProc(void* ctx);

    class RefHelper : public SkNVRefCnt<RefHelper> {
    public:
        RefHelper(sk_sp<GrTexture>,
                  GrDirectContext::DirectContextID owningContextID,
                  std::unique_ptr<GrSemaphore>);

        ~RefHelper();

        sk_sp<GrTexture>                 fOriginalTexture;
        GrDirectContext::DirectContextID fOwningContextID;

        // We use this key so that we don't rewrap the GrBackendTexture in a GrTexture for each
        // proxy created from this generator for a particular borrowing context.
        skgpu::UniqueKey                 fBorrowedTextureKey;
        // There is no ref associated with this pointer. We rely on our atomic bookkeeping with the
        // context ID to know when this pointer is valid and safe to use. This is used to make sure
        // all uses of the wrapped texture are finished on the borrowing context before we open
        // this back up to other contexts. In general a ref to this release proc is owned by all
        // proxies and gpu uses of the backend texture.
        skgpu::RefCntedCallback*         fBorrowingContextReleaseProc;
        GrDirectContext::DirectContextID fBorrowingContextID;

        std::unique_ptr<GrSemaphore>     fSemaphore;
    };

    RefHelper*       fRefHelper;
    // This Mutex is used to guard the borrowing of the texture to one GrContext at a time as well
    // as the creation of the fBorrowingContextReleaseProc. The latter happening if two threads with
    // the same consuming GrContext try to generate a texture at the same time.
    SkMutex          fBorrowingMutex;

    GrBackendTexture fBackendTexture;
    GrSurfaceOrigin  fSurfaceOrigin;

    using INHERITED = GrTextureGenerator;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
