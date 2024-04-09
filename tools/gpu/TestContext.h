
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestContext_DEFINED
#define TestContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkScopeExit.h"
#include "tools/gpu/FenceSync.h"

class GrDirectContext;
struct GrContextOptions;

namespace sk_gpu_test {

class GpuTimer;
class FlushFinishTracker;

/**
 * An offscreen 3D context. This class is intended for Skia's internal testing needs and not
 * for general use.
 */
class TestContext : public SkNoncopyable {
public:
    virtual ~TestContext();

    bool fenceSyncSupport() const { return fFenceSupport; }

    bool gpuTimingSupport() const { return fGpuTimer != nullptr; }
    GpuTimer* gpuTimer() const { SkASSERT(fGpuTimer); return fGpuTimer.get(); }

    bool getMaxGpuFrameLag(int *maxFrameLag) const {
        if (!this->fenceSyncSupport()) {
            return false;
        }
        *maxFrameLag = kMaxFrameLag;
        return true;
    }

    void makeNotCurrent() const;
    void makeCurrent() const;

    /**
     * Like makeCurrent() but this returns an object that will restore the previous current
     * context in its destructor. Useful to undo the effect making this current before returning to
     * a caller that doesn't expect the current context to be changed underneath it.
     *
     * The returned object restores the current context of the same type (e.g. egl, glx, ...) in its
     * destructor. It is undefined behavior if that context is destroyed before the destructor
     * executes. If the concept of a current context doesn't make sense for this context type then
     * the returned object's destructor is a no-op.
     */
    [[nodiscard]] SkScopeExit makeCurrentAndAutoRestore() const;

    virtual GrBackendApi backend() = 0;

    virtual sk_sp<GrDirectContext> makeContext(const GrContextOptions&);

    /**
     * This will flush work to the GPU. Additionally, if the platform supports fence syncs, we will
     * add a finished callback to our flush call. We allow ourselves to have kMaxFrameLag number of
     * unfinished flushes active on the GPU at a time. If we have 2 outstanding flushes then we will
     * wait on the CPU until one has finished.
     */
    void flushAndWaitOnSync(GrDirectContext* context);

    /**
     * This notifies the context that we are deliberately testing abandoning
     * the context. It is useful for debugging contexts that would otherwise
     * test that GPU resources are properly deleted. It also allows a debugging
     * context to test that further API calls are not made by Skia GPU code.
     */
    virtual void testAbandon();

    /** Flush and wait until all GPU work is finished. */
    void flushAndSyncCpu(GrDirectContext*);

protected:
    bool fFenceSupport = false;

    std::unique_ptr<GpuTimer>  fGpuTimer;

    TestContext();

    /** This should destroy the 3D context. */
    virtual void teardown();

    virtual void onPlatformMakeNotCurrent() const = 0;
    virtual void onPlatformMakeCurrent() const = 0;
    /**
     * Subclasses should implement such that the returned function will cause the current context
     * of this type to be made current again when it is called. It should additionally be the
     * case that if "this" is already current when this is called, then "this" is destroyed (thereby
     * setting the null context as current), and then the std::function is called the null context
     * should remain current.
     */
    virtual std::function<void()> onPlatformGetAutoContextRestore() const = 0;

private:
    enum {
        kMaxFrameLag = 3
    };

    sk_sp<FlushFinishTracker> fFinishTrackers[kMaxFrameLag - 1];
    int fCurrentFlushIdx = 0;

    using INHERITED = SkNoncopyable;
};
}  // namespace sk_gpu_test
#endif
