/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkMessageBus.h"
#include "tests/Test.h"

#include <cstdint>
#include <utility>

using namespace skia_private;

namespace {

struct TestMessage {
    TestMessage(int i, float f) : x(i), y(f) {}

    int x;
    float y;
};

static inline bool SkShouldPostMessageToBus(const TestMessage&, uint32_t) {
    return true;
}

}  // namespace

DECLARE_SKMESSAGEBUS_MESSAGE(TestMessage, uint32_t, true)

DEF_TEST(MessageBus, r) {
    using TestMessageBus = SkMessageBus<TestMessage, uint32_t>;
    // Register two inboxes to receive all TestMessages.
    TestMessageBus::Inbox inbox1(0), inbox2(0);

    // Send two messages.
    const TestMessage m1 = { 5, 4.2f };
    const TestMessage m2 = { 6, 4.3f };
    TestMessageBus::Post(std::move(m1));
    TestMessageBus::Post(std::move(m2));

    // Make sure we got two.
    TArray<TestMessage> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 2 == messages.size());
    REPORTER_ASSERT(r, 5 == messages[0].x);
    REPORTER_ASSERT(r, 6 == messages[1].x);

    // Send another; check we get just that one.
    const TestMessage m3 = { 1, 0.3f };
    TestMessageBus::Post(m3);
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 1 == messages.size());
    REPORTER_ASSERT(r, 1 == messages[0].x);

    // Nothing was sent since the last read.
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 0 == messages.size());

    // Over all this time, inbox2 should have piled up 3 messages.
    inbox2.poll(&messages);
    REPORTER_ASSERT(r, 3 == messages.size());
    REPORTER_ASSERT(r, 5 == messages[0].x);
    REPORTER_ASSERT(r, 6 == messages[1].x);
    REPORTER_ASSERT(r, 1 == messages[2].x);
}

namespace {

struct TestMessageRefCnt : public SkRefCnt {
    TestMessageRefCnt(int i, float f) : x(i), y(f) {}

    int x;
    float y;
};

static inline bool SkShouldPostMessageToBus(const sk_sp<TestMessageRefCnt>&, uint32_t) {
    return true;
}

}  // namespace

DECLARE_SKMESSAGEBUS_MESSAGE(sk_sp<TestMessageRefCnt>, uint32_t, false)

DEF_TEST(MessageBusSp, r) {
    // Register two inboxes to receive all TestMessages.
    using TestMessageBus = SkMessageBus<sk_sp<TestMessageRefCnt>, uint32_t, false>;
    TestMessageBus::Inbox inbox1(0);

    // Send two messages.
    auto m1 = sk_make_sp<TestMessageRefCnt>(5, 4.2f);
    auto m2 = sk_make_sp<TestMessageRefCnt>(6, 4.3f);
    TestMessageBus::Post(std::move(m1));
    TestMessageBus::Post(std::move(m2));

    // Make sure we got two.
    TArray<sk_sp<TestMessageRefCnt>> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 2 == messages.size());
    REPORTER_ASSERT(r, messages[0]->unique());
    REPORTER_ASSERT(r, messages[1]->unique());
    REPORTER_ASSERT(r, 5 == messages[0]->x);
    REPORTER_ASSERT(r, 6 == messages[1]->x);

    // Send another; check we get just that one.
    auto m3 = sk_make_sp<TestMessageRefCnt>(1, 0.3f);
    TestMessageBus::Post(std::move(m3));
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 1 == messages.size());
    REPORTER_ASSERT(r, messages[0]->unique());
    REPORTER_ASSERT(r, 1 == messages[0]->x);

    // Send another without std::move(), it should trigger SkASSERT().
    // auto m4 = sk_make_sp<TestMessageRefCnt>(1, 0.3f);
    // TestMessageBus::Post(m4);

    // Nothing was sent since the last read.
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 0 == messages.size());
}

namespace {

using ID = uint32_t;

struct AddressedMessage {
    ID fInboxID = 0;
};

static inline bool SkShouldPostMessageToBus(const AddressedMessage& msg, ID msgBusUniqueID) {
    SkASSERT(msgBusUniqueID != 0);
    return (msg.fInboxID == 0) || msgBusUniqueID == msg.fInboxID;
}

}  // namespace

DECLARE_SKMESSAGEBUS_MESSAGE(AddressedMessage, ID, true)

DEF_TEST(MessageBus_SkShouldPostMessageToBus, r) {
    using AddressedMessageBus = SkMessageBus<AddressedMessage, ID>;

    ID idInvalid = 0;
    ID id1 = 1,
       id2 = 2,
       id3 = 3;

    AddressedMessageBus::Inbox inbox1(id1), inbox2(id2);

    AddressedMessageBus::Post({idInvalid});  // Should go to both
    AddressedMessageBus::Post({id1});        // Should go to inbox1
    AddressedMessageBus::Post({id2});        // Should go to inbox2
    AddressedMessageBus::Post({id3});        // Should go nowhere

    TArray<AddressedMessage> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, messages.size() == 2);
    if (messages.size() == 2) {
        REPORTER_ASSERT(r, messages[0].fInboxID == 0);
        REPORTER_ASSERT(r, messages[1].fInboxID == id1);
    }
    inbox2.poll(&messages);
    REPORTER_ASSERT(r, messages.size() == 2);
    if (messages.size() == 2) {
        REPORTER_ASSERT(r, messages[0].fInboxID == 0);
        REPORTER_ASSERT(r, messages[1].fInboxID == id2);
    }
}

// Multithreaded tests tbd.
