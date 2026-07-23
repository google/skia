/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PaintParams_DEFINED
#define skgpu_graphite_PaintParams_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/private/SkEnumBitMask.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"

class SkColorInfo;
class SkImage;
class SkShader;

namespace skgpu::graphite {

class DrawContext;
class KeyContext;
class StorageBufferManager;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class Recorder;
class TextureProxy;
class UniquePaintParamsID;

// NOTE: Only represents the shading state of an SkPaint. Style and complex effects (mask filters,
// image filters, path effects) must be handled higher up. AA is not tracked since everything is
// assumed to be anti-aliased.
//
// The full shading of a draw is a combination of the PaintParams extracted from an SkPaint, any
// analytic NonMSAA clipping, analytic coverage from a RenderStep, and the target TextureFormat
// being rendered into. These additional parameters are aggregated together in ShadingParams.
//
// Both PaintParams and ShadingParams are meant to be short-lived objects used for processing a
// draw's effects into a UniquePaintID and extracted uniforms and textures. As such, they do not
// keep high-level Skia objects alive since these Params types should go out of scope by the end of
// the draw call.
class PaintParams {
public:
    // Stores just the parameters of the implicit image shading model used by drawImageRect and
    // other image-drawing APIs, e.g. to apply SkImageShader::MakeForDrawRect without the
    // overhead of creating additional SkShader objects. Assumes clamp tiling; if no clamping is
    // required, set to the image's bounds.
    struct SimpleImage {
        // fImage (required) and fLocalMatrix (optional) must outlive the PaintParams object
        const SkImage* fImage;
        const SkMatrix* fLocalMatrix = nullptr;
        // Post local matrix strict clamping rectangle (e.g. relative to image's texels)
        SkRect fSubset;
        SkSamplingOptions fSamplingOptions;
    };

    // Converts an SkPaint to PaintParams, possibly adding a primitive blender (e.g. for
    // drawVertices or text rendering).
    explicit PaintParams(const SkPaint& paint,
                         const SkBlender* primitiveBlender = nullptr,
                         bool skipColorXform = false,
                         bool ignoreShader = false);

    // Converts an SkPaint to PaintParams and accounts for the implicit SkImage shader override
    // from drawImageRect and related functions. Multiplies `xtraAlpha` with the paint's alpha.
    //
    // NOTE: Does not copy `imageOverride`, this must live for the lifetime of PaintParams
    PaintParams(const SkPaint&, const SimpleImage& imageOverride, float xtraAlpha=1.f);

    // Converts an SkPaint to PaintParams, overrides the paint's base color with `colorOverride`
    // and multiplies the base and override color alphas together. Since this is a color override,
    // any shader in the SkPaint is ignored.
    PaintParams(const SkPaint&, const SkColor4f& colorOverride);

    // Creates a constant color PaintParams with the specific blend mode.
    PaintParams(const SkColor4f& color, SkBlendMode finalBlendMode);

    // Creates a copy of this PaintParams with the specified primitive color and blender.
    PaintParams makeWithPrimitiveColor(const SkBlender* primitiveBlender,
                                       const SkColor4f& primitiveColorOverride) const;

    const SkColor4f& color() const { return fColor; }
    const SkShader* shader() const { return fShader; }
    const SimpleImage* imageShader() const { return fImageShader; }
    const SkColorFilter* colorFilter() const { return fColorFilter; }
    const SkBlender* primitiveBlender() const { return fPrimitiveBlender; }
    const std::optional<SkColor4f>& primitiveColorOverride() const {
        return fPrimitiveColorOverride;
    }
    bool skipPrimitiveColorXform() const { return fSkipColorXform; }

    const SkBlender* finalBlender() const { return fFinalBlend.first; }
    // Must also check finalBlender() to see if that overrides finalBlendMode() behavior.
    SkBlendMode finalBlendMode() const { SkASSERT(!fFinalBlend.first); return fFinalBlend.second; }

