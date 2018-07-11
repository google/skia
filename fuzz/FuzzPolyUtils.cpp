/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "FuzzCommon.h"
#include "SkPolyUtils.h"

void inline ignoreResult(bool ) {}

DEF_FUZZ(PolyUtils, fuzz) {
    int count;
    fuzz->nextRange(&count, 0, 512);
    SkAutoSTMalloc<64, SkPoint> polygon(count);
    for (int index = 0; index < count; ++index) {
        fuzz->next(&polygon[index].fX, &polygon[index].fY);
    }

    ignoreResult(SkGetPolygonWinding(polygon, count));
    ignoreResult(SkIsConvexPolygon(polygon, count));
    ignoreResult(SkIsSimplePolygon(polygon, count));

    SkScalar inset;
    fuzz->next(&inset);
    SkTDArray<SkPoint> output;
    ignoreResult(SkInsetConvexPolygon(polygon, count, inset, &output));
    std::function<SkScalar(const SkPoint&)> distanceFunc = [fuzz](const SkPoint& p) {
        SkScalar retVal;
        fuzz->next(&retVal);
        return retVal;
    };
    ignoreResult(SkInsetConvexPolygon(polygon, count, distanceFunc, &output));

    SkScalar offset;
    fuzz->next(&offset);
    ignoreResult(SkOffsetSimplePolygon(polygon, count, offset, &output));
    ignoreResult(SkOffsetSimplePolygon(polygon, count, distanceFunc, &output));

    SkAutoSTMalloc<64, uint16_t> indexMap(count);
    for (int index = 0; index < count; ++index) {
        fuzz->next(&indexMap[index]);
    }
    SkTDArray<uint16_t> outputIndices;
    ignoreResult(SkTriangulateSimplePolygon(polygon, indexMap, count, &outputIndices));
}
