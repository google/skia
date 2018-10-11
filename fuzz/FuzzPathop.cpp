/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "FuzzCommon.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkRect.h"

const uint8_t MAX_OPS = 20;

DEF_FUZZ(Pathop, fuzz) {

    uint8_t choice;
    fuzz->nextRange(&choice, 0, 4);
    switch (choice) {
        case 0: {
            SkPath path;
            FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
            SkPath::FillType ft;
            fuzz->nextEnum(&ft, 0, SkPath::kInverseEvenOdd_FillType);
            path.setFillType(ft);

            uint8_t ops;
            fuzz->nextRange(&ops, 0, MAX_OPS);
            SkOpBuilder builder;
            for (uint8_t i = 0; i < ops; i++) {
                SkPathOp op;
                fuzz->nextEnum(&op, 0, SkPathOp::kReverseDifference_SkPathOp);
                builder.add(path, op);
            }

            SkPath result;
            builder.resolve(&result);
            break;
        }
        case 1: {
            SkPath path;
            FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
            SkPath::FillType ft;
            fuzz->nextEnum(&ft, 0, SkPath::kInverseEvenOdd_FillType);
            path.setFillType(ft);

            SkPath result;
            bool isSame;
            fuzz->next(&isSame);
            if (isSame) {
                result = path;
            }
            Simplify(path, &result);
            break;
        }
        case 2: {
            SkPath path;
            FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
            SkPath::FillType ft;
            fuzz->nextEnum(&ft, 0, SkPath::kInverseEvenOdd_FillType);
            path.setFillType(ft);

            SkPath path2;
            FuzzEvilPath(fuzz, &path2, SkPath::Verb::kDone_Verb);
            fuzz->nextEnum(&ft, 0, SkPath::kInverseEvenOdd_FillType);
            path.setFillType(ft);

            SkPathOp op;
            fuzz->nextEnum(&op, 0, SkPathOp::kReverseDifference_SkPathOp);

            SkPath result;
            uint8_t pickOutput;
            fuzz->nextRange(&pickOutput, 0, 2);
            if (pickOutput == 1) {
                result = path;
            } else if (pickOutput == 2) {
                result = path2;
            }
            Op(path, path2, op, &result);
            break;
        }
        case 3: {
            SkPath path;
            FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
            SkPath::FillType ft;
            fuzz->nextEnum(&ft, 0, SkPath::kInverseEvenOdd_FillType);
            path.setFillType(ft);

            SkPath result;
            bool isSame;
            fuzz->next(&isSame);
            if (isSame) {
                result = path;
            }
            AsWinding(path, &result);
            break;
        }
        case 4: {
            SkPath path;
            FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
            SkPath::FillType ft;
            fuzz->nextEnum(&ft, 0, SkPath::kInverseEvenOdd_FillType);
            path.setFillType(ft);

            SkRect result;
            TightBounds(path, &result);
            break;
        }
        default: {
            SkASSERT(false);
            break;
        }
    }
}
