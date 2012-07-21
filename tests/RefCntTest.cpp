/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#include "SkRefCnt.h"
#include "SkThreadUtils.h"
#include "SkWeakRefCnt.h"
#include "SkTRefArray.h"

///////////////////////////////////////////////////////////////////////////////

class InstCounterClass {
public:
    InstCounterClass() {  gInstCounter += 1; }
    ~InstCounterClass() { gInstCounter -= 1; }

    static int gInstCounter;
};

int InstCounterClass::gInstCounter;

static void test_refarray(skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, 0 == InstCounterClass::gInstCounter);

    int N = 10;
    SkTRefArray<InstCounterClass>* array = SkTRefArray<InstCounterClass>::Create(N);
    REPORTER_ASSERT(reporter, 1 == array->getRefCnt());

    REPORTER_ASSERT(reporter, N == InstCounterClass::gInstCounter);
    REPORTER_ASSERT(reporter, array->count() == N);

    array->unref();
    REPORTER_ASSERT(reporter, 0 == InstCounterClass::gInstCounter);
}

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

    REPORTER_ASSERT(reporter, ref->getRefCnt() == 1);
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

    REPORTER_ASSERT(reporter, ref->getRefCnt() == 1);
    REPORTER_ASSERT(reporter, ref->getWeakCnt() == 1);
    ref->unref();
}

static void test_refCntTests(skiatest::Reporter* reporter) {
    test_refCnt(reporter);
    test_weakRefCnt(reporter);
    test_refarray(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("RefCnt", RefCntTestClass, test_refCntTests)
