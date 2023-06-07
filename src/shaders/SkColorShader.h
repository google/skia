/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/shaders/SkShaderBase.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

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

    ShaderType type() const override { return ShaderType::kColor; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    SkColor color() const { return fColor; }

private:
    friend void ::SkRegisterColorShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorShader)

    void flatten(SkWriteBuffer&) const override;

    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fColor;
        return true;
    }

    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;
#endif

    SkColor fColor;
};

class SkColor4Shader : public SkShaderBase {
public:
    SkColor4Shader(const SkColor4f&, sk_sp<SkColorSpace>);

    bool isOpaque() const override { return fColor.isOpaque(); }
    bool isConstant() const override { return true; }

    ShaderType type() const override { return ShaderType::kColor4; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    sk_sp<SkColorSpace> colorSpace() const { return fColorSpace; }
    SkColor4f color() const { return fColor; }

private:
    friend void ::SkRegisterColor4ShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColor4Shader)

    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;
#endif

    sk_sp<SkColorSpace> fColorSpace;
    const SkColor4f fColor;
};

class SkUpdatableColorShader : public SkShaderBase {
public:
    explicit SkUpdatableColorShader(SkColorSpace* cs);
#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder* builder,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override;
#endif

    ShaderType type() const override { return ShaderType::kUpdatableColor; }

    void updateColor(SkColor c) const;

private:
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

    SkColorSpaceXformSteps fSteps;
    mutable float fValues[4];
};

#endif
