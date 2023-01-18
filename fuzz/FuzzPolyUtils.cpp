/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

#include "include/core/SkPoint.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/utils/SkPolyUtils.h"

using namespace skia_private;

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
void inline ignoreResult(bool ) {}

// clamps the point to the nearest 16th of a pixel
static SkPoint sanitize_point(const SkPoint& in) {
    SkPoint out;
    out.fX = SkScalarRoundToScalar(16.f*in.fX)*0.0625f;
    out.fY = SkScalarRoundToScalar(16.f*in.fY)*0.0625f;
    return out;
}

DEF_FUZZ(PolyUtils, fuzz) {
    int count;
    fuzz->nextRange(&count, 0, 512);
    AutoSTMalloc<64, SkPoint> polygon(count);
    for (int index = 0; index < count; ++index) {
        fuzz->next(&polygon[index].fX, &polygon[index].fY);
        polygon[index] = sanitize_point(polygon[index]);
    }
    SkRect bounds;
    bounds.setBoundsCheck(polygon, count);

    ignoreResult(SkGetPolygonWinding(polygon, count));
    bool isConvex = SkIsConvexPolygon(polygon, count);
    bool isSimple = SkIsSimplePolygon(polygon, count);

    SkTDArray<SkPoint> output;
    if (isConvex) {
        SkScalar inset;
        fuzz->next(&inset);
        ignoreResult(SkInsetConvexPolygon(polygon, count, inset, &output));
    }

    if (isSimple) {
        SkScalar offset;
        // Limit this to prevent timeouts.
        // This should be fine, as this is roughly the range we expect from the shadow algorithm.
        fuzz->nextRange(&offset, -1000, 1000);
        ignoreResult(SkOffsetSimplePolygon(polygon, count, bounds, offset, &output));

        AutoSTMalloc<64, uint16_t> indexMap(count);
        for (int index = 0; index < count; ++index) {
            fuzz->next(&indexMap[index]);
        }
        SkTDArray<uint16_t> outputIndices;
        ignoreResult(SkTriangulateSimplePolygon(polygon, indexMap, count, &outputIndices));
    }
}
#else
DEF_FUZZ(PolyUtils, fuzz) {}
#endif // !defined(SK_ENABLE_OPTIMIZE_SIZE)
