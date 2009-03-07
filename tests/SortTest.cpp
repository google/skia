#include "Test.h"
#include "SkRandom.h"
#include "SkTSearch.h"

extern "C" {
    int compare_int(const void* a, const void* b) {
        return *(const int*)a - *(const int*)b;
    }
}

static void TestSort(skiatest::Reporter* reporter) {
    int         array[100];
    SkRandom    rand;

    for (int i = 0; i < 1000; i++) {
        int j, count = rand.nextRangeU(1, SK_ARRAY_COUNT(array));
        for (j = 0; j < count; j++) {
            array[j] = rand.nextS() & 0xFF;
        }
        SkQSort(array, count, sizeof(int), compare_int);
        for (j = 1; j < count; j++) {
            REPORTER_ASSERT(reporter, array[j-1] <= array[j]);
        }
    }
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("Sort", SortTestClass, TestSort)
