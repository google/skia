
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkRandom.h"
#include "SkChecksum.h"
#include "SkTSort.h"

// assert that as we change values (from 0 to non-zero) in our buffer, we
// get a different value
static void test_checksum(skiatest::Reporter* reporter, size_t size) {
    SkAutoMalloc storage(size);
    uint32_t*    ptr = (uint32_t*)storage.get();
    char*        cptr = (char*)ptr;

    sk_bzero(ptr, size);
    uint32_t prev = 0;
    for (size_t i = 0; i < size; ++i) {
        cptr[i] = 0x5B; // just need some non-zero value here
        uint32_t curr = SkChecksum::Compute(ptr, size);
        REPORTER_ASSERT(reporter, prev != curr);
        prev = curr;
    }
}

static void test_checksum(skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, SkChecksum::Compute(NULL, 0) == 0);

    for (size_t size = 4; size <= 1000; size += 4) {
        test_checksum(reporter, size);
    }
}

extern "C" {
    static int compare_int(const void* a, const void* b) {
        return *(const int*)a - *(const int*)b;
    }
}

static void rand_array(SkRandom& rand, int array[], int n) {
    for (int j = 0; j < n; j++) {
        array[j] = rand.nextS() & 0xFF;
    }
}

static void check_sort(skiatest::Reporter* reporter, const char label[],
                       const int array[], int n) {
    for (int j = 1; j < n; j++) {
        if (array[j-1] > array[j]) {
            SkString str;
           str.printf("%sSort [%d] failed %d %d", label, n,
                      array[j-1], array[j]);
            reporter->reportFailed(str);
        }
    }
}

static void TestSort(skiatest::Reporter* reporter) {
    int         array[500];
    SkRandom    rand;

    for (int i = 0; i < 10000; i++) {
        int count = rand.nextRangeU(1, SK_ARRAY_COUNT(array));

        rand_array(rand, array, count);
        SkTHeapSort<int>(array, count);
        check_sort(reporter, "Heap", array, count);

        rand_array(rand, array, count);
        SkTQSort<int>(array, array + count - 1);
        check_sort(reporter, "Quick", array, count);
    }
    if (false) { // avoid bit rot, suppress warning
        compare_int(array, array);
    }

    test_checksum(reporter);
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("Sort", SortTestClass, TestSort)
