/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathTessellateOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTriangulator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrMidpointContourParser.h"
#include "src/gpu/tessellate/GrResolveLevelCounter.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

constexpr static float kLinearizationIntolerance =
        GrTessellationPathRenderer::kLinearizationIntolerance;

constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

using OpFlags = GrTessellationPathRenderer::OpFlags;

void GrPathTessellateOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fPipelineForFills) {
        fPipelineForFills->visitProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrPathTessellateOp::FixedFunctionFlags GrPathTessellateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

namespace {

class CpuTriangleAllocator : public GrEagerVertexAllocator {
public:
    CpuTriangleAllocator(SkArenaAlloc* arena, const SkPoint** data) : fArena(arena), fData(data) {}

    void* lock(size_t stride, int eagerCount) override {
        SkASSERT(!*fData);
        SkASSERT(stride == sizeof(SkPoint));
        SkPoint* data = fArena->makeArray<SkPoint>(eagerCount);
        *fData = data;
        return data;
    }

    void unlock(int actualCount) override { SkASSERT(*fData); }

private:
    SkArenaAlloc* const fArena;
    const SkPoint** fData;
};

}

void GrPathTessellateOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrXferProcessor::DstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    SkArenaAlloc* recordTimeAllocator = context->priv().recordTimeAllocator();
    GrAppliedHardClip hardClip = GrAppliedHardClip(
            (clip) ? clip->hardClip() : GrAppliedHardClip::Disabled());
    CpuTriangleAllocator cpuTriangleAllocator(recordTimeAllocator, &fOffThreadInnerTriangulation);
    PrePrepareArgs args{recordTimeAllocator, writeView, &hardClip, clip, &dstProxyView,
                        renderPassXferBarriers, colorLoadOp, context->priv().caps(),
                        &cpuTriangleAllocator};

    this->prePreparePrograms(args);

    if (fStencilTrianglesProgram) {
        context->priv().recordProgramInfo(fStencilTrianglesProgram);
    }
    if (fStencilCubicsProgram) {
        context->priv().recordProgramInfo(fStencilCubicsProgram);
    }
    if (fFillTrianglesProgram) {
        context->priv().recordProgramInfo(fFillTrianglesProgram);
    }
    if (fFillPathProgram) {
        context->priv().recordProgramInfo(fFillPathProgram);
    }
}

void GrPathTessellateOp::prePreparePrograms(const PrePrepareArgs& args) {
    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }

    // First check if the path is large and/or simple enough that we can actually triangulate the
    // inner polygon(s) on the CPU. This is our fastest approach. It allows us to stencil only the
    // curves, and then fill the internal polygons directly to the final render target, thus drawing
    // the majority of pixels in a single render pass.
    SkScalar scales[2];
    SkAssertResult(fViewMatrix.getMinMaxScales(scales));  // Will fail if perspective.
    const SkRect& bounds = fPath.getBounds();
    float gpuFragmentWork = bounds.height() * scales[0] * bounds.width() * scales[1];
    float cpuTessellationWork = (float)numVerbs * SkNextLog2(numVerbs);  // N log N.
    if (cpuTessellationWork * 500 + (256 * 256) < gpuFragmentWork) {  // Don't try below 256x256.
        bool isLinear;
        // This will fail if the inner triangles do not form a simple polygon (e.g., self
        // intersection, double winding).
        if (this->prePrepareInnerPolygonTriangulation(args, &isLinear)) {
            if (!isLinear) {
                // Always use indirect draws for cubics instead of tessellation here. Our goal in
                // this mode is to maximize GPU performance, and the middle-out topology used by our
                // indirect draws is easier on the rasterizer than a tessellated fan. There also
                // seems to be a small amount of fixed tessellation overhead that this avoids.
                this->prePrepareStencilCubicsProgram<GrMiddleOutCubicShader>(args);
                // We will need one final pass to cover the convex hulls of the cubics after
                // drawing the inner triangles.
                this->prePrepareFillCubicHullsProgram(args);
            }
            return;
        }
    }

    // If we didn't triangulate the inner fan then the fill program will be a simple bounding box.
    this->prePrepareFillBoundingBoxProgram(args);

    // When there are only a few verbs, it seems to always be fastest to make a single indirect draw
    // that contains both the inner triangles and the outer cubics, instead of using hardware
    // tessellation. Also take this path if tessellation is not supported.
    bool drawTrianglesAsIndirectCubicDraw = (numVerbs < 50);
    if (drawTrianglesAsIndirectCubicDraw || (fOpFlags & OpFlags::kDisableHWTessellation)) {
        if (!drawTrianglesAsIndirectCubicDraw) {
            this->prePrepareStencilTrianglesProgram(args);
        }
        this->prePrepareStencilCubicsProgram<GrMiddleOutCubicShader>(args);
        return;
    }

    // The caller should have sent Flags::kDisableHWTessellation if it was not supported.
    SkASSERT(args.fCaps->shaderCaps()->tessellationSupport());

    // Next see if we can split up the inner triangles and outer cubics into two draw calls. This
    // allows for a more efficient inner triangle topology that can reduce the rasterizer load by a
    // large margin on complex paths, but also causes greater CPU overhead due to the extra shader
    // switches and draw calls.
    // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of multiplying.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
    if (rasterEdgeWork > 300 * 300) {
        this->prePrepareStencilTrianglesProgram(args);
        this->prePrepareStencilCubicsProgram<GrCubicTessellateShader>(args);
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    this->prePrepareStencilCubicsProgram<GrWedgeTessellateShader>(args);
}

