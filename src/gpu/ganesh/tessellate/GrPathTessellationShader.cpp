/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/tessellate/GrPathTessellationShader.h"

#include "include/core/SkMatrix.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/effects/GrDisableColorXP.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/Tessellation.h"

#include <cstdint>
#include <memory>

class GrAppliedHardClip;

using namespace skia_private;

namespace {

using namespace skgpu::tess;

// Draws a simple array of triangles.
class SimpleTriangleShader : public GrPathTessellationShader {
public:
    SimpleTriangleShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrPathTessellationShader(kTessellate_SimpleTriangleShader_ClassID,
                                       GrPrimitiveType::kTriangles,
                                       viewMatrix,
                                       color,
                                       PatchAttribs::kNone) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     SkSLType::kFloat2};
        this->setVertexAttributesWithImplicitOffsets(&kInputPointAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_SimpleTriangleShader"; }
    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const final {}
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> SimpleTriangleShader::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrShaderCaps&,
                            const GrPathTessellationShader&,
                            GrGLSLVertexBuilder* v,
                            GrGLSLVaryingHandler*,
                            GrGPArgs* gpArgs) override {
            v->codeAppend(
            "float2 localcoord = inputPoint;"
            "float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;");
            gpArgs->fLocalCoordVar.set(SkSLType::kFloat2, "localcoord");
            gpArgs->fPositionVar.set(SkSLType::kFloat2, "vertexpos");
        }
    };
    return std::make_unique<Impl>();
}


