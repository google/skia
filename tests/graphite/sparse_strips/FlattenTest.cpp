/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"

#include <cmath>

namespace skgpu::graphite {

namespace {

struct TrickyCurve {
    SkPoint fPoints[4];
    int fNumPts;
    float fScale = 1.0f;
};

void eval_quad_SIMD(const SkPoint pts[3], skvx::float4 t, skvx::float4& outX, skvx::float4& outY) {
    skvx::float4 mt = 1.0f - t;
    skvx::float4 a = mt * mt;
    skvx::float4 b = 2.0f * mt * t;
    skvx::float4 c = t * t;
    outX = pts[0].fX * a + pts[1].fX * b + pts[2].fX * c;
    outY = pts[0].fY * a + pts[1].fY * b + pts[2].fY * c;
}

void eval_conic_SIMD(
        const SkPoint pts[3], float w, skvx::float4 t, skvx::float4& outX, skvx::float4& outY) {
    skvx::float4 mt = 1.0f - t;
    skvx::float4 a = mt * mt;
    skvx::float4 b = 2.0f * w * t * mt;
    skvx::float4 c = t * t;
    skvx::float4 denom = a + b + c;
    skvx::float4 invDenom = 1.0f / denom;
    outX = (pts[0].fX * a + pts[1].fX * b + pts[2].fX * c) * invDenom;
    outY = (pts[0].fY * a + pts[1].fY * b + pts[2].fY * c) * invDenom;
}

void eval_cubic_SIMD(const SkPoint pts[4], skvx::float4 t, skvx::float4& outX, skvx::float4& outY) {
    skvx::float4 mt = 1.0f - t;
    skvx::float4 mt2 = mt * mt;
    skvx::float4 t2 = t * t;
    skvx::float4 a = mt2 * mt;
    skvx::float4 b = 3.0f * mt2 * t;
    skvx::float4 c = 3.0f * mt * t2;
    skvx::float4 d = t2 * t;
    outX = pts[0].fX * a + pts[1].fX * b + pts[2].fX * c + pts[3].fX * d;
    outY = pts[0].fY * a + pts[1].fY * b + pts[2].fY * c + pts[3].fY * d;
}

skvx::float4 dist_sq_point_SIMD(
        SkPoint p, skvx::float4 ax, skvx::float4 ay, skvx::float4 bx, skvx::float4 by) {
    skvx::float4 abx = bx - ax;
    skvx::float4 aby = by - ay;
    skvx::float4 apx = p.fX - ax;
    skvx::float4 apy = p.fY - ay;
    skvx::float4 lenSq = abx * abx + aby * aby;
    skvx::float4 dot = apx * abx + apy * aby;

    skvx::float4 t = skvx::if_then_else(lenSq == 0.0f, skvx::float4(0.0f), dot / lenSq);
    t = skvx::pin(t, skvx::float4(0.0f), skvx::float4(1.0f));

    skvx::float4 projx = ax + abx * t;
    skvx::float4 projy = ay + aby * t;
    skvx::float4 dx = p.fX - projx;
    skvx::float4 dy = p.fY - projy;

    return dx * dx + dy * dy;
}

float estimate_max_length_SIMD(const SkPoint pts[], int count) {
    if (count < 2) return 0.0f;
    skvx::float4 px(0.0f), py(0.0f), nx(0.0f), ny(0.0f);

    for (int i = 0; i < count - 1; ++i) {
        px[i] = pts[i].fX;
        py[i] = pts[i].fY;
        nx[i] = pts[i + 1].fX;
        ny[i] = pts[i + 1].fY;
    }

    skvx::float4 dx = nx - px;
    skvx::float4 dy = ny - py;
    skvx::float4 lens = skvx::sqrt(dx * dx + dy * dy);

    skvx::int4 mask = skvx::int4(0, 1, 2, 3) < skvx::int4(count - 1);
    lens = skvx::if_then_else(mask, lens, skvx::float4(0.0f));

    return lens[0] + lens[1] + lens[2] + lens[3];
}

template <typename SegmentCallback>
void for_each_path_segment_SIMD(const SkPath& path, float maxStepSize, SegmentCallback&& callback) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;

    SkPoint lastPt = {0, 0};
    SkPoint contourStart = {0, 0};
    bool hasContour = false;