bool GrPathTessellateOp::prePrepareInnerPolygonTriangulation(const PrePrepareArgs& args,
                                                             bool* isLinear) {
    SkASSERT(!fTriangleBuffer);
    SkASSERT(fTriangleVertexCount == 0);
    SkASSERT(!fStencilTrianglesProgram);
    SkASSERT(!fFillTrianglesProgram);

    using GrTriangulator::Mode;

    fTriangleVertexCount = GrTriangulator::PathToTriangles(fPath, 0, SkRect::MakeEmpty(),
                                                           args.fInnerTriangleAllocator,
                                                           Mode::kSimpleInnerPolygons,
                                                           isLinear);
    if (fTriangleVertexCount == 0) {
        // Mode::kSimpleInnerPolygons causes PathToTriangles to fail if the inner polygon(s) are not
        // simple.
        return false;
    }
    if ((fOpFlags & (OpFlags::kStencilOnly | OpFlags::kWireframe)) ||
        GrAAType::kCoverage == fAAType ||
        (args.fClip && args.fClip->hasStencilClip())) {
        // If we have certain flags, mixed samples, or a stencil clip then we unfortunately
        // can't fill the inner polygon directly. Indicate that these triangles need to be
        // stencilled.
        this->prePrepareStencilTrianglesProgram(args);
    }
    this->prePrepareFillTrianglesProgram(args, *isLinear);
    return true;
}

// Increments clockwise triangles and decrements counterclockwise. Used for "winding" fill.
constexpr static GrUserStencilSettings kIncrDecrStencil(
    GrUserStencilSettings::StaticInitSeparate<
        0x0000,                                0x0000,
        GrUserStencilTest::kAlwaysIfInClip,    GrUserStencilTest::kAlwaysIfInClip,
        0xffff,                                0xffff,
        GrUserStencilOp::kIncWrap,             GrUserStencilOp::kDecWrap,
        GrUserStencilOp::kKeep,                GrUserStencilOp::kKeep,
        0xffff,                                0xffff>());

// Inverts the bottom stencil bit. Used for "even/odd" fill.
constexpr static GrUserStencilSettings kInvertStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kInvert,
        GrUserStencilOp::kKeep,
        0x0001>());

constexpr static const GrUserStencilSettings* stencil_pass_settings(SkPathFillType fillType) {
    return (fillType == SkPathFillType::kWinding) ? &kIncrDecrStencil : &kInvertStencil;
}

void GrPathTessellateOp::prePrepareStencilTrianglesProgram(const PrePrepareArgs& args) {
    SkASSERT(!fStencilTrianglesProgram);

    this->prePreparePipelineForStencils(args);

    auto* shader = args.fArena->make<GrStencilTriangleShader>(fViewMatrix);
    fStencilTrianglesProgram = GrPathShader::MakeProgramInfo(
            shader, args.fArena, args.fWriteView, fPipelineForStencils, *args.fDstProxyView,
            args.fXferBarrierFlags, args.fColorLoadOp, stencil_pass_settings(fPath.getFillType()),
            *args.fCaps);
}

template<typename ShaderType>
void GrPathTessellateOp::prePrepareStencilCubicsProgram(const PrePrepareArgs& args) {
    SkASSERT(!fStencilCubicsProgram);

    this->prePreparePipelineForStencils(args);

    auto* shader = args.fArena->make<ShaderType>(fViewMatrix);
    fStencilCubicsProgram = GrPathShader::MakeProgramInfo(
            shader, args.fArena, args.fWriteView, fPipelineForStencils, *args.fDstProxyView,
            args.fXferBarrierFlags, args.fColorLoadOp, stencil_pass_settings(fPath.getFillType()),
            *args.fCaps);
}

