/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkMessageBus.h"
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
    SkTArray<TestMessage> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 2 == messages.count());
    REPORTER_ASSERT(r, 5 == messages[0].x);
    REPORTER_ASSERT(r, 6 == messages[1].x);

    // Send another; check we get just that one.
    const TestMessage m3 = { 1, 0.3f };
    TestMessageBus::Post(m3);
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
    SkTArray<sk_sp<TestMessageRefCnt>> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 2 == messages.count());
    REPORTER_ASSERT(r, messages[0]->unique());
    REPORTER_ASSERT(r, messages[1]->unique());
    REPORTER_ASSERT(r, 5 == messages[0]->x);
    REPORTER_ASSERT(r, 6 == messages[1]->x);

    // Send another; check we get just that one.
    auto m3 = sk_make_sp<TestMessageRefCnt>(1, 0.3f);
    TestMessageBus::Post(std::move(m3));
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 1 == messages.count());
    REPORTER_ASSERT(r, messages[0]->unique());
    REPORTER_ASSERT(r, 1 == messages[0]->x);

    // Send another without std::move(), it should trigger SkASSERT().
    // auto m4 = sk_make_sp<TestMessageRefCnt>(1, 0.3f);
    // TestMessageBus::Post(m4);

    // Nothing was sent since the last read.
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, 0 == messages.count());
}

namespace {

struct AddressedMessage {
    GrDirectContext::DirectContextID fInboxID;
};

static inline bool SkShouldPostMessageToBus(const AddressedMessage& msg,
                                            GrDirectContext::DirectContextID msgBusUniqueID) {
    SkASSERT(msgBusUniqueID.isValid());
    if (!msg.fInboxID.isValid()) {
        return true;
    }
    return msgBusUniqueID == msg.fInboxID;
}

}  // namespace

DECLARE_SKMESSAGEBUS_MESSAGE(AddressedMessage, GrDirectContext::DirectContextID, true)

DEF_TEST(MessageBus_SkShouldPostMessageToBus, r) {
    using ID = GrDirectContext::DirectContextID;
    using AddressedMessageBus = SkMessageBus<AddressedMessage, ID>;

    ID idInvalid;
    ID id1 = ID::Next(),
       id2 = ID::Next(),
       id3 = ID::Next();

    AddressedMessageBus::Inbox inbox1(id1), inbox2(id2);

    AddressedMessageBus::Post({idInvalid});  // Should go to both
    AddressedMessageBus::Post({id1});        // Should go to inbox1
    AddressedMessageBus::Post({id2});        // Should go to inbox2
    AddressedMessageBus::Post({id3});        // Should go nowhere

    SkTArray<AddressedMessage> messages;
    inbox1.poll(&messages);
    REPORTER_ASSERT(r, messages.count() == 2);
    if (messages.count() == 2) {
        REPORTER_ASSERT(r, !messages[0].fInboxID.isValid());
        REPORTER_ASSERT(r, messages[1].fInboxID == id1);
    }
    inbox2.poll(&messages);
    REPORTER_ASSERT(r, messages.count() == 2);
    if (messages.count() == 2) {
        REPORTER_ASSERT(r, !messages[0].fInboxID.isValid());
        REPORTER_ASSERT(r, messages[1].fInboxID == id2);
    }
}

// Multithreaded tests tbd.
