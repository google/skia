/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsCubic.h"

void CubicToQuads(const SkDCubic& cubic, double precision, SkTDArray<SkDQuad>& quads) {
    SkTDArray<double> ts;
    cubic.toQuadraticTs(precision, &ts);
    if (ts.count() <= 1) {
        SkDQuad quad = cubic.toQuad();
        *quads.append() = quad;
        return;
    }
    double tStart = 0;
    for (int i1 = 0; i1 <= ts.count(); ++i1) {
        const double tEnd = i1 < ts.count() ? ts[i1] : 1;
        SkDCubic part = cubic.subDivide(tStart, tEnd);
        SkDQuad quad = part.toQuad();
        *quads.append() = quad;
        tStart = tEnd;
    }
}