void GrPathTessellateOp::prePreparePipelineForStencils(const PrePrepareArgs& args) {
    if (fPipelineForStencils) {
        return;
    }

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    if (args.fCaps->wireframeSupport() && (OpFlags::kWireframe & fOpFlags)) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    }
    SkASSERT(SkPathFillType::kWinding == fPath.getFillType() ||
             SkPathFillType::kEvenOdd == fPath.getFillType());
    initArgs.fCaps = args.fCaps;
    fPipelineForStencils = args.fArena->make<GrPipeline>(
            initArgs, GrDisableColorXPFactory::MakeXferProcessor(), *args.fHardClip);
}

// Allows non-zero stencil values to pass and write a color, and resets the stencil value back to
// zero; discards immediately on stencil values of zero.
// NOTE: It's ok to not check the clip here because the previous stencil pass will have only written
// to samples already inside the clip.
constexpr static GrUserStencilSettings kTestAndResetStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,
        0xffff>());

void GrPathTessellateOp::prePrepareFillTrianglesProgram(const PrePrepareArgs& args, bool isLinear) {
    SkASSERT(!fFillTrianglesProgram);

    if (fOpFlags & OpFlags::kStencilOnly) {
        return;
    }

    // These are a twist on the standard red book stencil settings that allow us to fill the inner
    // polygon directly to the final render target. At this point, the curves are already stencilled
    // in. So if the stencil value is zero, then it means the path at our sample is not affected by
    // any curves and we fill the path in directly. If the stencil value is nonzero, then we don't
    // fill and instead continue the standard red book stencil process.
    //
    // NOTE: These settings are currently incompatible with a stencil clip.
    constexpr static GrUserStencilSettings kFillOrIncrDecrStencil(
        GrUserStencilSettings::StaticInitSeparate<
            0x0000,                        0x0000,
            GrUserStencilTest::kEqual,     GrUserStencilTest::kEqual,
            0xffff,                        0xffff,
            GrUserStencilOp::kKeep,        GrUserStencilOp::kKeep,
            GrUserStencilOp::kIncWrap,     GrUserStencilOp::kDecWrap,
            0xffff,                        0xffff>());

    constexpr static GrUserStencilSettings kFillOrInvertStencil(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kEqual,
            0xffff,
            GrUserStencilOp::kKeep,
            GrUserStencilOp::kZero,
            0xffff>());

    this->prePreparePipelineForFills(args);

    const GrUserStencilSettings* stencil;
    if (fStencilTrianglesProgram) {
        // The path was already stencilled. Here we just need to do a cover pass.
        stencil = &kTestAndResetStencil;
    } else if (isLinear) {
        // There are no stencilled curves. We can ignore stencil and fill the path directly.
        stencil = &GrUserStencilSettings::kUnused;
    } else if (SkPathFillType::kWinding == fPath.getFillType()) {
        // Fill in the path pixels not touched by curves, incr/decr stencil otherwise.
        SkASSERT(!fPipelineForFills->hasStencilClip());
        stencil = &kFillOrIncrDecrStencil;
    } else {
        // Fill in the path pixels not touched by curves, invert stencil otherwise.
        SkASSERT(!fPipelineForFills->hasStencilClip());
        stencil = &kFillOrInvertStencil;
    }

    auto* fillTriangleShader = args.fArena->make<GrFillTriangleShader>(fViewMatrix, fColor);
    fFillTrianglesProgram = GrPathShader::MakeProgramInfo(
            fillTriangleShader, args.fArena, args.fWriteView, fPipelineForFills,
            *args.fDstProxyView, args.fXferBarrierFlags, args.fColorLoadOp, stencil, *args.fCaps);
}

void GrPathTessellateOp::prePrepareFillCubicHullsProgram(const PrePrepareArgs& args) {
    SkASSERT(!fFillPathProgram);

    if (fOpFlags & OpFlags::kStencilOnly) {
        return;
    }

    this->prePreparePipelineForFills(args);

    auto* fillCubicHullsShader = args.fArena->make<GrFillCubicHullShader>(fViewMatrix, fColor);
    fFillPathProgram = GrPathShader::MakeProgramInfo(
            fillCubicHullsShader, args.fArena, args.fWriteView, fPipelineForFills,
            *args.fDstProxyView, args.fXferBarrierFlags, args.fColorLoadOp, &kTestAndResetStencil,
            *args.fCaps);
}

