/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellateStrokeOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStrokeGeometry.h"
#include "src/gpu/tessellate/GrStrokePathShader.h"

using OpFlags = GrTessellationPathRenderer::OpFlags;

GrDrawOp::FixedFunctionFlags GrTessellateStrokeOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kNone;
    if (fNeedsStencil) {
        flags |= FixedFunctionFlags::kUsesStencil;
    }
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

void GrTessellateStrokeOp::onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView* writeView,
                                        GrAppliedClip*, const GrXferProcessor::DstProxyView&) {
}

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static void line2cubic(const SkPoint& p0, const SkPoint& p1, SkPoint* out) {
    out[0] = p0;
    out[1] = lerp(p0, p1, 1/3.f);
    out[2] = lerp(p0, p1, 2/3.f);
    out[3] = p1;
}

static void quad2cubic(const SkPoint pts[], SkPoint* out) {
    out[0] = pts[0];
    out[1] = lerp(pts[0], pts[1], 2/3.f);
    out[2] = lerp(pts[1], pts[2], 1/3.f);
    out[3] = pts[2];
}

void GrTessellateStrokeOp::onPrepare(GrOpFlushState* flushState) {
    // Stencil is not yet implemented.
    SkASSERT(!fNeedsStencil);

    // Transform path to device space. This goes away once we update GrStrokeGeometry.
    SkASSERT(fViewMatrix.isIdentity());

    float strokeRadius = fStrokeRec.getWidth() * fMatrixScale * .5f;

    GrStrokeGeometry strokeGeometry(fPath.countPoints(), fPath.countVerbs());
    GrStrokeGeometry::InstanceTallies tallies = GrStrokeGeometry::InstanceTallies();
    strokeGeometry.beginPath(fStrokeRec, strokeRadius * 2, &tallies);
    SkPathVerb previousVerb = SkPathVerb::kClose;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
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

    auto cubicData = static_cast<SkPoint*>(flushState->makeVertexSpace(
            sizeof(SkPoint), strokeGeometry.verbs().count() * 2 * 5, &fCubicsBuffer,
            &fBaseCubicVertex));
    if (!cubicData) {
        return;
    }

    using Verb = GrStrokeGeometry::Verb;

    int ptsIdx = 0;
    const SkTArray<SkPoint, true>& pts = strokeGeometry.points();
    auto pendingJoin = Verb::kEndContour;
    SkPoint fLastJoinControlPoint;
    for (auto verb : strokeGeometry.verbs()) {
        const SkPoint* cubicPts;
        SkPoint scratchPts[4];
        switch (verb) {
            case Verb::kBeginPath:
                continue;
            case Verb::kLinearStroke:
                line2cubic(pts[ptsIdx], pts[ptsIdx+1], scratchPts);
                cubicPts = scratchPts;
                ++ptsIdx;
                break;
            case Verb::kQuadraticStroke:
                quad2cubic(&pts[ptsIdx], scratchPts);
                cubicPts = scratchPts;
                ptsIdx += 2;
                break;
            case Verb::kCubicStroke:
                cubicPts = &pts[ptsIdx];
                ptsIdx += 3;
                break;

            case Verb::kRotate:
                scratchPts[0] = scratchPts[3] = pts[ptsIdx];
                scratchPts[1] = pts[ptsIdx+1];
                scratchPts[2] = scratchPts[3]*2 - scratchPts[1];
                cubicPts = scratchPts;
                ptsIdx += 2;
                break;

            case Verb::kRoundJoin:
            case Verb::kInternalRoundJoin:
            case Verb::kMiterJoin:
            case Verb::kBevelJoin:
            case Verb::kInternalBevelJoin:
                pendingJoin = verb;
                continue;

            case Verb::kSquareCap:
            case Verb::kRoundCap:
                continue;

            case Verb::kEndContour:
                if (pendingJoin != Verb::kEndContour) {
                    SkDebugf("@@@@@@@> MISSED a join on close!\n");
                }
                pendingJoin = Verb::kEndContour;
                ++ptsIdx;
                continue;
        }
        if (pendingJoin != Verb::kEndContour) {
            cubicData[0] = cubicPts[0];
            cubicData[1] = fLastJoinControlPoint;
            cubicData[2] = (cubicPts[1] == cubicPts[0]) ? cubicPts[2] : cubicPts[1];
            cubicData[3] = cubicPts[0];
            switch (pendingJoin) {
                case Verb::kBevelJoin:
                    cubicData[4].set(1, strokeRadius);
                    break;
                case Verb::kMiterJoin:
                    cubicData[4].set(2, strokeRadius);
                    break;
                case Verb::kRoundJoin:
                    cubicData[4].set(3, strokeRadius);
                    break;
                case Verb::kInternalRoundJoin:
                case Verb::kInternalBevelJoin:
                default:
                    cubicData[4].set(-3, strokeRadius);
                    break;
            }
            cubicData += 5;
            fCubicVertexCount += 5;
            pendingJoin = Verb::kEndContour;
        }
        fLastJoinControlPoint = (cubicPts[2] == cubicPts[3]) ? cubicPts[1] : cubicPts[2];
        if (verb != Verb::kRotate) {
            memcpy(cubicData, cubicPts, sizeof(SkPoint) * 4);
            cubicData[4].set(0, strokeRadius);
            cubicData += 5;
            fCubicVertexCount += 5;
        }
    }
    SkASSERT(fCubicVertexCount <= strokeGeometry.verbs().count() * 2 * 5);
    flushState->putBackVertices(strokeGeometry.verbs().count() * 2 * 5 - fCubicVertexCount,
                                sizeof(SkPoint));
}

