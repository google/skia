/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceFillContext_DEFINED
#define GrSurfaceFillContext_DEFINED

#include "include/core/SkSize.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSwizzle.h"
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

class GrSurfaceFillContext : public GrSurfaceContext {
public:
    /**
     * Checks whether the passed color type is renderable with the passed context. If so, the
     * same color type is passed back along with the default format used for the color type. If
     * not, provides an alternative (perhaps lower bit depth and/or unorm instead of float) color
     * type that is supported along with it's default format or kUnknown if there no renderable
     * fallback format.
     */
    static std::tuple<GrColorType, GrBackendFormat> GetFallbackColorTypeAndFormat(GrImageContext*,
                                                                                  GrColorType,
                                                                                  int sampleCount);

    GrSurfaceFillContext(GrRecordingContext*,
                         GrSurfaceProxyView readView,
                         GrSurfaceProxyView writeView,
                         const GrColorInfo&,
                         bool flushTimeOpsTask = false);

    /**
     * Uses GrImageInfo's color type to pick the default texture format. Will return a
     * GrSurfaceDrawContext if possible.
     */
    static std::unique_ptr<GrSurfaceFillContext> Make(GrRecordingContext*,
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
    static std::unique_ptr<GrSurfaceFillContext> MakeWithFallback(
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
    static std::unique_ptr<GrSurfaceFillContext> Make(GrRecordingContext*,
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
    static std::unique_ptr<GrSurfaceFillContext> MakeFromBackendTexture(
            GrRecordingContext*,
            GrColorInfo,
            const GrBackendTexture&,
            int sampleCount,
            GrSurfaceOrigin,
            sk_sp<GrRefCntedCallback> releaseHelper);

    GrSurfaceFillContext* asFillContext() override { return this; }

    /**
     * Provides a performance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

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
    void fillRectWithFP(const SkIRect& dstRect, std::unique_ptr<GrFragmentProcessor> fp);

    /**
     * A convenience version of fillRectWithFP that applies a coordinate transformation via
     * GrMatrixEffect.
     */
    void fillRectWithFP(const SkIRect& dstRect,
                        const SkMatrix& localMatrix,
                        std::unique_ptr<GrFragmentProcessor> fp) {
        fp = GrMatrixEffect::Make(localMatrix, std::move(fp));
        this->fillRectWithFP(dstRect, std::move(fp));
    }

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
     * A convenience version of fillWithFP that applies a coordinate transformation via
     * GrMatrixEffect and fills the entire render target.
     */
    void fillWithFP(const SkMatrix& localMatrix, std::unique_ptr<GrFragmentProcessor> fp) {
        this->fillRectWithFP(
                SkIRect::MakeSize(fWriteView.proxy()->dimensions()), localMatrix, std::move(fp));
    }

    /**
     * Draws the src texture with no matrix. The dstRect is the dstPoint with the width and height
     * of the srcRect. The srcRect and dstRect are clipped to the bounds of the src and dst surfaces
     * respectively.
     */
    bool blitTexture(GrSurfaceProxyView view, const SkIRect& srcRect, const SkIPoint& dstPoint);

    GrOpsTask* getOpsTask();

    int numSamples() const { return this->asRenderTargetProxy()->numSamples(); }
    bool wrapsVkSecondaryCB() const { return this->asRenderTargetProxy()->wrapsVkSecondaryCB(); }
    GrMipmapped mipmapped() const;

#if GR_TEST_UTILS
    GrOpsTask* testingOnly_PeekLastOpsTask() { return fOpsTask.get(); }
#endif

    const GrSurfaceProxyView& writeSurfaceView() const { return fWriteView; }

protected:
    /**
     * Creates a constant color paint for a clear, using src-over if possible to improve batching.
     */
    static void ClearToGrPaint(std::array<float, 4> color, GrPaint* paint);

    void addOp(GrOp::Owner);

private:
    template <SkAlphaType AlphaType>
    static std::array<float, 4> ConvertColor(SkRGBA4f<AlphaType> color);

    template <SkAlphaType AlphaType>
    std::array<float, 4> adjustColorAlphaType(SkRGBA4f<AlphaType> color) const;

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
                       bool upgradePartialToFull = false);

    void addDrawOp(GrOp::Owner);

    SkDEBUGCODE(void onValidate() const override;)

    GrSurfaceProxyView fWriteView;

    // The GrOpsTask can be closed by some other surface context that has picked it up. For this
    // reason, the GrOpsTask should only ever be accessed via 'getOpsTask'.
    sk_sp<GrOpsTask> fOpsTask;

    bool fFlushTimeOpsTask;

    using INHERITED = GrSurfaceContext;
};

template<>
inline std::array<float, 4> GrSurfaceFillContext::ConvertColor<kPremul_SkAlphaType>(
        SkPMColor4f color) {
    return color.unpremul().array();
}

template<>
inline std::array<float, 4> GrSurfaceFillContext::ConvertColor<kUnpremul_SkAlphaType>(
        SkColor4f color) {
    return color.premul().array();
}

template <SkAlphaType AlphaType>
std::array<float, 4> GrSurfaceFillContext::adjustColorAlphaType(SkRGBA4f<AlphaType> color) const {
    if (AlphaType == kUnknown_SkAlphaType ||
        this->colorInfo().alphaType() == kUnknown_SkAlphaType) {
        return color.array();
    }
    return (AlphaType == this->colorInfo().alphaType()) ? color.array() : ConvertColor(color);
}

#endif
