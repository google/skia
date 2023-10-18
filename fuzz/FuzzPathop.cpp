/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/pathops/SkPathOps.h"

const uint8_t MAX_OPS = 20;

DEF_FUZZ(Pathop, fuzz) {

    uint8_t choice;
    fuzz->nextRange(&choice, 0, 4);
    switch (choice) {
        case 0: {
            uint8_t ops;
            fuzz->nextRange(&ops, 0, MAX_OPS);
            SkOpBuilder builder;
            for (uint8_t i = 0; i < ops && !fuzz->exhausted(); i++) {
                SkPath path;
                FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
                SkPathFillType ft;
                fuzz->nextRange(&ft, 0, (int)SkPathFillType::kInverseEvenOdd);
                path.setFillType(ft);

                SkPathOp op;
                fuzz->nextRange(&op, 0, SkPathOp::kReverseDifference_SkPathOp);
                builder.add(path, op);
            }

            SkPath result;
            builder.resolve(&result);
            break;
        }
        case 1: {
            SkPath path;
            FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, (int)SkPathFillType::kInverseEvenOdd);
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
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
            path.setFillType(ft);

            SkPath path2;
            FuzzEvilPath(fuzz, &path2, SkPath::Verb::kDone_Verb);
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
            path.setFillType(ft);

            SkPathOp op;
            fuzz->nextRange(&op, 0, SkPathOp::kReverseDifference_SkPathOp);

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
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
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
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
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


const int kLastOp = SkPathOp::kReverseDifference_SkPathOp;

void BuildPath(Fuzz* fuzz, SkPath* path) {
    while (!fuzz->exhausted()) {
    // Use a uint8_t to conserve bytes.  This makes our "fuzzed bytes footprint"
    // smaller, which leads to more efficient fuzzing.
    uint8_t operation;
    fuzz->next(&operation);
    SkScalar a,b,c,d,e,f;

    switch (operation % (SkPath::Verb::kDone_Verb + 1)) {
      case SkPath::Verb::kMove_Verb:
        if (fuzz->remainingSize() < (2*sizeof(SkScalar))) {
            fuzz->deplete();
            return;
        }
        fuzz->next(&a, &b);
        path->moveTo(a, b);
        break;

      case SkPath::Verb::kLine_Verb:
        if (fuzz->remainingSize() < (2*sizeof(SkScalar))) {
            fuzz->deplete();
            return;
        }
        fuzz->next(&a, &b);
        path->lineTo(a, b);
        break;

      case SkPath::Verb::kQuad_Verb:
        if (fuzz->remainingSize() < (4*sizeof(SkScalar))) {
            fuzz->deplete();
            return;
        }
        fuzz->next(&a, &b, &c, &d);
        path->quadTo(a, b, c, d);
        break;

      case SkPath::Verb::kConic_Verb:
        if (fuzz->remainingSize() < (5*sizeof(SkScalar))) {
            fuzz->deplete();
            return;
        }
        fuzz->next(&a, &b, &c, &d, &e);
        path->conicTo(a, b, c, d, e);
        break;

      case SkPath::Verb::kCubic_Verb:
        if (fuzz->remainingSize() < (6*sizeof(SkScalar))) {
            fuzz->deplete();
            return;
        }
        fuzz->next(&a, &b, &c, &d, &e, &f);
        path->cubicTo(a, b, c, d, e, f);
        break;

      case SkPath::Verb::kClose_Verb:
        path->close();
        break;

      case SkPath::Verb::kDone_Verb:
        // In this case, simply exit.
        return;
    }
  }
}

DEF_FUZZ(LegacyChromiumPathop, fuzz) {
    // See https://cs.chromium.org/chromium/src/testing/libfuzzer/fuzzers/skia_pathop_fuzzer.cc
    SkOpBuilder builder;
    while (!fuzz->exhausted()) {
        SkPath path;
        uint8_t op;
        fuzz->next(&op);
        if (fuzz->exhausted()) {
            break;
        }

        BuildPath(fuzz, &path);
        builder.add(path, static_cast<SkPathOp>(op % (kLastOp + 1)));
    }

    SkPath result;
    builder.resolve(&result);
}
