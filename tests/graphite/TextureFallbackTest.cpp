/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/gpu/graphite/StorageContext.h"
#include "src/gpu/graphite/Uniform.h"

namespace skgpu::graphite {

namespace {

class TestStep : public RenderStep {
public:
    TestStep(std::initializer_list<Uniform> storageUniforms)
            : RenderStep(
                      Layout::kStd430,
                      RenderStepID::kMesh,
                      Flags::kPerformsShading | Flags::kFsUsesStorage,
                      /*uniforms=*/{},
                      PrimitiveType::kTriangleStrip,
                      DepthStencilSettings{},
                      /*staticAttrs=*/{},
                      /*appendAttrs=*/{},
                      /*storageUniforms=*/SkSpan(storageUniforms.begin(), storageUniforms.size()),
                      /*varyings=*/{}) {
    }

    std::string vertexSkSL(const RootNodesInfo&) const override {
        return "stepLocalCoords = float2(0.0);";
    }
    const char* fragmentCoverageSkSL() const override { return ""; }
    void writeVertices(DrawWriter*, StorageContext*, const DrawParams&, uint32_t) const override {}
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override {}
};

}  // namespace

class TextureFallbackTest {
public:
    static std::string emitFallback(const ResourceBindingRequirements& reqs,
                                    const RenderStep* step) {
        return ShaderInfo::EmitStorageFallbackTexture(reqs, step);
    }
};

// 1. Primitive Scalar & Integer Bitcast Tests
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackScalarsAndBitcastsTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 0;
    reqs.fStorageBufferBinding = 0;