// Uses instanced draws to triangulate standalone closed curves with a "middle-out" topology.
// Middle-out draws a triangle with vertices at T=[0, 1/2, 1] and then recurses breadth first:
//
//   depth=0: T=[0, 1/2, 1]
//   depth=1: T=[0, 1/4, 2/4], T=[2/4, 3/4, 1]
//   depth=2: T=[0, 1/8, 2/8], T=[2/8, 3/8, 4/8], T=[4/8, 5/8, 6/8], T=[6/8, 7/8, 1]
//   ...
//
// The shader determines how many segments are required to render each individual curve smoothly,
// and emits empty triangles at any vertices whose sk_VertexIDs are higher than necessary. It is the
// caller's responsibility to draw enough vertices per instance for the most complex curve in the
// batch to render smoothly (i.e., NumTrianglesAtResolveLevel() * 3).
class MiddleOutShader : public GrPathTessellationShader {
public:
    MiddleOutShader(const GrShaderCaps& shaderCaps, const SkMatrix& viewMatrix,
                    const SkPMColor4f& color, PatchAttribs attribs)
            : GrPathTessellationShader(kTessellate_MiddleOutShader_ClassID,
                                       GrPrimitiveType::kTriangles, viewMatrix, color, attribs) {
        fInstanceAttribs.emplace_back("p01", kFloat4_GrVertexAttribType, SkSLType::kFloat4);
        fInstanceAttribs.emplace_back("p23", kFloat4_GrVertexAttribType, SkSLType::kFloat4);
        if (fAttribs & PatchAttribs::kFanPoint) {
            fInstanceAttribs.emplace_back("fanPointAttrib",
                                          kFloat2_GrVertexAttribType,
                                          SkSLType::kFloat2);
        }
        if (fAttribs & PatchAttribs::kColor) {
            fInstanceAttribs.emplace_back("colorAttrib",
                                          (fAttribs & PatchAttribs::kWideColorIfEnabled)
                                                  ? kFloat4_GrVertexAttribType
                                                  : kUByte4_norm_GrVertexAttribType,
                                          SkSLType::kHalf4);
        }
        if (fAttribs & PatchAttribs::kExplicitCurveType) {
            // A conic curve is written out with p3=[w,Infinity], but GPUs that don't support
            // infinity can't detect this. On these platforms we also write out an extra float with
            // each patch that explicitly tells the shader what type of curve it is.
            fInstanceAttribs.emplace_back("curveType", kFloat_GrVertexAttribType, SkSLType::kFloat);
        }
        this->setInstanceAttributesWithImplicitOffsets(fInstanceAttribs.data(),
                                                       fInstanceAttribs.size());
        SkASSERT(fInstanceAttribs.size() <= kMaxInstanceAttribCount);
        SkASSERT(this->instanceStride() ==
                 sizeof(SkPoint) * 4 + PatchAttribsStride(fAttribs));

        constexpr static Attribute kVertexAttrib("resolveLevel_and_idx", kFloat2_GrVertexAttribType,
                                                 SkSLType::kFloat2);
        this->setVertexAttributesWithImplicitOffsets(&kVertexAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_MiddleOutShader"; }
    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const final {
        // When color is in a uniform, it's always wide so we need to ignore kWideColorIfEnabled.
        // When color is in an attrib, its wideness is accounted for as part of the attrib key in
        // GrGeometryProcessor::getAttributeKey().
        // Either way, we get the correct key by ignoring .
        b->add32((uint32_t)(fAttribs & ~PatchAttribs::kWideColorIfEnabled));
    }
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    constexpr static int kMaxInstanceAttribCount = 5;
    STArray<kMaxInstanceAttribCount, Attribute> fInstanceAttribs;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> MiddleOutShader::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrShaderCaps& shaderCaps,
                            const GrPathTessellationShader& shader,
                            GrGLSLVertexBuilder* v,
                            GrGLSLVaryingHandler* varyingHandler,
                            GrGPArgs* gpArgs) override {
            const MiddleOutShader& middleOutShader = shader.cast<MiddleOutShader>();
            v->defineConstant("PRECISION", skgpu::tess::kPrecision);
            v->defineConstant("MAX_FIXED_RESOLVE_LEVEL",
                              (float)skgpu::tess::kMaxResolveLevel);
            v->defineConstant("MAX_FIXED_SEGMENTS",
                              (float)(skgpu::tess::kMaxParametricSegments));
            v->insertFunction(GrTessellationShader::WangsFormulaSkSL());
            if (middleOutShader.fAttribs & PatchAttribs::kExplicitCurveType) {
                v->insertFunction(SkStringPrintf(
                "bool is_conic_curve() {"
                    "return curveType != %g;"
                "}", skgpu::tess::kCubicCurveType).c_str());
                v->insertFunction(SkStringPrintf(
                "bool is_triangular_conic_curve() {"
                    "return curveType == %g;"
                "}", skgpu::tess::kTriangularConicCurveType).c_str());
            } else {
                SkASSERT(shaderCaps.fInfinitySupport);
                v->insertFunction(
                "bool is_conic_curve() { return isinf(p23.w); }"
                "bool is_triangular_conic_curve() { return isinf(p23.z); }");
            }
            if (shaderCaps.fBitManipulationSupport) {
                v->insertFunction(
                "float ldexp_portable(float x, float p) {"
                    "return ldexp(x, int(p));"
                "}");
            } else {
                v->insertFunction(
                "float ldexp_portable(float x, float p) {"
                    "return x * exp2(p);"
                "}");
            }
            v->codeAppend(
            "float resolveLevel = resolveLevel_and_idx.x;"
            "float idxInResolveLevel = resolveLevel_and_idx.y;"
            "float2 localcoord;");
            if (middleOutShader.fAttribs & PatchAttribs::kFanPoint) {
                v->codeAppend(
                // A negative resolve level means this is the fan point.
                "if (resolveLevel < 0) {"
                    "localcoord = fanPointAttrib;"
                "} else ");  // Fall through to next if (). Trailing space is important.
            }
            v->codeAppend(
            "if (is_triangular_conic_curve()) {"
                // This patch is an exact triangle.
                "localcoord = (resolveLevel != 0) ? p01.zw"
                           ": (idxInResolveLevel != 0) ? p23.xy"
                                                      ": p01.xy;"
            "} else {"
                "float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;"
                "float w = -1;"  // w < 0 tells us to treat the instance as an integral cubic.
                "float maxResolveLevel;"
                "if (is_conic_curve()) {"
                    // Conics are 3 points, with the weight in p3.
                    "w = p3.x;"
                    "maxResolveLevel = wangs_formula_conic_log2(PRECISION, AFFINE_MATRIX * p0,"
                                                                          "AFFINE_MATRIX * p1,"
                                                                          "AFFINE_MATRIX * p2, w);"
                    "p1 *= w;"  // Unproject p1.
                    "p3 = p2;"  // Duplicate the endpoint for shared code that also runs on cubics.
                "} else {"
                    // The patch is an integral cubic.
                    "maxResolveLevel = wangs_formula_cubic_log2(PRECISION, p0, p1, p2, p3,"
                                                               "AFFINE_MATRIX);"
                "}"
                "if (resolveLevel > maxResolveLevel) {"
                    // This vertex is at a higher resolve level than we need. Demote to a lower
                    // resolveLevel, which will produce a degenerate triangle.
                    "idxInResolveLevel = floor(ldexp_portable(idxInResolveLevel,"
                                                             "maxResolveLevel - resolveLevel));"
                    "resolveLevel = maxResolveLevel;"
                "}"
                // Promote our location to a discrete position in the maximum fixed resolve level.
                // This is extra paranoia to ensure we get the exact same fp32 coordinates for
                // colocated points from different resolve levels (e.g., the vertices T=3/4 and
                // T=6/8 should be exactly colocated).
                "float fixedVertexID = floor(.5 + ldexp_portable("
                        "idxInResolveLevel, MAX_FIXED_RESOLVE_LEVEL - resolveLevel));"
                "if (0 < fixedVertexID && fixedVertexID < MAX_FIXED_SEGMENTS) {"
                    "float T = fixedVertexID * (1 / MAX_FIXED_SEGMENTS);"

                    // Evaluate at T. Use De Casteljau's for its accuracy and stability.
                    "float2 ab = mix(p0, p1, T);"
                    "float2 bc = mix(p1, p2, T);"
                    "float2 cd = mix(p2, p3, T);"
                    "float2 abc = mix(ab, bc, T);"
                    "float2 bcd = mix(bc, cd, T);"
                    "float2 abcd = mix(abc, bcd, T);"

                    // Evaluate the conic weight at T.
                    "float u = mix(1.0, w, T);"
                    "float v = w + 1 - u;"  // == mix(w, 1, T)
                    "float uv = mix(u, v, T);"

                    "localcoord = (w < 0) ?" /*cubic*/ "abcd:" /*conic*/ "abc/uv;"
                "} else {"
                    "localcoord = (fixedVertexID == 0) ? p0.xy : p3.xy;"
                "}"
            "}"
            "float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;");
            gpArgs->fLocalCoordVar.set(SkSLType::kFloat2, "localcoord");
            gpArgs->fPositionVar.set(SkSLType::kFloat2, "vertexpos");
            if (middleOutShader.fAttribs & PatchAttribs::kColor) {
                GrGLSLVarying colorVarying(SkSLType::kHalf4);
                varyingHandler->addVarying("color",
                                           &colorVarying,
                                           GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
                v->codeAppendf("%s = colorAttrib;", colorVarying.vsOut());
                fVaryingColorName = colorVarying.fsIn();
            }
        }
    };
    return std::make_unique<Impl>();
}

}  // namespace

