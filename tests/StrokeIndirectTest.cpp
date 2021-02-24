/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/private/SkFloatingPoint.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/mock/GrMockOpTarget.h"
#include "src/gpu/tessellate/GrStrokeIndirectTessellator.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

using Tolerances = GrStrokeTessellateShader::Tolerances;

static sk_sp<GrDirectContext> make_mock_context() {
    GrMockOptions mockOptions;
    mockOptions.fDrawInstancedSupport = true;
    mockOptions.fMaxTessellationSegments = 64;
    mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fRenderability =
            GrMockOptions::ConfigOptions::Renderability::kMSAA;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fTexturable = true;
    mockOptions.fIntegerSupport = true;

    GrContextOptions ctxOptions;
    ctxOptions.fGpuPathRenderers = GpuPathRenderers::kTessellation;

    return GrDirectContext::MakeMock(&mockOptions, ctxOptions);
}

static void test_stroke(skiatest::Reporter* r, GrDirectContext* ctx, GrMockOpTarget* target,
                        const SkPath& path, SkRandom& rand) {
    SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
    stroke.setStrokeStyle(.1f);
    for (auto join : {SkPaint::kMiter_Join, SkPaint::kRound_Join}) {
        stroke.setStrokeParams(SkPaint::kButt_Cap, join, 4);
        for (int i = 0; i < 16; ++i) {
            float scale = ldexpf(rand.nextF() + 1, i);
            auto matrix = SkMatrix::Scale(scale, scale);
            GrStrokeTessellator::PathStrokeList pathStrokeList(path, stroke, SK_PMColor4fWHITE);
            GrStrokeIndirectTessellator tessellator(GrStrokeTessellateShader::ShaderFlags::kNone,
                                                    matrix, &pathStrokeList, path.countVerbs(),
                                                    target->allocator());
            tessellator.verifyResolveLevels(r, target, matrix, path, stroke);
            tessellator.prepare(target, matrix);
            tessellator.verifyBuffers(r, target, matrix, stroke);
        }
    }
}

