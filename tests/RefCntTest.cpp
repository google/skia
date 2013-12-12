/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRefCnt.h"
#include "SkTRefArray.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"
#include "SkWeakRefCnt.h"

class InstCounterClass {
public:
    InstCounterClass() { fCount = gInstCounter++; }
    InstCounterClass(const InstCounterClass& src) {
        fCount = src.fCount;
        gInstCounter += 1;
    }
    virtual ~InstCounterClass() { gInstCounter -= 1; }

    static int gInstCounter;
    int fCount;
};

int InstCounterClass::gInstCounter;

static void test_refarray(skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, 0 == InstCounterClass::gInstCounter);

    const int N = 10;
    SkTRefArray<InstCounterClass>* array = SkTRefArray<InstCounterClass>::Create(N);

    REPORTER_ASSERT(reporter, 1 == array->getRefCnt());
    REPORTER_ASSERT(reporter, N == array->count());

    REPORTER_ASSERT(reporter, N == InstCounterClass::gInstCounter);
    array->unref();
    REPORTER_ASSERT(reporter, 0 == InstCounterClass::gInstCounter);

    // Now test the copy factory

    int i;
    InstCounterClass* src = new InstCounterClass[N];
    REPORTER_ASSERT(reporter, N == InstCounterClass::gInstCounter);
    for (i = 0; i < N; ++i) {
        REPORTER_ASSERT(reporter, i == src[i].fCount);
    }

    array = SkTRefArray<InstCounterClass>::Create(src, N);
    REPORTER_ASSERT(reporter, 1 == array->getRefCnt());
    REPORTER_ASSERT(reporter, N == array->count());

    REPORTER_ASSERT(reporter, 2*N == InstCounterClass::gInstCounter);
    for (i = 0; i < N; ++i) {
        REPORTER_ASSERT(reporter, i == (*array)[i].fCount);
    }

    delete[] src;
    REPORTER_ASSERT(reporter, N == InstCounterClass::gInstCounter);

    for (i = 0; i < N; ++i) {
        REPORTER_ASSERT(reporter, i == (*array)[i].fCount);
    }
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

DEF_TEST(RefCnt, reporter) {
    test_refCnt(reporter);
    test_weakRefCnt(reporter);
    test_refarray(reporter);
}
