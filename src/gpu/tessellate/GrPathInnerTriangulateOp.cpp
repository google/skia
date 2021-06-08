/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathInnerTriangulateOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrPathCurveTessellator.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

using PathFlags = GrTessellationPathRenderer::PathFlags;

namespace {

// Fills an array of convex hulls surrounding 4-point cubic or conic instances. This shader is used
// for the "cover" pass after the curves have been fully stencilled.
class HullShader : public GrPathTessellationShader {
public:
    HullShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrPathTessellationShader(kTessellate_HullShader_ClassID,
                                       GrPrimitiveType::kTriangleStrip, 0, viewMatrix, color) {
        constexpr static Attribute kPtsAttribs[] = {
                {"input_points_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"input_points_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setInstanceAttributes(kPtsAttribs, SK_ARRAY_COUNT(kPtsAttribs));
    }

private:
    const char* name() const final { return "tessellate_HullShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

GrGLSLGeometryProcessor* HullShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(GrGLSLVertexBuilder* v, GrGPArgs* gpArgs) override {
            v->codeAppend(R"(
            float4x2 P = float4x2(input_points_0_1, input_points_2_3);
            if (isinf(P[3].y)) {  // Is the curve a conic?
                float w = P[3].x;
                if (isinf(w)) {
                    // A conic with w=Inf is an exact triangle.
                    P = float4x2(P[0], P[1], P[2], P[2]);
                } else {
                    // Convert the points to a trapeziodal hull that circumcscribes the conic.
                    float2 p1w = P[1] * w;
                    float T = .51;  // Bias outward a bit to ensure we cover the outermost samples.
                    float2 c1 = mix(P[0], p1w, T);
                    float2 c2 = mix(P[2], p1w, T);
                    float iw = 1 / mix(1, w, T);
                    P = float4x2(P[0], c1 * iw, c2 * iw, P[2]);
                }
            }

            // Translate the points to v0..3 where v0=0.
            float2 v1 = P[1] - P[0], v2 = P[2] - P[0], v3 = P[3] - P[0];

            // Reorder the points so v2 bisects v1 and v3.
            if (sign(determinant(float2x2(v2,v1))) == sign(determinant(float2x2(v2,v3)))) {
                float2 tmp = P[2];
                if (sign(determinant(float2x2(v1,v2))) != sign(determinant(float2x2(v1,v3)))) {
                    P[2] = P[1];  // swap(P2, P1)
                    P[1] = tmp;
                } else {
                    P[2] = P[3];  // swap(P2, P3)
                    P[3] = tmp;
                }
            }

            // sk_VertexID comes in fan order. Convert to strip order.
            int vertexidx = sk_VertexID;
            vertexidx ^= vertexidx >> 1;

            // Find the "turn direction" of each corner and net turn direction.
            float vertexdir = 0;
            float netdir = 0;
            for (int i = 0; i < 4; ++i) {
                float2 prev = P[i] - P[(i + 3) & 3], next = P[(i + 1) & 3] - P[i];
                float dir = sign(determinant(float2x2(prev, next)));
                if (i == vertexidx) {
                    vertexdir = dir;
                }
                netdir += dir;
            }

            // Remove the non-convex vertex, if any.
            if (vertexdir != sign(netdir)) {
                vertexidx = (vertexidx + 1) & 3;
            }

            float2 localcoord = P[vertexidx];
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        }
    };
    return new Impl;
}

}  // namespace

void GrPathInnerTriangulateOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fPipelineForFills) {
        fPipelineForFills->visitProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrDrawOp::FixedFunctionFlags GrPathInnerTriangulateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrPathInnerTriangulateOp::finalize(const GrCaps& caps,
                                                            const GrAppliedClip* clip,
                                                            GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void GrPathInnerTriangulateOp::pushFanStencilProgram(const GrTessellationShader::ProgramArgs& args,
                                                     const GrPipeline* pipelineForStencils,
                                                     const GrUserStencilSettings* stencil) {
    SkASSERT(pipelineForStencils);
    auto shader = GrPathTessellationShader::MakeSimpleTriangleShader(args.fArena, fViewMatrix,
                                                                     SK_PMColor4fTRANSPARENT);
    fFanPrograms.push_back(GrTessellationShader::MakeProgram(args, shader, pipelineForStencils,
                                                             stencil)); }

void GrPathInnerTriangulateOp::pushFanFillProgram(const GrTessellationShader::ProgramArgs& args,
                                                  const GrUserStencilSettings* stencil) {
    SkASSERT(fPipelineForFills);
    auto shader = GrPathTessellationShader::MakeSimpleTriangleShader(args.fArena, fViewMatrix,
                                                                     fColor);
    fFanPrograms.push_back(GrTessellationShader::MakeProgram(args, shader, fPipelineForFills,
                                                             stencil));
}

void GrPathInnerTriangulateOp::prePreparePrograms(const GrTessellationShader::ProgramArgs& args,
                                                  GrAppliedClip&& appliedClip) {
    SkASSERT(!fFanTriangulator);
    SkASSERT(!fFanPolys);
    SkASSERT(!fPipelineForFills);
    SkASSERT(!fTessellator);
    SkASSERT(!fStencilCurvesProgram);
    SkASSERT(fFanPrograms.empty());
    SkASSERT(!fCoverHullsProgram);

    if (fPath.countVerbs() <= 0) {
        return;
    }

    // If using wireframe, we have to fall back on a standard Redbook "stencil then cover" algorithm
    // instead of bypassing the stencil buffer to fill the fan directly.
    bool forceRedbookStencilPass = (fPathFlags & (PathFlags::kStencilOnly | PathFlags::kWireframe));
    bool doFill = !(fPathFlags & PathFlags::kStencilOnly);

    bool isLinear;
    fFanTriangulator = args.fArena->make<GrInnerFanTriangulator>(fPath, args.fArena);
    fFanPolys = fFanTriangulator->pathToPolys(&fFanBreadcrumbs, &isLinear);

    // Create a pipeline for stencil passes if needed.
    const GrPipeline* pipelineForStencils = nullptr;
    if (forceRedbookStencilPass || !isLinear) {  // Curves always get stencilled.
        pipelineForStencils = GrPathTessellationShader::MakeStencilOnlyPipeline(
                args, fAAType, fPathFlags, appliedClip.hardClip());
    }

    // Create a pipeline for fill passes if needed.
    if (doFill) {
        fPipelineForFills = GrTessellationShader::MakePipeline(args, fAAType,
                                                               std::move(appliedClip),
                                                               std::move(fProcessors));
    }

    // Pass 1: Tessellate the outer curves into the stencil buffer.
    if (!isLinear) {
        fTessellator = GrPathCurveTessellator::Make(args.fArena, fViewMatrix,
                                                    SK_PMColor4fTRANSPARENT,
                                                    GrPathTessellator::DrawInnerFan::kNo,
                                                    fPath.countVerbs(), *args.fCaps);
        const GrUserStencilSettings* stencilPathSettings =
                GrPathTessellationShader::StencilPathSettings(fPath.getFillType());
        fStencilCurvesProgram = GrTessellationShader::MakeProgram(args, fTessellator->shader(),
                                                                  pipelineForStencils,
                                                                  stencilPathSettings);
    }

    // Pass 2: Fill the path's inner fan with a stencil test against the curves.
    if (fFanPolys) {
        if (forceRedbookStencilPass) {
            // Use a standard Redbook "stencil then cover" algorithm instead of bypassing the
            // stencil buffer to fill the fan directly.
            const GrUserStencilSettings* stencilPathSettings =
                    GrPathTessellationShader::StencilPathSettings(fPath.getFillType());
            this->pushFanStencilProgram(args, pipelineForStencils, stencilPathSettings);
            if (doFill) {
                this->pushFanFillProgram(args,
                                         GrPathTessellationShader::TestAndResetStencilSettings());
            }
        } else if (isLinear) {
            // There are no outer curves! Ignore stencil and fill the path directly.
            SkASSERT(!pipelineForStencils);
            this->pushFanFillProgram(args, &GrUserStencilSettings::kUnused);
        } else if (!fPipelineForFills->hasStencilClip()) {
            // These are a twist on the standard Redbook stencil settings that allow us to fill the
            // inner polygon directly to the final render target. By the time these programs
            // execute, the outer curves will already be stencilled in. So if the stencil value is
            // zero, then it means the sample in question is not affected by any curves and we can
            // fill it in directly. If the stencil value is nonzero, then we don't fill and instead
            // continue the standard Redbook counting process.
            constexpr static GrUserStencilSettings kFillOrIncrDecrStencil(
                GrUserStencilSettings::StaticInitSeparate<
                    0x0000,                       0x0000,
                    GrUserStencilTest::kEqual,    GrUserStencilTest::kEqual,
                    0xffff,                       0xffff,
                    GrUserStencilOp::kKeep,       GrUserStencilOp::kKeep,
                    GrUserStencilOp::kIncWrap,    GrUserStencilOp::kDecWrap,
                    0xffff,                       0xffff>());

            constexpr static GrUserStencilSettings kFillOrInvertStencil(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    GrUserStencilTest::kEqual,
                    0xffff,
                    GrUserStencilOp::kKeep,
                    // "Zero" instead of "Invert" because the fan only touches any given pixel once.
                    GrUserStencilOp::kZero,
                    0xffff>());

            auto* stencil = (fPath.getFillType() == SkPathFillType::kWinding)
                    ? &kFillOrIncrDecrStencil
                    : &kFillOrInvertStencil;
            this->pushFanFillProgram(args, stencil);
        } else {
            // This is the same idea as above, but we use two passes instead of one because there is
            // a stencil clip. The stencil test isn't expressive enough to do the above tests and
            // also check the clip bit in a single pass.
            constexpr static GrUserStencilSettings kFillIfZeroAndInClip(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    GrUserStencilTest::kEqualIfInClip,
                    0xffff,
                    GrUserStencilOp::kKeep,
                    GrUserStencilOp::kKeep,
                    0xffff>());

            constexpr static GrUserStencilSettings kIncrDecrStencilIfNonzero(
                GrUserStencilSettings::StaticInitSeparate<
                    0x0000,                         0x0000,
                    // No need to check the clip because the previous stencil pass will have only
                    // written to samples already inside the clip.
                    GrUserStencilTest::kNotEqual,   GrUserStencilTest::kNotEqual,
                    0xffff,                         0xffff,
                    GrUserStencilOp::kIncWrap,      GrUserStencilOp::kDecWrap,
                    GrUserStencilOp::kKeep,         GrUserStencilOp::kKeep,
                    0xffff,                         0xffff>());

            constexpr static GrUserStencilSettings kInvertStencilIfNonZero(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    // No need to check the clip because the previous stencil pass will have only
                    // written to samples already inside the clip.
                    GrUserStencilTest::kNotEqual,
                    0xffff,
                    // "Zero" instead of "Invert" because the fan only touches any given pixel once.
                    GrUserStencilOp::kZero,
                    GrUserStencilOp::kKeep,
                    0xffff>());

            // Pass 2a: Directly fill fan samples whose stencil values (from curves) are zero.
            this->pushFanFillProgram(args, &kFillIfZeroAndInClip);

            // Pass 2b: Redbook counting on fan samples whose stencil values (from curves) != 0.
            auto* stencil = (fPath.getFillType() == SkPathFillType::kWinding)
                    ? &kIncrDecrStencilIfNonzero
                    : &kInvertStencilIfNonZero;
            this->pushFanStencilProgram(args, pipelineForStencils, stencil);
        }
    }

    // Pass 3: Draw convex hulls around each curve.
    if (doFill && !isLinear) {
        // By the time this program executes, every pixel will be filled in except the ones touched
        // by curves. We issue a final cover pass over the curves by drawing their convex hulls.
        // This will fill in any remaining samples and reset the stencil values back to zero.
        SkASSERT(fTessellator);
        auto* hullShader = args.fArena->make<HullShader>(fViewMatrix, fColor);
        fCoverHullsProgram = GrTessellationShader::MakeProgram(
                args, hullShader, fPipelineForFills,
                GrPathTessellationShader::TestAndResetStencilSettings());
    }
}

void GrPathInnerTriangulateOp::onPrePrepare(GrRecordingContext* context,
                                            const GrSurfaceProxyView& writeView,
                                            GrAppliedClip* clip,
                                            const GrDstProxyView& dstProxyView,
                                            GrXferBarrierFlags renderPassXferBarriers,
                                            GrLoadOp colorLoadOp) {
    this->prePreparePrograms({context->priv().recordTimeAllocator(), writeView, &dstProxyView,
                             renderPassXferBarriers, colorLoadOp, context->priv().caps()},
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    if (fStencilCurvesProgram) {
        context->priv().recordProgramInfo(fStencilCurvesProgram);
    }
    for (const GrProgramInfo* fanProgram : fFanPrograms) {
        context->priv().recordProgramInfo(fanProgram);
    }
    if (fCoverHullsProgram) {
        context->priv().recordProgramInfo(fCoverHullsProgram);
    }
}

void GrPathInnerTriangulateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fFanTriangulator) {
        this->prePreparePrograms({flushState->allocator(), flushState->writeView(),
                                 &flushState->dstProxyView(), flushState->renderPassBarriers(),
                                 flushState->colorLoadOp(), &flushState->caps()},
                                 flushState->detachAppliedClip());
        if (!fFanTriangulator) {
            return;
        }
    }

    if (fFanPolys) {
        GrEagerDynamicVertexAllocator alloc(flushState, &fFanBuffer, &fBaseFanVertex);
        fFanVertexCount = fFanTriangulator->polysToTriangles(fFanPolys, &alloc, &fFanBreadcrumbs);
    }

    if (fTessellator) {
        // Must be called after polysToTriangles() in order for fFanBreadcrumbs to be complete.
        fTessellator->prepare(flushState, this->bounds(), fPath, &fFanBreadcrumbs);
    }
}

void GrPathInnerTriangulateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fStencilCurvesProgram) {
        SkASSERT(fTessellator);
        flushState->bindPipelineAndScissorClip(*fStencilCurvesProgram, this->bounds());
        fTessellator->draw(flushState);
        if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
            flushState->gpu()->insertManualFramebufferBarrier();  // http://skbug.com/9739
        }
    }

    for (const GrProgramInfo* fanProgram : fFanPrograms) {
        SkASSERT(fFanBuffer);
        flushState->bindPipelineAndScissorClip(*fanProgram, this->bounds());
        flushState->bindTextures(fanProgram->geomProc(), nullptr, fanProgram->pipeline());
        flushState->bindBuffers(nullptr, nullptr, fFanBuffer);
        flushState->draw(fFanVertexCount, fBaseFanVertex);
    }

    if (fCoverHullsProgram) {
        SkASSERT(fTessellator);
        flushState->bindPipelineAndScissorClip(*fCoverHullsProgram, this->bounds());
        flushState->bindTextures(fCoverHullsProgram->geomProc(), nullptr, *fPipelineForFills);
        fTessellator->drawHullInstances(flushState);
    }
}
