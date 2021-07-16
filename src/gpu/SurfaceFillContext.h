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

class SurfaceFillContext_Base : public SurfaceContext {
public:
    /**
     * Uses GrImageInfo's color type to pick the default texture format. Will return a
     * GrSurfaceDrawContext if possible.
     */
    static std::unique_ptr<SurfaceFillContext_Base> Make(GrRecordingContext*,
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
    static std::unique_ptr<SurfaceFillContext_Base> MakeWithFallback(
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
    static std::unique_ptr<SurfaceFillContext_Base> Make(GrRecordingContext*,
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
    static std::unique_ptr<SurfaceFillContext_Base> MakeFromBackendTexture(
            GrRecordingContext*,
            GrColorInfo,
            const GrBackendTexture&,
            int sampleCount,
            GrSurfaceOrigin,
            sk_sp<GrRefCntedCallback> releaseHelper);

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
        this->fillRectWithFP(SkIRect::MakeSize(fWriteView1.proxy()->dimensions()), std::move(fp));
    }

    /**
     * A convenience version of fillWithFP that applies a coordinate transformation via
     * GrMatrixEffect and fills the entire render target.
     */
    void fillWithFP(const SkMatrix& localMatrix, std::unique_ptr<GrFragmentProcessor> fp) {
        this->fillRectWithFP(
                SkIRect::MakeSize(fWriteView1.proxy()->dimensions()), localMatrix, std::move(fp));
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
    template <SkAlphaType AlphaType>
    static std::array<float, 4> ConvertColor(SkRGBA4f<AlphaType> color);

    template <SkAlphaType AlphaType>
    std::array<float, 4> adjustColorAlphaType(SkRGBA4f<AlphaType> color) const;

    GrSurfaceProxyView fWriteView1;

private:
    using INHERITED = SurfaceContext;
};

template<>
inline std::array<float, 4> SurfaceFillContext_Base::ConvertColor<kPremul_SkAlphaType>(
        SkPMColor4f color) {
    return color.unpremul().array();
}

template<>
inline std::array<float, 4> SurfaceFillContext_Base::ConvertColor<kUnpremul_SkAlphaType>(
        SkColor4f color) {
    return color.premul().array();
}

template <SkAlphaType AlphaType>
std::array<float, 4> SurfaceFillContext_Base::adjustColorAlphaType(SkRGBA4f<AlphaType> color) const {
    if (AlphaType == kUnknown_SkAlphaType ||
        this->colorInfo().alphaType() == kUnknown_SkAlphaType) {
        return color.array();
    }
    return (AlphaType == this->colorInfo().alphaType()) ? color.array() : ConvertColor(color);
}

} // namespace skgpu

#endif // SurfaceFillContext_DEFINED
