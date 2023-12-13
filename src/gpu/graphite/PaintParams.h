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
#include <functional>  // std::function

class SkColorInfo;
class SkShader;

namespace skgpu::graphite {

class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
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
                         sk_sp<SkShader> clipShader,
                         DstReadRequirement dstReadReq,
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

    DstReadRequirement dstReadRequirement() const { return fDstReadReq; }
    bool skipColorXform() const { return fSkipColorXform; }
    bool dither() const { return fDither; }

    /** Converts an SkColor4f to the destination color space. */
    static SkColor4f Color4fPrepForDst(SkColor4f srgb, const SkColorInfo& dstColorInfo);

    void toKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

private:
    void addPaintColorToKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handlePrimitiveColor(const KeyContext&,
                              PaintParamsKeyBuilder*,
                              PipelineDataGatherer*) const;
    void handlePaintAlpha(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleColorFilter(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleDithering(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleDstRead(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

    SkColor4f            fColor;
    sk_sp<SkBlender>     fFinalBlender; // A nullptr here means SrcOver blending
    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fColorFilter;
    // A nullptr fPrimitiveBlender means there's no primitive color blending and it is skipped.
    // In the case where there is primitive blending, the primitive color is the source color and
    // the dest is the paint's color (or the paint's shader's computed color).
    sk_sp<SkBlender>     fPrimitiveBlender;
    sk_sp<SkShader>      fClipShader;
    DstReadRequirement   fDstReadReq;
    bool                 fSkipColorXform;
    bool                 fDither;
};

using AddToKeyFn = std::function<void()>;

void Blend(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*,
           AddToKeyFn addBlendToKey, AddToKeyFn addSrcToKey, AddToKeyFn addDstToKey);
void Compose(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*,
             AddToKeyFn addInnerToKey, AddToKeyFn addOuterToKey);
// Add a blend mode node for a specific SkBlendMode.
void AddKnownModeBlend(const KeyContext&,
                       PaintParamsKeyBuilder*,
                       PipelineDataGatherer*,
                       SkBlendMode);
// Add a blend mode node for an SkBlendMode that can vary
void AddModeBlend(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*, SkBlendMode);
void AddDstReadBlock(const KeyContext&,
                     PaintParamsKeyBuilder*,
                     PipelineDataGatherer*,
                     DstReadRequirement);
void AddDitherBlock(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*, SkColorType);

} // namespace skgpu::graphite

#endif // skgpu_PaintParams_DEFINED
