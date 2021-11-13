/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#include "src/core/SkMathPriv.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/PathTessellator.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

using skgpu::PatchAttribs;
using skgpu::VertexWriter;

namespace {

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
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color, attribs) {
        fInstanceAttribs.emplace_back("p01", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fInstanceAttribs.emplace_back("p23", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fAttribs & PatchAttribs::kFanPoint) {
            fInstanceAttribs.emplace_back("fanPointAttrib",
                                          kFloat2_GrVertexAttribType,
                                          kFloat2_GrSLType);
        }
        if (fAttribs & PatchAttribs::kColor) {
            fInstanceAttribs.emplace_back("colorAttrib",
                                          (fAttribs & PatchAttribs::kWideColorIfEnabled)
                                                  ? kFloat4_GrVertexAttribType
                                                  : kUByte4_norm_GrVertexAttribType,
                                          kHalf4_GrSLType);
        }
        if (fAttribs & PatchAttribs::kExplicitCurveType) {
            // A conic curve is written out with p3=[w,Infinity], but GPUs that don't support
            // infinity can't detect this. On these platforms we also write out an extra float with
            // each patch that explicitly tells the shader what type of curve it is.
            fInstanceAttribs.emplace_back("curveType", kFloat_GrVertexAttribType, kFloat_GrSLType);
        }
        this->setInstanceAttributes(fInstanceAttribs.data(), fInstanceAttribs.count());
        SkASSERT(fInstanceAttribs.count() <= kMaxInstanceAttribCount);
        SkASSERT(this->instanceStride() ==
                 sizeof(SkPoint) * 4 + skgpu::PatchAttribsStride(fAttribs));

        constexpr static Attribute kVertexAttrib("resolveLevel_and_idx", kFloat2_GrVertexAttribType,
                                                 kFloat2_GrSLType);
        this->setVertexAttributes(&kVertexAttrib, 1);
    }

    int maxTessellationSegments(const GrShaderCaps&) const override {
        return 1 << skgpu::PathTessellator::kMaxFixedResolveLevel;
    }

private:
    const char* name() const final { return "tessellate_MiddleOutShader"; }
    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        // When color is in a uniform, it's always wide so we need to ignore kWideColorIfEnabled.
        // When color is in an attrib, its wideness is accounted for as part of the attrib key in
        // GrGeometryProcessor::getAttributeKey().
        // Either way, we get the correct key by ignoring .
        b->add32((uint32_t)(fAttribs & ~PatchAttribs::kWideColorIfEnabled));
    }
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    constexpr static int kMaxInstanceAttribCount = 5;
    SkSTArray<kMaxInstanceAttribCount, Attribute> fInstanceAttribs;
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
            v->defineConstant("PRECISION", skgpu::kTessellationPrecision);
            v->defineConstant("MAX_FIXED_RESOLVE_LEVEL",
                              (float)skgpu::PathTessellator::kMaxFixedResolveLevel);
            v->defineConstant("MAX_FIXED_SEGMENTS",
                              (float)(1 << skgpu::PathTessellator::kMaxFixedResolveLevel));
            v->insertFunction(skgpu::wangs_formula::as_sksl().c_str());
            if (middleOutShader.fAttribs & PatchAttribs::kExplicitCurveType) {
                v->insertFunction(SkStringPrintf(R"(
                bool is_conic_curve() { return curveType != %g; })", kCubicCurveType).c_str());
                v->insertFunction(SkStringPrintf(R"(
                bool is_triangular_conic_curve() {
                    return curveType == %g;
                })", kTriangularConicCurveType).c_str());
            } else {
                SkASSERT(shaderCaps.infinitySupport());
                v->insertFunction(R"(
                bool is_conic_curve() { return isinf(p23.w); }
                bool is_triangular_conic_curve() { return isinf(p23.z); })");
            }
            if (shaderCaps.bitManipulationSupport()) {
                v->insertFunction(R"(
                float ldexp_portable(float x, float p) {
                    return ldexp(x, int(p));
                })");
            } else {
                v->insertFunction(R"(
                float ldexp_portable(float x, float p) {
                    return x * exp2(p);
                })");
            }
            v->codeAppend(R"(
            float resolveLevel = resolveLevel_and_idx.x;
            float idxInResolveLevel = resolveLevel_and_idx.y;
            float2 localcoord;)");
            if (middleOutShader.fAttribs & PatchAttribs::kFanPoint) {
                v->codeAppend(R"(
                // A negative resolve level means this is the fan point.
                if (resolveLevel < 0) {
                    localcoord = fanPointAttrib;
                } else)");  // Fall through to next if ().
            }
            v->codeAppend(R"(
            if (is_triangular_conic_curve()) {
                // This patch is an exact triangle.
                localcoord = (resolveLevel != 0)      ? p01.zw
                           : (idxInResolveLevel != 0) ? p23.xy
                                                      : p01.xy;
            } else {
                float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;
                float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
                float maxResolveLevel;
                if (is_conic_curve()) {
                    // Conics are 3 points, with the weight in p3.
                    w = p3.x;
                    maxResolveLevel = wangs_formula_conic_log2(PRECISION, AFFINE_MATRIX * p0,
                                                                          AFFINE_MATRIX * p1,
                                                                          AFFINE_MATRIX * p2, w);
                    p1 *= w;  // Unproject p1.
                    p3 = p2;  // Duplicate the endpoint for shared code that also runs on cubics.
                } else {
                    // The patch is an integral cubic.
                    maxResolveLevel = wangs_formula_cubic_log2(PRECISION, p0, p1, p2, p3,
                                                               AFFINE_MATRIX);
                }
                if (resolveLevel > maxResolveLevel) {
                    // This vertex is at a higher resolve level than we need. Demote to a lower
                    // resolveLevel, which will produce a degenerate triangle.
                    idxInResolveLevel = floor(ldexp_portable(idxInResolveLevel,
                                                             maxResolveLevel - resolveLevel));
                    resolveLevel = maxResolveLevel;
                }
                // Promote our location to a discrete position in the maximum fixed resolve level.
                // This is extra paranoia to ensure we get the exact same fp32 coordinates for
                // colocated points from different resolve levels (e.g., the vertices T=3/4 and
                // T=6/8 should be exactly colocated).
                float fixedVertexID = floor(.5 + ldexp_portable(
                        idxInResolveLevel, MAX_FIXED_RESOLVE_LEVEL - resolveLevel));
                if (0 < fixedVertexID && fixedVertexID < MAX_FIXED_SEGMENTS) {
                    float T = fixedVertexID * (1 / MAX_FIXED_SEGMENTS);

                    // Evaluate at T. Use De Casteljau's for its accuracy and stability.
                    float2 ab = mix(p0, p1, T);
                    float2 bc = mix(p1, p2, T);
                    float2 cd = mix(p2, p3, T);
                    float2 abc = mix(ab, bc, T);
                    float2 bcd = mix(bc, cd, T);
                    float2 abcd = mix(abc, bcd, T);

                    // Evaluate the conic weight at T.
                    float u = mix(1.0, w, T);
                    float v = w + 1 - u;  // == mix(w, 1, T)
                    float uv = mix(u, v, T);

                    localcoord = (w < 0) ? /*cubic*/ abcd : /*conic*/ abc/uv;
                } else {
                    localcoord = (fixedVertexID == 0) ? p0.xy : p3.xy;
                }
            }
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
            if (middleOutShader.fAttribs & PatchAttribs::kColor) {
                GrGLSLVarying colorVarying(GrSLType::kHalf4_GrSLType);
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

GrPathTessellationShader* GrPathTessellationShader::MakeMiddleOutFixedCountShader(
        const GrShaderCaps& shaderCaps,
        SkArenaAlloc* arena,
        const SkMatrix& viewMatrix,
        const SkPMColor4f& color,
        PatchAttribs attribs) {
    // We should use explicit curve type when, and only when, there isn't infinity support.
    // Otherwise the GPU can infer curve type based on infinity.
    SkASSERT(shaderCaps.infinitySupport() != (attribs & PatchAttribs::kExplicitCurveType));
    return arena->make<MiddleOutShader>(shaderCaps, viewMatrix, color, attribs);
}
