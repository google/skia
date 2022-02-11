/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/DashLinePathRenderer.h"

#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/gradients/GrGradientShader.h"
#include "src/gpu/ops/DashOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

namespace skgpu::v1 {

// Checks if our simple dash FP can work in the the given environment.
static bool can_use_dash_fp(const SkMatrix& viewMatrix,
                            GrAAType aaType,
                            const SkStrokeRec& stroke,
                            const GrCaps& caps) {
    return !viewMatrix.hasPerspective() &&  // NOTE: for prespective, aaInverseRampWidth=fwidth(d).
           aaType != GrAAType::kNone &&  // The simple FP doesn't have a non-aa variant.
           stroke.getCap() == SkPaint::kButt_Cap &&  // Our FP doesn't support caps yet.
           stroke.getWidth() > 0 &&  // drawStrokedLine doesn't support hairlines yet.
           // If our gradient falls back on a texture, it needs to be floating point.
           caps.getDefaultBackendFormat(GrColorType::kRGBA_F16, GrRenderable::kNo).isValid();
}

PathRenderer::CanDrawPath DashLinePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkPoint pts[2];
    bool inverted;
    if (args.fShape->style().isDashed() && args.fShape->asLine(pts, &inverted)) {
        // We should never have an inverse dashed case.
        SkASSERT(!inverted);
        if (can_use_dash_fp(*args.fViewMatrix,
                            args.fAAType,
                            args.fShape->style().strokeRec(),
                            *args.fCaps)) {
            return CanDrawPath::kYes;
        }
        if (!DashOp::CanDrawDashLine(pts, args.fShape->style(), *args.fViewMatrix)) {
            return CanDrawPath::kNo;
        }
        return CanDrawPath::kYes;
    }
    return CanDrawPath::kNo;
}

// Creates a coverage FP that implements the given dash pattern.
static std::unique_ptr<GrFragmentProcessor> make_dash_coverage_fp(
        GrRecordingContext* ctx,
        const SurfaceDrawContext* sdc,
        const SkMatrix& viewMatrix,
        SkPoint p0,
        SkVector lineDirection,
        const float intervals[],
        int numIntervals,
        float phase,
        std::unique_ptr<GrFragmentProcessor>&& inputCoverage) {
    // Build an SDF of the dash pattern as a linear gradient.
    SkSTArray<8, float> stops(numIntervals + 2);
    SkSTArray<8, SkColor4f> coverages(numIntervals + 2);
    stops.push_back(0);
    coverages.push_back({0,0,0,0});
    float dashLength = 0;
    for (int i = 0; i < numIntervals; ++i) {
        float halfInterval = intervals[i] * .5f;
        stops.push_back(dashLength + halfInterval);
        float coverage = (i & 1) ? -halfInterval /*off*/ : +halfInterval /*on*/;
        coverages.push_back({0, 0, 0, coverage});
        dashLength += intervals[i];
    }
    float dashInvLength = 1/dashLength;
    for (int i = 1; i < stops.count(); ++i) {
        stops[i] *= dashInvLength;
    }
    stops.push_back(1);
    coverages.push_back({0,0,0,0});

    SkPoint gradPts[2] = {p0 + lineDirection * -phase,
                          p0 + lineDirection * (dashLength - phase)};
    viewMatrix.mapPoints(gradPts, 2);

    SkGradientShaderBase::Descriptor gradDesc;
    gradDesc.fColors = coverages.data();
    gradDesc.fPos = stops.data();
    gradDesc.fCount = stops.count();
    gradDesc.fTileMode = SkTileMode::kRepeat;

    GrColorInfo gradColorInfo(GrColorType::kRGBA_F16, kPremul_SkAlphaType, nullptr);
    auto dashSDF = GrGradientShader::MakeLinear(SkLinearGradient(gradPts, gradDesc),
                                                GrFPArgs(ctx,
                                                         SkMatrixProvider(SkMatrix::I()),
                                                         &gradColorInfo));

    // Convert the dash SDF to coverage.
    static auto dashEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader dashSDF;
        uniform float aaInverseRampWidth;
        half4 main(float2 coord, half4 inCoverage) {
            // Use sk_FragCoord instead of the interpolated local "coord" input. This avoids
            // precision issues interpolating across very long lines.
            half d = dashSDF.eval(sk_FragCoord.xy).a;
            half dashCoverage = saturate(d * aaInverseRampWidth + .5);
            return inCoverage * dashCoverage;
        }
    )");

    SkVector gradSpan = gradPts[1] - gradPts[0];
    float gradLength = gradSpan.length();
    float aaRampWidth = dashLength / gradLength;
    if (sdc->alwaysAntialias()) {
        // We are rendering to a DMSAA surface. Convert the AA ramp width from Euclidian to
        // Manhattan. DMSAA uses FillRRectOp in drawStrokedLine, which does Manhattan AA.
        aaRampWidth *= (fabsf(gradSpan.x()) + fabsf(gradSpan.y())) / gradLength;
    }

    return GrSkSLFP::Make(dashEffect,
                          "dash_fp",
                          std::move(inputCoverage),
                          GrSkSLFP::OptFlags::kNone,
                          "dashSDF", std::move(dashSDF),
                          "aaInverseRampWidth", 1/aaRampWidth);
}

