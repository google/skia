/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/src/ContextPriv.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/RenderPipeline.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"

#if GRAPHITE_TEST_UTILS
// set to 1 if you want to do GPU capture of the commandBuffer
#define CAPTURE_COMMANDBUFFER 0
#endif

/*
 * This is to test the various pieces of the CommandBuffer interface.
 */
DEF_GRAPHITE_TEST_FOR_CONTEXTS(CommandBufferTest, reporter, context) {
    auto gpu = context->priv().gpu();
    REPORTER_ASSERT(reporter, gpu);

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_startCapture();
#endif
    auto commandBuffer = gpu->resourceProvider()->createCommandBuffer();

    // add test commands here

    bool result = gpu->submit(commandBuffer);
    REPORTER_ASSERT(reporter, result);

    gpu->checkForFinishedWork(skgpu::SyncToCpu::kYes);
#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_endCapture();
#endif
}
