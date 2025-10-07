/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkString.h"
#include "include/pathops/SkPathOps.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/Test.h"

DEF_TEST(PathOpsInverse, reporter) {
    const SkPathDirection dirs[] = {SkPathDirection::kCW, SkPathDirection::kCCW};
    const SkPathFillType fts[] = {
        SkPathFillType::kWinding,        SkPathFillType::kEvenOdd,
        SkPathFillType::kInverseWinding, SkPathFillType::kInverseEvenOdd
    };
    int testCount = 0;
    for (int op = kDifference_SkPathOp; op <= kReverseDifference_SkPathOp; ++op) {
        for (auto oneFill : fts) {
            for (auto oneDir : dirs) {
                SkPath one = SkPath::Rect({0, 0, 6, 6}, oneDir).makeFillType(oneFill);
                for (auto twoFill : fts) {
                    for (auto twoDir : dirs) {
                        SkPath two = SkPath::Rect({3, 3, 9, 9}, twoDir).makeFillType(twoFill);
                        SkString testName;
                        testName.printf("inverseTest%d", ++testCount);
                        testPathOp(reporter, one, two, (SkPathOp) op, testName.c_str());
                    }
                }
            }
        }
    }
}
