/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellateStrokeOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStrokeGeometry.h"
#include "src/gpu/tessellate/GrTessellateStrokeShader.h"

static SkPath transform_path(const SkMatrix& viewMatrix, const SkPath& path) {
    SkPath devPath;
    // The provided matrix must be a similarity matrix for the time being. This is so we can
    // bootstrap this Op on top of GrStrokeGeometry with minimal modifications.
    SkASSERT(viewMatrix.isSimilarity());
    path.transform(viewMatrix, &devPath);
    return devPath;
}

static SkStrokeRec transform_stroke(const SkMatrix& viewMatrix, const SkStrokeRec& stroke) {
    SkStrokeRec devStroke = stroke;
    // kStrokeAndFill_Style is not yet supported.
    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style ||
             stroke.getStyle() == SkStrokeRec::kHairline_Style);
    float strokeWidth = (stroke.getStyle() == SkStrokeRec::kHairline_Style) ?
            1 : viewMatrix.getMaxScale() * stroke.getWidth();
    devStroke.setStrokeStyle(strokeWidth, /*strokeAndFill=*/false);
    return devStroke;
}

static SkPMColor4f get_paint_constant_blended_color(const GrPaint& paint) {
    SkPMColor4f constantColor;
    // Patches can overlap, so until a stencil technique is implemented, the provided paints must be
    // constant blended colors.
    SkAssertResult(paint.isConstantBlendedColor(&constantColor));
    return constantColor;
}

GrTessellateStrokeOp::GrTessellateStrokeOp(const SkMatrix& viewMatrix, const SkPath& path,
                                           const SkStrokeRec& stroke, GrPaint&& paint,
                                           GrAAType aaType)
        : GrDrawOp(ClassID())
        , fPathStrokes(transform_path(viewMatrix, path), transform_stroke(viewMatrix, stroke))
        , fNumVerbs(path.countVerbs())
        , fNumPoints(path.countPoints())
        , fColor(get_paint_constant_blended_color(paint))
        , fAAType(aaType)
        , fProcessors(std::move(paint)) {
    SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples support yet.
    SkStrokeRec& headStroke = fPathStrokes.head().fStroke;
    if (headStroke.getJoin() == SkPaint::kMiter_Join) {
        float miter = headStroke.getMiter();
        if (miter <= 0) {
            headStroke.setStrokeParams(headStroke.getCap(), SkPaint::kBevel_Join, 0);
        } else {
            fMiterLimitOrZero = miter;
        }
    }
    SkRect devBounds = fPathStrokes.head().fPath.getBounds();
    float inflationRadius = fPathStrokes.head().fStroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
}

GrDrawOp::FixedFunctionFlags GrTessellateStrokeOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrTessellateStrokeOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip,
                                                        bool hasMixedSampledCoverage,
                                                        GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip,
                                &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps,
                                clampType, &fColor);
}

GrOp::CombineResult GrTessellateStrokeOp::onCombineIfPossible(GrOp* grOp,
                                                              GrRecordingContext::Arenas* arenas,
                                                              const GrCaps&) {
    auto* op = grOp->cast<GrTessellateStrokeOp>();
    if (fColor != op->fColor ||
        fViewMatrix != op->fViewMatrix ||
        fAAType != op->fAAType ||
        ((fMiterLimitOrZero * op->fMiterLimitOrZero != 0) &&  // Are both non-zero?
         fMiterLimitOrZero != op->fMiterLimitOrZero) ||
        fProcessors != op->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    fPathStrokes.concat(std::move(op->fPathStrokes), arenas->recordTimeAllocator());
    if (op->fMiterLimitOrZero != 0) {
        SkASSERT(fMiterLimitOrZero == 0 || fMiterLimitOrZero == op->fMiterLimitOrZero);
        fMiterLimitOrZero = op->fMiterLimitOrZero;
    }
    fNumVerbs += op->fNumVerbs;
    fNumPoints += op->fNumPoints;

    return CombineResult::kMerged;
}

void GrTessellateStrokeOp::onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView* writeView,
                                        GrAppliedClip*, const GrXferProcessor::DstProxyView&) {
}

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static void write_line(SkPoint* patch, const SkPoint& p0, const SkPoint& p1) {
    patch[0] = p0;
    patch[1] = lerp(p0, p1, 1/3.f);
    patch[2] = lerp(p0, p1, 2/3.f);
    patch[3] = p1;
}

