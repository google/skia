/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/Tessellation.h"

#include "include/core/SkPath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/CullTest.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu {

namespace {

// Writes a new path, chopping as necessary so no verbs require more segments than
// kMaxTessellationSegmentsPerCurve. Curves completely outside the viewport are flattened into
// lines.
class PathChopper {
public:
    PathChopper(const SkMatrix& matrix, const SkRect& viewport)
            : fCullTest(viewport, matrix)
            , fVectorXform(matrix) {
        fPath.setIsVolatile(true);
    }

    SkPath path() const { return fPath; }

    void moveTo(SkPoint p) { fPath.moveTo(p); }
    void lineTo(SkPoint p1) { fPath.lineTo(p1); }
    void close() { fPath.close(); }

    void quadTo(const SkPoint p[3]) {
        if (!fCullTest.areVisible3(p)) {
            this->lineTo(p[2]);
            return;
        }
        float n = wangs_formula::quadratic_pow4(kTessellationPrecision, p, fVectorXform);
        if (n > pow4(kMaxTessellationSegmentsPerCurve)) {
            SkPoint chops[5];
            SkChopQuadAtHalf(p, chops);
            this->quadTo(chops);
            this->quadTo(chops + 2);
            return;
        }
        fPath.quadTo(p[1], p[2]);
    }

    void conicTo(const SkPoint p[3], float w) {
        if (!fCullTest.areVisible3(p)) {
            this->lineTo(p[2]);
            return;
        }
        float n = wangs_formula::conic_pow2(kTessellationPrecision, p, w, fVectorXform);
        if (n > pow2(kMaxTessellationSegmentsPerCurve)) {
            SkConic chops[2];
            if (!SkConic(p,w).chopAt(.5, chops)) {
                this->lineTo(p[2]);
                return;
            }
            this->conicTo(chops[0].fPts, chops[0].fW);
            this->conicTo(chops[1].fPts, chops[1].fW);
            return;
        }
        fPath.conicTo(p[1], p[2], w);
    }

    void cubicTo(const SkPoint p[4]) {
        if (!fCullTest.areVisible4(p)) {
            this->lineTo(p[3]);
            return;
        }
        float n = wangs_formula::cubic_pow4(kTessellationPrecision, p, fVectorXform);
        if (n > pow4(kMaxTessellationSegmentsPerCurve)) {
            SkPoint chops[7];
            SkChopCubicAtHalf(p, chops);
            this->cubicTo(chops);
            this->cubicTo(chops + 3);
            return;
        }
        fPath.cubicTo(p[1], p[2], p[3]);
    }

private:
    const CullTest fCullTest;
    const wangs_formula::VectorXform fVectorXform;
    SkPath fPath;
};

}  // namespace

SkPath PreChopPathCurves(const SkPath& path, const SkMatrix& matrix, const SkRect& viewport) {
    PathChopper chopper(matrix, viewport);
    for (auto [verb, p, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                chopper.moveTo(p[0]);
                break;
            case SkPathVerb::kLine:
                chopper.lineTo(p[1]);
                break;
            case SkPathVerb::kQuad:
                chopper.quadTo(p);
                break;
            case SkPathVerb::kConic:
                chopper.conicTo(p, *w);
                break;
            case SkPathVerb::kCubic:
                chopper.cubicTo(p);
                break;
            case SkPathVerb::kClose:
                chopper.close();
                break;
        }
    }
    return chopper.path();
}

}  // namespace skgpu
