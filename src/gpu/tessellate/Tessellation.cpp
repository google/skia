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

// This value only protects us against getting stuck in infinite recursion due to fp32 precision
// issues. Mathematically, every curve should reduce to manageable visible sections in O(log N)
// chops, where N is the the magnitude of its control points.
//
// But, to define a protective upper bound, a cubic can enter or exit the viewport as many as 6
// times. So we may need to refine the curve (via binary search chopping at T=.5) up to 6 times.
//
// Furthermore, chopping a cubic at T=.5 may only reduce its length by 1/8 (.5^3), so we may require
// up to 6 chops in order to reduce the length by 1/2.
constexpr static int kMaxChopsPerCurve = 128/*magnitude of +fp32_max - -fp32_max*/ *
                                         6/*max number of chops to reduce the length by half*/ *
                                         6/*max number of viewport boundary crosses*/;

// Writes a new path, chopping as necessary so no verbs require more segments than
// kMaxTessellationSegmentsPerCurve. Curves completely outside the viewport are flattened into
// lines.
class PathChopper {
public:
    PathChopper(float tessellationPrecision, const SkMatrix& matrix, const SkRect& viewport)
            : fTessellationPrecision(tessellationPrecision)
            , fCullTest(viewport, matrix)
            , fVectorXform(matrix) {
        fPath.setIsVolatile(true);
    }

    SkPath path() const { return fPath; }

    void moveTo(SkPoint p) { fPath.moveTo(p); }
    void lineTo(const SkPoint p[2]) { fPath.lineTo(p[1]); }
    void close() { fPath.close(); }

    void quadTo(const SkPoint quad[3]) {
        SkASSERT(fPointStack.empty());
        // Use a heap stack to recursively chop the quad into manageable, on-screen segments.
        fPointStack.push_back_n(3, quad);
        int numChops = 0;
        while (!fPointStack.empty()) {
            const SkPoint* p = fPointStack.end() - 3;
            if (!fCullTest.areVisible3(p)) {
                fPath.lineTo(p[2]);
            } else {
                float n4 = wangs_formula::quadratic_pow4(fTessellationPrecision, p, fVectorXform);
                if (n4 > pow4(kMaxTessellationSegmentsPerCurve) && numChops < kMaxChopsPerCurve) {
                    SkPoint chops[5];
                    SkChopQuadAtHalf(p, chops);
                    fPointStack.pop_back_n(3);
                    fPointStack.push_back_n(3, chops+2);
                    fPointStack.push_back_n(3, chops);
                    ++numChops;
                    continue;
                }
                fPath.quadTo(p[1], p[2]);
            }
            fPointStack.pop_back_n(3);
        }
    }

    void conicTo(const SkPoint conic[3], float weight) {
        SkASSERT(fPointStack.empty());
        SkASSERT(fWeightStack.empty());
        // Use a heap stack to recursively chop the conic into manageable, on-screen segments.
        fPointStack.push_back_n(3, conic);
        fWeightStack.push_back(weight);
        int numChops = 0;
        while (!fPointStack.empty()) {
            const SkPoint* p = fPointStack.end() - 3;
            float w = fWeightStack.back();
            if (!fCullTest.areVisible3(p)) {
                fPath.lineTo(p[2]);
            } else {
                float n2 = wangs_formula::conic_pow2(fTessellationPrecision, p, w, fVectorXform);
                if (n2 > pow2(kMaxTessellationSegmentsPerCurve) && numChops < kMaxChopsPerCurve) {
                    SkConic chops[2];
                    if (!SkConic(p,w).chopAt(.5, chops)) {
                        SkPoint line[2] = {p[0], p[2]};
                        this->lineTo(line);
                        continue;
                    }
                    fPointStack.pop_back_n(3);
                    fWeightStack.pop_back();
                    fPointStack.push_back_n(3, chops[1].fPts);
                    fWeightStack.push_back(chops[1].fW);
                    fPointStack.push_back_n(3, chops[0].fPts);
                    fWeightStack.push_back(chops[0].fW);
                    ++numChops;
                    continue;
                }
                fPath.conicTo(p[1], p[2], w);
            }
            fPointStack.pop_back_n(3);
            fWeightStack.pop_back();
        }
        SkASSERT(fWeightStack.empty());
    }

    void cubicTo(const SkPoint cubic[4]) {
        SkASSERT(fPointStack.empty());
        // Use a heap stack to recursively chop the cubic into manageable, on-screen segments.
        fPointStack.push_back_n(4, cubic);
        int numChops = 0;
        while (!fPointStack.empty()) {
            SkPoint* p = fPointStack.end() - 4;
            if (!fCullTest.areVisible4(p)) {
                fPath.lineTo(p[3]);
            } else {
                float n4 = wangs_formula::cubic_pow4(fTessellationPrecision, p, fVectorXform);
                if (n4 > pow4(kMaxTessellationSegmentsPerCurve) && numChops < kMaxChopsPerCurve) {
                    SkPoint chops[7];
                    SkChopCubicAtHalf(p, chops);
                    fPointStack.pop_back_n(4);
                    fPointStack.push_back_n(4, chops+3);
                    fPointStack.push_back_n(4, chops);
                    ++numChops;
                    continue;
                }
                fPath.cubicTo(p[1], p[2], p[3]);
            }
            fPointStack.pop_back_n(4);
        }
    }

private:
    const float fTessellationPrecision;
    const CullTest fCullTest;
    const wangs_formula::VectorXform fVectorXform;
    SkPath fPath;

    // Used for stack-based recursion (instead of using the runtime stack).
    SkSTArray<8, SkPoint> fPointStack;
    SkSTArray<2, float> fWeightStack;
};

}  // namespace

SkPath PreChopPathCurves(float tessellationPrecision,
                         const SkPath& path,
                         const SkMatrix& matrix,
                         const SkRect& viewport) {
    // If the viewport is exceptionally large, we could end up blowing out memory with an unbounded
    // number of of chops. Therefore, we require that the viewport is manageable enough that a fully
    // contained curve can be tessellated in kMaxTessellationSegmentsPerCurve or fewer. (Any larger
    // and that amount of pixels wouldn't fit in memory anyway.)
    SkASSERT(wangs_formula::worst_case_cubic(
                     tessellationPrecision,
                     viewport.width(),
                     viewport.height()) <= kMaxTessellationSegmentsPerCurve);
    PathChopper chopper(tessellationPrecision, matrix, viewport);
    for (auto [verb, p, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                chopper.moveTo(p[0]);
                break;
            case SkPathVerb::kLine:
                chopper.lineTo(p);
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
