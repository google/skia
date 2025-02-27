/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SurfaceFillContext_DEFINED
#define SurfaceFillContext_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/OpsTask.h"

#include <array>
#include <memory>
#include <utility>

class GrPaint;
class GrRecordingContext;
class GrRenderTask;
class SkArenaAlloc;
struct SkIPoint;

namespace sktext::gpu {
class SubRunAllocator;
}

namespace skgpu::ganesh {

class SurfaceFillContext : public SurfaceContext {
public:
    SurfaceFillContext(GrRecordingContext* rContext,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       const GrColorInfo& colorInfo);

    SurfaceFillContext* asFillContext() override { return this; }

    OpsTask* getOpsTask();

#if defined(GPU_TEST_UTILS)
    OpsTask* testingOnly_PeekLastOpsTask() { return fOpsTask.get(); }
#endif

    /**
     * Provides a performance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

    void resolveMSAA();

    /**
     * Clear the rect of the render target to the given color.
     * @param rect  the rect to clear to
     * @param color the color to clear to.
     */
    template <SkAlphaType AlphaType>
    void clear(const SkIRect& rect, const SkRGBA4f<AlphaType>& color) {
        this->internalClear(&rect, this->adjustColorAlphaType(color));
    }

    /** Clears the entire render target to the color. */
    template <SkAlphaType AlphaType> void clear(const SkRGBA4f<AlphaType>& color) {
        this->internalClear(nullptr, this->adjustColorAlphaType(color));
    }

    /**
     * Clear at minimum the pixels within 'scissor', but is allowed to clear the full render target
     * if that is the more performant option.
     */
    template <SkAlphaType AlphaType>
    void clearAtLeast(const SkIRect& scissor, const SkRGBA4f<AlphaType>& color) {
        this->internalClear(&scissor,
                            this->adjustColorAlphaType(color),
                            /* upgrade to full */ true);
    }

    /** Fills 'dstRect' with 'fp' */
    void fillRectWithFP(const SkIRect& dstRect, std::unique_ptr<GrFragmentProcessor>);

    /**
     * A convenience version of fillRectWithFP that applies a coordinate transformation via
     * GrMatrixEffect.
     */
    void fillRectWithFP(const SkIRect& dstRect,
                        const SkMatrix& localMatrix,
                        std::unique_ptr<GrFragmentProcessor>);

    /** Fills 'dstRect' with 'fp' using a local matrix that maps 'srcRect' to 'dstRect' */
    void fillRectToRectWithFP(const SkRect& srcRect,
                              const SkIRect& dstRect,
                              std::unique_ptr<GrFragmentProcessor> fp) {
        SkMatrix lm = SkMatrix::RectToRect(SkRect::Make(dstRect), srcRect);
        this->fillRectWithFP(dstRect, lm, std::move(fp));
    }

    /** Fills 'dstRect' with 'fp' using a local matrix that maps 'srcRect' to 'dstRect' */
    void fillRectToRectWithFP(const SkIRect& srcRect,
                              const SkIRect& dstRect,
                              std::unique_ptr<GrFragmentProcessor> fp) {
        this->fillRectToRectWithFP(SkRect::Make(srcRect), dstRect, std::move(fp));
    }

    /** Fills the entire render target with the passed FP. */
    void fillWithFP(std::unique_ptr<GrFragmentProcessor> fp) {
        this->fillRectWithFP(SkIRect::MakeSize(fWriteView.proxy()->dimensions()), std::move(fp));
    }

    /**
     * Draws the src texture with no matrix. The dstRect is the dstPoint with the width and height
     * of the srcRect. The srcRect and dstRect are clipped to the bounds of the src and dst surfaces
     * respectively.
     */
    bool blitTexture(GrSurfaceProxyView,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);

    sk_sp<GrRenderTask> refRenderTask();

    int numSamples() const { return this->asRenderTargetProxy()->numSamples(); }
    bool wrapsVkSecondaryCB() const { return this->asRenderTargetProxy()->wrapsVkSecondaryCB(); }

    SkArenaAlloc* arenaAlloc() { return this->arenas()->arenaAlloc(); }
    sktext::gpu::SubRunAllocator* subRunAlloc() { return this->arenas()->subRunAlloc(); }

    const GrSurfaceProxyView& writeSurfaceView() const { return fWriteView; }

protected:
    OpsTask* replaceOpsTask();

    /**
     * Creates a constant color paint for a clear, using src-over if possible to improve batching.
     */
    static void ClearToGrPaint(std::array<float, 4> color, GrPaint* paint);

    void addOp(GrOp::Owner);

    template <SkAlphaType AlphaType>
    static std::array<float, 4> ConvertColor(SkRGBA4f<AlphaType> color);

    template <SkAlphaType AlphaType>
    std::array<float, 4> adjustColorAlphaType(SkRGBA4f<AlphaType> color) const;

    GrSurfaceProxyView fWriteView;

private:
    sk_sp<GrArenas> arenas() { return fWriteView.proxy()->asRenderTargetProxy()->arenas(); }

    void addDrawOp(GrOp::Owner);

    /** Override to be notified in subclass before the current ops task is replaced. */
    virtual void willReplaceOpsTask(OpsTask* prevTask, OpsTask* nextTask) {}

    /**
     * Override to be called to participate in the decision to discard all previous ops if a
     * fullscreen clear occurs.
     */
    virtual OpsTask::CanDiscardPreviousOps canDiscardPreviousOpsOnFullClear() const {
        return OpsTask::CanDiscardPreviousOps::kYes;
    }

    void internalClear(const SkIRect* scissor,
                       std::array<float, 4> color,
                       bool upgradePartialToFull = false);

    SkDEBUGCODE(void onValidate() const override;)

    // The OpsTask can be closed by some other surface context that has picked it up. For this
    // reason, the OpsTask should only ever be accessed via 'getOpsTask'.
    sk_sp<OpsTask> fOpsTask;

    using INHERITED = SurfaceContext;
};

template<>
inline std::array<float, 4> SurfaceFillContext::ConvertColor<kPremul_SkAlphaType>(
        SkPMColor4f color) {
    return color.unpremul().array();
}

template<>
inline std::array<float, 4> SurfaceFillContext::ConvertColor<kUnpremul_SkAlphaType>(
        SkColor4f color) {
    return color.premul().array();
}

template <SkAlphaType AlphaType>
std::array<float, 4> SurfaceFillContext::adjustColorAlphaType(SkRGBA4f<AlphaType> color) const {
    if (AlphaType == kUnknown_SkAlphaType ||
        this->colorInfo().alphaType() == kUnknown_SkAlphaType) {
        return color.array();
    }
    return (AlphaType == this->colorInfo().alphaType()) ? color.array() : ConvertColor(color);
}

}  // namespace skgpu::ganesh

#endif // SurfaceFillContext_DEFINED