GrPathTessellationShader* GrPathTessellationShader::Make(const GrShaderCaps& shaderCaps,
                                                         SkArenaAlloc* arena,
                                                         const SkMatrix& viewMatrix,
                                                         const SkPMColor4f& color,
                                                         PatchAttribs attribs) {
    // We should use explicit curve type when, and only when, there isn't infinity support.
    // Otherwise the GPU can infer curve type based on infinity.
    SkASSERT(shaderCaps.fInfinitySupport != (attribs & PatchAttribs::kExplicitCurveType));
    return arena->make<MiddleOutShader>(shaderCaps, viewMatrix, color, attribs);
}

GrPathTessellationShader* GrPathTessellationShader::MakeSimpleTriangleShader(
        SkArenaAlloc* arena, const SkMatrix& viewMatrix, const SkPMColor4f& color) {
    return arena->make<SimpleTriangleShader>(viewMatrix, color);
}

const GrPipeline* GrPathTessellationShader::MakeStencilOnlyPipeline(
        const ProgramArgs& args,
        GrAAType aaType,
        const GrAppliedHardClip& hardClip,
        GrPipeline::InputFlags pipelineFlags) {
    GrPipeline::InitArgs pipelineArgs;
    pipelineArgs.fInputFlags = pipelineFlags;
    pipelineArgs.fCaps = args.fCaps;
    return args.fArena->make<GrPipeline>(pipelineArgs,
                                         GrDisableColorXPFactory::MakeXferProcessor(),
                                         hardClip);
}

