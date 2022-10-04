/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_ClientMappedBufferManager_DEFINED
#define skgpu_ClientMappedBufferManager_DEFINED

#include "include/private/SkTArray.h"
#include "src/core/SkMessageBus.h"
#include <algorithm>
#include <forward_list>

namespace skgpu {
/**
 * We sometimes hand clients objects that contain mapped buffers. The client may consume
 * the mapped buffer on another thread. This object manages receiving messages that buffers are
 * ready to be unmapped (on the owner's thread). It also handles cleaning up mapped
 * buffers if the owner is destroyed before the client has finished with the buffer.
 *
 * Buffers are first registered using insert() before being passed the client. process() should be
 * called periodically on the owner's thread to poll for messages and process them.
 */
template <typename T, typename IDType>
class ClientMappedBufferManager {
public:
    /**
     * The message type that internal users of this should post to unmap the buffer.
     * Set fInboxID to inboxID(). fBuffer must have been previously passed to insert().
     */
    struct BufferFinishedMessage {
        BufferFinishedMessage(sk_sp<T> buffer,
                              IDType intendedRecipient)
                : fBuffer(std::move(buffer)), fIntendedRecipient(intendedRecipient) {}
        BufferFinishedMessage(BufferFinishedMessage&& other) {
            fBuffer = std::move(other.fBuffer);
            fIntendedRecipient = other.fIntendedRecipient;
            other.fIntendedRecipient.makeInvalid();
        }
        sk_sp<T> fBuffer;
        IDType   fIntendedRecipient;
    };
    using BufferFinishedMessageBus = SkMessageBus<BufferFinishedMessage,
                                                  IDType,
                                                  false>;

    ClientMappedBufferManager(IDType ownerID)
            : fFinishedBufferInbox(ownerID) {}
    ClientMappedBufferManager(const ClientMappedBufferManager&) = delete;
    ClientMappedBufferManager(ClientMappedBufferManager&&) = delete;

    ~ClientMappedBufferManager() {
        this->process();
        if (!fAbandoned) {
            // If we're going down before we got the messages we go ahead and unmap all the buffers.
            // It's up to the client to ensure that they aren't being accessed on another thread
            // while this is happening (or afterwards on any thread).
            for (auto& b : fClientHeldBuffers) {
                b->unmap();
            }
        }
    }

    ClientMappedBufferManager& operator=(const ClientMappedBufferManager&) = delete;
    ClientMappedBufferManager& operator=(ClientMappedBufferManager&&) = delete;

    /** Initialize BufferFinishedMessage::fIntendedRecipient to this value. It is the
     *  unique ID of the object that owns this buffer manager.
     */
    IDType ownerID() const {
        return fFinishedBufferInbox.uniqueID();
    }

    /**
     * Let the manager know to expect a message with buffer 'b'. It's illegal for a buffer to be
     * inserted again before it is unmapped by process().
     */
    void insert(sk_sp<T> b) {
        SkDEBUGCODE(auto end = fClientHeldBuffers.end());
        SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) == end);
        fClientHeldBuffers.emplace_front(std::move(b));
    }

    /** Poll for messages and unmap any incoming buffers. */
    void process() {
        SkSTArray<4, BufferFinishedMessage> messages;
        fFinishedBufferInbox.poll(&messages);
        if (!fAbandoned) {
            for (auto& m : messages) {
                this->remove(m.fBuffer);
                m.fBuffer->unmap();
            }
        }
    }

    /** Notifies the manager that the context has been abandoned. No more unmaps() will occur.*/
    void abandon() {
        fAbandoned = true;
        fClientHeldBuffers.clear();
    }

private:
    typename BufferFinishedMessageBus::Inbox fFinishedBufferInbox;
    std::forward_list<sk_sp<T>> fClientHeldBuffers;
    bool fAbandoned = false;

    void remove(const sk_sp<T>& b) {
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
};

}  // namespace skgpu

#endif  // skgpu_ClientMappedBufferManager_DEFINED