    // Helper to dispatch a single segment (Line, Close, Move), returns false to early out if the
    // tolerance has been met
    auto emitSingleSegment = [&](SkPoint p0, SkPoint p1, const char* name) -> bool {
        skvx::float4 ax(p0.fX), ay(p0.fY);
        skvx::float4 bx(p1.fX), by(p1.fY);
        return callback(ax, ay, bx, by, 1, name);
    };

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (hasContour && lastPt != contourStart) {
                    if (!emitSingleSegment(lastPt, contourStart, "ImplicitClose")) return;
                }
                contourStart = pts[0];
                lastPt = pts[0];
                hasContour = true;
                // Emit a 0-length segment so coverage checks evaluate the starting point
                if (!emitSingleSegment(pts[0], pts[0], "Move")) return;
                break;
            case SkPath::kLine_Verb:
                if (!emitSingleSegment(pts[0], pts[1], "Line")) return;
                lastPt = pts[1];
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kCubic_Verb: {
                int countPts = (verb == SkPath::kQuad_Verb || verb == SkPath::kConic_Verb) ? 3 : 4;
                int samples = std::max(
                        1, (int)std::ceil(estimate_max_length_SIMD(pts, countPts) / maxStepSize));
                SkPoint p0 = pts[0];
                float w = (verb == SkPath::kConic_Verb) ? iter.conicWeight() : 1.0f;

                for (int i = 0; i < samples; i += 4) {
                    int count = std::min(4, samples - i);
                    skvx::float4 t = skvx::float4(i + 1, i + 2, i + 3, i + 4) / (float)samples;
                    skvx::float4 px, py;

                    switch (verb) {
                        case SkPath::kQuad_Verb:
                            eval_quad_SIMD(pts, t, px, py);
                            break;
                        case SkPath::kConic_Verb:
                            eval_conic_SIMD(pts, w, t, px, py);
                            break;
                        case SkPath::kCubic_Verb:
                            eval_cubic_SIMD(pts, t, px, py);
                            break;
                        default:
                            SkUNREACHABLE;
                    }

                    skvx::float4 ax, ay;
                    ax[0] = p0.fX; ax[1] = px[0]; ax[2] = px[1]; ax[3] = px[2];
                    ay[0] = p0.fY; ay[1] = py[0]; ay[2] = py[1]; ay[3] = py[2];

                    const char* name = (verb == SkPath::kQuad_Verb)    ? "Quad"
                                       : (verb == SkPath::kConic_Verb) ? "Conic"
                                                                       : "Cubic";

                    if (!callback(ax, ay, px, py, count, name)) return;
                    p0 = SkPoint::Make(px[count - 1], py[count - 1]);
                }
                lastPt = pts[countPts - 1];
                break;
            }
            case SkPath::kClose_Verb:
                if (lastPt != contourStart) {
                    if (!emitSingleSegment(lastPt, contourStart, "Close")) return;
                }
                lastPt = contourStart;
                hasContour = false;
                break;
            default:
                break;
        }
    }

    if (hasContour && lastPt != contourStart) {
        emitSingleSegment(lastPt, contourStart, "ImplicitClose");
    }
}

}  // anonymous namespace

