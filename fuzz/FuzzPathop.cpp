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
  while (!fuzz->exhausted()) {
    uint8_t operation = fuzz->next<uint8_t>();

    switch (operation % (last_verb + 1)) {
      case SkPath::Verb::kMove_Verb:
        path->moveTo(fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
        break;

      case SkPath::Verb::kLine_Verb:
        path->lineTo(fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
        break;

      case SkPath::Verb::kQuad_Verb:
        path->quadTo(fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
                     fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
        break;

      case SkPath::Verb::kConic_Verb:
        path->conicTo(fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
                      fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
                      fuzz->next<SkScalar>());
        break;

      case SkPath::Verb::kCubic_Verb:
        path->cubicTo(fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
                      fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
                      fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
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

    uint8_t stragglerOp = fuzz->next<uint8_t>();
    SkPath path;

    BuildPath(fuzz, &path, SkPath::Verb::kDone_Verb);
    builder.add(path, static_cast<SkPathOp>(stragglerOp % (kLastOp + 1)));

    SkPath result;
    builder.resolve(&result);
}