    TestStep step({
            {"f", SkSLType::kFloat},
            {"h", SkSLType::kHalf},
            {"i", SkSLType::kInt},
            {"u", SkSLType::kUInt},
    });
    std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);

    REPORTER_ASSERT(reporter, sksl.find("float f;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("half h;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("int i;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("uint u;") != std::string::npos);

    REPORTER_ASSERT(reporter, sksl.find("data.f = t0.x;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.h = t0.y;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i = floatBitsToInt(t0.z);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.u = floatBitsToUint(t0.w);") != std::string::npos);

    REPORTER_ASSERT(reporter, sksl.find("int linearIdx0 = index * 1 + 0;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx1") == std::string::npos);
}

// 2. Exhaustive std430 Alignment & Padding Tests
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackAlignmentAndPaddingTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 0;
    reqs.fStorageBufferBinding = 0;

    // Case A: float + float2 + float (8-byte alignment skips t0.y)
    {
        TestStep step({
                {"a", SkSLType::kFloat},
                {"b", SkSLType::kFloat2},
                {"c", SkSLType::kFloat},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.x;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t0.zw;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.c = t1.x;") != std::string::npos);
    }

    // Case B: float + float + float2 (8-byte alignment packed tight: t0.xy, t0.zw)
    {
        TestStep step({
                {"a", SkSLType::kFloat},
                {"b", SkSLType::kFloat},
                {"c", SkSLType::kFloat2},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.x;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t0.y;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.c = t0.zw;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("int linearIdx1") == std::string::npos);
    }

    // Case C: float + float3 (16-byte alignment skips t0.yzw)
    {
        TestStep step({
                {"a", SkSLType::kFloat},
                {"b", SkSLType::kFloat3},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.x;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t1.xyz;") != std::string::npos);
    }

    // Case D: float + float4 (16-byte alignment skips t0.yzw)
    {
        TestStep step({
                {"a", SkSLType::kFloat},
                {"b", SkSLType::kFloat4},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.x;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t1;") != std::string::npos);
    }

    // Case E: float2 + float3 (16-byte alignment skips t0.zw)
    {
        TestStep step({
                {"a", SkSLType::kFloat2},
                {"b", SkSLType::kFloat3},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.xy;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t1.xyz;") != std::string::npos);
    }

    // Case F: float2 + float4 (16-byte alignment skips t0.zw)
    {
        TestStep step({
                {"a", SkSLType::kFloat2},
                {"b", SkSLType::kFloat4},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.xy;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t1;") != std::string::npos);
    }

    // Case G: float3 + float (packed tight in t0: t0.xyz, t0.w)
    {
        TestStep step({
                {"a", SkSLType::kFloat3},
                {"b", SkSLType::kFloat},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.xyz;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t0.w;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("int linearIdx1") == std::string::npos);
    }

    // Case H: float3 + float2 (1 padding float t0.w, float2 in t1.xy)
    {
        TestStep step({
                {"a", SkSLType::kFloat3},
                {"b", SkSLType::kFloat2},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.xyz;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t1.xy;") != std::string::npos);
    }

    // Case I: float3 + float3 (1 padding float t0.w, float3 in t1.xyz)
    {
        TestStep step({
                {"a", SkSLType::kFloat3},
                {"b", SkSLType::kFloat3},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.a = t0.xyz;") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("data.b = t1.xyz;") != std::string::npos);
    }
}

// 3. Matrix Unpacking Tests (mat2x2, mat3x3, mat4x4)
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackMatricesTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 0;
    reqs.fStorageBufferBinding = 0;

    // Case A: mat2x2 starting at component 0 (fits in 1 texel: t0.xy, t0.zw)
    {
        TestStep step({
                {"m2", SkSLType::kFloat2x2},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter,
                        sksl.find("data.m2 = float2x2(t0.xy, t0.zw);") != std::string::npos);
        REPORTER_ASSERT(reporter, sksl.find("int linearIdx1") == std::string::npos);
    }

    // Case B: mat2x2 starting at component 2 (spans across t0.zw and t1.xy)
    {
        TestStep step({
                {"v2", SkSLType::kFloat2},
                {"m2", SkSLType::kFloat2x2},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter, sksl.find("data.v2 = t0.xy;") != std::string::npos);
        REPORTER_ASSERT(reporter,
                        sksl.find("data.m2 = float2x2(t0.zw, t1.xy);") != std::string::npos);
    }

    // Case C: mat3x3 (3 columns of 4 floats each in std430: t0.xyz, t1.xyz, t2.xyz)
    {
        TestStep step({
                {"m3", SkSLType::kFloat3x3},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(
                reporter,
                sksl.find("data.m3 = float3x3(t0.xyz, t1.xyz, t2.xyz);") != std::string::npos);
    }

    // Case D: mat4x4 (4 full texels: t0, t1, t2, t3)
    {
        TestStep step({
                {"m4", SkSLType::kFloat4x4},
        });
        std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);
        REPORTER_ASSERT(reporter,
                        sksl.find("data.m4 = float4x4(t0, t1, t2, t3);") != std::string::npos);
    }
}

// 4. Vector Bitcast Tests for Signed and Unsigned Integers
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackVectorBitcastsTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 0;
    reqs.fStorageBufferBinding = 0;

    TestStep step({
            {"i2", SkSLType::kInt2},
            {"u2", SkSLType::kUInt2},
            {"i3", SkSLType::kInt3},
            {"u3", SkSLType::kUInt3},
            {"i4", SkSLType::kInt4},
            {"u4", SkSLType::kUInt4},
    });
    std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);

    REPORTER_ASSERT(reporter, sksl.find("data.i2 = floatBitsToInt(t0.xy);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.u2 = floatBitsToUint(t0.zw);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i3 = floatBitsToInt(t1.xyz);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.u3 = floatBitsToUint(t2.xyz);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i4 = floatBitsToInt(t3);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.u4 = floatBitsToUint(t4);") != std::string::npos);
}

// 5. Multi-Instance Indexing & 2D Coordinates Math Tests
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackMultiInstanceMathTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 2;
    reqs.fStorageBufferBinding = 5;

    // 3-texel struct: verifies linearIdx and coords for t0, t1, t2 with stride 3
    TestStep step({
            {"v4_0", SkSLType::kFloat4},
            {"v4_1", SkSLType::kFloat4},
            {"v4_2", SkSLType::kFloat4},
    });
    std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);

    REPORTER_ASSERT(reporter,
                    sksl.find("layout(set=2, binding=5) readonly texture2D "
                              "storageFallbackTexture;") != std::string::npos);
    const int maxAtlasWidth = reqs.fMaxFallbackTextureSize;
    std::string expectedWidth =
            "const int texWidth = " + std::to_string(maxAtlasWidth) + ";";
    REPORTER_ASSERT(reporter, sksl.find(expectedWidth) != std::string::npos);
    REPORTER_ASSERT(reporter, reqs.fMaxFallbackTextureBytes == maxAtlasWidth * maxAtlasWidth * 16);

    // Verify custom maxAtlasWidth in ResourceBindingRequirements is respected
    ResourceBindingRequirements customReqs = reqs;
    customReqs.fMaxFallbackTextureSize = 4096;
    std::string customSksl = TextureFallbackTest::emitFallback(customReqs, &step);
    REPORTER_ASSERT(reporter, customSksl.find("const int texWidth = 4096;") != std::string::npos);

    REPORTER_ASSERT(reporter, sksl.find("int linearIdx0 = index * 3 + 0;") != std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("int2 coords0 = int2(linearIdx0 % texWidth, linearIdx0 / texWidth);") !=
                    std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("float4 t0 = float4(textureRead(storageFallbackTexture, uint2(coords0)));") !=
                    std::string::npos);

    REPORTER_ASSERT(reporter, sksl.find("int linearIdx1 = index * 3 + 1;") != std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("int2 coords1 = int2(linearIdx1 % texWidth, linearIdx1 / texWidth);") !=
                    std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("float4 t1 = float4(textureRead(storageFallbackTexture, uint2(coords1)));") !=
                    std::string::npos);

    REPORTER_ASSERT(reporter, sksl.find("int linearIdx2 = index * 3 + 2;") != std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("int2 coords2 = int2(linearIdx2 % texWidth, linearIdx2 / texWidth);") !=
                    std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("float4 t2 = float4(textureRead(storageFallbackTexture, uint2(coords2)));") !=
                    std::string::npos);

    REPORTER_ASSERT(reporter, step.storageUniformStride() == 48);
    REPORTER_ASSERT(reporter, step.storageUniformAlignment() == 16);

    // Verify RenderStep uses standard std430 alignment/stride without artificial padding
    TestStep singleFloatStep({{"s", SkSLType::kFloat}});
    REPORTER_ASSERT(reporter, singleFloatStep.storageUniformStride() == 4);
    REPORTER_ASSERT(reporter, singleFloatStep.storageUniformAlignment() == 4);

    TestStep float2Step({{"v2", SkSLType::kFloat2}});
    REPORTER_ASSERT(reporter, float2Step.storageUniformStride() == 8);
    REPORTER_ASSERT(reporter, float2Step.storageUniformAlignment() == 8);

    TestStep float3Step({{"v3", SkSLType::kFloat3}});
    REPORTER_ASSERT(reporter, float3Step.storageUniformStride() == 16);
    REPORTER_ASSERT(reporter, float3Step.storageUniformAlignment() == 16);

    // Verify StorageContext with storageBufferSupport=false bumps alignment and stride to 16
    {
        StorageContext fallbackStorageContext(/*storageBufferSupport=*/false);
        fallbackStorageContext.recordAlignment(singleFloatStep.storageUniformStride(),
                                               singleFloatStep.storageUniformAlignment());
        REPORTER_ASSERT(reporter, fallbackStorageContext.runningLCM() == 16);

        fallbackStorageContext.recordAlignment(float2Step.storageUniformStride(),
                                               float2Step.storageUniformAlignment());
        REPORTER_ASSERT(reporter, fallbackStorageContext.runningLCM() == 16);
    }

    // Verify StorageContext with storageBufferSupport=true preserves natural std430 alignment/stride
    {
        StorageContext ssboStorageContext(/*storageBufferSupport=*/true);
        ssboStorageContext.recordAlignment(singleFloatStep.storageUniformStride(),
                                           singleFloatStep.storageUniformAlignment());
        REPORTER_ASSERT(reporter, ssboStorageContext.runningLCM() == 4);

        ssboStorageContext.recordAlignment(float2Step.storageUniformStride(),
                                           float2Step.storageUniformAlignment());
        REPORTER_ASSERT(reporter, ssboStorageContext.runningLCM() == 8);
    }

    REPORTER_ASSERT(reporter,
                    sksl.find("inline StepStorageData readStepStorageData(uint index)") !=
                            std::string::npos);
}

// 6. Comprehensive Stress Test: Mixed Scalars, Integers, Vectors, and Matrices with Varied
// Alignments
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackStressMixedTypesAndAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 1;
    reqs.fStorageBufferBinding = 3;

    TestStep step({
            {"s0", SkSLType::kFloat},      {"i0", SkSLType::kInt},
            {"v2_0", SkSLType::kFloat2},   {"m2_0", SkSLType::kFloat2x2},
            {"s1", SkSLType::kFloat},      {"m2_1", SkSLType::kFloat2x2},
            {"u0", SkSLType::kUInt},       {"s2", SkSLType::kFloat},
            {"v3_0", SkSLType::kFloat3},   {"i1", SkSLType::kInt},
            {"m3_0", SkSLType::kFloat3x3}, {"h0", SkSLType::kHalf},
            {"i2_0", SkSLType::kInt2},     {"v4_0", SkSLType::kFloat4},
            {"u3_0", SkSLType::kUInt3},    {"m4_0", SkSLType::kFloat4x4},
            {"u4_0", SkSLType::kUInt4},    {"s3", SkSLType::kFloat},
            {"v3_1", SkSLType::kFloat3},   {"i4_0", SkSLType::kInt4},
    });
    std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);

    // Verify field unpacks and exact swizzles/casts
    REPORTER_ASSERT(reporter, sksl.find("data.s0 = t0.x;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i0 = floatBitsToInt(t0.y);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v2_0 = t0.zw;") != std::string::npos);
    REPORTER_ASSERT(reporter,
                    sksl.find("data.m2_0 = float2x2(t1.xy, t1.zw);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.s1 = t2.x;") != std::string::npos);
    REPORTER_ASSERT(reporter,
                    sksl.find("data.m2_1 = float2x2(t2.zw, t3.xy);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.u0 = floatBitsToUint(t3.z);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.s2 = t3.w;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v3_0 = t4.xyz;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i1 = floatBitsToInt(t4.w);") != std::string::npos);
    REPORTER_ASSERT(
            reporter,
            sksl.find("data.m3_0 = float3x3(t5.xyz, t6.xyz, t7.xyz);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.h0 = t8.x;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i2_0 = floatBitsToInt(t8.zw);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v4_0 = t9;") != std::string::npos);
    REPORTER_ASSERT(reporter,
                    sksl.find("data.u3_0 = floatBitsToUint(t10.xyz);") != std::string::npos);
    REPORTER_ASSERT(reporter,
                    sksl.find("data.m4_0 = float4x4(t11, t12, t13, t14);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.u4_0 = floatBitsToUint(t15);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.s3 = t16.x;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v3_1 = t17.xyz;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.i4_0 = floatBitsToInt(t18);") != std::string::npos);

    // Verify 19-texel linearIdx calculation across instances
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx0 = index * 19 + 0;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx18 = index * 19 + 18;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx19") == std::string::npos);
}

// 7. Stress Test: Back-to-Back Matrices and Boundary Crossings
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackStressBackToBackComplexStructuresTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceBindingRequirements reqs = recorder->priv().caps()->resourceBindingRequirements();
    reqs.fUniformsSetIdx = 0;
    reqs.fStorageBufferBinding = 0;

    TestStep step({
            {"offset_pad", SkSLType::kFloat2},
            {"m2", SkSLType::kFloat2x2},
            {"m3", SkSLType::kFloat3x3},
            {"m4", SkSLType::kFloat4x4},
            {"v3_a", SkSLType::kFloat3},
            {"v2_a", SkSLType::kFloat2},
            {"v3_b", SkSLType::kFloat3},
            {"v4_a", SkSLType::kFloat4},
    });
    std::string sksl = TextureFallbackTest::emitFallback(reqs, &step);

    REPORTER_ASSERT(reporter, sksl.find("data.offset_pad = t0.xy;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.m2 = float2x2(t0.zw, t1.xy);") != std::string::npos);
    REPORTER_ASSERT(reporter,
                    sksl.find("data.m3 = float3x3(t2.xyz, t3.xyz, t4.xyz);") != std::string::npos);
    REPORTER_ASSERT(reporter,
                    sksl.find("data.m4 = float4x4(t5, t6, t7, t8);") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v3_a = t9.xyz;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v2_a = t10.xy;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v3_b = t11.xyz;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("data.v4_a = t12;") != std::string::npos);

    // Verify 13-texel stride
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx0 = index * 13 + 0;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx12 = index * 13 + 12;") != std::string::npos);
    REPORTER_ASSERT(reporter, sksl.find("int linearIdx13") == std::string::npos);
}

// 8. End-to-end Draw Test with Multi-Stop (> 8 stops) Gradients
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFallbackMultiStopGradientsDrawTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(100, 100),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    REPORTER_ASSERT(reporter, surface != nullptr);
    SkCanvas* canvas = surface->getCanvas();

    constexpr int kNumStops = 12;
    SkColor4f colors[kNumStops];
    SkScalar pos[kNumStops];
    for (int i = 0; i < kNumStops; ++i) {
        float t = static_cast<float>(i) / (kNumStops - 1);
        colors[i] = {t, 1.0f - t, 0.5f, 1.0f};
        pos[i] = t;
    }

    SkPoint pts[2] = {{0.f, 0.f}, {100.f, 100.f}};

    // 1. Linear gradient (> 8 stops)
    {
        SkPaint paint;
        paint.setShader(SkShaders::LinearGradient(
                pts, SkGradient{SkGradient::Colors{colors, pos, SkTileMode::kClamp}, {}}));
        canvas->drawRect(SkRect::MakeWH(50, 50), paint);
    }

    // 2. Radial gradient (> 8 stops)
    {
        SkPaint paint;
        paint.setShader(SkShaders::RadialGradient(
                {50.f, 50.f}, 50.f,
                SkGradient{SkGradient::Colors{colors, pos, SkTileMode::kRepeat}, {}}));
        canvas->drawRect(SkRect::MakeXYWH(50, 0, 50, 50), paint);
    }

    // 3. Sweep gradient (> 8 stops)
    {
        SkPaint paint;
        paint.setShader(SkShaders::SweepGradient(
                {50.f, 50.f},
                SkGradient{SkGradient::Colors{colors, pos, SkTileMode::kMirror}, {}}));
        canvas->drawRect(SkRect::MakeXYWH(0, 50, 50, 50), paint);
    }

    // 4. Conical gradient (> 8 stops)
    {
        SkPaint paint;
        paint.setShader(SkShaders::TwoPointConicalGradient(
                pts[0], 10.f, pts[1], 50.f,
                SkGradient{SkGradient::Colors{colors, pos, SkTileMode::kClamp}, {}}));
        canvas->drawRect(SkRect::MakeXYWH(50, 50, 50, 50), paint);
    }

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    context->submit(SyncToCpu::kYes);
}

}  // namespace skgpu::graphite