DEF_TEST(tessellate_GrStrokeIndirectTessellator, r) {
    auto ctx = make_mock_context();
    auto target = std::make_unique<GrMockOpTarget>(ctx);
    SkRandom rand;

    // Empty strokes.
    SkPath path = SkPath();
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.moveTo(1,1);
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.moveTo(1,1);
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.close();
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.moveTo(1,1);
    test_stroke(r, ctx.get(), target.get(), path, rand);

    // Single line.
    path = SkPath().lineTo(1,1);
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.close();
    test_stroke(r, ctx.get(), target.get(), path, rand);

    // Single quad.
    path = SkPath().quadTo(1,0,1,1);
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.close();
    test_stroke(r, ctx.get(), target.get(), path, rand);

    // Single cubic.
    path = SkPath().cubicTo(1,0,0,1,1,1);
    test_stroke(r, ctx.get(), target.get(), path, rand);
    path.close();
    test_stroke(r, ctx.get(), target.get(), path, rand);

    // All types of lines.
    path.reset();
    for (int i = 0; i < (1 << 4); ++i) {
        path.moveTo((i>>0)&1, (i>>1)&1);
        path.lineTo((i>>2)&1, (i>>3)&1);
        path.close();
    }
    test_stroke(r, ctx.get(), target.get(), path, rand);

    // All types of quads.
    path.reset();
    for (int i = 0; i < (1 << 6); ++i) {
        path.moveTo((i>>0)&1, (i>>1)&1);
        path.quadTo((i>>2)&1, (i>>3)&1, (i>>4)&1, (i>>5)&1);
        path.close();
    }
    test_stroke(r, ctx.get(), target.get(), path, rand);

    // All types of cubics.
    path.reset();
    for (int i = 0; i < (1 << 8); ++i) {
        path.moveTo((i>>0)&1, (i>>1)&1);
        path.cubicTo((i>>2)&1, (i>>3)&1, (i>>4)&1, (i>>5)&1, (i>>6)&1, (i>>7)&1);
        path.close();
    }
    test_stroke(r, ctx.get(), target.get(), path, rand);

    {
        // This cubic has a convex-180 chop at T=1-"epsilon"
        static const uint32_t hexPts[] = {0x3ee0ac74, 0x3f1e061a, 0x3e0fc408, 0x3f457230,
                                          0x3f42ac7c, 0x3f70d76c, 0x3f4e6520, 0x3f6acafa};
        SkPoint pts[4];
        memcpy(pts, hexPts, sizeof(pts));
        test_stroke(r, ctx.get(), target.get(),
                    SkPath().moveTo(pts[0]).cubicTo(pts[1], pts[2], pts[3]).close(), rand);
    }

    // Random paths.
    for (int j = 0; j < 50; ++j) {
        path.reset();
        // Empty contours behave differently if closed.
        path.moveTo(0,0);
        path.moveTo(0,0);
        path.close();
        path.moveTo(0,0);
        SkPoint startPoint = {rand.nextF(), rand.nextF()};
        path.moveTo(startPoint);
        // Degenerate curves get skipped.
        path.lineTo(startPoint);
        path.quadTo(startPoint, startPoint);
        path.cubicTo(startPoint, startPoint, startPoint);
        for (int i = 0; i < 100; ++i) {
            switch (rand.nextRangeU(0, 4)) {
                case 0:
                    path.lineTo(rand.nextF(), rand.nextF());
                    break;
                case 1:
                    path.quadTo(rand.nextF(), rand.nextF(), rand.nextF(), rand.nextF());
                    break;
                case 2:
                case 3:
                case 4:
                    path.cubicTo(rand.nextF(), rand.nextF(), rand.nextF(), rand.nextF(),
                                 rand.nextF(), rand.nextF());
                    break;
                default:
                    SkUNREACHABLE;
            }
            if (i % 19 == 0) {
                switch (i/19 % 4) {
                    case 0:
                        break;
                    case 1:
                        path.lineTo(startPoint);
                        break;
                    case 2:
                        path.quadTo(SkPoint::Make(1.1f, 1.1f), startPoint);
                        break;
                    case 3:
                        path.cubicTo(SkPoint::Make(1.1f, 1.1f), SkPoint::Make(1.1f, 1.1f),
                                     startPoint);
                        break;
                }
                path.close();
                if (rand.nextU() & 1) {  // Implicit or explicit move?
                    startPoint = {rand.nextF(), rand.nextF()};
                    path.moveTo(startPoint);
                }
            }
        }
        test_stroke(r, ctx.get(), target.get(), path, rand);
    }
}

// Returns the control point for the first/final join of a contour.
// If the contour is not closed, returns the start point.
static SkPoint get_contour_closing_control_point(SkPathPriv::RangeIter iter,
                                                 const SkPathPriv::RangeIter& end) {
    auto [verb, p, w] = *iter;
    SkASSERT(verb == SkPathVerb::kMove);
    // Peek ahead to find the last control point.
    SkPoint startPoint=p[0], lastControlPoint=p[0];
    for (++iter; iter != end; ++iter) {
        auto [verb, p, w] = *iter;
        switch (verb) {
            case SkPathVerb::kMove:
                return startPoint;
            case SkPathVerb::kCubic:
                if (p[2] != p[3]) {
                    lastControlPoint = p[2];
                    break;
                }
                [[fallthrough]];
            case SkPathVerb::kQuad:
                if (p[1] != p[2]) {
                    lastControlPoint = p[1];
                    break;
                }
                [[fallthrough]];
            case SkPathVerb::kLine:
                if (p[0] != p[1]) {
                    lastControlPoint = p[0];
                }
                break;
            case SkPathVerb::kConic:
                SkUNREACHABLE;
            case SkPathVerb::kClose:
                return (p[0] == startPoint) ? lastControlPoint : p[0];
        }
    }
    return startPoint;
}