// Trims off the ends of the given line if they begin or end in dashing "whitespace". This prevents
// us from picking up partial coverage from dashes that extend beyond the line.
static void trim_dashed_line(SkPoint lineDirection,
                             float lineLength,
                             const float intervals[],
                             int numIntervals,
                             float phase,
                             SkPoint trimmedPts[2]) {
    SkASSERT(numIntervals > 0);

    float dashLength = intervals[0];
    for (int i = 1; i < numIntervals; ++i) {
        dashLength += intervals[i];
    }

    {
        // Trim the beginning of the line, if necessary.
        float t0 = phase;
        t0 = t0 - dashLength * floorf(t0/dashLength);  // mod to [0, dashLength).
        float intervalEnd = 0;
        for (int i = 0; i < numIntervals; ++i) {
            intervalEnd += intervals[i];
            if (fabsf(intervalEnd - t0) < SK_ScalarNearlyZero) {
                intervalEnd = t0;
            }
            if (t0 < intervalEnd) {
                if (i & 1 /*off*/) {
                    // p0 is in an "off" interval. Trim the whitespace.
                    trimmedPts[0] += lineDirection * (intervalEnd - t0);
                }
                break;
            }
        }
    }
    {
        // Trim the end of the line, if necessary.
        float t1 = lineLength + phase;
        t1 = dashLength * (1 - ceilf(t1/dashLength)) + t1;  // mod to (0, dashLength].
        float intervalBegin = dashLength;
        for (int i = numIntervals - 1; i >= 0; --i) {
            intervalBegin -= intervals[i];
            if (fabsf(t1 - intervalBegin) < SK_ScalarNearlyZero) {
                intervalBegin = t1;
            }
            if (t1 > intervalBegin) {
                if (i & 1 /*off*/) {
                    // p1 is in an "off" interval. Trim the whitespace.
                    trimmedPts[1] -= lineDirection * (t1 - intervalBegin);
                }
                break;
            }
        }
    }
}

bool DashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "DashLinePathRenderer::onDrawPath");
    SkPoint pts[2];
    SkAssertResult(args.fShape->asLine(pts, nullptr));
    const auto& stroke = args.fShape->style().strokeRec();

    // First check if we can draw the dash with a simple FP.
    if (can_use_dash_fp(*args.fViewMatrix, args.fAAType, stroke, *args.fContext->priv().caps())) {
        // Unpack dash params.
        const float* intervals = args.fShape->style().dashIntervals();
        int numIntervals = args.fShape->style().dashIntervalCnt();
        if (numIntervals == 0) {
            return true;
        }
        float phase = args.fShape->style().dashPhase();

        SkVector lineDirection = pts[1] - pts[0];
        float lineLength = lineDirection.length();
        lineDirection *= 1/lineLength;

        // Add a "dash" coverage processor to the paint.
        auto dashFP = make_dash_coverage_fp(args.fContext,
                                            args.fSurfaceDrawContext,
                                            *args.fViewMatrix,
                                            pts[0],
                                            lineDirection,
                                            intervals,
                                            numIntervals,
                                            phase,
                                            args.fPaint.detachCoverageFragmentProcessor());
        if (!dashFP) {
            // The linear gradient in make_dash_coverage_fp will only fail in unexpected
            // circumstances. (It shouldn't even have a limit in # of stops.)
            SkDebugf("WARNING: failed to create FP for dash with %i intervals\n", numIntervals);
            return false;
        }
        args.fPaint.setCoverageFragmentProcessor(std::move(dashFP));

        // Trim off the ends of line if they begin or end in dashing "whitespace". This prevents
        // us from picking up partial coverage from dashes that extend beyond the line.
        SkPoint trimmedPts[2] = {pts[0], pts[1]};
        trim_dashed_line(lineDirection, lineLength, intervals, numIntervals, phase, trimmedPts);

        // Draw the dashed line.
        args.fSurfaceDrawContext->drawStrokedLine(args.fClip,
                                                  std::move(args.fPaint),
                                                  GrAA::kYes,
                                                  *args.fViewMatrix,
                                                  trimmedPts,
                                                  stroke);

        return true;
    }

    DashOp::AAMode aaMode;
    switch (args.fAAType) {
        case GrAAType::kNone:
            aaMode = DashOp::AAMode::kNone;
            break;
        case GrAAType::kMSAA:
            // In this mode we will use aa between dashes but the outer border uses MSAA. Otherwise,
            // we can wind up with external edges antialiased and internal edges unantialiased.
            aaMode = DashOp::AAMode::kCoverageWithMSAA;
            break;
        case GrAAType::kCoverage:
            aaMode = DashOp::AAMode::kCoverage;
            break;
    }
    GrOp::Owner op = DashOp::MakeDashLineOp(args.fContext, std::move(args.fPaint),
                                            *args.fViewMatrix, pts, aaMode, args.fShape->style(),
                                            args.fUserStencilSettings);
    if (!op) {
        return false;
    }
    args.fSurfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

} // namespace skgpu::v1
