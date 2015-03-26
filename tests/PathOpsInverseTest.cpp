/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"

DEF_TEST(PathOpsInverse, reporter) {
    SkPath one, two;
    for (int op = kDifference_SkPathOp; op <= kReverseDifference_SkPathOp; ++op) {
        for (int oneFill = SkPath::kWinding_FillType; oneFill <= SkPath::kInverseEvenOdd_FillType;
                    ++oneFill) {
            for (int oneDir = SkPath::kCW_Direction; oneDir != SkPath::kCCW_Direction; ++oneDir) {
                one.reset();
                one.setFillType((SkPath::FillType) oneFill);
                one.addRect(0, 0, 6, 6, (SkPath::Direction) oneDir);
                for (int twoFill = SkPath::kWinding_FillType;
                        twoFill <= SkPath::kInverseEvenOdd_FillType; ++twoFill) {
                    for (int twoDir = SkPath::kCW_Direction; twoDir != SkPath::kCCW_Direction;
                            ++twoDir) {
                        two.reset();
                        two.setFillType((SkPath::FillType) twoFill);
                        two.addRect(3, 3, 9, 9, (SkPath::Direction) twoDir);
                        testPathOp(reporter, one, two, (SkPathOp) op, "inverseTest");
                    }
                }
            }
        }
    }
}
