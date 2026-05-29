/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"

namespace skgpu::graphite {
namespace {

static bool is_offset_aligned(size_t offset, size_t alignment) {
    return 0 == (offset & (alignment - 1));
}

}  // namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BufferManagerGpuOnlyBufferTest, reporter, context,
                                         CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    DrawBufferManager* mgr = recorder->priv().drawBufferManager();

    // Allocate a series of GPU-only buffers. These buffers should not be mapped before and after
    // they get transferred to the recording.
    auto ssbo = mgr->getStorage(10);
    auto vertex = mgr->getVertexStorage(10);
    auto index = mgr->getIndexStorage(10);
    auto indirect = mgr->getIndirectStorage(10);

    REPORTER_ASSERT(reporter, !ssbo.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, !vertex.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, !index.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, !indirect.fBuffer->isMapped());

    // Ensure that the buffers' starting alignment matches the required storage buffer alignment.
    size_t requiredAlignment = context->priv().caps()->requiredStorageBufferAlignment();
    REPORTER_ASSERT(reporter, is_offset_aligned(ssbo.fOffset, requiredAlignment));
    REPORTER_ASSERT(reporter, is_offset_aligned(vertex.fOffset, requiredAlignment));
    REPORTER_ASSERT(reporter, is_offset_aligned(index.fOffset, requiredAlignment));
    REPORTER_ASSERT(reporter, is_offset_aligned(indirect.fOffset, requiredAlignment));

    // Transfers the ownership of used buffers to a Recording.
    auto recording = recorder->snap();

    // Ensure that the buffers are still unmapped.
    REPORTER_ASSERT(reporter, !ssbo.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, !vertex.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, !index.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, !indirect.fBuffer->isMapped());

    // Since these buffers never need their contents to be host-visible, no buffer transfer/copy
    // tasks should have been created for them.
    REPORTER_ASSERT(reporter, !recording->priv().hasTasks());

    // Request a mapped ssbo followed by an unmapped one. The two buffers should be distinct.
    auto [ssboWriter, mappedSsbo, _] = mgr->getMappedStorageBuffer(/*count=*/10, /*stride=*/1);
    ssbo = mgr->getStorage(10);
    REPORTER_ASSERT(reporter, !ssbo.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, ssbo.fBuffer != mappedSsbo.fBuffer);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BufferManagerStaleAllocatorTest, reporter, context,
                                         CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    DrawBufferManager* dbm = recorder->priv().drawBufferManager();

    // Keep a reference to the buffer to prevent reuse false positives.
    sk_sp<const Buffer> rawBuffer;
    {
        auto [writer, binding, allocator] = dbm->getMappedIndexBuffer(/*count=*/16);
        REPORTER_ASSERT(reporter, allocator.isValid());
        rawBuffer = sk_ref_sp(binding.fBuffer);
        REPORTER_ASSERT(reporter, rawBuffer != nullptr);

        // Force the buffer allocator to fail to simulate an allocation failure
        dbm->testingOnly_onFailedBuffer();
        REPORTER_ASSERT(reporter, dbm->hasMappingFailed());

        // BufferSubAllocator::reset() is called on allocator destruction
    }

    // Recorder::snap() failure branch clears fMappingFailed and drops the failed Recording.
    auto failedRecording = recorder->snap();
    REPORTER_ASSERT(reporter, !failedRecording);

    // Purge everything in the cache by setting the max budget to 0 and freeing all resources
    size_t originalBudget = recorder->maxBudgetedBytes();
    recorder->setMaxBudgetedBytes(0);
    recorder->freeGpuResources();
    recorder->setMaxBudgetedBytes(originalBudget); // probably unnecessary but safe

    // Get another allocator
    auto [staleWriter, staleBinding, staleAllocator] = dbm->getMappedIndexBuffer(/*count=*/16);
    REPORTER_ASSERT(reporter, staleAllocator.isValid());

    // The buffer should not be the same as rawBuffer
    REPORTER_ASSERT(reporter, staleBinding.fBuffer != rawBuffer.get());

    auto successRecording = recorder->snap();
    REPORTER_ASSERT(reporter, successRecording);
}

}  // namespace skgpu::graphite