void GrPathTessellateOp::prePrepareFillBoundingBoxProgram(const PrePrepareArgs& args) {
    SkASSERT(!fFillPathProgram);

    if (fOpFlags & OpFlags::kStencilOnly) {
        return;
    }

    this->prePreparePipelineForFills(args);

    auto* fillBoundingBoxShader = args.fArena->make<GrFillBoundingBoxShader>(fViewMatrix, fColor,
                                                                             fPath.getBounds());
    fFillPathProgram = GrPathShader::MakeProgramInfo(
            fillBoundingBoxShader, args.fArena, args.fWriteView, fPipelineForFills,
            *args.fDstProxyView, args.fXferBarrierFlags, args.fColorLoadOp, &kTestAndResetStencil,
            *args.fCaps);
}

void GrPathTessellateOp::prePreparePipelineForFills(const PrePrepareArgs& args) {
    SkASSERT(!(fOpFlags & OpFlags::kStencilOnly));

    if (fPipelineForFills) {
        return;
    }

    auto pipelineFlags = GrPipeline::InputFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        if (args.fWriteView.asRenderTargetProxy()->numSamples() == 1) {
            // We are mixed sampled. We need to either enable conservative raster (preferred) or
            // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
            // the cover geometry, the stencil test is still multisampled and will still produce
            // smooth results.)
            SkASSERT(GrAAType::kCoverage == fAAType);
            if (args.fCaps->conservativeRasterSupport()) {
                pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
                pipelineFlags |= GrPipeline::InputFlags::kConservativeRaster;
            }
        } else {
            // We are standard MSAA. Leave MSAA enabled for the cover geometry.
            pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
    }

    fPipelineForFills = GrSimpleMeshDrawOpHelper::CreatePipeline(
            args.fCaps, args.fArena, args.fWriteView.swizzle(),
            (args.fClip) ? std::move(*args.fClip) : GrAppliedClip::Disabled(), *args.fDstProxyView,
            std::move(fProcessors), pipelineFlags);
}

void GrPathTessellateOp::onPrepare(GrOpFlushState* flushState) {
    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }

    if (!fPipelineForStencils && !fPipelineForFills) {
        // Nothing has been prePrepared yet. Do it now.
        GrEagerDynamicVertexAllocator innerTriangleAllocator(flushState, &fTriangleBuffer,
                                                             &fBaseTriangleVertex);
        GrAppliedHardClip hardClip = GrAppliedHardClip(flushState->appliedHardClip());
        GrAppliedClip clip = flushState->detachAppliedClip();
        PrePrepareArgs args{flushState->allocator(), flushState->writeView(), &hardClip,
                            &clip, &flushState->dstProxyView(),
                            flushState->renderPassBarriers(), flushState->colorLoadOp(),
                            &flushState->caps(), &innerTriangleAllocator};
        this->prePreparePrograms(args);
    }

    if (fTriangleVertexCount != 0) {
        // prePreparePrograms was able to generate an inner polygon triangulation. It will exist in
        // either fOffThreadInnerTriangulation or fTriangleBuffer exclusively.
        SkASSERT(SkToBool(fOffThreadInnerTriangulation) != SkToBool(fTriangleBuffer));
        if (fOffThreadInnerTriangulation) {
            // DDL generated the triangle buffer data off thread. Copy it to GPU.
            void* data = flushState->makeVertexSpace(sizeof(SkPoint), fTriangleVertexCount,
                                                     &fTriangleBuffer, &fBaseTriangleVertex);
            memcpy(data, fOffThreadInnerTriangulation, fTriangleVertexCount * sizeof(SkPoint));
        }
        if (fStencilCubicsProgram) {
            // We always use indirect draws for inner-polygon-triangulation mode instead of
            // tessellation.
            SkASSERT(GrPrimitiveType::kPatches !=
                     fStencilCubicsProgram->primProc().cast<GrStencilPathShader>().primitiveType());
            GrResolveLevelCounter resolveLevelCounter;
            resolveLevelCounter.reset(fPath, fViewMatrix, kLinearizationIntolerance);
            this->prepareIndirectOuterCubics(flushState, resolveLevelCounter);
        }
        return;
    }

    SkASSERT(fStencilCubicsProgram);
    const auto& stencilCubicsShader = fStencilCubicsProgram->primProc().cast<GrPathShader>();

    if (stencilCubicsShader.primitiveType() != GrPrimitiveType::kPatches) {
        // Outer cubics need indirect draws.
        GrResolveLevelCounter resolveLevelCounter;
        this->prepareMiddleOutTrianglesAndCubics(flushState, &resolveLevelCounter);
        return;
    }

    if (stencilCubicsShader.tessellationPatchVertexCount() == 4) {
        // Triangles and tessellated curves will be drawn separately.
        this->prepareMiddleOutTrianglesAndCubics(flushState);
        return;
    }

    // We are drawing tessellated wedges.
    SkASSERT(stencilCubicsShader.tessellationPatchVertexCount() == 5);
    this->prepareTessellatedCubicWedges(flushState);
}