static void write_quadratic(SkPoint* patch, const SkPoint pts[]) {
    patch[0] = pts[0];
    patch[1] = lerp(pts[0], pts[1], 2/3.f);
    patch[2] = lerp(pts[1], pts[2], 1/3.f);
    patch[3] = pts[2];
}

static void write_loop(SkPoint* patch, const SkPoint& intersectionPoint,
                       const SkPoint lastControlPt, const SkPoint& nextControlPt) {
    patch[0] = intersectionPoint;
    patch[1] = lastControlPt;
    patch[2] = nextControlPt;
    patch[3] = intersectionPoint;
}

static void write_square_cap(SkPoint* patch, const SkPoint& endPoint,
                             const SkPoint controlPoint, float strokeRadius) {
    SkVector v = (endPoint - controlPoint);
    v.normalize();
    SkPoint capPoint = endPoint + v*strokeRadius;
    // Construct a line that incorporates controlPoint so we get a water tight edge with the rest of
    // the stroke. The cubic will technically step outside the cap, but we will force it to only
    // have one segment, giving edges only at the endpoints.
    patch[0] = endPoint;
    patch[1] = controlPoint;
    // Straddle the midpoint of the cap because the tessellated geometry emits a center point at
    // T=.5, and we need to ensure that point stays inside the cap.
    patch[2] = endPoint + capPoint - controlPoint;
    patch[3] = capPoint;
}

