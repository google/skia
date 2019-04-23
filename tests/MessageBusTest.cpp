/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMessageBus.h"
#include "tests/Test.h"

namespace {

struct TestMessage {
    TestMessage(int i, float f) : x(i), y(f) {}

    int x;
    float y;
};

static inline bool SkShouldPostMessageToBus(const TestMessage&, uint32_t) {
    return true;
}

}
DECLARE_SKMESSAGEBUS_MESSAGE(TestMessage)

DEF_TEST(MessageBus, r) {
    // Register two inboxes to receive all TestMessages.
    SkMessageBus<TestMessage>::Inbox inbox1, inbox2;

    // Send two messages.
    const TestMessage m1 = { 5, 4.2f };
    const TestMessage m2 = { 6, 4.3f };
    SkMessageBus<TestMessage>::Post(m1);
    SkMessageBus<TestMessage>::Post(m2);

    // Make sure we got two.
    SkTArray<TestMessage> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 2 == messages.count());
    REPORTER_ASSERT(r, 5 == messages[0].x);
    REPORTER_ASSERT(r, 6 == messages[1].x);

    // Send another; check we get just that one.
    const TestMessage m3 = { 1, 0.3f };
    SkMessageBus<TestMessage>::Post(m3);
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 1 == messages.count());
    REPORTER_ASSERT(r, 1 == messages[0].x);

    // Nothing was sent since the last read.
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 0 == messages.count());

    // Over all this time, inbox2 should have piled up 3 messages.
    inbox2.poll(&messages);
    REPORTER_ASSERT(r, 3 == messages.count());
    REPORTER_ASSERT(r, 5 == messages[0].x);
    REPORTER_ASSERT(r, 6 == messages[1].x);
    REPORTER_ASSERT(r, 1 == messages[2].x);
}

namespace {

struct AddressedMessage {
    uint32_t fInboxID;
};

static inline bool SkShouldPostMessageToBus(const AddressedMessage& msg, uint32_t msgBusUniqueID) {
    SkASSERT(msgBusUniqueID);
    if (!msg.fInboxID) {
        return true;
    }
    return msgBusUniqueID == msg.fInboxID;
}

}

DECLARE_SKMESSAGEBUS_MESSAGE(AddressedMessage)

DEF_TEST(MessageBus_SkShouldPostMessageToBus, r) {
    SkMessageBus<AddressedMessage>::Inbox inbox1(1), inbox2(2);

    SkMessageBus<AddressedMessage>::Post({0});  // Should go to both
    SkMessageBus<AddressedMessage>::Post({1});  // Should go to inbox1
    SkMessageBus<AddressedMessage>::Post({2});  // Should go to inbox2
    SkMessageBus<AddressedMessage>::Post({3});  // Should go nowhere

    SkTArray<AddressedMessage> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, messages.count() == 2);
    if (messages.count() == 2) {
        REPORTER_ASSERT(r, messages[0].fInboxID == 0);
        REPORTER_ASSERT(r, messages[1].fInboxID == 1);
    }
    inbox2.poll(&messages);
    REPORTER_ASSERT(r, messages.count() == 2);
    if (messages.count() == 2) {
        REPORTER_ASSERT(r, messages[0].fInboxID == 0);
        REPORTER_ASSERT(r, messages[1].fInboxID == 2);
    }
}

// Multithreaded tests tbd.
