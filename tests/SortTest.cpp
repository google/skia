#include "Test.h"
#include "SkRandom.h"
#include "SkTSearch.h"
#include "SkTSort.h"

extern "C" {
    int compare_int(const void* a, const void* b) {
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
        SkQSort(array, count, sizeof(int), compare_int);
        check_sort(reporter, "Quick", array, count);

        rand_array(rand, array, count);
        SkTHeapSort<int>(array, count);
        check_sort(reporter, "Heap", array, count);
    }
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("Sort", SortTestClass, TestSort)
