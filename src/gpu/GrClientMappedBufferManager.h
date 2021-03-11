/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClientMappedBufferManager_DEFINED
#define GrClientMappedBufferManager_DEFINED

#include "include/gpu/GrDirectContext.h"
#include "include/private/SkTArray.h"
#include "src/core/SkMessageBus.h"
#include "src/gpu/GrGpuBuffer.h"
#include <forward_list>

/**
 * We sometimes hand clients objects that contain mapped GrGpuBuffers. The client may consume
 * the mapped buffer on another thread. This object manages receiving messages that buffers are
 * ready to be unmapped (on the GrDirectContext's thread). It also handles cleaning up mapped
 * buffers if the GrDirectContext is destroyed before the client has finished with the buffer.
 *
 * Buffers are first registered using insert() before being passed the client. process() should be
 * called periodically on the GrDirectContext thread to poll for messages and process them.
 */
class GrClientMappedBufferManager final {
public:
    /**
     * The message type that internal users of this should post to unmap the buffer.
     * Set fInboxID to inboxID(). fBuffer must have been previously passed to insert().
     */
    struct BufferFinishedMessage {
        BufferFinishedMessage(sk_sp<GrGpuBuffer> buffer,
                              GrDirectContext::DirectContextID intendedRecipient)
                : fBuffer(std::move(buffer)), fIntendedRecipient(intendedRecipient) {}
        BufferFinishedMessage(BufferFinishedMessage&& other) {
            fBuffer = std::move(other.fBuffer);
            fIntendedRecipient = other.fIntendedRecipient;
            other.fIntendedRecipient.makeInvalid();
        }
        sk_sp<GrGpuBuffer>               fBuffer;
        GrDirectContext::DirectContextID fIntendedRecipient;
    };
    using BufferFinishedMessageBus = SkMessageBus<BufferFinishedMessage,
                                                  GrDirectContext::DirectContextID,
                                                  false>;

    GrClientMappedBufferManager(GrDirectContext::DirectContextID owningDirectContext);
    GrClientMappedBufferManager(const GrClientMappedBufferManager&) = delete;
    GrClientMappedBufferManager(GrClientMappedBufferManager&&) = delete;

    ~GrClientMappedBufferManager();

    GrClientMappedBufferManager& operator=(const GrClientMappedBufferManager&) = delete;
    GrClientMappedBufferManager& operator=(GrClientMappedBufferManager&&) = delete;

    /** Initialize BufferFinishedMessage::fIntendedRecipient to this value. It is the
     *  unique ID of the GrDirectContext that owns this buffer manager.
     */
    GrDirectContext::DirectContextID owningDirectContext() const {
        return fFinishedBufferInbox.uniqueID();
    }

    /**
     * Let the manager know to expect a message with buffer 'b'. It's illegal for a buffer to be
     * inserted again before it is unmapped by process().
     */
    void insert(sk_sp<GrGpuBuffer> b);

    /** Poll for messages and unmap any incoming buffers. */
    void process();

    /** Notifies the manager that the context has been abandoned. No more unmaps() will occur.*/
    void abandon();

private:
    BufferFinishedMessageBus::Inbox fFinishedBufferInbox;
    std::forward_list<sk_sp<GrGpuBuffer>> fClientHeldBuffers;
    bool fAbandoned = false;

    void remove(const sk_sp<GrGpuBuffer>& b);
};

bool SkShouldPostMessageToBus(const GrClientMappedBufferManager::BufferFinishedMessage&,
                              GrDirectContext::DirectContextID potentialRecipient);

#endif
