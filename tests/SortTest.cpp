/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRandom.h"
#include "SkTSort.h"

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
                       const int array[], const int reference[], int n) {
    for (int j = 0; j < n; ++j) {
        if (array[j] != reference[j]) {
            SkString str;
            str.printf("%sSort [%d] failed %d %d", label, n, array[j], reference[j]);
            reporter->reportFailed(str);
        }
    }
}

DEF_TEST(Sort, reporter) {
    /** An array of random numbers to be sorted. */
    int randomArray[500];
    /** The reference sort of the random numbers. */
    int sortedArray[SK_ARRAY_COUNT(randomArray)];
    /** The random numbers are copied into this array, sorted by an SkSort,
        then this array is compared against the reference sort. */
    int workingArray[SK_ARRAY_COUNT(randomArray)];
    SkRandom    rand;

    for (int i = 0; i < 10000; i++) {
        int count = rand.nextRangeU(1, SK_ARRAY_COUNT(randomArray));
        rand_array(rand, randomArray, count);

        // Use qsort as the reference sort.
        memcpy(sortedArray, randomArray, sizeof(randomArray));
        qsort(sortedArray, count, sizeof(sortedArray[0]), compare_int);

        memcpy(workingArray, randomArray, sizeof(randomArray));
        SkTHeapSort<int>(workingArray, count);
        check_sort(reporter, "Heap", workingArray, sortedArray, count);

        memcpy(workingArray, randomArray, sizeof(randomArray));
        SkTQSort<int>(workingArray, workingArray + count - 1);
        check_sort(reporter, "Quick", workingArray, sortedArray, count);
    }
}

// need tests for SkStrSearch
