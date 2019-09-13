/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/PathOpsExtendedTest.h"

DEF_TEST(PathOpsInverse, reporter) {
    SkPath one, two;
    int testCount = 0;
    for (int op = kDifference_SkPathOp; op <= kReverseDifference_SkPathOp; ++op) {
        for (int oneFill = (int)SkPathFillType::kWinding; oneFill <= (int)SkPathFillType::kInverseEvenOdd;
                    ++oneFill) {
            for (int oneDir = (int)SkPathDirection::kCW; oneDir != (int)SkPathDirection::kCCW; ++oneDir) {
                one.reset();
                one.setFillType((SkPath::FillType) oneFill);
                one.addRect(0, 0, 6, 6, (SkPath::Direction) oneDir);
                for (int twoFill = (int)SkPathFillType::kWinding;
                        twoFill <= (int)SkPathFillType::kInverseEvenOdd; ++twoFill) {
                    for (int twoDir = SkPath::kCW_Direction; twoDir != SkPath::kCCW_Direction;
                            ++twoDir) {
                        two.reset();
                        two.setFillType((SkPathFillType) twoFill);
                        two.addRect({3, 3, 9, 9}, (SkPathDirection) twoDir);
                        SkString testName;
                        testName.printf("inverseTest%d", ++testCount);
                        testPathOp(reporter, one, two, (SkPathOp) op, testName.c_str());
                    }
                }
            }
        }
    }
}
