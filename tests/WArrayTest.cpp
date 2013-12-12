/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"

// Include the implementation so we can make an appropriate template instance.
#include "SkAdvancedTypefaceMetrics.h"

using namespace skia_advanced_typeface_metrics_utils;

// Negative values and zeros in a range plus trailing zeros.
//                        0  1   2  3  4  5  6  7  8  9 10 11 12 13 14
static const int16_t data1[] = {-1, 0, -3, 4, 5, 6, 7, 0, 0, 0, 8, 0, 0, 0, 0};
static const char* expected1 = "0[-1 0 -3 4 5 6 7 0 0 0 8]";

// Run with leading and trailing zeros.
// Test rules: d         0  1  2    3    4    5    6    7    8    9 10 11
static const int16_t data2[] = {0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 0, 0};
static const char* expected2 = "3 9 100";

// Removing 0's from a range.
// Test rules: a         0  1  2  3  4  5  6  7  8  9 10 11
static const int16_t data3[] = {1, 2, 0, 0, 0, 3, 4, 0, 0, 0, 0, 5};
static const char* expected3 = "0[1 2 0 0 0 3 4] 11[5]";

// Removing 0's from a run/range and between runs.
// Test rules: a, b      0  1  2  3  4  5  6  7  8  9 10 11 12 14 15
static const int16_t data4[] = {1, 0, 0, 0, 1, 2, 2, 2, 3, 0, 0, 0, 0, 3, 4};
static const char* expected4 = "0[1 0 0 0 1] 5 7 2 8[3] 13[3 4]";

// Runs that starts outside a range.
// Test rules: a, e      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
static const int16_t data5[] = {1, 1, 2, 3, 0, 0, 0, 0, 5, 5, 6, 7, 0, 0, 0, 0, 8, 0};
static const char* expected5 = "0 1 1 2[2 3] 8 9 5 10[6 7] 16[8]";

// Zeros and runs that should be broken out.
// Test rules: a, b, e   0  1  2  3  4  5  6  7  8  9 10 11 12 13
static const int16_t data6[] = {1, 0, 0, 0, 0, 1, 2, 3, 3, 4, 5, 5, 5, 6};
static const char* expected6 = "0[1] 5[1 2 3 3 4] 10 12 5 13[6]";

// Don't cares that aren't enough to break out a run.
// Test rules: c         0  1   2   3  4  5
static const int16_t data7[] = {1, 2, 10, 11, 2, 3};
static const char* expected7 = "0[1 2 10 11 2 3]";
static const uint32_t subset7[] = {0, 1, 4, 5};
static const char* expectedSubset7 = "0[1 2 0 0 2 3]";

// Don't cares that are enough to break out a run.
// Test rules: c         0  1   2   3  4   5  6
static const int16_t data8[] = {1, 2, 10, 11, 12, 2, 3};
static const char* expected8 = "0[1 2 10 11 12 2 3]";
static const uint32_t subset8[] = {0, 1, 5, 6};
static const char* expectedSubset8 = "0[1] 1 5 2 6[3]";

// Leading don't cares.
// Test rules: d         0  1   2  3  4
static const int16_t data9[] = {1, 1, 10, 2, 3};
static const char* expected9 = "0 1 1 2[10 2 3]";
static const uint32_t subset9[] = {0, 1, 3, 4};
static const char* expectedSubset9 = "0 1 1 3[2 3]";

// Almost run of don't cares inside a range.
// Test rules: c          0  1   2   3   4  5
static const int16_t data10[] = {1, 2, 10, 11, 12, 3};
static const char* expected10 = "0[1 2 10 11 12 3]";
static const uint32_t subset10[] = {0, 1, 5};
static const char* expectedSubset10 = "0[1 2 0 0 0 3]";

// Run of don't cares inside a range.
// Test rules: c          0  1   2   3   4   5  6
static const int16_t data11[] = {1, 2, 10, 11, 12, 13, 3};
static const char* expected11 = "0[1 2 10 11 12 13 3]";
static const uint32_t subset11[] = {0, 1, 6};
static const char* expectedSubset11 = "0[1 2] 6[3]";

// Almost run within a range with leading don't cares.
// Test rules: c          0   1   2  3   4   5  6
static const int16_t data12[] = {1, 10, 11, 2, 12, 13, 3};
static const char* expected12 = "0[1 10 11 2 12 13 3]";
static const uint32_t subset12[] = {0, 3, 6};
static const char* expectedSubset12 = "0[1 0 0 2 0 0 3]";

// Run within a range with leading don't cares.
// Test rules: c          0   1   2  3  4   5   6  7
static const int16_t data13[] = {1, 10, 11, 2, 2, 12, 13, 3};
static const char* expected13 = "0[1 10 11 2 2 12 13 3]";
static const uint32_t subset13[] = {0, 3, 4, 7};
static const char* expectedSubset13 = "0[1] 1 6 2 7[3]";

// Enough don't cares to breakup something.
// Test rules: a          0  1  2  3  4  5
static const int16_t data14[] = {1, 0, 0, 0, 0, 2};
static const char* expected14 = "0[1] 5[2]";
static const uint32_t subset14[] = {0, 5};
static const char* expectedSubset14 = "0[1] 5[2]";

