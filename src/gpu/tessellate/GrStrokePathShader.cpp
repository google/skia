/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokePathShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

class GrTessellateCubicStrokeShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrTessellateCubicStrokeShader>();
        args.fVaryingHandler->emitAttributes(shader);
        args.fVertBuilder->declareGlobal(GrShaderVar("P", kFloat2_GrSLType,
                                                     GrShaderVar::TypeModifier::Out));
        args.fVertBuilder->codeAppendf("P = inputPoint;");

        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(
                nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &color);
        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, color);
// args.fFragBuilder->codeAppendf("%s = (sk_Clockwise) ? half4(0,1,1,1) : half4(1,0,1,1);",
//                                args.fOutputColor);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const auto& shader = primProc.cast<GrTessellateCubicStrokeShader>();

        if (!shader.viewMatrix().isIdentity()) {
            pdman.setSkMatrix(fViewMatrixUniform, shader.viewMatrix());
        }

        const SkPMColor4f& color = shader.fColor;
        pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);

        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

SkString GrTessellateCubicStrokeShader::getTessControlShaderGLSL(
        const char* versionAndExtensionDecls, const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append("#define MAX_LINEARIZATION_ERROR 0.125  // 1/8 pixel\n");
    code.append(kWangsFormulaCubicFn);
    code.append(R"(
            layout(vertices = 1) out;

            in vec2 P[];
            out vec4 X[];
            out vec4 Y[];
            out float strokeRadius[];

            void main() {
                float numSegments = wangs_formula_cubic(P[0], P[1], P[2], P[3]);
                gl_TessLevelInner[0] = numSegments;
                gl_TessLevelInner[1] = 2.0;
                gl_TessLevelOuter[0] = 2.0;
                gl_TessLevelOuter[1] = numSegments;
                gl_TessLevelOuter[2] = 2.0;
                gl_TessLevelOuter[3] = numSegments;
                X[gl_InvocationID /*== 0*/] = vec4(P[0].x, P[1].x, P[2].x, P[3].x);
                Y[gl_InvocationID /*== 0*/] = vec4(P[0].y, P[1].y, P[2].y, P[3].y);
                strokeRadius[gl_InvocationID /*== 0*/] = P[4].y;
            })");

    return code;
}

SkString GrTessellateCubicStrokeShader::getTessEvaluationShaderGLSL(
        const char* versionAndExtensionDecls, const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(R"(
            layout(quads, equal_spacing, ccw) in;

            uniform vec4 sk_RTAdjust;

            in vec4 X[];
            in vec4 Y[];
            in float strokeRadius[];

            void main() {
                float T = gl_TessCoord.x;
                float outset = gl_TessCoord.y * 2 - 1;
                mat4x2 P = transpose(mat2x4(X[0], Y[0]));

                // Evaluate the cubic using De Casteljau's method for its accuracy and stablility.
                vec2 ab = mix(P[0], P[1], T);
                vec2 bc = mix(P[1], P[2], T);
                vec2 cd = mix(P[2], P[3], T);
                vec2 abc = mix(ab, bc, T);
                vec2 bcd = mix(bc, cd, T);
                vec2 abcd = mix(abc, bcd, T);

                vec2 tan = bcd - abc;
                // When P0=P1 and T=0 we will get 0 for the tangent. We need to use the special
                // case of P2 - P0 instead.
                tan = mix(tan, P[2] - P[0], (P[0] == P[1]) && (T == 0));
                // When P2=P3 and T=1 we will get 0 for the tangent. We need to use the special
                // case of P3 - P1 instead.
                tan = mix(tan, P[3] - P[1], (P[2] == P[3]) && (T == 1));
                vec2 ortho = normalize(vec2(-tan.y, tan.x)) * strokeRadius[0];

                vec2 vertexpos = abcd + outset * ortho;
                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            })");

    return code;
}

GrGLSLPrimitiveProcessor* GrTessellateCubicStrokeShader::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl;
}

