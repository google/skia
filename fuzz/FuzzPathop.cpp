/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkPath.h"
#include "SkPathOps.h"

const int kLastOp = SkPathOp::kReverseDifference_SkPathOp;

void BuildPath(Fuzz* fuzz,
               SkPath* path,
               int last_verb) {
  uint8_t operation;
  SkScalar a, b, c, d, e, f;
  while (fuzz->next<uint8_t>(&operation)) {

    switch (operation % (last_verb + 1)) {
      case SkPath::Verb::kMove_Verb:
        if (!fuzz->next<SkScalar>(&a) || !fuzz->next<SkScalar>(&b))
          return;
        path->moveTo(a, b);
        break;

      case SkPath::Verb::kLine_Verb:
        if (!fuzz->next<SkScalar>(&a) || !fuzz->next<SkScalar>(&b))
          return;
        path->lineTo(a, b);
        break;

      case SkPath::Verb::kQuad_Verb:
        if (!fuzz->next<SkScalar>(&a) ||
            !fuzz->next<SkScalar>(&b) ||
            !fuzz->next<SkScalar>(&c) ||
            !fuzz->next<SkScalar>(&d))
          return;
        path->quadTo(a, b, c, d);
        break;

      case SkPath::Verb::kConic_Verb:
        if (!fuzz->next<SkScalar>(&a) ||
            !fuzz->next<SkScalar>(&b) ||
            !fuzz->next<SkScalar>(&c) ||
            !fuzz->next<SkScalar>(&d) ||
            !fuzz->next<SkScalar>(&e))
          return;
        path->conicTo(a, b, c, d, e);
        break;

      case SkPath::Verb::kCubic_Verb:
        if (!fuzz->next<SkScalar>(&a) ||
            !fuzz->next<SkScalar>(&b) ||
            !fuzz->next<SkScalar>(&c) ||
            !fuzz->next<SkScalar>(&d) ||
            !fuzz->next<SkScalar>(&e) ||
            !fuzz->next<SkScalar>(&f))
          return;
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

DEF_FUZZ(Pathop, fuzz) {
    SkOpBuilder builder;
    while (fuzz->remaining() >= sizeof(uint8_t)) {
        SkPath path;
        uint8_t op = fuzz->nextB();

        BuildPath(fuzz, &path, SkPath::Verb::kDone_Verb);
        builder.add(path, static_cast<SkPathOp>(op % (kLastOp + 1)));
    }

    SkPath result;
    builder.resolve(&result);
}
