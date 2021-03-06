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
#include "include/private/SkMutex.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"

/**
 * The following method must have a specialization for type 'Message':
 *
 *     bool SkShouldPostMessageToBus(const Message&, uint32_t msgBusUniqueID)
 *
 * We may want to consider providing a default template implementation, to avoid this requirement by
 * sending to all inboxes when the specialization for type 'Message' is not present.
 */
template <typename Message, bool AllowCopyableMessage = true> class SkMessageBus : SkNoncopyable {
public:
    template <typename T> struct is_sk_sp : std::false_type {};
    template <typename T> struct is_sk_sp<sk_sp<T>> : std::true_type {};

    // We want to make sure the caller of Post() method will not keep a ref or copy of the message,
    // so the message type must be sk_sp, non copyable or trivially copyable.
    static_assert(AllowCopyableMessage || std::is_trivially_copyable<Message>::value ||
                          !std::is_copy_constructible<Message>::value || is_sk_sp<Message>::value,
                  "The message type must be sk_sp, non copyable or trivially copyable.");

    // Post a message to be received by Inboxes for this Message type. Checks
    // SkShouldPostMessageToBus() for each inbox. Threadsafe.
    static void Post(Message m);

    class Inbox {
    public:
        Inbox(uint32_t uniqueID = SK_InvalidUniqueID);
        ~Inbox();

        uint32_t uniqueID() const { return fUniqueID; }

        // Overwrite out with all the messages we've received since the last call.  Threadsafe.
        void poll(SkTArray<Message>* out);

    private:
        SkTArray<Message>  fMessages;
        SkMutex            fMessagesMutex;
        uint32_t           fUniqueID;

        friend class SkMessageBus;
        void receive(Message m);  // SkMessageBus is a friend only to call this.
    };

private:
    SkMessageBus();
    template <typename T> static void AssertMessageUnique(const T& m);
    template <typename T> static void AssertMessageUnique(const sk_sp<T>& m);
    static SkMessageBus* Get();

    SkTDArray<Inbox*> fInboxes;
    SkMutex           fInboxesMutex;
};

// This must go in a single .cpp file, not some .h, or we risk creating more than one global
// SkMessageBus per type when using shared libraries.  NOTE: at most one per file will compile.
#define DECLARE_SKMESSAGEBUS_MESSAGE(Message, AllowCopyableMessage)            \
    template <>                                                                \
    SkMessageBus<Message, AllowCopyableMessage>*                               \
    SkMessageBus<Message, AllowCopyableMessage>::Get() {                       \
        static SkOnce once;                                                    \
        static SkMessageBus<Message, AllowCopyableMessage>* bus;               \
        once([] { bus = new SkMessageBus<Message, AllowCopyableMessage>(); }); \
        return bus;                                                            \
    }

//   ----------------------- Implementation of SkMessageBus::Inbox -----------------------

template <typename Message, bool AllowCopyableMessage>
SkMessageBus<Message, AllowCopyableMessage>::Inbox::Inbox(uint32_t uniqueID) : fUniqueID(uniqueID) {
    // Register ourselves with the corresponding message bus.
    auto* bus = SkMessageBus<Message, AllowCopyableMessage>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    bus->fInboxes.push_back(this);
}

template <typename Message, bool AllowCopyableMessage>
SkMessageBus<Message, AllowCopyableMessage>::Inbox::~Inbox() {
    // Remove ourselves from the corresponding message bus.
    auto* bus = SkMessageBus<Message, AllowCopyableMessage>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    // This is a cheaper fInboxes.remove(fInboxes.find(this)) when order doesn't matter.
    for (int i = 0; i < bus->fInboxes.count(); i++) {
        if (this == bus->fInboxes[i]) {
            bus->fInboxes.removeShuffle(i);
            break;
        }
    }
}

template <typename Message, bool AllowCopyableMessage>
void SkMessageBus<Message, AllowCopyableMessage>::Inbox::receive(Message m) {
    SkAutoMutexExclusive lock(fMessagesMutex);
    fMessages.push_back(std::move(m));
}

template <typename Message, bool AllowCopyableMessage>
void SkMessageBus<Message, AllowCopyableMessage>::Inbox::poll(SkTArray<Message>* messages) {
    SkASSERT(messages);
    messages->reset();
    SkAutoMutexExclusive lock(fMessagesMutex);
    fMessages.swap(*messages);
}

//   ----------------------- Implementation of SkMessageBus -----------------------

template <typename Message, bool AllowCopyableMessage>
SkMessageBus<Message, AllowCopyableMessage>::SkMessageBus() {}

template <typename Message, false>
template <typename T>
/* static */ void SkMessageBus<Message, false>::AssertMessageUnique(const T& m) {}

template <typename Message, false>
template <typename T>
/* static */ void SkMessageBus<Message, false>::AssertMessageUnique(
        const sk_sp<T>& m) {
    SkASSERT(m->unique());
}

template <typename Message, true>
/*static*/ void SkMessageBus<Message, true>::Post(Message m) {
    auto* bus = SkMessageBus<Message, true>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    for (int i = 0; i < bus->fInboxes.count(); i++) {
        if (SkShouldPostMessageToBus(m, bus->fInboxes[i]->fUniqueID)) {
	    bus->fInboxes[i]->receive(m);
        }
    }
}

template <typename Message, false>
/*static*/ void SkMessageBus<Message, false>::Post(Message m) {
    AssertMessageUnique(m);
    auto* bus = SkMessageBus<Message, false>::Get();
    SkAutoMutexExclusive lock(bus->fInboxesMutex);
    for (int i = 0; i < bus->fInboxes.count(); i++) {
        if (SkShouldPostMessageToBus(m, bus->fInboxes[i]->fUniqueID)) {
	    bus->fInboxes[i]->receive(std::move(m));
            break;
        }
    }
}

#endif  // SkMessageBus_DEFINED