void GrPathTessellateOp::prepareMiddleOutTrianglesAndCubics(
        GrMeshDrawOp::Target* target, GrResolveLevelCounter* resolveLevelCounter) {
    SkASSERT(fStencilCubicsProgram);
    SkASSERT(!fTriangleBuffer);
    SkASSERT(!fFillTrianglesProgram);
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fIndirectDrawBuffer);
    SkASSERT(fTriangleVertexCount == 0);
    SkASSERT(fCubicVertexCount == 0);

    // No initial moveTo, plus an implicit close at the end; n-2 triangles fill an n-gon.
    int maxInnerTriangles = fPath.countVerbs() - 1;
    int maxCubics = fPath.countVerbs();

    SkPoint* vertexData;
    int vertexAdvancePerTriangle;
    if (!fStencilTrianglesProgram) {
        // Allocate the triangles as 4-point instances at the beginning of the cubic buffer.
        SkASSERT(resolveLevelCounter);
        vertexAdvancePerTriangle = 4;
        int baseTriangleInstance;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint) * 4, maxInnerTriangles + maxCubics, &fCubicBuffer,
                &baseTriangleInstance));
        fBaseCubicVertex = baseTriangleInstance * 4;
    } else {
        // Allocate the triangles as normal 3-point instances in the triangle buffer.
        vertexAdvancePerTriangle = 3;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint), maxInnerTriangles * 3, &fTriangleBuffer, &fBaseTriangleVertex));
    }
    if (!vertexData) {
        return;
    }

    GrVectorXform xform(fViewMatrix);
    GrMiddleOutPolygonTriangulator middleOut(vertexData, vertexAdvancePerTriangle,
                                             fPath.countVerbs());
    if (resolveLevelCounter) {
        resolveLevelCounter->reset();
    }
    int numCountedCurves = 0;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            case SkPathVerb::kMove:
                middleOut.closeAndMove(pts[0]);
                break;
            case SkPathVerb::kLine:
                middleOut.pushVertex(pts[1]);
                break;
            case SkPathVerb::kConic:
                // We use the same quadratic formula for conics, ignoring w. This appears to be an
                // upper bound on what the actual number of subdivisions would have been.
                [[fallthrough]];
            case SkPathVerb::kQuad:
                middleOut.pushVertex(pts[2]);
                if (resolveLevelCounter) {
                    resolveLevelCounter->countInstance(GrWangsFormula::quadratic_log2(
                            kLinearizationIntolerance, pts, xform));
                    break;
                }
                ++numCountedCurves;
                break;
            case SkPathVerb::kCubic:
                middleOut.pushVertex(pts[3]);
                if (resolveLevelCounter) {
                    resolveLevelCounter->countInstance(GrWangsFormula::cubic_log2(
                            kLinearizationIntolerance, pts, xform));
                    break;
                }
                ++numCountedCurves;
                break;
            case SkPathVerb::kClose:
                middleOut.close();
                break;
        }
    }
    int triangleCount = middleOut.close();
    SkASSERT(triangleCount <= maxInnerTriangles);

    if (!fStencilTrianglesProgram) {
        SkASSERT(resolveLevelCounter);
        int totalInstanceCount = triangleCount + resolveLevelCounter->totalInstanceCount();
        SkASSERT(vertexAdvancePerTriangle == 4);
        target->putBackVertices(maxInnerTriangles + maxCubics - totalInstanceCount,
                                sizeof(SkPoint) * 4);
        if (totalInstanceCount) {
            this->prepareIndirectOuterCubicsAndTriangles(target, *resolveLevelCounter, vertexData,
                                                         triangleCount);
        }
    } else {
        SkASSERT(vertexAdvancePerTriangle == 3);
        target->putBackVertices(maxInnerTriangles - triangleCount, sizeof(SkPoint) * 3);
        fTriangleVertexCount = triangleCount * 3;
        if (resolveLevelCounter) {
            this->prepareIndirectOuterCubics(target, *resolveLevelCounter);
        } else {
            this->prepareTessellatedOuterCubics(target, numCountedCurves);
        }
    }
}