// Evaluate our point of interest using numerically stable linear interpolations. We add our own
// "safe_mix" method to guarantee we get exactly "b" when T=1. The builtin mix() function seems
// spec'd to behave this way, but empirical results results have shown it does not always.
const char* GrPathTessellationShader::Impl::kEvalRationalCubicFn =
"float3 safe_mix(float3 a, float3 b, float T, float one_minus_T) {"
    "return a*one_minus_T + b*T;"
"}"
"float2 eval_rational_cubic(float4x3 P, float T) {"
    "float one_minus_T = 1.0 - T;"
    "float3 ab = safe_mix(P[0], P[1], T, one_minus_T);"
    "float3 bc = safe_mix(P[1], P[2], T, one_minus_T);"
    "float3 cd = safe_mix(P[2], P[3], T, one_minus_T);"
    "float3 abc = safe_mix(ab, bc, T, one_minus_T);"
    "float3 bcd = safe_mix(bc, cd, T, one_minus_T);"
    "float3 abcd = safe_mix(abc, bcd, T, one_minus_T);"
    "return abcd.xy / abcd.z;"
"}";

void GrPathTessellationShader::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const auto& shader = args.fGeomProc.cast<GrPathTessellationShader>();
    args.fVaryingHandler->emitAttributes(shader);

    // Vertex shader.
    const char* affineMatrix, *translate;
    fAffineMatrixUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                            SkSLType::kFloat4, "affineMatrix",
                                                            &affineMatrix);
    fTranslateUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                         SkSLType::kFloat2, "translate", &translate);
    args.fVertBuilder->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s.xy, %s.zw);",
                                   affineMatrix, affineMatrix);
    args.fVertBuilder->codeAppendf("float2 TRANSLATE = %s;", translate);
    this->emitVertexCode(*args.fShaderCaps,
                         shader,
                         args.fVertBuilder,
                         args.fVaryingHandler,
                         gpArgs);

    // Fragment shader.
    if (!(shader.fAttribs & PatchAttribs::kColor)) {
        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                         SkSLType::kHalf4, "color", &color);
        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, color);
    } else {
        args.fFragBuilder->codeAppendf("half4 %s = %s;",
                                       args.fOutputColor, fVaryingColorName.c_str());
    }
    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
}

void GrPathTessellationShader::Impl::setData(const GrGLSLProgramDataManager& pdman, const
                                             GrShaderCaps&, const GrGeometryProcessor& geomProc) {
    const auto& shader = geomProc.cast<GrPathTessellationShader>();
    const SkMatrix& m = shader.viewMatrix();
    pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY());
    pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());

    if (!(shader.fAttribs & PatchAttribs::kColor)) {
        const SkPMColor4f& color = shader.color();
        pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);
    }
}
