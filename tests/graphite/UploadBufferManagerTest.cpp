/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UploadBufferManager.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UploadBufferManagerTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    UploadBufferManager* bufferManager = recorder->priv().uploadBufferManager();
    sk_sp<CommandBuffer> commandBuffer = resourceProvider->createCommandBuffer();

    // The test source data.
    char src[8] = {
            1, 2, 3, 4,
            5, 6, 7, 8,
    };

    // Test multiple small writes to a reused buffer.
    auto [smWriter0, smBufferInfo0] = bufferManager->getUploadWriter(10, 1);
    smWriter0.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/3, /*rowCount=*/2);
    smWriter0.write(/*offset=*/6, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/2);

    auto [smWriter1, smBufferInfo1] = bufferManager->getUploadWriter(4, 1);
    smWriter1.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/2);

    REPORTER_ASSERT(reporter, smBufferInfo0.fBuffer == smBufferInfo1.fBuffer);
    REPORTER_ASSERT(reporter, smBufferInfo0.fOffset == 0);
    REPORTER_ASSERT(reporter, smBufferInfo1.fOffset == 10);

    // Test a large write, which should get its own dedicated buffer.
    auto [lgWriter, lgBufferInfo] = bufferManager->getUploadWriter((64 << 10) + 1, 1);
    lgWriter.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/2);

    REPORTER_ASSERT(reporter, lgBufferInfo.fBuffer != smBufferInfo0.fBuffer);
    REPORTER_ASSERT(reporter, lgBufferInfo.fOffset == 0);
    REPORTER_ASSERT(reporter, lgBufferInfo.fBuffer->isMapped());
    const void* lgBufferMap = const_cast<Buffer*>(lgBufferInfo.fBuffer)->map();
    const char expectedLgBufferMap[4] = {
            1, 2,
            5, 6,
    };
    REPORTER_ASSERT(reporter,
                    memcmp(lgBufferMap, expectedLgBufferMap, sizeof(expectedLgBufferMap)) == 0);

    // Test another small write after the large write.
    auto [smWriter2, smBufferInfo2] = bufferManager->getUploadWriter(2, 1);
    smWriter2.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/1);

    REPORTER_ASSERT(reporter, smBufferInfo2.fBuffer == smBufferInfo0.fBuffer);
    REPORTER_ASSERT(reporter, smBufferInfo2.fOffset == 14);

    REPORTER_ASSERT(reporter, smBufferInfo0.fBuffer->isMapped());
    const void* smBufferMap = const_cast<Buffer*>(smBufferInfo0.fBuffer)->map();
    const char expectedSmBufferMap[16] = {
            // From smWriter0.
            1, 2, 3,
            5, 6, 7,

            1, 2,
            5, 6,

            // From smWriter1.
            1, 2,
            5, 6,

            // From smWriter2.
            1, 2,
    };
    REPORTER_ASSERT(reporter,
                    memcmp(smBufferMap, expectedSmBufferMap, sizeof(expectedSmBufferMap)) == 0);

    // Transfer resources to the command buffer.
    bufferManager->transferToCommandBuffer(commandBuffer.get());

    // Test writes with a required alignment.
    auto [alWriter0, alBufferInfo0] = bufferManager->getUploadWriter(6, 4);
    alWriter0.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/3, /*rowCount=*/2);

    auto [alWriter1, alBufferInfo1] = bufferManager->getUploadWriter(2, 4);
    alWriter1.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/1);

    // Should not share a buffer with earlier small writes, since we've transferred previously-
    // allocated resources to the command buffer.
    REPORTER_ASSERT(reporter, alBufferInfo0.fBuffer != smBufferInfo0.fBuffer);
    REPORTER_ASSERT(reporter, alBufferInfo0.fBuffer == alBufferInfo1.fBuffer);
    REPORTER_ASSERT(reporter, alBufferInfo0.fOffset == 0);
    REPORTER_ASSERT(reporter, alBufferInfo1.fOffset == 8);

    // From alWriter0.
    const char expectedAlBufferMap0[6] = {
            1, 2, 3,
            5, 6, 7,
    };
    // From alWriter1.
    const char expectedAlBufferMap1[2] = {
            1, 2,
    };

    REPORTER_ASSERT(reporter, alBufferInfo0.fBuffer->isMapped());
    const void* alBufferMap = const_cast<Buffer*>(alBufferInfo0.fBuffer)->map();
    REPORTER_ASSERT(reporter,
                    memcmp(alBufferMap, expectedAlBufferMap0, sizeof(expectedAlBufferMap0)) == 0);

    alBufferMap = SkTAddOffset<const void>(alBufferMap, 8);
    REPORTER_ASSERT(reporter,
                    memcmp(alBufferMap, expectedAlBufferMap1, sizeof(expectedAlBufferMap1)) == 0);
}

}  // namespace skgpu::graphite