static bool check_resolve_level(skiatest::Reporter* r,  float numCombinedSegments,
                                int8_t actualLevel, float tolerance, bool printError = true) {
    int8_t expectedLevel = sk_float_nextlog2(numCombinedSegments);
    if ((actualLevel > expectedLevel &&
         actualLevel > sk_float_nextlog2(numCombinedSegments + tolerance)) ||
        (actualLevel < expectedLevel &&
         actualLevel < sk_float_nextlog2(numCombinedSegments - tolerance))) {
        if (printError) {
            ERRORF(r, "expected %f segments => resolveLevel=%i (got %i)\n",
                   numCombinedSegments, expectedLevel, actualLevel);
        }
        return false;
    }
    return true;
}

static bool check_first_resolve_levels(skiatest::Reporter* r,
                                       const SkTArray<float>& firstNumSegments,
                                       int8_t** nextResolveLevel, float tolerance) {
    for (float numSegments : firstNumSegments) {
        if (numSegments < 0) {
            int8_t val = *(*nextResolveLevel)++;
            REPORTER_ASSERT(r, val == (int)numSegments);
            continue;
        }
        // The first stroke's resolve levels aren't  written out until the end of
        // the contour.
        if (!check_resolve_level(r, numSegments, *(*nextResolveLevel)++, tolerance)) {
            return false;
        }
    }
    return true;
}

static float test_tolerance(SkPaint::Join joinType) {
    // Ensure our fast approximation falls within 1.15 tessellation segments of the "correct"
    // answer. This is more than good enough when our matrix scale can go up to 2^17.
    float tolerance = 1.15f;
    if (joinType == SkPaint::kRound_Join) {
        // We approximate two different angles when there are round joins. Double the tolerance.
        tolerance *= 2;
    }
    return tolerance;
}