void GrPathTessellateOp::prepareIndirectOuterCubics(
        GrMeshDrawOp::Target* target, const GrResolveLevelCounter& resolveLevelCounter) {
    SkASSERT(resolveLevelCounter.totalInstanceCount() >= 0);
    if (resolveLevelCounter.totalInstanceCount() == 0) {
        return;
    }
    // Allocate a buffer to store the cubic data.
    SkPoint* cubicData;
    int baseInstance;
    cubicData = static_cast<SkPoint*>(target->makeVertexSpace(
            sizeof(SkPoint) * 4, resolveLevelCounter.totalInstanceCount(), &fCubicBuffer,
            &baseInstance));
    if (!cubicData) {
        return;
    }
    fBaseCubicVertex = baseInstance * 4;
    this->prepareIndirectOuterCubicsAndTriangles(target, resolveLevelCounter, cubicData,
                                                 /*numTrianglesAtBeginningOfData=*/0);
}

void GrPathTessellateOp::prepareIndirectOuterCubicsAndTriangles(
        GrMeshDrawOp::Target* target, const GrResolveLevelCounter& resolveLevelCounter,
        SkPoint* cubicData, int numTrianglesAtBeginningOfData) {
    SkASSERT(target->caps().drawInstancedSupport());
    SkASSERT(numTrianglesAtBeginningOfData + resolveLevelCounter.totalInstanceCount() > 0);
    SkASSERT(fStencilCubicsProgram);
    SkASSERT(cubicData);
    SkASSERT(fCubicVertexCount == 0);

    fIndirectIndexBuffer = GrMiddleOutCubicShader::FindOrMakeMiddleOutIndexBuffer(
            target->resourceProvider());
    if (!fIndirectIndexBuffer) {
        return;
    }

    // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with the base
    // vertex on an instance boundary in order to accommodate this.
    SkASSERT(fBaseCubicVertex % 4 == 0);
    int baseInstance = fBaseCubicVertex >> 2;

    // Start preparing the indirect draw buffer.
    fIndirectDrawCount = resolveLevelCounter.totalIndirectDrawCount();
    if (numTrianglesAtBeginningOfData) {
        ++fIndirectDrawCount;  // Add an indirect draw for the triangles at the beginning.
    }

    // Allocate space for the GrDrawIndexedIndirectCommand structs.
    GrDrawIndexedIndirectCommand* indirectData = target->makeDrawIndexedIndirectSpace(
            fIndirectDrawCount, &fIndirectDrawBuffer, &fIndirectDrawOffset);
    if (!indirectData) {
        SkASSERT(!fIndirectDrawBuffer);
        return;
    }

    // Fill out the GrDrawIndexedIndirectCommand structs and determine the starting instance data
    // location at each resolve level.
    SkPoint* instanceLocations[kMaxResolveLevel + 1];
    int indirectIdx = 0;
    int runningInstanceCount = 0;
    if (numTrianglesAtBeginningOfData) {
        // The caller has already packed "triangleInstanceCount" triangles into 4-point instances
        // at the beginning of the instance buffer. Add a special-case indirect draw here that will
        // emit the triangles [P0, P1, P2] from these 4-point instances.
        indirectData[0] = GrMiddleOutCubicShader::MakeDrawTrianglesIndirectCmd(
                numTrianglesAtBeginningOfData, baseInstance);
        indirectIdx = 1;
        runningInstanceCount = numTrianglesAtBeginningOfData;
    }
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        int instanceCountAtCurrLevel = resolveLevelCounter[resolveLevel];
        if (!instanceCountAtCurrLevel) {
            SkDEBUGCODE(instanceLocations[resolveLevel] = nullptr;)
            continue;
        }
        instanceLocations[resolveLevel] = cubicData + runningInstanceCount * 4;
        indirectData[indirectIdx++] = GrMiddleOutCubicShader::MakeDrawCubicsIndirectCmd(
                resolveLevel, instanceCountAtCurrLevel, baseInstance + runningInstanceCount);
        runningInstanceCount += instanceCountAtCurrLevel;
    }

