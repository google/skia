/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "src/utils/SkPolyUtils.h"

void inline ignoreResult(bool ) {}

DEF_FUZZ(PolyUtils, fuzz) {
    int count;
    fuzz->nextRange(&count, 0, 512);
    SkAutoSTMalloc<64, SkPoint> polygon(count);
    for (int index = 0; index < count; ++index) {
        fuzz->next(&polygon[index].fX, &polygon[index].fY);
    }
    SkRect bounds;
    bounds.setBoundsCheck(polygon, count);

    ignoreResult(SkGetPolygonWinding(polygon, count));
    ignoreResult(SkIsConvexPolygon(polygon, count));
    ignoreResult(SkIsSimplePolygon(polygon, count));

    SkScalar inset;
    fuzz->next(&inset);
    SkTDArray<SkPoint> output;
    ignoreResult(SkInsetConvexPolygon(polygon, count, inset, &output));

    SkScalar offset;
    fuzz->next(&offset);
    ignoreResult(SkOffsetSimplePolygon(polygon, count, bounds, offset, &output));

    SkAutoSTMalloc<64, uint16_t> indexMap(count);
    for (int index = 0; index < count; ++index) {
        fuzz->next(&indexMap[index]);
    }
    SkTDArray<uint16_t> outputIndices;
    ignoreResult(SkTriangulateSimplePolygon(polygon, indexMap, count, &outputIndices));
}