void GrStrokeIndirectTessellator::verifyResolveLevels(skiatest::Reporter* r,
                                                      GrMockOpTarget* target,
                                                      const SkMatrix& viewMatrix,
                                                      const SkPath& path,
                                                      const SkStrokeRec& stroke) {
    auto tolerances = Tolerances::MakeNonHairline(viewMatrix.getMaxScale(), stroke.getWidth());
    int8_t resolveLevelForCircles = SkTPin<float>(
            sk_float_nextlog2(tolerances.fNumRadialSegmentsPerRadian * SK_ScalarPI),
            1, kMaxResolveLevel);
    float tolerance = test_tolerance(stroke.getJoin());
    int8_t* nextResolveLevel = fResolveLevels;
    auto iterate = SkPathPriv::Iterate(path);
    SkSTArray<3, float> firstNumSegments;
    bool isFirstStroke = true;
    SkPoint startPoint = {0,0};
    SkPoint lastControlPoint;
    for (auto iter = iterate.begin(); iter != iterate.end(); ++iter) {
        auto [verb, pts, w] = *iter;
        switch (verb) {
            int n;
            SkPoint chops[10];
            case SkPathVerb::kMove:
                startPoint = pts[0];
                lastControlPoint = get_contour_closing_control_point(iter, iterate.end());
                if (!check_first_resolve_levels(r, firstNumSegments, &nextResolveLevel,
                                                tolerance)) {
                    return;
                }
                firstNumSegments.reset();
                isFirstStroke = true;
                break;
            case SkPathVerb::kLine:
                if (pts[0] == pts[1]) {
                    break;
                }
                if (stroke.getJoin() == SkPaint::kRound_Join) {
                    float rotation = SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint,
                                                                  pts[1] - pts[0]);
                    float numSegments = rotation * tolerances.fNumRadialSegmentsPerRadian;
                    if (isFirstStroke) {
                        firstNumSegments.push_back(numSegments);
                    } else if (!check_resolve_level(r, numSegments, *nextResolveLevel++,
                                                    tolerance)) {
                        return;
                    }
                }
                lastControlPoint = pts[0];
                isFirstStroke = false;
                break;
            case SkPathVerb::kQuad: {
                if (pts[0] == pts[1] && pts[1] == pts[2]) {
                    break;
                }
                SkVector a = pts[1] - pts[0];
                SkVector b = pts[2] - pts[1];
                bool hasCusp = (a.cross(b) == 0 && a.dot(b) < 0);
                if (hasCusp) {
                    // The quad has a cusp. Make sure we wrote out a -resolveLevelForCircles.
                    if (isFirstStroke) {
                        firstNumSegments.push_back(-resolveLevelForCircles);
                    } else {
                        REPORTER_ASSERT(r, *nextResolveLevel++ == -resolveLevelForCircles);
                    }
                }
                float numParametricSegments = (hasCusp) ? 0 : GrWangsFormula::quadratic(
                        tolerances.fParametricIntolerance, pts);
                float rotation = (hasCusp) ? 0 : SkMeasureQuadRotation(pts);
                if (stroke.getJoin() == SkPaint::kRound_Join) {
                    SkVector controlPoint = (pts[0] == pts[1]) ? pts[2] : pts[1];
                    rotation += SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint,
                                                             controlPoint - pts[0]);
                }
                float numRadialSegments = rotation * tolerances.fNumRadialSegmentsPerRadian;
                float numSegments = numParametricSegments + numRadialSegments;
                if (!hasCusp || stroke.getJoin() == SkPaint::kRound_Join) {
                    if (isFirstStroke) {
                        firstNumSegments.push_back(numSegments);
                    } else if (!check_resolve_level(r, numSegments, *nextResolveLevel++,
                                                    tolerance)) {
                        return;
                    }
                }
                lastControlPoint = (pts[2] == pts[1]) ? pts[0] : pts[1];
                isFirstStroke = false;
                break;
            }
            case SkPathVerb::kCubic: {
                if (pts[0] == pts[1] && pts[1] == pts[2] && pts[2] == pts[3]) {
                    break;
                }
                float T[2];
                bool areCusps = false;
                n = GrPathUtils::findCubicConvex180Chops(pts, T, &areCusps);
                SkChopCubicAt(pts, chops, T, n);
                if (n > 0) {
                    int cuspResolveLevel = (areCusps) ? resolveLevelForCircles : 0;
                    int signal = -((n << 4) | cuspResolveLevel);
                    if (isFirstStroke) {
                        firstNumSegments.push_back((float)signal);
                    } else {
                        REPORTER_ASSERT(r, *nextResolveLevel++ == signal);
                    }
                }
                for (int i = 0; i <= n; ++i) {
                    // Find the number of segments with our unoptimized approach and make sure
                    // it matches the answer we got already.
                    SkPoint* p = chops + i*3;
                    float numParametricSegments =
                            GrWangsFormula::cubic(tolerances.fParametricIntolerance, p);
                    SkVector tan0 =
                            ((p[0] == p[1]) ? (p[1] == p[2]) ? p[3] : p[2] : p[1]) - p[0];
                    SkVector tan1 =
                            p[3] - ((p[3] == p[2]) ? (p[2] == p[1]) ? p[0] : p[1] : p[2]);
                    float rotation = SkMeasureAngleBetweenVectors(tan0, tan1);
                    if (i == 0 && stroke.getJoin() == SkPaint::kRound_Join) {
                        rotation += SkMeasureAngleBetweenVectors(p[0] - lastControlPoint, tan0);
                    }
                    float numRadialSegments = rotation * tolerances.fNumRadialSegmentsPerRadian;
                    float numSegments = numParametricSegments + numRadialSegments;
                    if (isFirstStroke) {
                        firstNumSegments.push_back(numSegments);
                    } else if (!check_resolve_level(r, numSegments, *nextResolveLevel++,
                                                    tolerance)) {
                        return;
                    }
                }
                lastControlPoint =
                        (pts[3] == pts[2]) ? (pts[2] == pts[1]) ? pts[0] : pts[1] : pts[2];
                isFirstStroke = false;
                break;
            }
            case SkPathVerb::kConic:
                SkUNREACHABLE;
            case SkPathVerb::kClose:
                if (pts[0] != startPoint) {
                    SkASSERT(!isFirstStroke);
                    if (stroke.getJoin() == SkPaint::kRound_Join) {
                        // Line from pts[0] to startPoint, with a preceding join.
                        float rotation = SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint,
                                                                      startPoint - pts[0]);
                        if (!check_resolve_level(
                                r, rotation * tolerances.fNumRadialSegmentsPerRadian,
                                *nextResolveLevel++, tolerance)) {
                            return;
                        }
                    }
                }
                if (!check_first_resolve_levels(r, firstNumSegments, &nextResolveLevel,
                                                tolerance)) {
                    return;
                }
                firstNumSegments.reset();
                isFirstStroke = true;
                break;
        }
    }
    if (!check_first_resolve_levels(r, firstNumSegments, &nextResolveLevel, tolerance)) {
        return;
    }
    firstNumSegments.reset();
    SkASSERT(nextResolveLevel == fResolveLevels + fResolveLevelArrayCount);
}

