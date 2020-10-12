/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ManagedBackendTexture_DEFINED
#define ManagedBackendTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrDirectContext.h"

namespace sk_gpu_test {

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

    /** GrGpuFinishedProc or image/surface release proc. */
    static void ReleaseProc(void* context);

    ~ManagedBackendTexture();

    /**
     * The context to use with ReleaseProc. This adds a ref so it *must* be balanced by a call to
     * ReleaseProc.
     */
    void* releaseContext();

    const GrBackendTexture& texture() { return fTexture; }

private:
    ManagedBackendTexture() = default;
    ManagedBackendTexture(const ManagedBackendTexture&) = delete;
    ManagedBackendTexture(ManagedBackendTexture&&) = delete;

    GrDirectContext* fDContext = nullptr;
    GrBackendTexture fTexture;
};

template <typename... Args>
inline sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeWithData(GrDirectContext* dContext,
                                                                        Args&&... args) {
    sk_sp<ManagedBackendTexture> mbet(new ManagedBackendTexture);
    mbet->fDContext = dContext;
    mbet->fTexture = dContext->createBackendTexture(std::forward<Args>(args)...,
                                                    ReleaseProc,
                                                    mbet->releaseContext());
    return mbet;
}

template <typename... Args>
inline sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeWithoutData(
        GrDirectContext* dContext,
        Args&&... args) {
    sk_sp<ManagedBackendTexture> mbet(new ManagedBackendTexture);
    mbet->fDContext = dContext;
    mbet->fTexture = dContext->createBackendTexture(std::forward<Args>(args)...);
    return mbet;
}

}  // namespace sk_gpu_test

#endif