#if 0
static void prepareindirectcubics() {
    GrDrawIndirectCommand* indirectData = flushState->makeDrawIndirectSpace(
            fIndirectDrawCount, &fIndirectDrawBuffer, &fIndirectDrawOffset);
    if (!indirectData) {
        SkASSERT(!fIndirectDrawBuffer);
        return;
    }

    SkPoint* cubicData;
    int baseInstance;
    cubicData = static_cast<SkPoint*>(flushState->makeVertexSpace(
            sizeof(SkPoint) * 4, totalInstanceCount, &fInstanceBuffer, &baseInstance));
    if (!cubicData) {
        return;
    }

    SkPoint* instanceLocations[kMaxResolveLevel + 1];
#ifdef SK_DEBUG
    SkPoint* endInstanceLocations[kMaxResolveLevel + 1];
    GrDrawIndirectCommand* endIndirectData = indirectData + fIndirectDrawCount;
    SkPoint* endCubicData = cubicData + totalInstanceCount * 4;
#endif
    for (int resolveLevel = 0; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        uint32_t instanceCountAtLevel = tallies.fStrokes[resolveLevel];
        if (!instanceCountAtLevel) {
            continue;
        }
        *indirectData++ = GrStrokePathShader::MakeDrawStrokesIndirectCmd(
                resolveLevel, instanceCountAtLevel, baseInstance);
        instanceLocations[resolveLevel] = cubicData;
        cubicData += instanceCountAtLevel * 4;
        SkDEBUGCODE(endInstanceLocations[resolveLevel] = cubicData;)
        baseInstance += instanceCountAtLevel;
    }
    SkASSERT(indirectData == endIndirectData);
    SkASSERT(cubicData == endCubicData);

    int ptsIdx = 0;
    int paramsIdx = 0;
    const SkTArray<GrStrokeGeometry::Parameter, true>& params = strokeGeometry.params();
    const SkTArray<SkPoint, true>& pts = strokeGeometry.points();
    for (auto verb : strokeGeometry.verbs()) {
        int resolveLevel;
        switch (verb) {
            using Verb = GrStrokeGeometry::Verb;
            case Verb::kBeginPath:
                continue;
            case Verb::kLinearStroke:
                resolveLevel = 0;
                line2cubic(pts[ptsIdx], pts[ptsIdx+1], instanceLocations[0]);
                ++ptsIdx;
                break;
            case Verb::kQuadraticStroke:
                resolveLevel = params[paramsIdx++].fNumLinearSegmentsLog2;
                SkASSERT(resolveLevel >= 0 && resolveLevel <= kMaxResolveLevel);
                quad2cubic(&pts[ptsIdx], instanceLocations[resolveLevel]);
                ptsIdx += 2;
                break;
            case Verb::kCubicStroke:
                resolveLevel = params[paramsIdx++].fNumLinearSegmentsLog2;
                SkASSERT(resolveLevel >= 0 && resolveLevel <= kMaxResolveLevel);
                memcpy(instanceLocations[resolveLevel], &pts[ptsIdx], sizeof(SkPoint) * 4);
                ptsIdx += 3;
                break;

            case Verb::kRoundJoin:
            case Verb::kInternalRoundJoin:
                ++paramsIdx;
                // fallthru
            case Verb::kMiterJoin:
                ++paramsIdx;
                continue;

            case Verb::kBevelJoin:
            case Verb::kInternalBevelJoin:
            case Verb::kSquareCap:
            case Verb::kRoundCap:
                continue;

            case Verb::kEndContour:
                ++ptsIdx;
                continue;
        }
        instanceLocations[resolveLevel] += 4;
    }
#ifdef SK_DEBUG
    for (int resolveLevel = 0; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        if (tallies.fStrokes[resolveLevel]) {
            SkASSERT(instanceLocations[resolveLevel] == endInstanceLocations[resolveLevel]);
        }
    }
#endif

    fVertexBuffer = GrStrokePathShader::FindOrMakeVertexBuffer(flushState->resourceProvider());
}
#endif

void GrTessellateStrokeOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fCubicsBuffer) {
        return;
    }

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        SkASSERT(flushState->proxy()->numSamples() > 1);  // No mixed samples yet.
        SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples yet.
    }
    // initArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    initArgs.fCaps = &flushState->caps();
    initArgs.fDstProxyView = flushState->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = flushState->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    // GrStrokePathShader strokePathShader(fViewMatrix, fColor);
    GrTessellateCubicStrokeShader strokeShader(fViewMatrix, fStrokeRec.getMiter(), fColor);
    GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline, &strokeShader);

    flushState->bindPipelineAndScissorClip(programInfo, this->bounds() /*chainBounds??*/);
    flushState->bindTextures(strokeShader, nullptr, pipeline);
    // flushState->bindBuffers(nullptr, fInstanceBuffer.get(), fVertexBuffer.get());
    // flushState->drawIndirect(fIndirectDrawBuffer.get(), fIndirectDrawOffset, fIndirectDrawCount);

    flushState->bindBuffers(nullptr, nullptr, fCubicsBuffer.get());
    flushState->draw(fCubicVertexCount, fBaseCubicVertex);
}
