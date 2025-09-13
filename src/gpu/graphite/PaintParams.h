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
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"

class SkColorInfo;
class SkShader;

namespace skgpu::graphite {

class DrawContext;
class KeyContext;
class FloatStorageManager;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class Recorder;
class TextureProxy;

// TBD: If occlusion culling is eliminated as a phase, we can easily move the paint conversion
// back to Device when the command is recorded (similar to SkPaint -> GrPaint), and then
// PaintParams is not required as an intermediate representation.
// NOTE: Only represents the shading state of an SkPaint. Style and complex effects (mask filters,
// image filters, path effects) must be handled higher up. AA is not tracked since everything is
// assumed to be anti-aliased.
class PaintParams {
public:
    explicit PaintParams(const SkPaint&,
                         sk_sp<SkBlender> primitiveBlender,
                         const NonMSAAClip& nonMSAAClip,
                         sk_sp<SkShader> clipShader,
                         Coverage coverage,
                         TextureFormat targetFormat,
                         bool skipColorXform);

    PaintParams(const PaintParams&);
    ~PaintParams();

    PaintParams& operator=(const PaintParams&);

    SkColor4f color() const { return fColor; }

    std::optional<SkBlendMode> asFinalBlendMode() const;
    SkBlender* finalBlender() const { return fFinalBlender.get(); }
    sk_sp<SkBlender> refFinalBlender() const;

    SkShader* shader() const { return fShader.get(); }
    sk_sp<SkShader> refShader() const;

    SkColorFilter* colorFilter() const { return fColorFilter.get(); }
    sk_sp<SkColorFilter> refColorFilter() const;

    SkBlender* primitiveBlender() const { return fPrimitiveBlender.get(); }
    sk_sp<SkBlender> refPrimitiveBlender() const;

    TextureFormat targetFormat() const { return fTargetFormat;   }
    bool skipColorXform()        const { return fSkipColorXform; }
    bool dither()                const { return fDither;         }

    /** Converts an SkColor4f to the destination color space. */
    static SkColor4f Color4fPrepForDst(SkColor4f srgb, const SkColorInfo& dstColorInfo);

    using Result = std::tuple</*dependsOnDst*/bool, /*dstReadRequired*/bool,
                              /*usesAdvancedBlend*/bool>;
    std::optional<Result> toKey(const KeyContext&) const;

    void notifyImagesInUse(Recorder*, DrawContext*) const;

private:
    bool addPaintColorToKey(const KeyContext&) const;
    bool handlePrimitiveColor(const KeyContext&) const;
    bool handlePaintAlpha(const KeyContext&) const;
    bool handleColorFilter(const KeyContext&) const;
    bool handleDithering(const KeyContext&) const;
    bool handleDstRead(const KeyContext&) const;
    void handleClipping(const KeyContext&) const;

    SkColor4f            fColor;
    sk_sp<SkBlender>     fFinalBlender; // A nullptr here means SrcOver blending
    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fColorFilter;
    // A nullptr fPrimitiveBlender means there's no primitive color blending and it is skipped.
    // In the case where there is primitive blending, the primitive color is the source color and
    // the dest is the paint's color (or the paint's shader's computed color).
    sk_sp<SkBlender>     fPrimitiveBlender;
    NonMSAAClip          fNonMSAAClip;
    sk_sp<SkShader>      fClipShader;
    Coverage             fRendererCoverage;
    TextureFormat        fTargetFormat;
    bool                 fSkipColorXform;
    bool                 fDither;
};

// Add a fixed blend mode node for a specific SkBlendMode.
void AddFixedBlendMode(const KeyContext&, SkBlendMode);
// Add a blend mode node for an SkBlendMode that can vary
void AddBlendMode(const KeyContext&, SkBlendMode);
void AddDitherBlock(const KeyContext&, SkColorType);

} // namespace skgpu::graphite

#endif // skgpu_PaintParams_DEFINED
