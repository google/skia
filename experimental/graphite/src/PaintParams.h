/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_PaintParams_DEFINED
#define skgpu_PaintParams_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"

class SkShader;

namespace skgpu {

// TBD: If occlusion culling is eliminated as a phase, we can easily move the paint conversion
// back to Device when the command is recorded (similar to SkPaint -> GrPaint), and then
// PaintParams is not required as an intermediate representation.
// NOTE: Only represents the shading state of an SkPaint. Style and complex effects (mask filters,
// image filters, path effects) must be handled higher up. AA is not tracked since everything is
// assumed to be anti-aliased.
class PaintParams {
public:
    PaintParams(const SkColor4f& color, SkBlendMode, sk_sp<SkShader>);
    explicit PaintParams(const SkPaint&);

    PaintParams(const PaintParams&);
    ~PaintParams();

    PaintParams& operator=(const PaintParams&);

    SkColor4f color() const { return fColor; }
    SkBlendMode blendMode() const { return fBlendMode; }
    SkShader* shader() const { return fShader.get(); }
    sk_sp<SkShader> refShader() const;

private:
    SkColor4f       fColor;
    SkBlendMode     fBlendMode;
    sk_sp<SkShader> fShader; // For now only use SkShader::asAGradient() when converting to GPU
    // TODO: Will also store ColorFilter, custom Blender, dither, and any extra shader from an
    // active clipShader().
};

} // namespace skgpu

#endif // skgpu_PaintParams_DEFINED
