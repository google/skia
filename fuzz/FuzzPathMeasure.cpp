/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathMeasure.h"

#include <array>

void inline ignoreResult(bool ) {}

DEF_FUZZ(PathMeasure, fuzz) {
    uint8_t bits;
    fuzz->next(&bits);
    std::array<SkScalar, 6> distance = {};
    for (SkScalar& d : distance) {
        fuzz->next(&d);
    }
    SkPath path = FuzzEvilPath(fuzz, SkPath::Verb::kDone_Verb);
    SkRect bounds = path.getBounds();
    SkScalar maxDim = std::max(bounds.width(), bounds.height());
    if (maxDim > 1000000) {
        return;
    }
    SkScalar resScale = maxDim / 1000;
    SkPathMeasure measure(path, bits & 1, resScale);
    SkPoint position;
    SkVector tangent;
    ignoreResult(measure.getPosTan(distance[0], &position, &tangent));
    SkPathBuilder dst;
    ignoreResult(measure.getSegment(distance[1], distance[2], &dst, (bits >> 1) & 1));
    ignoreResult(measure.nextContour());
    ignoreResult(measure.getPosTan(distance[3], &position, &tangent));
    ignoreResult(measure.getSegment(distance[4], distance[5], &dst, (bits >> 2) & 1));
}
