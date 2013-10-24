/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMessageBus_DEFINED
#define SkMessageBus_DEFINED

#include "SkOnce.h"
#include "SkTDArray.h"
#include "SkThread.h"
#include "SkTypes.h"

template <typename Message>
class SkMessageBus : SkNoncopyable {
public:
    // Post a message to be received by all Inboxes for this Message type.  Threadsafe.
    static void Post(const Message& m);

    class Inbox {
    public:
        Inbox();
        ~Inbox();

        // Overwrite out with all the messages we've received since the last call.  Threadsafe.
        void poll(SkTDArray<Message>* out);

    private:
        SkTDArray<Message> fMessages;
        SkMutex            fMessagesMutex;

        friend class SkMessageBus;
        void receive(const Message& m);  // SkMessageBus is a friend only to call this.
    };

private:
    SkMessageBus();
    static SkMessageBus* Get();
    static void New(SkMessageBus**);

    SkTDArray<Inbox*> fInboxes;
    SkMutex           fInboxesMutex;
};

//   ----------------------- Implementation of SkMessageBus::Inbox -----------------------

template<typename Message>
SkMessageBus<Message>::Inbox::Inbox() {
    // Register ourselves with the corresponding message bus.
    SkMessageBus<Message>* bus = SkMessageBus<Message>::Get();
    SkAutoMutexAcquire lock(bus->fInboxesMutex);
    bus->fInboxes.push(this);
}

template<typename Message>
SkMessageBus<Message>::Inbox::~Inbox() {
    // Remove ourselves from the corresponding message bus.
    SkMessageBus<Message>* bus = SkMessageBus<Message>::Get();
    SkAutoMutexAcquire lock(bus->fInboxesMutex);
    // This is a cheaper fInboxes.remove(fInboxes.find(this)) when order doesn't matter.
    for (int i = 0; i < bus->fInboxes.count(); i++) {
        if (this == bus->fInboxes[i]) {
            bus->fInboxes.removeShuffle(i);
            break;
        }
    }
}

template<typename Message>
void SkMessageBus<Message>::Inbox::receive(const Message& m) {
    SkAutoMutexAcquire lock(fMessagesMutex);
    fMessages.push(m);
}

template<typename Message>
void SkMessageBus<Message>::Inbox::poll(SkTDArray<Message>* messages) {
    SkASSERT(NULL != messages);
    messages->reset();
    SkAutoMutexAcquire lock(fMessagesMutex);
    messages->swap(fMessages);
}

//   ----------------------- Implementation of SkMessageBus -----------------------

template <typename Message>
SkMessageBus<Message>::SkMessageBus() {}

template <typename Message>
/*static*/ void SkMessageBus<Message>::New(SkMessageBus<Message>** bus) {
    *bus = new SkMessageBus<Message>();
}

template <typename Message>
/*static*/ SkMessageBus<Message>* SkMessageBus<Message>::Get() {
    // The first time this method is called, create the singleton bus for this message type.
    static SkMessageBus<Message>* bus = NULL;
    SK_DECLARE_STATIC_ONCE(once);
    SkOnce(&once, &New, &bus);

    SkASSERT(bus != NULL);
    return bus;
}

template <typename Message>
/*static*/ void SkMessageBus<Message>::Post(const Message& m) {
    SkMessageBus<Message>* bus = SkMessageBus<Message>::Get();
    SkAutoMutexAcquire lock(bus->fInboxesMutex);
    for (int i = 0; i < bus->fInboxes.count(); i++) {
        bus->fInboxes[i]->receive(m);
    }
}

#endif  // SkMessageBus_DEFINED
