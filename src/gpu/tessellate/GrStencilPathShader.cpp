/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStencilPathShader.h"

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

// Wang's formula for cubics (1985) gives us the number of evenly spaced (in the
// parametric sense) line segments that are guaranteed to be within a distance of
// "MAX_LINEARIZATION_ERROR" from the actual curve.
constexpr static char kWangsFormulaCubicFn[] = R"(
#define MAX_LINEARIZATION_ERROR 0.25  // 1/4 pixel
float length_pow2(vec2 v) {
    return dot(v, v);
}
float wangs_formula_cubic(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
    float k = (3.0 * 2.0) / (8.0 * MAX_LINEARIZATION_ERROR);
    float m = max(length_pow2(-2.0*p1 + p2 + p0),
                  length_pow2(-2.0*p2 + p3 + p1));
    return max(1.0, ceil(sqrt(k * sqrt(m))));
})";

constexpr static char kSkSLTypeDefs[] = R"(
#define float4x3 mat4x3
#define float3 vec3
#define float2 vec2
)";

// Converts a 4-point input patch into the rational cubic it intended to represent.
constexpr static char kUnpackRationalCubicFn[] = R"(
float4x3 unpack_rational_cubic(float2 p0, float2 p1, float2 p2, float2 p3) {
    float4x3 P = float4x3(p0,1, p1,1, p2,1, p3,1);
    if (isinf(P[3].y)) {
        // This patch is actually a conic. Convert to a rational cubic.
        float w = P[3].x;
        float3 c = P[1] * ((2.0/3.0) * w);
        P = float4x3(P[0], fma(P[0], float3(1.0/3.0), c), fma(P[2], float3(1.0/3.0), c), P[2]);
    }
    return P;
})";

// Evaluate our point of interest using numerically stable linear interpolations. We add our own
// "safe_mix" method to guarantee we get exactly "b" when T=1. The builtin mix() function seems
// spec'd to behave this way, but empirical results results have shown it does not always.
constexpr static char kEvalRationalCubicFn[] = R"(
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

class GrStencilPathShader::Impl : public GrGLSLGeometryProcessor {
protected:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrStencilPathShader>();
        args.fVaryingHandler->emitAttributes(shader);
        auto v = args.fVertBuilder;

        GrShaderVar vertexPos = (*shader.vertexAttributes().begin()).asShaderVar();
        if (!shader.viewMatrix().isIdentity()) {
            const char* viewMatrix;
            fViewMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);
            v->codeAppendf("float2 vertexpos = (%s * float3(inputPoint, 1)).xy;", viewMatrix);
            if (shader.willUseTessellationShaders()) {
                // If y is infinity then x is a conic weight. Don't transform.
                v->codeAppendf("vertexpos = (isinf(vertexpos.y)) ? inputPoint : vertexpos;");
            }
            vertexPos.set(kFloat2_GrSLType, "vertexpos");
        }

        if (!shader.willUseTessellationShaders()) {  // This is the case for the triangle shader.
            gpArgs->fPositionVar = vertexPos;
        } else {
            v->declareGlobal(GrShaderVar("vsPt", kFloat2_GrSLType, GrShaderVar::TypeModifier::Out));
            v->codeAppendf("vsPt = %s;", vertexPos.c_str());
        }

        // No fragment shader.
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const auto& shader = primProc.cast<GrStencilPathShader>();
        if (!shader.viewMatrix().isIdentity()) {
            pdman.setSkMatrix(fViewMatrixUniform, shader.viewMatrix());
        }
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
};

GrGLSLPrimitiveProcessor* GrStencilPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