// This is the data sent to each vertex invocation. It identifies a segment of the curve to
// linearize, and how to triangulate it.
struct VertexInfo {
    // Parametric "T" values at the beginning and end of our segment of the curve.
    float fT0;
    float fT1;
    // Should we place this vertex at the beginning (0) or end (1) of our segment?
    float fSide;
    // Should we outset this vertex left (-1) or right (+1) from the curve? (When facing in the
    // direction of increasing T.)
    float fOutset;
    // If the above triangulation has a backwards triangle, we try going down the other diagonal.
    float fAlternateSide;
    float fAlternateOutset;
    // If we get backwards triangles from both triangulations above, it means the edges of our
    // segment self-intersect. Arrange the triangles in a hour glass shape instead.
    float fSelfIntersectionSide;
    float fSelfIntersectionOutset;
};

static_assert(sizeof(VertexInfo) == 8*4);

static VertexInfo* write_vertex_info(int resolveLevel, int segmentIdx, VertexInfo* vertexInfo) {
    float T0 = std::ldexp(segmentIdx, -resolveLevel);
    float T1 = std::ldexp(segmentIdx + 1, -resolveLevel);
#ifdef SK_DEBUG
    SkASSERT(segmentIdx >= 0);
    if (segmentIdx == 0) {
        SkASSERT(T0 == 0);
    }
    SkASSERT(segmentIdx < (1 << resolveLevel));
    if (segmentIdx == (1 << resolveLevel) - 1) {
        SkASSERT(T1 == 1);
    }
#endif
    *vertexInfo++ = {T0,T1, 0,1, 1,-1,   0,-1};
    *vertexInfo++ = {T0,T1, 1,3, 1,+1,   1,-1};
    *vertexInfo++ = {T0,T1, 2,0, 0,-1, .5f, 0};
    *vertexInfo++ = {T0,T1, 2,0, 0,-1, .5f, 0};
    *vertexInfo++ = {T0,T1, 1,3, 1,+1,   1,+1};
    *vertexInfo++ = {T0,T1, 3,2, 0,+1,   0,+1};
    return vertexInfo;
}

class GrStrokePathShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        auto& shader = args.fGP.cast<GrStrokePathShader>();

        args.fVaryingHandler->emitAttributes(shader);

        args.fVertBuilder->codeAppend(R"(
                float2 TT = vertexInfo.xy; // See the VertexInfo struct above.
                float4x2 P = float4x2(inputPoints_0_1, inputPoints_2_3);
                float strokeRadius = 20;

                // Evaluate the cubic on both sides of our segment of the curve. Pack the answers
                // for the beginning and end of the segment into the "xy" and "zw" lanes
                // respectively. Use De Casteljau's method for its accuracy and numeric stablility.
                float4 ab = mix(P[0].xyxy, P[1].xyxy, TT.xxyy);
                float4 bc = mix(P[1].xyxy, P[2].xyxy, TT.xxyy);
                float4 cd = mix(P[2].xyxy, P[3].xyxy, TT.xxyy);
                float4 abc = mix(ab, bc, TT.xxyy);
                float4 bcd = mix(bc, cd, TT.xxyy);
                float4 abcd = mix(abc, bcd, TT.xxyy);

                // Calculate the normals on each end of our segment.
                float4 tans = bcd - abc;
                if (P[0] == P[1]) {
                    // When P0=P1 and T=0 we will get 0 for the tangent. We need to use the special
                    // case of P2 - P0 instead.
                    tans = mix(tans, (P[2] - P[0]).xyxy, equal(TT, float2(0)).xxyy);
                }
                if (P[2] == P[3]) {
                    // When P2=P3 and T=1 we will get 0 for the tangent. We need to use the special
                    // case of P3 - P1 instead.
                    tans = mix(tans, (P[3] - P[1]).xyxy, equal(TT, float2(1)).xxyy);
                }
                // Rotate orthos clockwise so -ortho points left when facing toward increasing T.
                float4 orthos = float4(-tans.y, tans.x, -tans.w, tans.z);
                orthos = float4(normalize(orthos.xy), normalize(orthos.zw)) * strokeRadius;
                // if (dot(P[1] - P[0], P[0] - P[3]) > 0) {
                //     orthos.xy = -orthos.xy;
                // }
                // if (dot(P[2] - P[3], P[3] - P[0]) > 0) {
                //     orthos.zw = -orthos.zw;
                // }

                float4 positions = abcd;
                float4x2 corners = float4x2(positions - orthos, positions + orthos);
                if (determinant(float2x2(corners[0] - corners[2], positions.zw - positions.xy)) < 0) {
                    corners = float4x2(corners[2], corners[1], corners[0], corners[3]);
                }
                if (determinant(float2x2(corners[1] - corners[3], positions.zw - positions.xy)) < 0) {
                    corners = float4x2(corners[0], corners[3], corners[2], corners[1]);
                }
                if (dot(corners[1] - corners[0], corners[3] - corners[2]) < 0) {
                    corners = float4x2(corners[0], corners[1], corners[3], corners[2]);
                }
                int cid = int(vertexInfo.z);
                float2 vertexPlacement = vertexInfo.zw;
                float c = min(determinant(float2x2(corners[0]-corners[1], corners[2]-corners[1])),
                              determinant(float2x2(corners[2]-corners[1], corners[3]-corners[1])));
                float c_ = min(determinant(float2x2(corners[1]-corners[3], corners[0]-corners[3])),
                               determinant(float2x2(corners[0]-corners[3], corners[2]-corners[3])));
                if (c_ > c) {
                    cid = int(vertexInfo.w);
                    c = c_;
                }
                float2 position = corners[cid];
        )");

        GrShaderVar position("position", kFloat2_GrSLType);
        this->emitTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                             position, args.fFPCoordTransformHandler);

        GrShaderVar vertexPos = position;
        if (!shader.viewMatrix().isIdentity()) {
            const char* viewMatrix;
            fViewMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat3x3_GrSLType, "viewMatrix", &viewMatrix);
            args.fVertBuilder->codeAppendf("float2 devPosition = (%s * float3(position, 1)).xy;",
                                           viewMatrix);
            vertexPos.set(kFloat2_GrSLType, "devPosition");
        }
        gpArgs->fPositionVar = vertexPos;

        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(
                nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &color);
        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, color);