template <FlattenMode kMode> class FlattenTestRunner {
    static constexpr float kQuadErrTolerance = Flatten::kQuadErrTolerance;
    static constexpr float kAllowedTolSq = Flatten::kQuadTolerance2 + 0.0001f;
    static constexpr float kMinDistSq = 1e10f;
    static constexpr float kMaxStepSize =
            .1f > kQuadErrTolerance * 0.5f ? .1f : kQuadErrTolerance * 0.5f;
    static constexpr int kLineSamples = 100;
    static constexpr float kLineDt = 1.0f / static_cast<float>(kLineSamples);
    static constexpr float kViewDim = 10000.0f;

    static float DistSqPointToPath(SkPoint pt, const SkPath& path) {
        float minDistSq = kMinDistSq;
        auto processSegment = [&minDistSq, pt](skvx::float4 ax, skvx::float4 ay,
                                               skvx::float4 bx, skvx::float4 by,
                                               int validCount, const char* verbName) {
            skvx::float4 dists = dist_sq_point_SIMD(pt, ax, ay, bx, by);
            skvx::int4 mask = skvx::int4(0, 1, 2, 3) < skvx::int4(validCount);
            dists = skvx::if_then_else(mask, dists, skvx::float4(kMinDistSq));
            minDistSq = std::min(minDistSq, skvx::min(dists));
            // Return false to potentially early out
            return minDistSq > kAllowedTolSq;
        };
        for_each_path_segment_SIMD(path, kMaxStepSize, processSegment);
        return minDistSq;
    }

    static void CheckFlattenedPath(skiatest::Reporter* reporter,
                                   const SkPath& originalPath,
                                   const char* testName) {
        SkRect bounds = originalPath.computeTightBounds();
        SkMatrix shift = SkMatrix::Translate(1000.0f - bounds.fLeft, 1000.0f - bounds.fTop);
        SkPath referencePath = originalPath.makeTransform(shift);

        Flatten flattener;
        Polyline polyline;

        flattener.processPaths<kMode>(originalPath, shift, kViewDim, kViewDim, &polyline);

        if (originalPath.isEmpty()) {
            REPORTER_ASSERT(reporter, polyline.empty(), "[%s] Expected empty output", testName);
            return;
        }

        const SkTDArray<SkPoint>& rawPts = polyline.points();

        // =========================================================================================
        // Watertightness Check
        // Iterate through the raw flattened points (which are separated by NaNs to denote distinct
        // contours) and assert that the final point of every contour matches its starting point.
        // =========================================================================================
        {
            SkPoint contourStart = {0, 0};
            SkPoint lastPt = {0, 0};
            bool inContour = false;
            for (int i = 0; i < rawPts.size(); ++i) {
                if (std::isnan(rawPts[i].fX)) {
                    if (inContour) {
                        REPORTER_ASSERT(reporter,
                                        lastPt == contourStart,
                                        "[%s] Contour did not close tightly. End: (%f, %f), Start: "
                                        "(%f, %f)",
                                        testName,
                                        lastPt.fX,
                                        lastPt.fY,
                                        contourStart.fX,
                                        contourStart.fY);
                    }
                    inContour = false;
                } else {
                    if (!inContour) {
                        contourStart = rawPts[i];
                        inContour = true;
                    }
                    lastPt = rawPts[i];
                }
            }
            // Catch the final contour if the array doesn't end with a NaN separator. (Should never
            // happen)
            if (inContour) {
                REPORTER_ASSERT(
                        reporter,
                        lastPt == contourStart,
                        "[%s] Contour did not close tightly. End: (%f, %f), Start: (%f, %f)",
                        testName,
                        lastPt.fX,
                        lastPt.fY,
                        contourStart.fX,
                        contourStart.fY);
            }
        }

        // =========================================================================================
        // Polyline-to-Path (Hausdorff Distance Approximation -> True Curve)
        //
        // Ensure that the flattener didn't generate line segments that stray too far from the true
        // path. To avoid numerical solvers, we sample *along the generated line segments* and
        // projecting them back onto the true path to find the shortest distance.
        // =========================================================================================
        for (auto [line, index] : polyline) {
            for (int i = 0; i < kLineSamples; i += 4) {
                skvx::float4 t03 = skvx::float4(i, i + 1, i + 2, i + 3) * kLineDt;
                skvx::float4 px03 = line.p0.fX * (1.0f - t03) + line.p1.fX * t03;
                skvx::float4 py03 = line.p0.fY * (1.0f - t03) + line.p1.fY * t03;

                int count = std::min(4, kLineSamples - i);
                for (int j = 0; j < count; ++j) {
                    SkPoint samplePt = SkPoint::Make(px03[j], py03[j]);
                    float minDistSq = DistSqPointToPath(samplePt, referencePath);
                    REPORTER_ASSERT(
                            reporter,
                            minDistSq <= kAllowedTolSq,
                            "[%s] Interpolated line point (%f, %f) deviated from true curve by "
                            "%f (tol: %f)",
                            testName,
                            samplePt.fX,
                            samplePt.fY,
                            std::sqrt(minDistSq),
                            kAllowedTolSq);
                }
            }

            // Check terminal t=1.0 separately
            SkPoint pt4 = line.p1;
            float minDistSq = DistSqPointToPath(pt4, referencePath);
            REPORTER_ASSERT(reporter,
                            minDistSq <= kAllowedTolSq,
                            "[%s] Interpolated line point (%f, %f) deviated from true curve by %f "
                            "(tol: %f)",
                            testName,
                            pt4.fX,
                            pt4.fY,
                            std::sqrt(minDistSq),
                            kAllowedTolSq);
        }

        // ==============================================================================
        // Path-to-Polyline (Hausdorff Distance True Curve -> Approximation)
        //
        // Ensure that the flattenner produces geometry for the length of the entire original curve,
        // instead of collapsing the curve into a single valid point or drawing an incomplete
        // segment that just happens to lie on the mathematical path.
        // ==============================================================================
        auto distSqPointToPolyline = [&polyline, &rawPts](SkPoint pt) {
            float minDistSq = kMinDistSq;
            skvx::float4 ax, ay, bx, by;
            int lane = 0;

            for (auto [line, index] : polyline) {
                ax[lane] = line.p0.fX;
                ay[lane] = line.p0.fY;
                bx[lane] = line.p1.fX;
                by[lane] = line.p1.fY;

                if (++lane == 4) {
                    skvx::float4 dists = dist_sq_point_SIMD(pt, ax, ay, bx, by);
                    minDistSq = std::min(minDistSq, skvx::min(dists));
                    if (minDistSq <= kAllowedTolSq) return minDistSq;
                    lane = 0;
                }
            }

            if (lane > 0) {
                skvx::int4 valid = skvx::int4(0, 1, 2, 3) < skvx::int4(lane);
                skvx::float4 dists = dist_sq_point_SIMD(pt, ax, ay, bx, by);
                dists = skvx::if_then_else(valid, dists, skvx::float4(kMinDistSq));
                minDistSq = std::min(minDistSq, skvx::min(dists));
                if (minDistSq <= kAllowedTolSq) return minDistSq;
            }

            for (SkPoint rawPt : rawPts) {
                if (!std::isnan(rawPt.fX)) {
                    SkPoint d = pt - rawPt;
                    minDistSq = std::min(minDistSq, SkPoint::DotProduct(d, d));
                    if (minDistSq <= kAllowedTolSq) return minDistSq;
                }
            }

            return minDistSq;
        };

        auto checkPathCoverage = [&reporter, testName, &distSqPointToPolyline](
                                         skvx::float4 ax, skvx::float4 ay,
                                         skvx::float4 bx, skvx::float4 by,
                                         int validCount, const char* verbName) {
            for (int i = 0; i < validCount; ++i) {
                SkPoint pt = SkPoint::Make(bx[i], by[i]);
                float minDistSq = distSqPointToPolyline(pt);
                REPORTER_ASSERT(reporter,
                                minDistSq <= kAllowedTolSq,
                                "[%s] Path %s point (%f, %f) lacks polyline "
                                "coverage. Deviated by %f (tol: %f)",
                                testName,
                                verbName,
                                pt.fX,
                                pt.fY,
                                std::sqrt(minDistSq),
                                kAllowedTolSq);
            }
            return true;  // continue checking the rest of the path
        };

        for_each_path_segment_SIMD(referencePath, kMaxStepSize, checkPathCoverage);
    }

    static void TestCulling(skiatest::Reporter* reporter) {
        const float kViewW = 100.0f;
        const float kViewH = 100.0f;

        enum class Expect {
            kCulled,      // Completely dropped: Start pt + NaN (2 points)
            kSimplified,  // Left side simplification: Start pt + End pt + Close pt + NaN (4 points)
            kSubdivided   // Intersects viewport: Subdivided curve (> 4 points)
        };

        auto checkPath = [&](const SkPath& path, Expect expect, const char* testName) {
            Flatten flattener;
            Polyline polyline;
            flattener.processPaths<kMode>(path, SkMatrix(), kViewW, kViewH, &polyline);

            int count = polyline.points().size();

            if (expect == Expect::kCulled) {
                REPORTER_ASSERT(
                        reporter,
                        count == 2,
                        "[%s] Expected curve to be entirely culled (2 points), got %d points",
                        testName,
                        count);
            } else if (expect == Expect::kSimplified) {
                REPORTER_ASSERT(
                        reporter,
                        count == 4,
                        "[%s] Expected curve to be simplified to 1 line (4 points), got %d points",
                        testName,
                        count);
            } else {
                REPORTER_ASSERT(reporter,
                                count > 4,
                                "[%s] Expected curve to cross viewport and be subdivided (>4 "
                                "points), got %d points",
                                testName,
                                count);
            }
        };

        checkPath(SkPathBuilder().moveTo(10, -20).quadTo(50, -80, 90, -20).detach(),
                  Expect::kCulled,
                  "Cull_Top");

        checkPath(SkPathBuilder().moveTo(10, 120).cubicTo(30, 180, 70, 180, 90, 120).detach(),
                  Expect::kCulled,
                  "Cull_Bottom");

        checkPath(SkPathBuilder().moveTo(120, 10).quadTo(180, 50, 120, 90).detach(),
                  Expect::kCulled,
                  "Cull_Right");

        checkPath(SkPathBuilder().moveTo(-50, 10).cubicTo(-20, 30, -20, 70, -50, 90).detach(),
                  Expect::kSimplified,
                  "Simplify_Left");

        checkPath(SkPathBuilder().moveTo(-20, -20).quadTo(120, -20, 120, 120).detach(),
                  Expect::kSubdivided,
                  "Crosses_Viewport");
    }

    static void TestTrickyStrokes(skiatest::Reporter* reporter) {
        const float kStrokeWidth = 30.0f;
        static const TrickyCurve kTrickyCurves[] = {
            {{{122, 737}, {348, 553}, {403, 761}, {400, 760}}, 4},
            {{{244, 520}, {244, 518}, {1141, 634}, {394, 688}}, 4},
            {{{550, 194}, {138, 130}, {1035, 246}, {288, 300}}, 4},
            {{{226, 733}, {556, 779}, {-43, 471}, {348, 683}}, 4},
            {{{268, 204}, {492, 304}, {352, 23}, {433, 412}}, 4},
            {{{172, 480}, {396, 580}, {256, 299}, {338, 677}}, 4},
            {{{731, 340}, {318, 252}, {1026, -64}, {367, 265}}, 4},
            {{{475, 708}, {62, 620}, {770, 304}, {220, 659}}, 4},
            {{{0, 0}, {128, 128}, {128, 0}, {0, 128}}, 4},
            {{{0, .01f}, {128, 127.999f}, {128, .01f}, {0, 127.99f}}, 4},
            {{{0, -.01f}, {128, 128.001f}, {128, -.01f}, {0, 128.001f}}, 4},
            {{{0, 0}, {0, -10}, {0, -10}, {0, 10}}, 4, 1.098283f},
            {{{10, 0}, {0, 0}, {20, 0}, {10, 0}}, 4},
            {{{39, -39}, {40, -40}, {40, -40}, {0, 0}}, 4},
            {{{39, -39}, {40, -40}, {37, -39}, {0, 0}}, 4},
            {{{40, 40}, {0, 0}, {200, 200}, {0, 0}}, 4},
            {{{0, 0}, {1e-2f, 0}, {-1e-2f, 0}, {0, 0}}, 4},
            {{{400.75f, 100.05f}, {400.75f, 100.05f}, {100.05f, 300.95f}, {100.05f, 300.95f}}, 4},
            {{{0.5f, 0}, {0, 0}, {20, 0}, {10, 0}}, 4},
            {{{10, 0}, {0, 0}, {10, 0}, {10, 0}}, 4},
            {{{1, 1}, {2, 1}, {1, 1}, {1, SK_ScalarNaN}}, 3},
            {{{1, 1}, {100, 1}, {25, 1}, {.3f, SK_ScalarNaN}}, 3},
            {{{1, 1}, {100, 1}, {25, 1}, {1.5f, SK_ScalarNaN}}, 3},
        };

        auto strokeAndCheck = [&](const SkPath& originalPath, const char* nameBase, int index) {
            SkStrokeRec miterStyle(SkStrokeRec::kHairline_InitStyle);
            miterStyle.setStrokeStyle(kStrokeWidth);
            miterStyle.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kMiter_Join, 4.0f);

            SkPathBuilder miterBuilder;
            miterStyle.applyToPath(&miterBuilder, originalPath);
            CheckFlattenedPath(reporter,
                               miterBuilder.detach(),
                               SkStringPrintf("%s_%d_Miter", nameBase, index).c_str());

            SkStrokeRec roundStyle(SkStrokeRec::kHairline_InitStyle);
            roundStyle.setStrokeStyle(kStrokeWidth);
            roundStyle.setStrokeParams(SkPaint::kRound_Cap, SkPaint::kRound_Join, 4.0f);

            SkPathBuilder roundBuilder;
            roundStyle.applyToPath(&roundBuilder, originalPath);
            CheckFlattenedPath(reporter,
                               roundBuilder.detach(),
                               SkStringPrintf("%s_%d_Round", nameBase, index).c_str());
        };

        for (size_t i = 0; i < std::size(kTrickyCurves); ++i) {
            auto [originalPts, numPts, scale] = kTrickyCurves[i];

            SkPoint p[4];
            memcpy(p, originalPts, sizeof(SkPoint) * numPts);
            for (int j = 0; j < numPts; ++j) {
                p[j] *= scale;
            }

            SkPathBuilder builder;
            builder.moveTo(p[0]);

            float w = originalPts[3].fX;

            if (numPts == 4) {
                builder.cubicTo(p[1], p[2], p[3]);
            } else if (w == 1.0f) {
                builder.quadTo(p[1], p[2]);
            } else {
                builder.conicTo(p[1], p[2], w);
            }

            strokeAndCheck(builder.detach(), "TrickyStroke", static_cast<int>(i));
        }

        SkPathBuilder largeRad;
        for (int y = 0; y < 2; ++y) {
            float shift = 210.f * y;
            float dy = 5.f * y;
            largeRad.moveTo(159.429f, 149.808f + shift)
                    .cubicTo({232.5f, 149.808f + dy + shift},
                             {232.5f, 149.808f + dy + shift},
                             {305.572f, 149.808f + shift});
        }

        SkStrokeRec thickStyle(SkStrokeRec::kHairline_InitStyle);
        thickStyle.setStrokeStyle(200.0f);
        thickStyle.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kMiter_Join, 4.0f);

        SkPathBuilder thickBuilder;
        SkPath largeRadPath = largeRad.detach();
        thickStyle.applyToPath(&thickBuilder, largeRadPath);

        CheckFlattenedPath(reporter, thickBuilder.detach(), "TrickyStroke_LargeRadius");
    }

