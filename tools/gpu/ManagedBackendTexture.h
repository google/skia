/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ManagedBackendTexture_DEFINED
#define ManagedBackendTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkYUVAInfo.h"
#ifdef SK_GANESH
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#endif
#ifdef SK_GRAPHITE
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/RecorderPriv.h"
#endif

namespace skgpu {
class RefCntedCallback;
}
namespace skgpu::graphite {
class Recorder;
}
class SkBitmap;
struct SkImageInfo;

#ifdef SK_GRAPHITE
using Recorder = skgpu::graphite::Recorder;
#endif

namespace sk_gpu_test {

#ifdef SK_GANESH
class ManagedBackendTexture : public SkNVRefCnt<ManagedBackendTexture> {
public:
    /**
     * Make a managed backend texture with initial pixmap/color data. The 'Args' are any valid set
     * of arguments to GrDirectContext::createBackendTexture that takes data but with the release
     * proc/context omitted as the ManagedBackendTexture will provide them.
     */
    template <typename... Args>
    static sk_sp<ManagedBackendTexture> MakeWithData(GrDirectContext*, Args&&...);

    /**
     * Make a managed backend texture without initial data. The 'Args' are any valid set of
     * arguments to GrDirectContext::createBackendTexture that does not take data. Because our
     * createBackendTexture methods that *do* take data also use default args for the proc/context
     * this can be used to make a texture with data but then the MBET won't be able to ensure that
     * the upload has completed before the texture is deleted. Use the WithData variant instead to
     * avoid this issue.
     */
    template <typename... Args>
    static sk_sp<ManagedBackendTexture> MakeWithoutData(GrDirectContext*, Args&&...);


    static sk_sp<ManagedBackendTexture> MakeFromInfo(GrDirectContext* dContext,
                                                     const SkImageInfo&,
                                                     skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                                     skgpu::Renderable = skgpu::Renderable::kNo,
                                                     skgpu::Protected = skgpu::Protected::kNo);

    static sk_sp<ManagedBackendTexture> MakeFromBitmap(GrDirectContext*,
                                                       const SkBitmap&,
                                                       skgpu::Mipmapped,
                                                       skgpu::Renderable,
                                                       skgpu::Protected = skgpu::Protected::kNo);

    static sk_sp<ManagedBackendTexture> MakeFromPixmap(GrDirectContext*,
                                                       const SkPixmap&,
                                                       skgpu::Mipmapped,
                                                       skgpu::Renderable,
                                                       skgpu::Protected = skgpu::Protected::kNo);

    /** GrGpuFinishedProc or image/surface release proc. */
    static void ReleaseProc(void* context);

    ~ManagedBackendTexture();

    /**
     * The context to use with ReleaseProc. This adds a ref so it *must* be balanced by a call to
     * ReleaseProc. If a wrappedProc is provided then it will be called by ReleaseProc.
     */
    void* releaseContext(GrGpuFinishedProc wrappedProc = nullptr,
                         GrGpuFinishedContext wrappedContext = nullptr) const;

    sk_sp<skgpu::RefCntedCallback> refCountedCallback() const;

    /**
     * Call if the underlying GrBackendTexture was adopted by a GrContext. This clears this out the
     * MBET without deleting the texture.
     */
    void wasAdopted();

    /**
     * SkImages::TextureFromYUVATextures takes a single release proc that is called once for all the
     * textures. This makes a single release context for the group of textures. It's used with the
     * standard ReleaseProc. Like releaseContext(), it must be balanced by a ReleaseProc call for
     * proper ref counting.
     */
    static void* MakeYUVAReleaseContext(const sk_sp<ManagedBackendTexture>[SkYUVAInfo::kMaxPlanes]);

    const GrBackendTexture& texture() { return fTexture; }

private:
    ManagedBackendTexture() = default;
    ManagedBackendTexture(const ManagedBackendTexture&) = delete;
    ManagedBackendTexture(ManagedBackendTexture&&) = delete;

    sk_sp<GrDirectContext> fDContext;
    GrBackendTexture fTexture;
};

template <typename... Args>
inline sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeWithData(GrDirectContext* dContext,
                                                                        Args&&... args) {
    sk_sp<ManagedBackendTexture> mbet(new ManagedBackendTexture);
    mbet->fDContext = sk_ref_sp(dContext);
    mbet->fTexture = dContext->createBackendTexture(std::forward<Args>(args)...,
                                                    ReleaseProc,
                                                    mbet->releaseContext());
    if (!mbet->fTexture.isValid()) {
        return nullptr;
    }
    return mbet;
}

template <typename... Args>
inline sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeWithoutData(
        GrDirectContext* dContext,
        Args&&... args) {
    GrBackendTexture texture =
            dContext->createBackendTexture(std::forward<Args>(args)...);
    if (!texture.isValid()) {
        return nullptr;
    }
    sk_sp<ManagedBackendTexture> mbet(new ManagedBackendTexture);
    mbet->fDContext = sk_ref_sp(dContext);
    mbet->fTexture = std::move(texture);
    return mbet;
}
#endif  // SK_GANESH

#ifdef SK_GRAPHITE
/*
 * Graphite version of ManagedBackendTexture
 */
class ManagedGraphiteTexture : public SkNVRefCnt<ManagedGraphiteTexture> {
public:
    static sk_sp<ManagedGraphiteTexture> MakeFromPixmap(Recorder*,
                                                        const SkPixmap&,
                                                        skgpu::Mipmapped,
                                                        skgpu::Renderable,
                                                        skgpu::Protected = skgpu::Protected::kNo);

    /** finished and image/surface release procs */
    static void FinishedProc(void* context, skgpu::CallbackResult);
    static void ImageReleaseProc(void* context);

    ~ManagedGraphiteTexture();

    /**
     * The context to use with the ReleaseProcs. This adds a ref so it *must* be balanced by a call
     * to TextureReleaseProc or ImageReleaseProc.
     */
    void* releaseContext() const;

    sk_sp<skgpu::RefCntedCallback> refCountedCallback() const;

    /**
     * SkImage::MakeGraphiteFromYUVABackendTextures takes a single release proc that is called once
     * for all the textures. This makes a single release context for the group of textures. It's
     * used with the standard ReleaseProc. Like releaseContext(), it must be balanced by a
     * ReleaseProc call for proper ref counting.
     */
    static void* MakeYUVAReleaseContext(const sk_sp<ManagedGraphiteTexture>[SkYUVAInfo::kMaxPlanes]);

    const skgpu::graphite::BackendTexture& texture() { return fTexture; }

private:
    ManagedGraphiteTexture() = default;
    ManagedGraphiteTexture(const ManagedGraphiteTexture&) = delete;
    ManagedGraphiteTexture(ManagedGraphiteTexture&&) = delete;

    skgpu::graphite::Context* fContext;
    skgpu::graphite::BackendTexture fTexture;
};

#endif  // SK_GRAPHITE

}  // namespace sk_gpu_test

#endif  // ManagedBackendTexture_DEFINED