static SkString stringify_advance_data(SkAdvancedTypefaceMetrics::AdvanceMetric<int16_t>* data) {
    SkString result;
    bool leadingSpace = false;
    while (data != NULL) {
      if (leadingSpace) {
        result.append(" ");
      } else {
        leadingSpace = true;
      }
      switch(data->fType) {
        case SkAdvancedTypefaceMetrics::AdvanceMetric<int16_t>::kRun:
          result.appendf("%d %d %d", data->fStartId, data->fEndId, data->fAdvance[0]);
          break;
        case SkAdvancedTypefaceMetrics::AdvanceMetric<int16_t>::kRange:
          result.appendf("%d[", data->fStartId);
          for (int i = 0; i < data->fAdvance.count(); ++i) {
            if (i > 0) {
              result.append(" ");
            }
            result.appendf("%d", data->fAdvance[i]);
          }
          result.append("]");
          break;
        case SkAdvancedTypefaceMetrics::AdvanceMetric<int16_t>::kDefault:
          result.appendf("<Default=%d>", data->fAdvance[0]);
          break;
      }
      data = data->fNext.get();
    }
    return result;
}

class TestWData {
  public:
    TestWData(skiatest::Reporter* reporter,
              const int16_t advances[], int advanceLen,
              const uint32_t subset[], int subsetLen,
              const char* expected)
            : fAdvances(advances)
            , fAdvancesLen(advanceLen)
            , fSubset(subset)
            , fSubsetLen(subsetLen)
            , fExpected(expected) {
        REPORTER_ASSERT(reporter, RunTest());
    }

  private:
    const int16_t* fAdvances;
    const int fAdvancesLen;
    const uint32_t* fSubset;
    const int fSubsetLen;
    const char* fExpected;

    static bool getAdvance(void* tc, int gId, int16_t* advance) {
        TestWData* testCase = (TestWData*)tc;
        if (gId >= 0 && gId < testCase->fAdvancesLen) {
            *advance = testCase->fAdvances[gId];
            return true;
        }
        return false;
    }

    bool RunTest() {
        SkAutoTDelete<SkAdvancedTypefaceMetrics::AdvanceMetric<int16_t> > result;
        result.reset(getAdvanceData((void*)this, fAdvancesLen, fSubset, fSubsetLen, getAdvance));

        SkString stringResult = stringify_advance_data(result.get());
        if (!stringResult.equals(fExpected)) {
            SkDebugf("Expected: %s\n  Result: %s\n", fExpected, stringResult.c_str());
            return false;
        }
        return true;
    }
};

DEF_TEST(WArray, reporter) {
    TestWData(reporter, data1, SK_ARRAY_COUNT(data1), NULL, 0, expected1);
    TestWData(reporter, data2, SK_ARRAY_COUNT(data2), NULL, 0, expected2);
    TestWData(reporter, data3, SK_ARRAY_COUNT(data3), NULL, 0, expected3);
    TestWData(reporter, data4, SK_ARRAY_COUNT(data4), NULL, 0, expected4);
    TestWData(reporter, data5, SK_ARRAY_COUNT(data5), NULL, 0, expected5);
    TestWData(reporter, data6, SK_ARRAY_COUNT(data6), NULL, 0, expected6);
    TestWData(reporter, data7, SK_ARRAY_COUNT(data7), NULL, 0, expected7);
    TestWData(reporter, data7, SK_ARRAY_COUNT(data7), subset7,
              SK_ARRAY_COUNT(subset7), expectedSubset7);
    TestWData(reporter, data8, SK_ARRAY_COUNT(data8), NULL, 0, expected8);
    TestWData(reporter, data8, SK_ARRAY_COUNT(data8), subset8,
              SK_ARRAY_COUNT(subset8), expectedSubset8);
    TestWData(reporter, data9, SK_ARRAY_COUNT(data9), NULL, 0, expected9);
    TestWData(reporter, data9, SK_ARRAY_COUNT(data9), subset9,
              SK_ARRAY_COUNT(subset9), expectedSubset9);
    TestWData(reporter, data10, SK_ARRAY_COUNT(data10), NULL, 0, expected10);
    TestWData(reporter, data10, SK_ARRAY_COUNT(data10), subset10,
              SK_ARRAY_COUNT(subset10), expectedSubset10);
    TestWData(reporter, data11, SK_ARRAY_COUNT(data11), NULL, 0, expected11);
    TestWData(reporter, data11, SK_ARRAY_COUNT(data11), subset11,
              SK_ARRAY_COUNT(subset11), expectedSubset11);
    TestWData(reporter, data12, SK_ARRAY_COUNT(data12), NULL, 0, expected12);
    TestWData(reporter, data12, SK_ARRAY_COUNT(data12), subset12,
              SK_ARRAY_COUNT(subset12), expectedSubset12);
    TestWData(reporter, data13, SK_ARRAY_COUNT(data13), NULL, 0, expected13);
    TestWData(reporter, data13, SK_ARRAY_COUNT(data13), subset13,
              SK_ARRAY_COUNT(subset13), expectedSubset13);
    TestWData(reporter, data14, SK_ARRAY_COUNT(data14), NULL, 0, expected14);
    TestWData(reporter, data14, SK_ARRAY_COUNT(data14), subset14,
              SK_ARRAY_COUNT(subset14), expectedSubset14);
}