void GrTessellateStrokeOp::onPrepare(GrOpFlushState* flushState) {
    // Rebuild the stroke using GrStrokeGeometry.
    GrStrokeGeometry strokeGeometry(flushState->caps().shaderCaps()->maxTessellationSegments(),
                                    fNumPoints, fNumVerbs);
    for (auto& [path, stroke] : fPathStrokes) {
        float strokeRadius = stroke.getWidth() * .5f;
        GrStrokeGeometry::InstanceTallies tallies = GrStrokeGeometry::InstanceTallies();
        strokeGeometry.beginPath(stroke, strokeRadius * 2, &tallies);
        SkPathVerb previousVerb = SkPathVerb::kClose;
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    if (previousVerb != SkPathVerb::kClose) {
                        strokeGeometry.capContourAndExit();
                    }
                    strokeGeometry.moveTo(pts[0]);
                    break;
                case SkPathVerb::kClose:
                    strokeGeometry.closeContour();
                    break;
                case SkPathVerb::kLine:
                    strokeGeometry.lineTo(pts[1]);
                    break;
                case SkPathVerb::kQuad:
                    strokeGeometry.quadraticTo(pts);
                    break;
                case SkPathVerb::kCubic:
                    strokeGeometry.cubicTo(pts);
                    break;
                case SkPathVerb::kConic:
                    SkUNREACHABLE;
            }
            previousVerb = verb;
        }
        if (previousVerb != SkPathVerb::kClose) {
            strokeGeometry.capContourAndExit();
        }
    }

    auto vertexData = static_cast<SkPoint*>(flushState->makeVertexSpace(
            sizeof(SkPoint), strokeGeometry.verbs().count() * 2 * 5, &fVertexBuffer, &fBaseVertex));
    if (!vertexData) {
        return;
    }

    using Verb = GrStrokeGeometry::Verb;

    // Dump GrStrokeGeometry into tessellation patches.
    //
    // This loop is only a temporary adapter for GrStrokeGeometry so we can bootstrap the
    // tessellation shaders. Once the shaders are landed and tested, we will overhaul
    // GrStrokeGeometry and remove this loop.
    int i = 0;
    const SkTArray<SkPoint, true>& pathPts = strokeGeometry.points();
    auto pendingJoin = Verb::kEndContour;
    SkPoint firstJoinControlPoint = {0, 0};
    SkPoint lastJoinControlPoint = {0, 0};
    bool hasFirstControlPoint = false;
    float currStrokeRadius = 0;
    auto pathStrokesIter = fPathStrokes.begin();
    for (auto verb : strokeGeometry.verbs()) {
        SkPoint patch[4];
        float overrideNumSegments = 0;
        switch (verb) {
            case Verb::kBeginPath:
                SkASSERT(pathStrokesIter != fPathStrokes.end());
                pendingJoin = Verb::kEndContour;
                firstJoinControlPoint = {0, 0};
                lastJoinControlPoint = {0, 0};
                hasFirstControlPoint = false;
                currStrokeRadius = (*pathStrokesIter).fStroke.getWidth() * .5f;
                ++pathStrokesIter;
                continue;
            case Verb::kRoundJoin:
            case Verb::kInternalRoundJoin:
            case Verb::kMiterJoin:
            case Verb::kBevelJoin:
            case Verb::kInternalBevelJoin:
                pendingJoin = verb;
                continue;
            case Verb::kLinearStroke:
                write_line(patch, pathPts[i], pathPts[i+1]);
                ++i;
                break;
            case Verb::kQuadraticStroke:
                write_quadratic(patch, &pathPts[i]);
                i += 2;
                break;
            case Verb::kCubicStroke:
                memcpy(patch, &pathPts[i], sizeof(SkPoint) * 4);
                i += 3;
                break;
            case Verb::kRotate:
                write_loop(patch, pathPts[i], pathPts[i+1], pathPts[i]*2 - pathPts[i+1]);
                i += 2;
                break;
            case Verb::kSquareCap: {
                SkASSERT(pendingJoin == Verb::kEndContour);
                write_square_cap(patch, pathPts[i], lastJoinControlPoint, currStrokeRadius);
                // This cubic steps outside the cap, but if we force it to only have one segment, we
                // will just get the rectangular cap.
                overrideNumSegments = 1;
                break;
            }
            case Verb::kRoundCap:
                // A round cap is the same thing as a 180-degree round join.
                SkASSERT(pendingJoin == Verb::kEndContour);
                pendingJoin = Verb::kRoundJoin;
                write_loop(patch, pathPts[i], lastJoinControlPoint, lastJoinControlPoint);
                break;
            case Verb::kEndContour:
                // Final join
                write_loop(patch, pathPts[i], firstJoinControlPoint, lastJoinControlPoint);
                ++i;
                break;
        }

        SkPoint c1 = (patch[1] == patch[0]) ? patch[2] : patch[1];
        SkPoint c2 = (patch[2] == patch[3]) ? patch[1] : patch[2];

        if (pendingJoin != Verb::kEndContour) {
            vertexData[0] = patch[0];
            vertexData[1] = lastJoinControlPoint;
            vertexData[2] = c1;
            vertexData[3] = patch[0];
            switch (pendingJoin) {
                case Verb::kBevelJoin:
                    vertexData[4].set(1, currStrokeRadius);
                    break;
                case Verb::kMiterJoin:
                    vertexData[4].set(2, currStrokeRadius);
                    break;
                case Verb::kRoundJoin:
                    vertexData[4].set(3, currStrokeRadius);
                    break;
                case Verb::kInternalRoundJoin:
                case Verb::kInternalBevelJoin:
                default:
                    vertexData[4].set(4, currStrokeRadius);
                    break;
            }
            vertexData += 5;
            fVertexCount += 5;
            pendingJoin = Verb::kEndContour;
        }

        if (verb != Verb::kRoundCap) {
            if (!hasFirstControlPoint) {
                firstJoinControlPoint = c1;
                hasFirstControlPoint = true;
            }
            lastJoinControlPoint = c2;
        }

        if (verb == Verb::kEndContour) {
            // Temporary hack for this adapter in case the next contour is a round cap.
            lastJoinControlPoint = firstJoinControlPoint;
            hasFirstControlPoint = false;
        } else if (verb != Verb::kRotate && verb != Verb::kRoundCap) {
            memcpy(vertexData, patch, sizeof(SkPoint) * 4);
            vertexData[4].set(-overrideNumSegments, currStrokeRadius);
            vertexData += 5;
            fVertexCount += 5;
        }
    }
    SkASSERT(pathStrokesIter == fPathStrokes.end());

    SkASSERT(fVertexCount <= strokeGeometry.verbs().count() * 2 * 5);
    flushState->putBackVertices(strokeGeometry.verbs().count() * 2 * 5 - fVertexCount,
                                sizeof(SkPoint));
}

void GrTessellateStrokeOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fVertexBuffer) {
        return;
    }

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        SkASSERT(flushState->proxy()->numSamples() > 1);  // No mixed samples yet.
        SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples yet.
    }
    initArgs.fCaps = &flushState->caps();
    initArgs.fDstProxyView = flushState->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = flushState->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    SkASSERT(fViewMatrix.isIdentity());  // Only identity matrices supported for now.
    GrTessellateStrokeShader strokeShader(fViewMatrix, fColor, fMiterLimitOrZero);
    GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline, &strokeShader);

    flushState->bindPipelineAndScissorClip(programInfo, this->bounds() /*chainBounds??*/);
    flushState->bindTextures(strokeShader, nullptr, pipeline);

    flushState->bindBuffers(nullptr, nullptr, fVertexBuffer.get());
    flushState->draw(fVertexCount, fBaseVertex);
}
