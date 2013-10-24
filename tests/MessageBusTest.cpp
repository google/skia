/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMessageBus.h"
#include "Test.h"
#include "TestClassDef.h"

namespace {

struct TestMessage {
    int x;
    float y;
};

}  // namespace

DEF_TEST(MessageBus, r) {
    // Register two inboxes to receive all TestMessages.
    SkMessageBus<TestMessage>::Inbox inbox1, inbox2;

    // Send two messages.
    const TestMessage m1 = { 5, 4.2f };
    const TestMessage m2 = { 6, 4.3f };
    SkMessageBus<TestMessage>::Post(m1);
    SkMessageBus<TestMessage>::Post(m2);

    // Make sure we got two.
    SkTDArray<TestMessage> messages;
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

// Multithreaded tests tbd.
