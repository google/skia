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
                                         CtsEnforcement::kNextRelease) {
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
    auto [ssboPtr, mappedSsbo] = mgr->getStoragePointer(10);
    ssbo = mgr->getStorage(10);
    REPORTER_ASSERT(reporter, !ssbo.fBuffer->isMapped());
    REPORTER_ASSERT(reporter, ssbo.fBuffer != mappedSsbo.fBuffer);
}

}  // namespace skgpu::graphite
