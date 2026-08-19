/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/StorageContext.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"
#include "src/shaders/gradients/SkLinearGradient.h"

#include <vector>

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad1 =
            sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});
    auto grad2 =
            sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kRepeat}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data for shader1 (2 stops * 5 floats = 10 floats = 40 bytes)
    auto [ptr1, offset1] = ctxHandle->allocateGradientData(2, grad1.get());
    REPORTER_ASSERT(reporter, ptr1 != nullptr);
    REPORTER_ASSERT(reporter, offset1 == 0);

    // 2. Allocate gradient data again for shader1 (deduplication check)
    auto [ptr1Dup, offset1Dup] = ctxHandle->allocateGradientData(2, grad1.get());
    REPORTER_ASSERT(reporter, ptr1Dup == nullptr);
    REPORTER_ASSERT(reporter, offset1Dup == offset1);

    // 3. Allocate gradient data for shader2 (2 stops * 5 floats = 10 floats = 40 bytes)
    auto [ptr2, offset2] = ctxHandle->allocateGradientData(2, grad2.get());
    REPORTER_ASSERT(reporter, ptr2 != nullptr);
    REPORTER_ASSERT(reporter, offset2 == 10);  // 10 floats offset

    // Record vertex requirement with stride 16 and align 16, setting running LCM to 16
    ctxHandle->recordAlignment(/*stride=*/16, /*align=*/16);

    ctxHandle->finalizePrecachedStorageData();

    // Finalize storage buffer allocation and check 16-byte alignment
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 80);
    REPORTER_ASSERT(reporter, bindInfo.fSize % 16 == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextPaddingAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // Allocate gradient data for 1 shader with 2 stops = 40 bytes
    auto [ptr, offset] = ctxHandle->allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, ptr != nullptr);
    REPORTER_ASSERT(reporter, offset == 0);

    // Record vertex requirement with stride 16 and align 16, setting running LCM to 16
    ctxHandle->recordAlignment(/*stride=*/16, /*align=*/16);

    ctxHandle->finalizePrecachedStorageData();

    // Finalize: 40 bytes should be padded to 48 bytes (aligned to 16 bytes)
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 48);
    REPORTER_ASSERT(reporter, bindInfo.fSize % 16 == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextAppendVertexTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data (40 bytes)
    auto [gradPtr, gradOffset] = ctxHandle->allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, gradPtr != nullptr);
    REPORTER_ASSERT(reporter, gradOffset == 0);

    // 2. Record vertex alignment requirement: stride 24, align 16 -> running LCM = 48
    ctxHandle->recordAlignment(/*stride=*/24, /*align=*/16);

    // 3. Finalize precached storage data: 40 bytes aligned to running LCM (48 bytes)
    ctxHandle->finalizePrecachedStorageData();

    // 4. Append vertices with stride 24, align 16, count 2 -> 48 bytes
    float verts[12] = {0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f};
    uint32_t vOffset = ctxHandle->appendVertices(verts, /*count=*/2, /*stride=*/24, /*align=*/16);
    REPORTER_ASSERT(reporter, vOffset == 48);

    // 5. Finalize storage buffer
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    // Total size = 48 (aligned gradient) + 48 (vertices) = 96 bytes
    REPORTER_ASSERT(reporter, bindInfo.fSize == 96);
    REPORTER_ASSERT(reporter, bindInfo.fSize % 48 == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextMultipleRenderStepsTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data (40 bytes)
    auto [gradPtr, gradOffset] = ctxHandle->allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, gradPtr != nullptr);
    REPORTER_ASSERT(reporter, gradOffset == 0);

    // 2. Record alignments from multiple render steps:
    // Step A: stride 24, align 16 -> LCM = 48
    // Step B: stride 32, align 16 -> LCM = 32
    // Running LCM = LCM(48, 32) = 96
    ctxHandle->recordAlignment(/*stride=*/24, /*align=*/16);
    ctxHandle->recordAlignment(/*stride=*/32, /*align=*/16);

    // 3. Finalize precached storage data: 40 bytes aligned to running LCM (96 bytes)
    ctxHandle->finalizePrecachedStorageData();

    // 4. Step A appends 1 vertex of 24 bytes (stride 24, align 16)
    float dataA[6] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    uint32_t offsetA = ctxHandle->appendVertices(dataA, /*count=*/1, /*stride=*/24, /*align=*/16);
    REPORTER_ASSERT(reporter, offsetA == 96);

    // 5. Step B appends 1 vertex of 32 bytes (stride 32, align 16)
    // Local vertex size is currently 24; next 32-byte aligned offset is 32 (8 bytes zero-padding)
    float dataB[8] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
    uint32_t offsetB = ctxHandle->appendVertices(dataB, /*count=*/1, /*stride=*/32, /*align=*/16);
    REPORTER_ASSERT(reporter, offsetB == 96 + 32);  // 128

    // 6. Finalize storage buffer: Total size = 96 (gradient) + 32 (local offset) + 32 (dataB) = 160
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 160);
    REPORTER_ASSERT(reporter, bindInfo.fSize % 32 == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextLCMVariantsTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    struct TestCase {
        size_t stride;
        size_t align;
        uint32_t expectedLCM;
    };

    const TestCase testCases[] = {
            {/*stride=*/12, /*align=*/16, /*expectedLCM=*/48},
            {/*stride=*/20, /*align=*/16, /*expectedLCM=*/80},
            {/*stride=*/24, /*align=*/16, /*expectedLCM=*/48},
            {/*stride=*/28, /*align=*/16, /*expectedLCM=*/112},
            {/*stride=*/36, /*align=*/16, /*expectedLCM=*/144},
            {/*stride=*/40, /*align=*/16, /*expectedLCM=*/80},
            {/*stride=*/64, /*align=*/16, /*expectedLCM=*/64},
    };

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    for (const auto& tc : testCases) {
        StorageContext ctx;
        // Allocate 2 gradient stops = 40 bytes
        auto [gradPtr, gradOffset] = ctx.allocateGradientData(2, grad.get());
        REPORTER_ASSERT(reporter, gradPtr != nullptr);
        REPORTER_ASSERT(reporter, gradOffset == 0);

        ctx.recordAlignment(tc.stride, tc.align);
        ctx.finalizePrecachedStorageData();

        uint32_t expectedPaddedGradSize = SkAlignNonPow2(40u, tc.expectedLCM);

        // Append 2 vertices
        std::vector<char> vert(tc.stride * 2, 0);
        uint32_t vOffset = ctx.appendVertices(vert.data(), /*count=*/2, tc.stride, tc.align);
        REPORTER_ASSERT(reporter, vOffset == expectedPaddedGradSize);
        REPORTER_ASSERT(reporter, vOffset % tc.align == 0);
        REPORTER_ASSERT(reporter, vOffset % tc.expectedLCM == 0);

        auto bindInfo = ctx.finalize(recorder->priv().drawBufferManager());
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        REPORTER_ASSERT(reporter, bindInfo.fSize == expectedPaddedGradSize + tc.stride * 2);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextVertexOnlyAndResetTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    StorageContext ctx;

    REPORTER_ASSERT(reporter, ctx.isEmpty());

    // 1. Finalize on empty context returns null buffer
    ctx.finalizePrecachedStorageData();
    auto emptyInfo = ctx.finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, emptyInfo.fBuffer == nullptr);
    REPORTER_ASSERT(reporter, emptyInfo.fSize == 0);

    // 2. Vertex-only allocation without gradients
    ctx.recordAlignment(/*stride=*/24, /*align=*/16);
    ctx.finalizePrecachedStorageData();

    float vert[6] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    uint32_t offset = ctx.appendVertices(vert, /*count=*/1, /*stride=*/24, /*align=*/16);
    REPORTER_ASSERT(reporter, offset == 0);
    REPORTER_ASSERT(reporter, !ctx.isEmpty());

    auto bindInfo = ctx.finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 24);

    // 3. Reset cache and verify clean state
    ctx.resetCache();
    REPORTER_ASSERT(reporter, ctx.isEmpty());

    // 4. Subsequent allocation after reset starts at offset 0
    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});
    auto [gradPtr, gradOffset] = ctx.allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, gradPtr != nullptr);
    REPORTER_ASSERT(reporter, gradOffset == 0);
}

}  // namespace skgpu::graphite