SkString GrCubicTessellateShader::getTessControlShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                                           const char* versionAndExtensionDecls,
                                                           const GrGLSLUniformHandler&,
                                                           const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kWangsFormulaCubicFn);
    code.append(kSkSLTypeDefs);
    code.append(kUnpackRationalCubicFn);
    code.append(R"(
    layout(vertices = 1) out;

    in vec2 vsPt[];
    out vec4 X[];
    out vec4 Y[];
    out float w[];

    void main() {
        mat4x3 P = unpack_rational_cubic(vsPt[0], vsPt[1], vsPt[2], vsPt[3]);

        // Chop the curve at T=1/2. Here we take advantage of the fact that a uniform scalar has no
        // effect on homogeneous coordinates in order to eval quickly at .5:
        //
        //    mix(p0, p1, .5) / mix(w0, w1, .5)
        //    == ((p0 + p1) * .5) / ((w0 + w1) * .5)
        //    == (p0 + p1) / (w0 + w1)
        //
        vec3 ab = P[0] + P[1];
        vec3 bc = P[1] + P[2];
        vec3 cd = P[2] + P[3];
        vec3 abc = ab + bc;
        vec3 bcd = bc + cd;
        vec3 abcd = abc + bcd;

        // Calculate how many triangles we need to linearize each half of the curve. We simply call
        // Wang's formula for integral cubics with the down-projected points. This appears to be an
        // upper bound on what the actual number of subdivisions would have been.
        float w0 = wangs_formula_cubic(P[0].xy, ab.xy/ab.z, abc.xy/abc.z, abcd.xy/abcd.z);
        float w1 = wangs_formula_cubic(abcd.xy/abcd.z, bcd.xy/bcd.z, cd.xy/cd.z, P[3].xy);

        gl_TessLevelOuter[0] = w1;
        gl_TessLevelOuter[1] = 1.0;
        gl_TessLevelOuter[2] = w0;

        // Changing the inner level to 1 when w0 == w1 == 1 collapses the entire patch to a single
        // triangle. Otherwise, we need an inner level of 2 so our curve triangles have an interior
        // point to originate from.
        gl_TessLevelInner[0] = min(max(w0, w1), 2.0);

        X[gl_InvocationID /*== 0*/] = vec4(P[0].x, P[1].x, P[2].x, P[3].x);
        Y[gl_InvocationID /*== 0*/] = vec4(P[0].y, P[1].y, P[2].y, P[3].y);
        w[gl_InvocationID /*== 0*/] = P[1].z;
    })");

    return code;
}

SkString GrCubicTessellateShader::getTessEvaluationShaderGLSL(
        const GrGLSLPrimitiveProcessor*, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler&, const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kSkSLTypeDefs);
    code.append(kEvalRationalCubicFn);
    code.append(R"(
    layout(triangles, equal_spacing, ccw) in;

    uniform vec4 sk_RTAdjust;

    in vec4 X[];
    in vec4 Y[];
    in float w[];

    void main() {
        // Locate our parametric point of interest. T ramps from [0..1/2] on the left edge of the
        // triangle, and [1/2..1] on the right. If we are the patch's interior vertex, then we want
        // T=1/2. Since the barycentric coords are (1/3, 1/3, 1/3) at the interior vertex, the below
        // fma() works in all 3 scenarios.
        float T = fma(.5, gl_TessCoord.y, gl_TessCoord.z);

        mat4x3 P = transpose(mat3x4(X[0], Y[0], 1,w[0],w[0],1));
        vec2 vertexpos = eval_rational_cubic(P, T);
        if (all(notEqual(gl_TessCoord.xz, vec2(0)))) {
            // We are the interior point of the patch; center it inside [C(0), C(.5), C(1)].
            vertexpos = (P[0].xy + vertexpos + P[3].xy) / 3.0;
        }

        gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
    })");

    return code;
}

