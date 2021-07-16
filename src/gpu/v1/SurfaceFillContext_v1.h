/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceFillContext_v1_DEFINED
#define SurfaceFillContext_v1_DEFINED

#include "include/core/SkSize.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrMatrixEffect.h"

#include <array>
#include <tuple>

class GrFragmentProcessor;
class GrImageContext;
class GrOp;
class GrBackendFormat;
class GrRecordingContext;
class GrSurfaceProxyView;
class SkColorSpace;

namespace skgpu::v1 {

class SurfaceFillContext : public skgpu::SurfaceFillContext_Base {
public:
    SurfaceFillContext(GrRecordingContext*,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       const GrColorInfo&,
                       bool flushTimeOpsTask = false);

    /**
     * Uses GrImageInfo's color type to pick the default texture format. Will return a
     * GrSurfaceDrawContext if possible.
     */
    static std::unique_ptr<SurfaceFillContext> Make(GrRecordingContext*,
                                                    GrImageInfo,
                                                    SkBackingFit = SkBackingFit::kExact,
                                                    int sampleCount = 1,
                                                    GrMipmapped = GrMipmapped::kNo,
                                                    GrProtected = GrProtected::kNo,
                                                    GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
                                                    SkBudgeted = SkBudgeted::kYes);

    /**
     * Like the above but uses GetFallbackColorTypeAndFormat to find a fallback color type (and
     * compatible format) if the passed GrImageInfo's color type is not renderable.
     */
    static std::unique_ptr<SurfaceFillContext> MakeWithFallback(
            GrRecordingContext*,
            GrImageInfo,
            SkBackingFit = SkBackingFit::kExact,
            int sampleCount = 1,
            GrMipmapped = GrMipmapped::kNo,
            GrProtected = GrProtected::kNo,
            GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
            SkBudgeted = SkBudgeted::kYes);

    /**
     * Makes a custom configured GrSurfaceFillContext where the caller specifies the specific
     * texture format and swizzles. The color type will be kUnknown. Returns a GrSurfaceDrawContext
     * if possible.
     */
    static std::unique_ptr<SurfaceFillContext> Make(GrRecordingContext*,
                                                    SkAlphaType,
                                                    sk_sp<SkColorSpace>,
                                                    SkISize dimensions,
                                                    SkBackingFit,
                                                    const GrBackendFormat&,
                                                    int sampleCount,
                                                    GrMipmapped,
                                                    GrProtected,
                                                    GrSwizzle readSwizzle,
                                                    GrSwizzle writeSwizzle,
                                                    GrSurfaceOrigin,
                                                    SkBudgeted);

    /**
     * Creates a GrSurfaceFillContext from an existing GrBackendTexture. The GrColorInfo's color
     * type must be compatible with backend texture's format or this will fail. All formats are
     * considered compatible with kUnknown. Returns a GrSurfaceDrawContext if possible.
     */
    static std::unique_ptr<SurfaceFillContext> MakeFromBackendTexture(
            GrRecordingContext*,
            GrColorInfo,
            const GrBackendTexture&,
            int sampleCount,
            GrSurfaceOrigin,
            sk_sp<GrRefCntedCallback> releaseHelper);

    SurfaceFillContext_Base* asFillContext() override { return this; }

    /**
     * Provides a performance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

    /**/
    void fillRectWithFP(const SkIRect& dstRect, std::unique_ptr<GrFragmentProcessor> fp) override;

    bool blitTexture(GrSurfaceProxyView view,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) override;
    /**/

    GrOpsTask* getOpsTask();
    sk_sp<GrRenderTask> refRenderTask() override;

    int numSamples() const { return this->asRenderTargetProxy()->numSamples(); }
    bool wrapsVkSecondaryCB() const { return this->asRenderTargetProxy()->wrapsVkSecondaryCB(); }

    SkArenaAlloc* arenaAlloc() { return this->arenas()->arenaAlloc(); }
    GrSubRunAllocator* subRunAlloc() { return this->arenas()->subRunAlloc(); }

#if GR_TEST_UTILS
    GrOpsTask* testingOnly_PeekLastOpsTask() { return fOpsTask.get(); }
#endif

    const GrSurfaceProxyView& writeSurfaceView() const { return fWriteView1; }

protected:
    /**
     * Creates a constant color paint for a clear, using src-over if possible to improve batching.
     */
    static void ClearToGrPaint(std::array<float, 4> color, GrPaint* paint);

    void addOp(GrOp::Owner);

    GrOpsTask* replaceOpsTask();

private:
    sk_sp<GrArenas> arenas() { return fWriteView1.proxy()->asRenderTargetProxy()->arenas(); }

    /** Override to be notified in subclass before the current ops task is replaced. */
    virtual void willReplaceOpsTask(GrOpsTask* prevTask, GrOpsTask* nextTask) {}

    /**
     * Override to be called to participate in the decision to discard all previous ops if a
     * fullscreen clear occurs.
     */
    virtual GrOpsTask::CanDiscardPreviousOps canDiscardPreviousOpsOnFullClear() const {
        return GrOpsTask::CanDiscardPreviousOps::kYes;
    }

    void internalClear(const SkIRect* scissor,
                       std::array<float, 4> color,
                       bool upgradePartialToFull = false) override;

    void addDrawOp(GrOp::Owner);

    SkDEBUGCODE(void onValidate() const override;)

//    GrSurfaceProxyView fWriteView;

    // The GrOpsTask can be closed by some other surface context that has picked it up. For this
    // reason, the GrOpsTask should only ever be accessed via 'getOpsTask'.
    sk_sp<GrOpsTask> fOpsTask;

    bool fFlushTimeOpsTask;

    using INHERITED = GrSurfaceContext;
};

} // namespace skgpu::v1

#endif // SurfaceFillContext_v1_DEFINED