#ifdef SK_DEBUG
    SkASSERT(indirectIdx == fIndirectDrawCount);
    SkASSERT(runningInstanceCount == numTrianglesAtBeginningOfData +
                                     resolveLevelCounter.totalInstanceCount());
    SkASSERT(fIndirectDrawCount > 0);

    SkPoint* endLocations[kMaxResolveLevel + 1];
    int lastResolveLevel = 0;
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        if (!instanceLocations[resolveLevel]) {
            endLocations[resolveLevel] = nullptr;
            continue;
        }
        endLocations[lastResolveLevel] = instanceLocations[resolveLevel];
        lastResolveLevel = resolveLevel;
    }
    int totalInstanceCount = numTrianglesAtBeginningOfData +
                             resolveLevelCounter.totalInstanceCount();
    endLocations[lastResolveLevel] = cubicData + totalInstanceCount * 4;
#endif

    fCubicVertexCount = numTrianglesAtBeginningOfData * 4;

    if (resolveLevelCounter.totalInstanceCount()) {
        GrVectorXform xform(fViewMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
            int level;
            switch (verb) {
                default:
                    continue;
                case SkPathVerb::kConic:
                    // We use the same quadratic formula for conics, ignoring w. This appears to be
                    // an upper bound on what the actual number of subdivisions would have been.
                    [[fallthrough]];
                case SkPathVerb::kQuad:
                    level = GrWangsFormula::quadratic_log2(kLinearizationIntolerance, pts, xform);
                    break;
                case SkPathVerb::kCubic:
                    level = GrWangsFormula::cubic_log2(kLinearizationIntolerance, pts, xform);
                    break;
            }
            if (level == 0) {
                continue;
            }
            level = std::min(level, kMaxResolveLevel);
            switch (verb) {
                case SkPathVerb::kQuad:
                    GrPathUtils::convertQuadToCubic(pts, instanceLocations[level]);
                    break;
                case SkPathVerb::kCubic:
                    memcpy(instanceLocations[level], pts, sizeof(SkPoint) * 4);
                    break;
                case SkPathVerb::kConic:
                    GrPathShader::WriteConicPatch(pts, *w, instanceLocations[level]);
                    break;
                default:
                    SkUNREACHABLE;
            }
            instanceLocations[level] += 4;
            fCubicVertexCount += 4;
        }
    }

#ifdef SK_DEBUG
    for (int i = 1; i <= kMaxResolveLevel; ++i) {
        SkASSERT(instanceLocations[i] == endLocations[i]);
    }
    SkASSERT(fCubicVertexCount == (numTrianglesAtBeginningOfData +
                                   resolveLevelCounter.totalInstanceCount()) * 4);
#endif
}

void GrPathTessellateOp::prepareTessellatedOuterCubics(GrMeshDrawOp::Target* target,
                                                       int numCountedCurves) {
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());
    SkASSERT(numCountedCurves >= 0);
    SkASSERT(!fCubicBuffer);
    SkASSERT(fStencilCubicsProgram);
    SkASSERT(fCubicVertexCount == 0);

    if (numCountedCurves == 0) {
        return;
    }

    auto* vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
            sizeof(SkPoint), numCountedCurves * 4, &fCubicBuffer, &fBaseCubicVertex));
    if (!vertexData) {
        return;
    }

    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            default:
                continue;
            case SkPathVerb::kQuad:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                GrPathUtils::convertQuadToCubic(pts, vertexData + fCubicVertexCount);
                break;
            case SkPathVerb::kCubic:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                memcpy(vertexData + fCubicVertexCount, pts, sizeof(SkPoint) * 4);
                break;
            case SkPathVerb::kConic:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                GrPathShader::WriteConicPatch(pts, *w, vertexData + fCubicVertexCount);
                break;
        }
        fCubicVertexCount += 4;
    }
    SkASSERT(fCubicVertexCount == numCountedCurves * 4);
}