SkString GrWedgeTessellateShader::getTessControlShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                                           const char* versionAndExtensionDecls,
                                                           const GrGLSLUniformHandler&,
                                                           const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kWangsFormulaCubicFn);
    code.append(kSkSLTypeDefs);
    code.append(kUnpackRationalCubicFn);
    code.append(R"(
    layout(vertices = 1) out;

    in vec2 vsPt[];
    out vec4 X[];
    out vec4 Y[];
    out float w[];
    out vec2 fanpoint[];

    void main() {
        mat4x3 P = unpack_rational_cubic(vsPt[0], vsPt[1], vsPt[2], vsPt[3]);

        // Figure out how many segments to divide the curve into. To do this we simply call Wang's
        // formula for integral cubics with the down-projected points. This appears to be an upper
        // bound on what the actual number of subdivisions would have been.
        float num_segments = wangs_formula_cubic(P[0].xy, P[1].xy/P[1].z, P[2].xy/P[2].z, P[3].xy);

        // Tessellate the first side of the patch into num_segments triangles.
        gl_TessLevelOuter[0] = num_segments;

        // Leave the other two sides of the patch as single segments.
        gl_TessLevelOuter[1] = 1.0;
        gl_TessLevelOuter[2] = 1.0;

        // Changing the inner level to 1 when num_segments == 1 collapses the entire
        // patch to a single triangle. Otherwise, we need an inner level of 2 so our curve
        // triangles have an interior point to originate from.
        gl_TessLevelInner[0] = min(num_segments, 2.0);

        X[gl_InvocationID /*== 0*/] = vec4(P[0].x, P[1].x, P[2].x, P[3].x);
        Y[gl_InvocationID /*== 0*/] = vec4(P[0].y, P[1].y, P[2].y, P[3].y);
        w[gl_InvocationID /*== 0*/] = P[1].z;
        fanpoint[gl_InvocationID /*== 0*/] = vsPt[4];
    })");

    return code;
}

SkString GrWedgeTessellateShader::getTessEvaluationShaderGLSL(
        const GrGLSLPrimitiveProcessor*, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler&, const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kSkSLTypeDefs);
    code.append(kEvalRationalCubicFn);
    code.append(R"(
    layout(triangles, equal_spacing, ccw) in;

    uniform vec4 sk_RTAdjust;

    in vec4 X[];
    in vec4 Y[];
    in float w[];
    in vec2 fanpoint[];

    void main() {
        // Locate our parametric point of interest. It is equal to the barycentric y-coordinate if
        // we are a vertex on the tessellated edge of the triangle patch, 0.5 if we are the patch's
        // interior vertex, or N/A if we are the fan point.
        // NOTE: We are on the tessellated edge when the barycentric x-coordinate == 0.
        float T = (gl_TessCoord.x == 0.0) ? gl_TessCoord.y : 0.5;

        mat4x3 P = transpose(mat3x4(X[0], Y[0], 1,w[0],w[0],1));
        vec2 vertexpos = eval_rational_cubic(P, T);

        if (gl_TessCoord.x == 1.0) {
            // We are the anchor point that fans from the center of the curve's contour.
            vertexpos = fanpoint[0];
        } else if (gl_TessCoord.x != 0.0) {
            // We are the interior point of the patch; center it inside [C(0), C(.5), C(1)].
            vertexpos = (P[0].xy + vertexpos + P[3].xy) / 3.0;
        }

        gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
    })");

    return code;
}

constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

GR_DECLARE_STATIC_UNIQUE_KEY(gMiddleOutIndexBufferKey);

