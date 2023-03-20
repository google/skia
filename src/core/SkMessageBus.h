/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMessageBus_DEFINED
#define SkMessageBus_DEFINED

#include <type_traits>

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"

/**
 * The following method must have a specialization for type 'Message':
 *
 *     bool SkShouldPostMessageToBus(const Message&, IDType msgBusUniqueID)
 *
 * We may want to consider providing a default template implementation, to avoid this requirement by
 * sending to all inboxes when the specialization for type 'Message' is not present.
 */
template <typename Message, typename IDType, bool AllowCopyableMessage = true>
class SkMessageBus : SkNoncopyable {
public:
    template <typename T> struct is_sk_sp : std::false_type {};
    template <typename T> struct is_sk_sp<sk_sp<T>> : std::true_type {};

    // We want to make sure the caller of Post() method will not keep a ref or copy of the message,
    // so the message type must be sk_sp or non copyable.
    static_assert(AllowCopyableMessage || is_sk_sp<Message>::value ||
                          !std::is_copy_constructible<Message>::value,
                  "The message type must be sk_sp or non copyable.");

    // Post a message to be received by Inboxes for this Message type. Checks
    // SkShouldPostMessageToBus() for each inbox. Threadsafe.
    static void Post(Message m);

    class Inbox {
    public:
        Inbox(IDType uniqueID);
        ~Inbox();

        IDType uniqueID() const { return fUniqueID; }

        // Overwrite out with all the messages we've received since the last call.  Threadsafe.
        void poll(skia_private::TArray<Message>* out);

    private:
        skia_private::TArray<Message> fMessages;
        SkMutex                       fMessagesMutex;
        const IDType                  fUniqueID;

        friend class SkMessageBus;
        void receive(Message m);  // SkMessageBus is a friend only to call this.
    };

private:
    SkMessageBus();
    static SkMessageBus* Get();

    SkTDArray<Inbox*> fInboxes;
    SkMutex           fInboxesMutex;
};

// This must go in a single .cpp file, not some .h, or we risk creating more than one global
// SkMessageBus per type when using shared libraries.  NOTE: at most one per file will compile.
#define DECLARE_SKMESSAGEBUS_MESSAGE(Message, IDType, AllowCopyableMessage)            \
    template <>                                                                        \
    SkMessageBus<Message, IDType, AllowCopyableMessage>*                               \
    SkMessageBus<Message, IDType, AllowCopyableMessage>::Get() {                       \
        static SkOnce once;                                                            \
        static SkMessageBus<Message, IDType, AllowCopyableMessage>* bus;               \
        once([] { bus = new SkMessageBus<Message, IDType, AllowCopyableMessage>(); }); \
        return bus;                                                                    \
    }

//   ----------------------- Implementation of SkMessageBus::Inbox -----------------------

template <typename Message, typename IDType, bool AllowCopyableMessage>
SkMessageBus<Message, IDType, AllowCopyableMessage>::Inbox::Inbox(IDType uniqueID)
        : fUniqueID(uniqueID) {
    // Register ourselves with the corresponding message bus.
    auto* bus = SkMessageBus<Message, IDType, AllowCopyableMessage>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    bus->fInboxes.push_back(this);
}

template <typename Message, typename IDType, bool AllowCopyableMessage>
SkMessageBus<Message, IDType, AllowCopyableMessage>::Inbox::~Inbox() {
    // Remove ourselves from the corresponding message bus.
    auto* bus = SkMessageBus<Message, IDType, AllowCopyableMessage>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    // This is a cheaper fInboxes.remove(fInboxes.find(this)) when order doesn't matter.
    for (int i = 0; i < bus->fInboxes.size(); i++) {
        if (this == bus->fInboxes[i]) {
            bus->fInboxes.removeShuffle(i);
            break;
        }
    }
}

template <typename Message, typename IDType, bool AllowCopyableMessage>
void SkMessageBus<Message, IDType, AllowCopyableMessage>::Inbox::receive(Message m) {
    SkAutoMutexExclusive lock(fMessagesMutex);
    fMessages.push_back(std::move(m));
}

template <typename Message, typename IDType, bool AllowCopyableMessage>
void SkMessageBus<Message, IDType, AllowCopyableMessage>::Inbox::poll(
        skia_private::TArray<Message>* messages) {
    SkASSERT(messages);
    messages->clear();
    SkAutoMutexExclusive lock(fMessagesMutex);
    fMessages.swap(*messages);
}

//   ----------------------- Implementation of SkMessageBus -----------------------

template <typename Message, typename IDType, bool AllowCopyableMessage>
SkMessageBus<Message, IDType, AllowCopyableMessage>::SkMessageBus() = default;

template <typename Message, typename IDType, bool AllowCopyableMessage>
/*static*/ void SkMessageBus<Message, IDType, AllowCopyableMessage>::Post(Message m) {
    auto* bus = SkMessageBus<Message, IDType, AllowCopyableMessage>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    for (int i = 0; i < bus->fInboxes.size(); i++) {
        if (SkShouldPostMessageToBus(m, bus->fInboxes[i]->fUniqueID)) {
            if constexpr (AllowCopyableMessage) {
                bus->fInboxes[i]->receive(m);
            } else {
                if constexpr (is_sk_sp<Message>::value) {
                    SkASSERT(m->unique());
                }
                bus->fInboxes[i]->receive(std::move(m));
                break;
            }
        }
    }

    if constexpr (is_sk_sp<Message>::value && !AllowCopyableMessage) {
        // Make sure sk_sp has been sent to an inbox.
        SkASSERT(!m);  // NOLINT(bugprone-use-after-move)
    }
}

#endif  // SkMessageBus_DEFINED
