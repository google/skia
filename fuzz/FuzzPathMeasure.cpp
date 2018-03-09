/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "FuzzCommon.h"
#include "SkPathMeasure.h"

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
    build_path(fuzz, &path, SkPath::Verb::kDone_Verb);
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
