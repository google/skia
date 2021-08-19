/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#include "src/gpu/effects/GrDisableColorXP.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

// Draws a simple array of triangles.
class SimpleTriangleShader : public GrPathTessellationShader {
public:
    SimpleTriangleShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrPathTessellationShader(kTessellate_SimpleTriangleShader_ClassID,
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_SimpleTriangleShader"; }
    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> SimpleTriangleShader::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrShaderCaps&, const GrPathTessellationShader&,
                            GrGLSLVertexBuilder* v, GrGPArgs* gpArgs) override {
            v->codeAppend(R"(
            float2 localcoord = inputPoint;
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        }
    };
    return std::make_unique<Impl>();
}

}  // namespace

GrPathTessellationShader* GrPathTessellationShader::MakeSimpleTriangleShader(
        SkArenaAlloc* arena, const SkMatrix& viewMatrix, const SkPMColor4f& color) {
    return arena->make<SimpleTriangleShader>(viewMatrix, color);
}

const GrPipeline* GrPathTessellationShader::MakeStencilOnlyPipeline(
        const ProgramArgs& args,
        GrAAType aaType,
        GrTessellationPathFlags pathFlags,
        const GrAppliedHardClip& hardClip) {
    using PathFlags = GrTessellationPathFlags;
    GrPipeline::InitArgs pipelineArgs;
    if (args.fCaps->wireframeSupport() && (pathFlags & PathFlags::kWireframe)) {
        pipelineArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    }
    pipelineArgs.fCaps = args.fCaps;
    return args.fArena->make<GrPipeline>(pipelineArgs,
                                            GrDisableColorXPFactory::MakeXferProcessor(),
                                            hardClip);
}

// Evaluate our point of interest using numerically stable linear interpolations. We add our own
// "safe_mix" method to guarantee we get exactly "b" when T=1. The builtin mix() function seems
// spec'd to behave this way, but empirical results results have shown it does not always.
const char* GrPathTessellationShader::Impl::kEvalRationalCubicFn = R"(
float3 safe_mix(float3 a, float3 b, float T, float one_minus_T) {
    return a*one_minus_T + b*T;
}
float2 eval_rational_cubic(float4x3 P, float T) {
    float one_minus_T = 1.0 - T;
    float3 ab = safe_mix(P[0], P[1], T, one_minus_T);
    float3 bc = safe_mix(P[1], P[2], T, one_minus_T);
    float3 cd = safe_mix(P[2], P[3], T, one_minus_T);
    float3 abc = safe_mix(ab, bc, T, one_minus_T);
    float3 bcd = safe_mix(bc, cd, T, one_minus_T);
    float3 abcd = safe_mix(abc, bcd, T, one_minus_T);
    return abcd.xy / abcd.z;
})";

void GrPathTessellationShader::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const auto& shader = args.fGeomProc.cast<GrPathTessellationShader>();
    args.fVaryingHandler->emitAttributes(shader);

    // Vertex shader.
    const char* affineMatrix, *translate;
    fAffineMatrixUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                            kFloat4_GrSLType, "affineMatrix",
                                                            &affineMatrix);
    fTranslateUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                         kFloat2_GrSLType, "translate", &translate);
    args.fVertBuilder->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s);", affineMatrix);
    args.fVertBuilder->codeAppendf("float2 TRANSLATE = %s;", translate);
    this->emitVertexCode(*args.fShaderCaps, shader, args.fVertBuilder, gpArgs);

    // Fragment shader.
    const char* color;
    fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                     kHalf4_GrSLType, "color", &color);
    args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, color);
    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
}

void GrPathTessellationShader::Impl::setData(const GrGLSLProgramDataManager& pdman, const
                                             GrShaderCaps&, const GrGeometryProcessor& geomProc) {
    const auto& shader = geomProc.cast<GrTessellationShader>();
    const SkMatrix& m = shader.viewMatrix();
    pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY());
    pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());

    const SkPMColor4f& color = shader.color();
    pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);
}
