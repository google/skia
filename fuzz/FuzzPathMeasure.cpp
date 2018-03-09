/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkPathMeasure.h"

static void BuildPath(Fuzz* fuzz,
               SkPath* path,
               int last_verb) {
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

DEF_FUZZ(PathMeasure, fuzz) {
    SkScalar resScale;
    fuzz->next(&resScale);
    uint8_t bits;
    fuzz->next(&bits);
    SkScalar distance[6];
    for (auto index = 0; index < 6; ++index) {
        fuzz->next(&distance[index]);
    }
    SkPath path;
    BuildPath(fuzz, &path, SkPath::Verb::kDone_Verb);
    SkPathMeasure measure(path, bits & 1, resScale);
    SkPoint position;
    SkVector tangent;
    (void) measure.getPosTan(distance[0], &position, &tangent);
    SkPath dst;
    (void) measure.getSegment(distance[1], distance[2], &dst, (bits >> 1) & 1);
    (void) measure.nextContour();
    (void) measure.getPosTan(distance[3], &position, &tangent);
    (void) measure.getSegment(distance[4], distance[5], &dst, (bits >> 2) & 1);
}