public:
    static void RunAll(skiatest::Reporter* reporter) {
        CheckFlattenedPath(reporter, SkPath(), "EmptyPath");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).lineTo(50, 50).lineTo(90, 10).detach(),
                           "SimpleLines");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).quadTo(50, 100, 90, 10).detach(),
                           "SimpleQuad");
        CheckFlattenedPath(
                reporter,
                SkPathBuilder().moveTo(10, 10).cubicTo(30, 100, 70, 100, 90, 10).detach(),
                "SimpleCubic");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(0, 0).quadTo(50, 100, 100, 0).close().detach(),
                           "ClosedQuad");
        CheckFlattenedPath(
                reporter,
                SkPathBuilder().moveTo(10, 10).cubicTo(100, 10, 10, 100, 100, 100).detach(),
                "SCurveCubic");
        CheckFlattenedPath(reporter,
                           SkPathBuilder()
                                   .moveTo(10, 10)
                                   .quadTo(10, 10, 10, 10)
                                   .cubicTo(10, 10, 10, 10, 10, 10)
                                   .detach(),
                           "DegenerateCurves");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).cubicTo(40, 10, 70, 10, 100, 10).detach(),
                           "CollinearCubic");
        CheckFlattenedPath(
                reporter,
                SkPathBuilder().moveTo(50, 50).cubicTo(150, -50, 150, 150, 50, 50).detach(),
                "LoopCubic");
        CheckFlattenedPath(reporter,
                           SkPathBuilder()
                                   .moveTo(10, 10)
                                   .lineTo(20, 20)
                                   .moveTo(30, 30)
                                   .quadTo(50, 80, 70, 30)
                                   .close()
                                   .moveTo(100, 100)
                                   .cubicTo(120, 150, 180, 150, 200, 100)
                                   .detach(),
                           "MultiContour");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).conicTo(50, 100, 90, 10, 0.5f).detach(),
                           "SimpleConic_WeightHalf");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).conicTo(50, 100, 90, 10, 2.0f).detach(),
                           "SimpleConic_WeightTwo");
        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).conicTo(50, 100, 90, 10, 1.0f).detach(),
                           "SimpleConic_WeightOne");
        CheckFlattenedPath(reporter,
                           SkPathBuilder()
                                   .moveTo(100, 0)
                                   .conicTo(100, 100, 0, 100, std::sqrt(2.0f) / 2.0f)
                                   .detach(),
                           "QuarterCircleConic");

        CheckFlattenedPath(reporter,
                           SkPathBuilder().moveTo(0, 0).cubicTo(50, 0, 100, 0, 100, 50).detach(),
                           "Cubic_Partial_Linear");

        TestCulling(reporter);
        TestTrickyStrokes(reporter);
    }
};

DEF_TEST(SparseStrips_Flatten_Scalar, reporter) {
    skgpu::graphite::FlattenTestRunner<FlattenMode::kScalar>::RunAll(reporter);
}

DEF_TEST(SparseStrips_Flatten_SIMD, reporter) {
    skgpu::graphite::FlattenTestRunner<FlattenMode::kSimd>::RunAll(reporter);
}

}  // namespace skgpu::graphite
