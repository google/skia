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
#include "src/pathops/SkPathOpsCommon.h"

#include <utility>

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
                SkPath path = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
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
            SkPath path = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, (int)SkPathFillType::kInverseEvenOdd);
            std::ignore = Simplify(path.makeFillType(ft));
            break;
        }
        case 2: {
            SkPath path = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
            path.setFillType(ft);

            SkPath path2 = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
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
            if (auto res = Op(path, path2, op)) {
                result = *res;
            }
            break;
        }
        case 3: {
            SkPath path = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
            path.setFillType(ft);

            SkPath result;
            bool isSame;
            fuzz->next(&isSame);
            if (isSame) {
                result = path;
            }
            std::ignore = AsWinding(path);
            break;
        }
        case 4: {
            SkPath path = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
            SkPathFillType ft;
            fuzz->nextRange(&ft, 0, SkPathFillType::kInverseEvenOdd);
            path.setFillType(ft);

            SkRect result;
            ComputeTightBounds(path, &result);
            break;
        }
        default: {
            SkASSERT(false);
            break;
        }
    }
}


const int kLastOp = SkPathOp::kReverseDifference_SkPathOp;

SkPath BuildPath(Fuzz* fuzz) {
    SkPathBuilder builder;
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
            return builder.detach();
        }
        fuzz->next(&a, &b);
        builder.moveTo(a, b);
        break;

      case SkPath::Verb::kLine_Verb:
        if (fuzz->remainingSize() < (2*sizeof(SkScalar))) {
            fuzz->deplete();
            return builder.detach();
        }
        fuzz->next(&a, &b);
        builder.lineTo(a, b);
        break;

      case SkPath::Verb::kQuad_Verb:
        if (fuzz->remainingSize() < (4*sizeof(SkScalar))) {
            fuzz->deplete();
            return builder.detach();
        }
        fuzz->next(&a, &b, &c, &d);
        builder.quadTo(a, b, c, d);
        break;

      case SkPath::Verb::kConic_Verb:
        if (fuzz->remainingSize() < (5*sizeof(SkScalar))) {
            fuzz->deplete();
            return builder.detach();
        }
        fuzz->next(&a, &b, &c, &d, &e);
        if (!(e > 0)) {
            e = 1.0f;
        }
        builder.conicTo(a, b, c, d, e);
        break;

      case SkPath::Verb::kCubic_Verb:
        if (fuzz->remainingSize() < (6*sizeof(SkScalar))) {
            fuzz->deplete();
            return builder.detach();
        }
        fuzz->next(&a, &b, &c, &d, &e, &f);
        builder.cubicTo(a, b, c, d, e, f);
        break;

      case SkPath::Verb::kClose_Verb:
        builder.close();
        break;

      case SkPath::Verb::kDone_Verb:
        // In this case, simply exit.
        return builder.detach();
    }
  }
  return builder.detach();
}

DEF_FUZZ(LegacyChromiumPathop, fuzz) {
    // See https://cs.chromium.org/chromium/src/testing/libfuzzer/fuzzers/skia_pathop_fuzzer.cc
    SkOpBuilder builder;
    while (!fuzz->exhausted()) {
        uint8_t op;
        fuzz->next(&op);
        if (fuzz->exhausted()) {
            break;
        }

        SkPath path = BuildPath(fuzz);
        builder.add(path, static_cast<SkPathOp>(op % (kLastOp + 1)));
    }

    SkPath result;
    builder.resolve(&result);
}