void GrPathTessellateOp::prepareTessellatedCubicWedges(GrMeshDrawOp::Target* target) {
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());
    SkASSERT(!fCubicBuffer);
    SkASSERT(fStencilCubicsProgram);
    SkASSERT(fCubicVertexCount == 0);

    // No initial moveTo, one wedge per verb, plus an implicit close at the end.
    // Each wedge has 5 vertices.
    int maxVertices = (fPath.countVerbs() + 1) * 5;

    GrEagerDynamicVertexAllocator vertexAlloc(target, &fCubicBuffer, &fBaseCubicVertex);
    auto* vertexData = vertexAlloc.lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return;
    }

    GrMidpointContourParser parser(fPath);
    while (parser.parseNextContour()) {
        SkPoint midpoint = parser.currentMidpoint();
        SkPoint startPoint = {0, 0};
        SkPoint lastPoint = startPoint;
        for (auto [verb, pts, w] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove:
                    startPoint = lastPoint = pts[0];
                    continue;
                case SkPathVerb::kClose:
                    continue;  // Ignore. We can assume an implicit close at the end.
                case SkPathVerb::kLine:
                    GrPathUtils::convertLineToCubic(pts[0], pts[1], vertexData + fCubicVertexCount);
                    lastPoint = pts[1];
                    break;
                case SkPathVerb::kQuad:
                    GrPathUtils::convertQuadToCubic(pts, vertexData + fCubicVertexCount);
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kCubic:
                    memcpy(vertexData + fCubicVertexCount, pts, sizeof(SkPoint) * 4);
                    lastPoint = pts[3];
                    break;
                case SkPathVerb::kConic:
                    GrPathShader::WriteConicPatch(pts, *w, vertexData + fCubicVertexCount);
                    lastPoint = pts[2];
                    break;
            }
            vertexData[fCubicVertexCount + 4] = midpoint;
            fCubicVertexCount += 5;
        }
        if (lastPoint != startPoint) {
            GrPathUtils::convertLineToCubic(lastPoint, startPoint, vertexData + fCubicVertexCount);
            vertexData[fCubicVertexCount + 4] = midpoint;
            fCubicVertexCount += 5;
        }
    }

    vertexAlloc.unlock(fCubicVertexCount);
}

void GrPathTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    this->drawStencilPass(flushState);
    this->drawCoverPass(flushState);
}

void GrPathTessellateOp::drawStencilPass(GrOpFlushState* flushState) {
    if (fStencilTrianglesProgram && fTriangleVertexCount > 0) {
        SkASSERT(fTriangleBuffer);
        flushState->bindPipelineAndScissorClip(*fStencilTrianglesProgram, this->bounds());
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer);
        flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);
    }

    if (fCubicVertexCount > 0) {
        SkASSERT(fStencilCubicsProgram);
        SkASSERT(fCubicBuffer);
        flushState->bindPipelineAndScissorClip(*fStencilCubicsProgram, this->bounds());
        if (fIndirectDrawBuffer) {
            SkASSERT(fIndirectIndexBuffer);
            flushState->bindBuffers(fIndirectIndexBuffer, fCubicBuffer, nullptr);
            flushState->drawIndexedIndirect(fIndirectDrawBuffer.get(), fIndirectDrawOffset,
                                            fIndirectDrawCount);
        } else {
            flushState->bindBuffers(nullptr, nullptr, fCubicBuffer);
            flushState->draw(fCubicVertexCount, fBaseCubicVertex);
            if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
                flushState->gpu()->insertManualFramebufferBarrier();  // http://skbug.com/9739
            }
        }
    }
}

void GrPathTessellateOp::drawCoverPass(GrOpFlushState* flushState) {
    if (fFillTrianglesProgram) {
        SkASSERT(fTriangleBuffer);
        SkASSERT(fTriangleVertexCount > 0);

        // We have a triangulation of the path's inner polygon. This is the fast path. Fill those
        // triangles directly to the screen.
        flushState->bindPipelineAndScissorClip(*fFillTrianglesProgram, this->bounds());
        flushState->bindTextures(fFillTrianglesProgram->primProc(), nullptr, *fPipelineForFills);
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer);
        flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);

        if (fCubicVertexCount > 0) {
            SkASSERT(fFillPathProgram);
            SkASSERT(fCubicBuffer);

            // At this point, every pixel is filled in except the ones touched by curves.
            // fFillPathProgram will issue a final cover pass over the curves by drawing their
            // convex hulls. This will fill in any remaining samples and reset the stencil buffer.
            flushState->bindPipelineAndScissorClip(*fFillPathProgram, this->bounds());
            flushState->bindTextures(fFillPathProgram->primProc(), nullptr, *fPipelineForFills);

            // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with
            // the base vertex on an instance boundary in order to accommodate this.
            SkASSERT((fCubicVertexCount % 4) == 0);
            SkASSERT((fBaseCubicVertex % 4) == 0);
            flushState->bindBuffers(nullptr, fCubicBuffer, nullptr);
            flushState->drawInstanced(fCubicVertexCount >> 2, fBaseCubicVertex >> 2, 4, 0);
        }
    } else if (fFillPathProgram) {
        // There are no triangles to fill. Just draw a bounding box.
        flushState->bindPipelineAndScissorClip(*fFillPathProgram, this->bounds());
        flushState->bindTextures(fFillPathProgram->primProc(), nullptr, *fPipelineForFills);
        flushState->bindBuffers(nullptr, nullptr, nullptr);
        flushState->draw(4, 0);
    }
}
