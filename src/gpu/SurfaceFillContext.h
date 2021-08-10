/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceFillContext_DEFINED
#define SurfaceFillContext_DEFINED

#include "src/gpu/SurfaceContext.h"

namespace skgpu {

class SurfaceFillContext : public SurfaceContext {
public:

    SurfaceFillContext* asFillContext() override { return this; }

    /**
     * Provides a performance hint that the render target's contents are allowed
     * to become undefined.
     */
    virtual void discard() = 0;

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
    virtual void fillRectWithFP(const SkIRect& dstRect, std::unique_ptr<GrFragmentProcessor>) = 0;

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
     * A convenience version of fillWithFP that applies a coordinate transformation via
     * GrMatrixEffect and fills the entire render target.
     */
    void fillWithFP(const SkMatrix& localMatrix, std::unique_ptr<GrFragmentProcessor> fp) {
        this->fillRectWithFP(SkIRect::MakeSize(fWriteView.proxy()->dimensions()),
                             localMatrix,
                             std::move(fp));
    }

    /**
     * Draws the src texture with no matrix. The dstRect is the dstPoint with the width and height
     * of the srcRect. The srcRect and dstRect are clipped to the bounds of the src and dst surfaces
     * respectively.
     */
    virtual bool blitTexture(GrSurfaceProxyView,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) = 0;

    virtual sk_sp<GrRenderTask> refRenderTask() = 0;

protected:
    SurfaceFillContext(GrRecordingContext* rContext,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       const GrColorInfo& colorInfo)
            : SurfaceContext(rContext, std::move(readView), colorInfo)
            , fWriteView(std::move(writeView)) {
        SkASSERT(this->asSurfaceProxy() == fWriteView.proxy());
        SkASSERT(this->origin() == fWriteView.origin());
    }

    template <SkAlphaType AlphaType>
    static std::array<float, 4> ConvertColor(SkRGBA4f<AlphaType> color);

    template <SkAlphaType AlphaType>
    std::array<float, 4> adjustColorAlphaType(SkRGBA4f<AlphaType> color) const;

    GrSurfaceProxyView fWriteView;

private:
    virtual void internalClear(const SkIRect* scissor,
                               std::array<float, 4> color,
                               bool upgradePartialToFull = false) = 0;

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

} // namespace skgpu

#endif // SurfaceFillContext_DEFINED
