/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClientMappedBufferManager.h"

#include <algorithm>

GrClientMappedBufferManager::GrClientMappedBufferManager(uint32_t contextID)
        : fFinishedBufferInbox(contextID) {}

GrClientMappedBufferManager::~GrClientMappedBufferManager() {
    this->process();
    if (!fAbandoned) {
        // If we're going down before we got the messages we go ahead and unmap all the buffers.
        // It's up to the client to ensure that they aren't being accessed on another thread while
        // this is happening (or afterwards on any thread).
        for (auto& b : fClientHeldBuffers) {
            b->unmap();
        }
    }
}

void GrClientMappedBufferManager::insert(sk_sp<GrGpuBuffer> b) {
    SkDEBUGCODE(auto end = fClientHeldBuffers.end());
    SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) == end);
    fClientHeldBuffers.emplace_front(std::move(b));
}

void GrClientMappedBufferManager::process() {
    SkSTArray<4, BufferFinishedMessage> messages;
    fFinishedBufferInbox.poll(&messages);
    if (!fAbandoned) {
        for (auto& m : messages) {
            this->remove(m.fBuffer);
            m.fBuffer->unmap();
        }
    }
}

void GrClientMappedBufferManager::abandon() {
    fAbandoned = true;
    fClientHeldBuffers.clear();
}

void GrClientMappedBufferManager::remove(const sk_sp<GrGpuBuffer>& b) {
    // There is no convenient remove only the first element that equals a value functionality in
    // std::forward_list.
    auto prev = fClientHeldBuffers.before_begin();
    auto end = fClientHeldBuffers.end();
    SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) != end);
    for (auto cur = fClientHeldBuffers.begin(); cur != end; prev = cur++) {
        if (*cur == b) {
            fClientHeldBuffers.erase_after(prev);
            break;
        }
    }
    SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) == end);
}

//////////////////////////////////////////////////////////////////////////////

DECLARE_SKMESSAGEBUS_MESSAGE(GrClientMappedBufferManager::BufferFinishedMessage)

bool SkShouldPostMessageToBus(const GrClientMappedBufferManager::BufferFinishedMessage& m,
                              uint32_t msgBusUniqueID) {
    return m.fInboxID == msgBusUniqueID;
}
