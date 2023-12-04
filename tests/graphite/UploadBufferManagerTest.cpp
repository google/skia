/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/UploadBufferManager.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(UploadBufferManagerTest, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    UploadBufferManager* bufferManager = recorder->priv().uploadBufferManager();

    // The test source data.
    char src[8] = {
            1, 2, 3, 4,
            5, 6, 7, 8,
    };

    // Test multiple small writes to a reused buffer.
    auto [smWriter0, smBufferInfo0] = bufferManager->getTextureUploadWriter(10, 1);
    smWriter0.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*dstRowBytes=*/3, /*trimRowBytes=*/3,
                    /*rowCount=*/2);
    smWriter0.write(/*offset=*/6, src, /*srcRowBytes=*/4, /*dstRowBytes=*/2, /*trimRowBytes=*/2,
                    /*rowCount=*/2);

    auto [smWriter1, smBufferInfo1] = bufferManager->getTextureUploadWriter(4, 1);
    smWriter1.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*dstRowBytes=*/2, /*trimRowBytes=*/2,
                    /*rowCount=*/2);

    REPORTER_ASSERT(reporter, smBufferInfo0.fBuffer == smBufferInfo1.fBuffer);
    REPORTER_ASSERT(reporter, smBufferInfo0.fOffset == 0);
    REPORTER_ASSERT(reporter, smBufferInfo1.fOffset >= 10);

    // Test a large write, which should get its own dedicated buffer.
    auto [lgWriter, lgBufferInfo] = bufferManager->getTextureUploadWriter((64 << 10) + 1, 1);
    lgWriter.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*dstRowBytes=*/2, /*trimRowBytes=*/2,
                   /*rowCount=*/2);

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
    auto [smWriter2, smBufferInfo2] = bufferManager->getTextureUploadWriter(2, 1);
    smWriter2.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*dstRowBytes=*/2, /*trimRowBytes=*/2,
                    /*rowCount=*/1);

    REPORTER_ASSERT(reporter, smBufferInfo2.fBuffer == smBufferInfo0.fBuffer);
    REPORTER_ASSERT(reporter, smBufferInfo2.fOffset >= 4 + smBufferInfo1.fOffset);

    REPORTER_ASSERT(reporter, smBufferInfo0.fBuffer->isMapped());
    const char* smBufferMap =
                reinterpret_cast<const char*>(const_cast<Buffer*>(smBufferInfo0.fBuffer)->map());
    // Each section of written data could be offset and aligned by GPU-required rules, so we can't
    // easily validate the contents of the buffer in one go, and instead test at each of the three
    // reported offsets.
    const char expectedSmBuffer0[10] = { 1, 2, 3, 5, 6, 7, 1, 2, 5, 6 };
    const char expectedSmBuffer1[4] = { 1, 2, 5, 6 };
    const char expectedSmBuffer2[2] = { 1, 2};
    REPORTER_ASSERT(reporter, memcmp(smBufferMap + smBufferInfo0.fOffset,
                                     expectedSmBuffer0,
                                     sizeof(expectedSmBuffer0)) == 0);
    REPORTER_ASSERT(reporter, memcmp(smBufferMap + smBufferInfo1.fOffset,
                                     expectedSmBuffer1,
                                     sizeof(expectedSmBuffer1)) == 0);
    REPORTER_ASSERT(reporter, memcmp(smBufferMap + smBufferInfo2.fOffset,
                                     expectedSmBuffer1,
                                     sizeof(expectedSmBuffer2)) == 0);

    // Snap a Recording from the Recorder. This will transfer resources from the UploadBufferManager
    // to the Recording.
    auto recording = recorder->snap();

    // Test writes with a required alignment.
    auto [alWriter0, alBufferInfo0] = bufferManager->getTextureUploadWriter(6, 4);
    alWriter0.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*dstRowBytes=*/3, /*trimRowBytes=*/3,
                    /*rowCount=*/2);

    auto [alWriter1, alBufferInfo1] = bufferManager->getTextureUploadWriter(2, 4);
    alWriter1.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*dstRowBytes=*/2, /*trimRowBytes=*/2,
                    /*rowCount=*/1);

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