args.fFragBuilder->codeAppendf("%s = (sk_Clockwise) ? half4(0,1,1,1) : half4(1,0,1,1);",
                               args.fOutputColor);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const GrStrokePathShader& shader = primProc.cast<GrStrokePathShader>();

        if (!shader.viewMatrix().isIdentity()) {
            pdman.setSkMatrix(fViewMatrixUniform, shader.viewMatrix());
        }

        const SkPMColor4f& color = shader.fColor;
        pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);

        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

GrGLSLPrimitiveProcessor* GrStrokePathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

GR_DECLARE_STATIC_UNIQUE_KEY(gStrokingVertexBufferKey);

sk_sp<const GrGpuBuffer> GrStrokePathShader::FindOrMakeVertexBuffer(
        GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gStrokingVertexBufferKey);

    // Each resolveLevel has 2^level segments, and each segment is composed of 6 vertices (2
    // triangles).
    constexpr static int kVertexCount = ((1 << (kMaxResolveLevel + 1)) - 1) * 6;

    if (auto buffer = resourceProvider->findByUniqueKey<GrGpuBuffer>(gStrokingVertexBufferKey)) {
        return std::move(buffer);
    }

    auto buffer = resourceProvider->createBuffer(
            kVertexCount * sizeof(VertexInfo), GrGpuBufferType::kVertex, kStatic_GrAccessPattern);
    if (!buffer) {
        return nullptr;
    }

    // We shouldn't bin and/or cache static buffers.
    SkASSERT(buffer->size() == kVertexCount * sizeof(VertexInfo));
    SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());
    auto vertexInfo = static_cast<VertexInfo*>(buffer->map());
    SkAutoTMalloc<VertexInfo> stagingBuffer;
    if (!vertexInfo) {
        SkASSERT(!buffer->isMapped());
        vertexInfo = stagingBuffer.reset(kVertexCount);
    }

    // Write the vertex data.
    SkDEBUGCODE(VertexInfo* endVertexInfo = vertexInfo + kVertexCount;)
    for (int resolveLevel = 0; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        for (int segmentIdx = 0; segmentIdx < (1 << resolveLevel); ++segmentIdx) {
            vertexInfo = write_vertex_info(resolveLevel, segmentIdx, vertexInfo);
        }
    }
    SkASSERT(vertexInfo == endVertexInfo);

    if (buffer->isMapped()) {
        buffer->unmap();
    } else {
        buffer->updateData(stagingBuffer, kVertexCount * sizeof(VertexInfo));
    }
    buffer->resourcePriv().setUniqueKey(gStrokingVertexBufferKey);
    return std::move(buffer);
}
