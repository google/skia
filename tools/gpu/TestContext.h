
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestContext_DEFINED
#define TestContext_DEFINED

#include "FenceSync.h"
#include "GrTypes.h"
#include "SkRefCnt.h"
#include "../private/SkTemplates.h"

class GrContext;
struct GrContextOptions;

namespace sk_gpu_test {

class GpuTimer;

/**
 * An offscreen 3D context. This class is intended for Skia's internal testing needs and not
 * for general use.
 */
class TestContext : public SkNoncopyable {
public:
    virtual ~TestContext();

    bool fenceSyncSupport() const { return fFenceSync != nullptr; }
    FenceSync* fenceSync() { SkASSERT(fFenceSync); return fFenceSync.get(); }

    bool gpuTimingSupport() const { return fGpuTimer != nullptr; }
    GpuTimer* gpuTimer() const { SkASSERT(fGpuTimer); return fGpuTimer.get(); }

    bool getMaxGpuFrameLag(int *maxFrameLag) const {
        if (!fFenceSync) {
            return false;
        }
        *maxFrameLag = kMaxFrameLag;
        return true;
    }

    void makeCurrent() const;

    virtual GrBackend backend() = 0;
    virtual GrBackendContext backendContext() = 0;

    virtual sk_sp<GrContext> makeGrContext(const GrContextOptions&);

    /** Swaps front and back buffer (if the context has such buffers) */
    void swapBuffers();

    /**
     * The only purpose of this function it to provide a means of scheduling
     * work on the GPU (since all of the subclasses create primary buffers for
     * testing that are small and not meant to be rendered to the screen).
     *
     * If the platform supports fence syncs (OpenGL 3.2+ or EGL_KHR_fence_sync),
     * this will not swap any buffers, but rather emulate triple buffer synchronization
     * using fences.
     *
     * Otherwise it will call the platform SwapBuffers method. This may or may
     * not perform some sort of synchronization, depending on whether the
     * drawing surface provided by the platform is double buffered.
     *
     * Implicitly performs a submit().
     */
    void waitOnSyncOrSwap();

    /**
     * This notifies the context that we are deliberately testing abandoning
     * the context. It is useful for debugging contexts that would otherwise
     * test that GPU resources are properly deleted. It also allows a debugging
     * context to test that further API calls are not made by Skia GPU code.
     */
    virtual void testAbandon();

    /** Ensures all work is submitted to the GPU for execution. */
    virtual void submit() = 0;

    /** Wait until all GPU work is finished. */
    virtual void finish() = 0;

protected:
    std::unique_ptr<FenceSync> fFenceSync;
    std::unique_ptr<GpuTimer>  fGpuTimer;

    TestContext();

    /** This should destroy the 3D context. */
    virtual void teardown();

    virtual void onPlatformMakeCurrent() const = 0;
    virtual void onPlatformSwapBuffers() const = 0;

private:
    enum {
        kMaxFrameLag = 3
    };

    PlatformFence fFrameFences[kMaxFrameLag - 1];
    int fCurrentFenceIdx;

    typedef SkNoncopyable INHERITED;
};
}  // namespace sk_gpu_test
#endif
