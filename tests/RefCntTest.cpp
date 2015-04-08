/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"
#include "SkWeakRefCnt.h"
#include "Test.h"

static void bounce_ref(void* data) {
    SkRefCnt* ref = static_cast<SkRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        ref->ref();
        ref->unref();
    }
}

static void test_refCnt(skiatest::Reporter* reporter) {
    SkRefCnt* ref = new SkRefCnt();

    SkThread thing1(bounce_ref, ref);
    SkThread thing2(bounce_ref, ref);

    thing1.setProcessorAffinity(0);
    thing2.setProcessorAffinity(23);

    SkASSERT(thing1.start());
    SkASSERT(thing2.start());

    thing1.join();
    thing2.join();

    REPORTER_ASSERT(reporter, ref->unique());
    ref->unref();
}

static void bounce_weak_ref(void* data) {
    SkWeakRefCnt* ref = static_cast<SkWeakRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        if (ref->try_ref()) {
            ref->unref();
        }
    }
}

static void bounce_weak_weak_ref(void* data) {
    SkWeakRefCnt* ref = static_cast<SkWeakRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        ref->weak_ref();
        ref->weak_unref();
    }
}

static void test_weakRefCnt(skiatest::Reporter* reporter) {
    SkWeakRefCnt* ref = new SkWeakRefCnt();

    SkThread thing1(bounce_ref, ref);
    SkThread thing2(bounce_ref, ref);
    SkThread thing3(bounce_weak_ref, ref);
    SkThread thing4(bounce_weak_weak_ref, ref);

    thing1.setProcessorAffinity(0);
    thing2.setProcessorAffinity(23);
    thing3.setProcessorAffinity(2);
    thing4.setProcessorAffinity(17);

    SkASSERT(thing1.start());
    SkASSERT(thing2.start());
    SkASSERT(thing3.start());
    SkASSERT(thing4.start());

    thing1.join();
    thing2.join();
    thing3.join();
    thing4.join();

    REPORTER_ASSERT(reporter, ref->unique());
    REPORTER_ASSERT(reporter, ref->getWeakCnt() == 1);
    ref->unref();
}

DEF_TEST(RefCnt, reporter) {
    test_refCnt(reporter);
    test_weakRefCnt(reporter);
}
