/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "FuzzCommon.h"

// allows some float values for path points
void FuzzPath(Fuzz* fuzz, SkPath* path, int maxOps) {
    if (maxOps < 2) {
        maxOps = 2;
    }
    uint8_t fillType;
    fuzz->nextRange(&fillType, 0, (uint8_t)SkPath::kInverseEvenOdd_FillType);
    path->setFillType((SkPath::FillType)fillType);
    uint8_t numOps;
    fuzz->nextRange(&numOps, 2, maxOps);
    for (uint8_t i = 0; i < numOps; ++i) {
        uint8_t op;
        fuzz->nextRange(&op, 0, 6);
        SkScalar a, b, c, d, e, f;
        switch (op) {
            case 0:
                fuzz_nice_float(fuzz, &a, &b);
                path->moveTo(a, b);
                break;
            case 1:
                fuzz_nice_float(fuzz, &a, &b);
                path->lineTo(a, b);
                break;
            case 2:
                fuzz_nice_float(fuzz, &a, &b, &c, &d);
                path->quadTo(a, b, c, d);
                break;
            case 3:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->conicTo(a, b, c, d, e);
                break;
            case 4:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e, &f);
                path->cubicTo(a, b, c, d, e, f);
                break;
            case 5:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->arcTo(a, b, c, d, e);
                break;
            case 6:
                path->close();
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
}

// allows all float values for path points
void BuildPath(Fuzz* fuzz, SkPath* path, int last_verb) {
  while (!fuzz->exhausted()) {
    // Use a uint8_t to conserve bytes.  This makes our "fuzzed bytes footprint"
    // smaller, which leads to more efficient fuzzing.
    uint8_t operation;
    fuzz->next(&operation);
    SkScalar a,b,c,d,e,f;

    switch (operation % (last_verb + 1)) {
      case SkPath::Verb::kMove_Verb:
        fuzz->next(&a, &b);
        path->moveTo(a, b);
        break;

      case SkPath::Verb::kLine_Verb:
        fuzz->next(&a, &b);
        path->lineTo(a, b);
        break;

      case SkPath::Verb::kQuad_Verb:
        fuzz->next(&a, &b, &c, &d);
        path->quadTo(a, b, c, d);
        break;

      case SkPath::Verb::kConic_Verb:
        fuzz->next(&a, &b, &c, &d, &e);
        path->conicTo(a, b, c, d, e);
        break;

      case SkPath::Verb::kCubic_Verb:
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
