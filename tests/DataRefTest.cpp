
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkData.h"

static void* gGlobal;

static void delete_int_proc(const void* ptr, size_t len, void* context) {
    int* data = (int*)ptr;
    SkASSERT(context == gGlobal);
    delete[] data;
}

static void assert_len(skiatest::Reporter* reporter, SkData* ref, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
}

static void assert_data(skiatest::Reporter* reporter, SkData* ref,
                        const void* data, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
    REPORTER_ASSERT(reporter, !memcmp(ref->data(), data, len));
}

static void TestDataRef(skiatest::Reporter* reporter) {
    const char* str = "We the people, in order to form a more perfect union.";
    const int N = 10;

    SkData* r0 = SkData::NewEmpty();
    SkData* r1 = SkData::NewWithCopy(str, strlen(str));
    SkData* r2 = SkData::NewWithProc(new int[N], N*sizeof(int),
                                           delete_int_proc, gGlobal);
    SkData* r3 = SkData::NewSubset(r1, 7, 6);

    SkAutoUnref aur0(r0);
    SkAutoUnref aur1(r1);
    SkAutoUnref aur2(r2);
    SkAutoUnref aur3(r3);
    
    assert_len(reporter, r0, 0);
    assert_len(reporter, r1, strlen(str));
    assert_len(reporter, r2, N * sizeof(int));
    assert_len(reporter, r3, 6);
    
    assert_data(reporter, r1, str, strlen(str));
    assert_data(reporter, r3, "people", 6);

    SkData* tmp = SkData::NewSubset(r1, strlen(str), 10);
    assert_len(reporter, tmp, 0);
    tmp->unref();
    tmp = SkData::NewSubset(r1, 0, 0);
    assert_len(reporter, tmp, 0);
    tmp->unref();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DataRef", DataRefTestClass, TestDataRef)
