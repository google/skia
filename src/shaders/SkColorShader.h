/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "src/shaders/SkShaderBase.h"

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be
    accomplished by just using the color field on the paint, but if an
    actual shader object is needed, this provides that feature.
*/
class SkColorShader : public SkShaderBase {
public:
    /** Create a ColorShader that ignores the color in the paint, and uses the
        specified color. Note: like all shaders, at draw time the paint's alpha
        will be respected, and is applied to the specified color.
    */
    explicit SkColorShader(SkColor c);

    bool isOpaque() const override;
    bool isConstant() const override { return true; }

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkColorShader)

    void flatten(SkWriteBuffer&) const override;

    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fColor;
        return true;
    }

    bool onAppendStages(const SkStageRec&) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::F32 x, skvm::F32 y, skvm::Color paint,
                          const SkMatrix& ctm, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    SkColor fColor;
};

class SkColor4Shader : public SkShaderBase {
public:
    SkColor4Shader(const SkColor4f&, sk_sp<SkColorSpace>);

    bool isOpaque()   const override { return fColor.isOpaque(); }
    bool isConstant() const override { return true; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkColor4Shader)

    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::F32 x, skvm::F32 y, skvm::Color paint,
                          const SkMatrix& ctm, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    sk_sp<SkColorSpace> fColorSpace;
    const SkColor4f     fColor;
};

#endif
