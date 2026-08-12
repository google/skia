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

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    sk_sp<SkShader> shader1 =
            SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    sk_sp<SkShader> shader2 =
            SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kRepeat}, {}});

    const auto* grad1 = static_cast<const SkGradientBaseShader*>(shader1.get());
    const auto* grad2 = static_cast<const SkGradientBaseShader*>(shader2.get());

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data for shader1 (2 stops * 5 floats = 10 floats = 40 bytes)
    auto [ptr1, offset1] = ctxHandle->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, ptr1 != nullptr);
    REPORTER_ASSERT(reporter, offset1 == 0);

    // 2. Allocate gradient data again for shader1 (deduplication check)
    auto [ptr1Dup, offset1Dup] = ctxHandle->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, ptr1Dup == nullptr);
    REPORTER_ASSERT(reporter, offset1Dup == offset1);

    // 3. Allocate gradient data for shader2 (2 stops * 5 floats = 10 floats = 40 bytes)
    auto [ptr2, offset2] = ctxHandle->allocateGradientData(2, grad2);
    REPORTER_ASSERT(reporter, ptr2 != nullptr);
    REPORTER_ASSERT(reporter, offset2 == 10);  // 10 floats offset

    ctxHandle->finalizePrecachedStorageData();

    // Finalize storage buffer allocation and check 16-byte alignment
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 80);
    REPORTER_ASSERT(reporter, bindInfo.fSize % StorageContext::kStructAlignment == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextPaddingAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    sk_sp<SkShader> shader =
            SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    const auto* grad = static_cast<const SkGradientBaseShader*>(shader.get());

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // Allocate gradient data for 1 shader with 2 stops = 40 bytes
    auto [ptr, offset] = ctxHandle->allocateGradientData(2, grad);
    REPORTER_ASSERT(reporter, ptr != nullptr);
    REPORTER_ASSERT(reporter, offset == 0);

    ctxHandle->finalizePrecachedStorageData();

    // Finalize: 40 bytes should be padded to 48 bytes (aligned to 16 bytes)
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 48);
    REPORTER_ASSERT(reporter, bindInfo.fSize % StorageContext::kStructAlignment == 0);
}

}  // namespace skgpu::graphite
