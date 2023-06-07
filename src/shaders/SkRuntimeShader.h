/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRuntimeShader_DEFINED
#define SkRuntimeShader_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#include <vector>

class SkColorSpace;
class SkReadBuffer;
class SkWriteBuffer;
struct SkIPoint;
struct SkStageRec;

using UniformsCallback = SkRuntimeEffectPriv::UniformsCallback;

class SkRuntimeShader : public SkShaderBase {
public:
    SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
                    sk_sp<SkSL::DebugTracePriv> debugTrace,
                    sk_sp<const SkData> uniforms,
                    SkSpan<SkRuntimeEffect::ChildPtr> children);

    SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
                    sk_sp<SkSL::DebugTracePriv> debugTrace,
                    UniformsCallback uniformsCallback,
                    SkSpan<SkRuntimeEffect::ChildPtr> children);

    SkRuntimeEffect::TracedShader makeTracedClone(const SkIPoint& coord);

    bool isOpaque() const override { return fEffect->alwaysOpaque(); }

    ShaderType type() const override { return ShaderType::kRuntime; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override;
#endif

    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder* p,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec& mRec,
                        const SkColorInfo& colorInfo,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override;
#endif

    void flatten(SkWriteBuffer& buffer) const override;

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    sk_sp<SkRuntimeEffect> effect() const { return fEffect; }
    std::vector<SkRuntimeEffect::ChildPtr> children() const { return fChildren; }

    sk_sp<const SkData> uniformData(const SkColorSpace* dstCS) const;

    SK_FLATTENABLE_HOOKS(SkRuntimeShader)

private:
    enum Flags {
        kHasLegacyLocalMatrix_Flag = 1 << 1,
    };

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkSL::DebugTracePriv> fDebugTrace;
    sk_sp<const SkData> fUniformData;
    UniformsCallback fUniformsCallback;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

#endif