sk_sp<const GrGpuBuffer> GrMiddleOutCubicShader::FindOrMakeMiddleOutIndexBuffer(
        GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gMiddleOutIndexBufferKey);
    if (auto buffer = resourceProvider->findByUniqueKey<GrGpuBuffer>(gMiddleOutIndexBufferKey)) {
        return std::move(buffer);
    }

    // One explicit triangle at index 0, and one middle-out cubic with kMaxResolveLevel line
    // segments beginning at index 3.
    constexpr static int kIndexCount = 3 + NumVerticesAtResolveLevel(kMaxResolveLevel);
    auto buffer = resourceProvider->createBuffer(
            kIndexCount * sizeof(uint16_t), GrGpuBufferType::kIndex, kStatic_GrAccessPattern);
    if (!buffer) {
        return nullptr;
    }

    // We shouldn't bin and/or cache static buffers.
    SkASSERT(buffer->size() == kIndexCount * sizeof(uint16_t));
    SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());
    auto indexData = static_cast<uint16_t*>(buffer->map());
    SkAutoTMalloc<uint16_t> stagingBuffer;
    if (!indexData) {
        SkASSERT(!buffer->isMapped());
        indexData = stagingBuffer.reset(kIndexCount);
    }

    // Indices 0,1,2 contain special values that emit points P0, P1, and P2 respectively. (When the
    // vertex shader is fed an index value larger than (1 << kMaxResolveLevel), it emits
    // P[index % 4].)
    int i = 0;
    indexData[i++] = (1 << kMaxResolveLevel) + 4;  // % 4 == 0
    indexData[i++] = (1 << kMaxResolveLevel) + 5;  // % 4 == 1
    indexData[i++] = (1 << kMaxResolveLevel) + 6;  // % 4 == 2

    // Starting at index 3, we triangulate a cubic with 2^kMaxResolveLevel line segments. Each
    // index value corresponds to parametric value T=(index / 2^kMaxResolveLevel). Since the
    // triangles are arranged in "middle-out" order, we will be able to conveniently control the
    // resolveLevel by changing only the indexCount.
    for (uint16_t advance = 1 << (kMaxResolveLevel - 1); advance; advance >>= 1) {
        uint16_t T = 0;
        do {
            indexData[i++] = T;
            indexData[i++] = (T += advance);
            indexData[i++] = (T += advance);
        } while (T != (1 << kMaxResolveLevel));
    }
    SkASSERT(i == kIndexCount);

    if (buffer->isMapped()) {
        buffer->unmap();
    } else {
        buffer->updateData(stagingBuffer, kIndexCount * sizeof(uint16_t));
    }
    buffer->resourcePriv().setUniqueKey(gMiddleOutIndexBufferKey);
    return std::move(buffer);
}

class GrMiddleOutCubicShader::Impl : public GrStencilPathShader::Impl {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrMiddleOutCubicShader>();
        args.fVaryingHandler->emitAttributes(shader);
        args.fVertBuilder->defineConstantf("int", "kMaxVertexID", "%i", 1 << kMaxResolveLevel);
        args.fVertBuilder->defineConstantf("float", "kInverseMaxVertexID",
                                           "(1.0 / float(kMaxVertexID))");
        args.fVertBuilder->insertFunction(kUnpackRationalCubicFn);
        args.fVertBuilder->insertFunction(kEvalRationalCubicFn);
        args.fVertBuilder->codeAppend(R"(
        float2 pos;
        if (sk_VertexID > kMaxVertexID) {
            // This is a special index value that instructs us to emit a specific point.
            pos = ((sk_VertexID & 3) == 0) ? inputPoints_0_1.xy :
                  ((sk_VertexID & 2) == 0) ? inputPoints_0_1.zw : inputPoints_2_3.xy;
        } else {
            // Evaluate the cubic at T = (sk_VertexID / 2^kMaxResolveLevel).
            float T = float(sk_VertexID) * kInverseMaxVertexID;
            float4x3 P = unpack_rational_cubic(inputPoints_0_1.xy, inputPoints_0_1.zw,
                                               inputPoints_2_3.xy, inputPoints_2_3.zw);
            pos = eval_rational_cubic(P, T);
        })");

        GrShaderVar vertexPos("pos", kFloat2_GrSLType);
        if (!shader.viewMatrix().isIdentity()) {
            const char* viewMatrix;
            fViewMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);
            args.fVertBuilder->codeAppendf(R"(
            float2 transformedPoint = (%s * float3(pos, 1)).xy;)", viewMatrix);
            vertexPos.set(kFloat2_GrSLType, "transformedPoint");
        }
        gpArgs->fPositionVar = vertexPos;
        // No fragment shader.
    }
};

GrGLSLPrimitiveProcessor* GrMiddleOutCubicShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}
