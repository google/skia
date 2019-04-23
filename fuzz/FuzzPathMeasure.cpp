/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkPathMeasure.h"

void inline ignoreResult(bool ) {}

DEF_FUZZ(PathMeasure, fuzz) {
    uint8_t bits;
    fuzz->next(&bits);
    SkScalar distance[6];
    for (auto index = 0; index < 6; ++index) {
        fuzz->next(&distance[index]);
    }
    SkPath path;
    FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);
    SkRect bounds = path.getBounds();
    SkScalar maxDim = SkTMax(bounds.width(), bounds.height());
    SkScalar resScale = maxDim / 1000;
    SkPathMeasure measure(path, bits & 1, resScale);
    SkPoint position;
    SkVector tangent;
    ignoreResult(measure.getPosTan(distance[0], &position, &tangent));
    SkPath dst;
    ignoreResult(measure.getSegment(distance[1], distance[2], &dst, (bits >> 1) & 1));
    ignoreResult(measure.nextContour());
    ignoreResult(measure.getPosTan(distance[3], &position, &tangent));
    ignoreResult(measure.getSegment(distance[4], distance[5], &dst, (bits >> 2) & 1));
}