void GrStrokeIndirectTessellator::verifyBuffers(skiatest::Reporter* r, GrMockOpTarget* target,
                                                const SkMatrix& viewMatrix,
                                                const SkStrokeRec& stroke) {
    // Make sure the resolve level we assigned to each instance agrees with the actual data.
    struct IndirectInstance {
        SkPoint fPts[4];
        SkPoint fLastControlPoint;
        float fNumTotalEdges;
    };
    auto instance = static_cast<const IndirectInstance*>(target->peekStaticVertexData());
    auto* indirect = static_cast<const GrDrawIndirectCommand*>(target->peekStaticIndirectData());
    auto tolerances = Tolerances::MakeNonHairline(viewMatrix.getMaxScale(), stroke.getWidth());
    float tolerance = test_tolerance(stroke.getJoin());
    for (int i = 0; i < fChainedDrawIndirectCount; ++i) {
        int numExtraEdgesInJoin = (stroke.getJoin() == SkPaint::kMiter_Join) ? 4 : 3;
        int numStrokeEdges = indirect->fVertexCount/2 - numExtraEdgesInJoin;
        int numSegments = numStrokeEdges - 1;
        bool isPow2 = !(numSegments & (numSegments - 1));
        REPORTER_ASSERT(r, isPow2);
        int resolveLevel = sk_float_nextlog2(numSegments);
        REPORTER_ASSERT(r, 1 << resolveLevel == numSegments);
        for (unsigned j = 0; j < indirect->fInstanceCount; ++j) {
            SkASSERT(fabsf(instance->fNumTotalEdges) == indirect->fVertexCount/2);
            const SkPoint* p = instance->fPts;
            float numParametricSegments = GrWangsFormula::cubic(
                    tolerances.fParametricIntolerance, p);
            float alternateNumParametricSegments = numParametricSegments;
            if (p[0] == p[1] && p[2] == p[3]) {
                // We articulate lines as "p0,p0,p1,p1". This one might actually expect 0 parametric
                // segments.
                alternateNumParametricSegments = 0;
            }
            SkVector tan0 = ((p[0] == p[1]) ? (p[1] == p[2]) ? p[3] : p[2] : p[1]) - p[0];
            SkVector tan1 = p[3] - ((p[3] == p[2]) ? (p[2] == p[1]) ? p[0] : p[1] : p[2]);
            float rotation = SkMeasureAngleBetweenVectors(tan0, tan1);
            // Negative fNumTotalEdges means the curve is a chop, and chops always get treated as a
            // bevel join.
            if (stroke.getJoin() == SkPaint::kRound_Join && instance->fNumTotalEdges > 0) {
                SkVector lastTangent = p[0] - instance->fLastControlPoint;
                rotation += SkMeasureAngleBetweenVectors(lastTangent, tan0);
            }
            // Degenerate strokes are a special case that actually mean the GPU should draw a cusp
            // (i.e. circle).
            if (p[0] == p[1] && p[1] == p[2] && p[2] == p[3]) {
                rotation = SK_ScalarPI;
            }
            float numRadialSegments = rotation * tolerances.fNumRadialSegmentsPerRadian;
            float numSegments = numParametricSegments + numRadialSegments;
            float alternateNumSegments = alternateNumParametricSegments + numRadialSegments;
            if (!check_resolve_level(r, numSegments, resolveLevel, tolerance, false) &&
                !check_resolve_level(r, alternateNumSegments, resolveLevel, tolerance, true)) {
                return;
            }
            ++instance;
        }
        ++indirect;
    }
}