    bool dither() const { return fDither; }

    /** Converts an SkColor4f to the destination color space. */
    static SkColor4f Color4fPrepForDst(SkColor4f srgb, const SkColorInfo& dstColorInfo);

#if defined(SK_DEBUG)
    // Creates a new PaintParams that is opaque and won't depend on the dst
    static PaintParams MakeOpaque(const PaintParams& paint);
#endif

private:
    PaintParams(const SkPaint&,
                const SimpleImage* imageOverride,
                const SkBlender* primitiveBlender,
                bool skipColorXform,
                bool ignoreShader);

    SkColor4f fColor;

    // Either a non-null SkBlender for runtime blending, or the SkBlendMode to use instead. If
    // the blender is non-null, the blend mode is set to kSrc to match the HW blend config used for
    // shader-based blending.
    std::pair<const SkBlender*, SkBlendMode> fFinalBlend;

    const SkShader*      fShader;
    const SimpleImage*   fImageShader; // Overrides fShader for color images, mixes for alpha
    const SkColorFilter* fColorFilter;

    // A nullptr fPrimitiveBlender means there's no primitive color blending and it is skipped.
    // In the case where there is primitive blending, the primitive color is the source color and
    // the dest is the paint's color (or the paint's shader's computed color).
    const SkBlender* fPrimitiveBlender;
    // When a fPrimitiveColorOverride is present, it is used instead of any defined primitive colors
    // in the vertices for primitive color blending. This is done to enable primitive color blending
    // for render steps which don't emit primitive colors.
    std::optional<SkColor4f> fPrimitiveColorOverride;

    bool fSkipColorXform;
    bool fDither;
};

// ShadingParams wraps a PaintParams with the additional per-pixel state to handle clipping and
// anti-aliasing, as well as making the final determinations for how blending will be implemented
// given the current hardware.
class ShadingParams {
public:
    // NOTE: Does not copy `paint`, `nonMSAAClip` or `clipShader`; these must outlive ShadingParams.
    ShadingParams(const Caps* caps,
                  const PaintParams& paint,
                  const NonMSAAClip& nonMSAAClip,
                  const SkShader* clipShader,
                  Coverage coverage,
                  TextureFormat targetFormat);

    bool dstReadRequired() const { return SkToBool(fDstUsage & DstUsage::kDstReadRequired); }

    using Result = std::tuple<UniquePaintParamsID, SkEnumBitMask<DstUsage>>;
    std::optional<Result> toKey(const KeyContext&) const;

    // Quickly produce a new paint ID that is the same paint effects described by `origPaint`
    // except that it will be combined with a RenderStep that has Coverage::kNone.
    //
    // This must *only* be called if the prior call to toKey() returned kDstOnlyUsedByRenderer.
    // The KeyContext's PaintParamsKeyBuilder must not have been modified after toKey() returned
    // the key producing `origPaint`.
    UniquePaintParamsID optimizeForOpacity(const KeyContext&, UniquePaintParamsID origPaint) const;

private:
    bool addPaintColorToKey(const KeyContext&) const;
    bool handlePrimitiveColor(const KeyContext&) const;
    bool handlePaintAlpha(const KeyContext&) const;
    bool handleColorFilter(const KeyContext&) const;
    bool handleDithering(const KeyContext&) const;
    void handleClipping(const KeyContext&) const;

    const PaintParams& fPaint;
    const NonMSAAClip& fNonMSAAClip;
    const SkShader*    fClipShader;

    // Base (incomplete) dst usage that will be augmented by opacity analysis calculated in toKey().
    // This is only relevant for kSrcOver, fDstUsage is set assuming the paint is opaque; if it's
    // not actually opaque it will be adjusted accordingly.
    const SkEnumBitMask<DstUsage> fDstUsage;

#if defined(SK_DEBUG)
    UniquePaintParamsID validateOpacityOptimization(const KeyContext&) const;

    const Coverage fCoverage;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_PaintParams_DEFINED
