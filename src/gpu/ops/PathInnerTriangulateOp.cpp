/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/PathInnerTriangulateOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/PathCurveTessellator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

namespace skgpu::v1 {

namespace {

// Fills an array of convex hulls surrounding 4-point cubic or conic instances. This shader is used
// for the "cover" pass after the curves have been fully stencilled.
class HullShader : public GrPathTessellationShader {
public:
    HullShader(const SkMatrix& viewMatrix, SkPMColor4f color, const GrShaderCaps& shaderCaps)
            : GrPathTessellationShader(kTessellate_HullShader_ClassID,
                                       GrPrimitiveType::kTriangleStrip, 0, viewMatrix, color,
                                       skgpu::PatchAttribs::kNone) {
        fInstanceAttribs.emplace_back("p01", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fInstanceAttribs.emplace_back("p23", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (!shaderCaps.infinitySupport()) {
            // A conic curve is written out with p3=[w,Infinity], but GPUs that don't support
            // infinity can't detect this. On these platforms we also write out an extra float with
            // each patch that explicitly tells the shader what type of curve it is.
            fInstanceAttribs.emplace_back("curveType", kFloat_GrVertexAttribType, kFloat_GrSLType);
        }
        this->setInstanceAttributesWithImplicitOffsets(fInstanceAttribs.data(),
                                                       fInstanceAttribs.count());
        SkASSERT(fInstanceAttribs.count() <= kMaxInstanceAttribCount);

        if (!shaderCaps.vertexIDSupport()) {
            constexpr static Attribute kVertexIdxAttrib("vertexidx", kFloat_GrVertexAttribType,
                                                        kFloat_GrSLType);
            this->setVertexAttributesWithImplicitOffsets(&kVertexIdxAttrib, 1);
        }
    }

    int maxTessellationSegments(const GrShaderCaps&) const override { SkUNREACHABLE; }

private:
    const char* name() const final { return "tessellate_HullShader"; }
    void addToKey(const GrShaderCaps&, KeyBuilder*) const final {}
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    constexpr static int kMaxInstanceAttribCount = 3;
    SkSTArray<kMaxInstanceAttribCount, Attribute> fInstanceAttribs;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> HullShader::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrShaderCaps& shaderCaps,
                            const GrPathTessellationShader&,
                            GrGLSLVertexBuilder* v,
                            GrGLSLVaryingHandler*,
                            GrGPArgs* gpArgs) override {
            if (shaderCaps.infinitySupport()) {
                v->insertFunction(R"(
                bool is_conic_curve() { return isinf(p23.w); }
                bool is_non_triangular_conic_curve() {
                    // We consider a conic non-triangular as long as its weight isn't infinity.
                    // NOTE: "isinf == false" works on Mac Radeon GLSL; "!isinf" can get the wrong
                    // answer.
                    return isinf(p23.z) == false;
                })");
            } else {
                v->insertFunction(SkStringPrintf(R"(
                bool is_conic_curve() { return curveType != %g; })", kCubicCurveType).c_str());
                v->insertFunction(SkStringPrintf(R"(
                bool is_non_triangular_conic_curve() {
                    return curveType == %g;
                })", kConicCurveType).c_str());
            }
            v->codeAppend(R"(
            float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;
            if (is_conic_curve()) {
                // Conics are 3 points, with the weight in p3.
                float w = p3.x;
                p3 = p2;  // Duplicate the endpoint for shared code that also runs on cubics.
                if (is_non_triangular_conic_curve()) {
                    // Convert the points to a trapeziodal hull that circumcscribes the conic.
                    float2 p1w = p1 * w;
                    float T = .51;  // Bias outward a bit to ensure we cover the outermost samples.
                    float2 c1 = mix(p0, p1w, T);
                    float2 c2 = mix(p2, p1w, T);
                    float iw = 1 / mix(1, w, T);
                    p2 = c2 * iw;
                    p1 = c1 * iw;
                }
            }

            // Translate the points to v0..3 where v0=0.
            float2 v1 = p1 - p0;
            float2 v2 = p2 - p0;
            float2 v3 = p3 - p0;

            // Reorder the points so v2 bisects v1 and v3.
            if (sign(cross_length_2d(v2, v1)) == sign(cross_length_2d(v2, v3))) {
                float2 tmp = p2;
                if (sign(cross_length_2d(v1, v2)) != sign(cross_length_2d(v1, v3))) {
                    p2 = p1;  // swap(p2, p1)
                    p1 = tmp;
                } else {
                    p2 = p3;  // swap(p2, p3)
                    p3 = tmp;
                }
            })");

            if (shaderCaps.vertexIDSupport()) {
                // If we don't have sk_VertexID support then "vertexidx" already came in as a
                // vertex attrib.
                v->codeAppend(R"(
                // sk_VertexID comes in fan order. Convert to strip order.
                int vertexidx = sk_VertexID;
                vertexidx ^= vertexidx >> 1;)");
            }

            v->codeAppend(R"(
            // Find the "turn direction" of each corner and net turn direction.
            float vertexdir = 0;
            float netdir = 0;
            float2 prev, next;
            float dir;
            float2 localcoord;
            float2 nextcoord;)");

            for (int i = 0; i < 4; ++i) {
                v->codeAppendf(R"(
                prev = p%i - p%i;)", i, (i + 3) % 4);
                v->codeAppendf(R"(
                next = p%i - p%i;)", (i + 1) % 4, i);
                v->codeAppendf(R"(
                dir = sign(cross_length_2d(prev, next));
                if (vertexidx == %i) {
                    vertexdir = dir;
                    localcoord = p%i;
                    nextcoord = p%i;
                }
                netdir += dir;)", i, i, (i + 1) % 4);
            }

            v->codeAppend(R"(
            // Remove the non-convex vertex, if any.
            if (vertexdir != sign(netdir)) {
                localcoord = nextcoord;
            }

            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        }
    };
    return std::make_unique<Impl>();
}

}  // anonymous namespace

void PathInnerTriangulateOp::visitProxies(const GrVisitProxyFunc& func) const {
    if (fPipelineForFills) {
        fPipelineForFills->visitProxies(func);
    } else {
        fProcessors.visitProxies(func);
    }
}

GrDrawOp::FixedFunctionFlags PathInnerTriangulateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis PathInnerTriangulateOp::finalize(const GrCaps& caps,
                                                          const GrAppliedClip* clip,
                                                          GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void PathInnerTriangulateOp::pushFanStencilProgram(const GrTessellationShader::ProgramArgs& args,
                                                   const GrPipeline* pipelineForStencils,
                                                   const GrUserStencilSettings* stencil) {
    SkASSERT(pipelineForStencils);
    auto shader = GrPathTessellationShader::MakeSimpleTriangleShader(args.fArena, fViewMatrix,
                                                                     SK_PMColor4fTRANSPARENT);
    fFanPrograms.push_back(GrTessellationShader::MakeProgram(args, shader, pipelineForStencils,
                                                             stencil)); }

void PathInnerTriangulateOp::pushFanFillProgram(const GrTessellationShader::ProgramArgs& args,
                                                const GrUserStencilSettings* stencil) {
    SkASSERT(fPipelineForFills);
    auto shader = GrPathTessellationShader::MakeSimpleTriangleShader(args.fArena, fViewMatrix,
                                                                     fColor);
    fFanPrograms.push_back(GrTessellationShader::MakeProgram(args, shader, fPipelineForFills,
                                                             stencil));
}

void PathInnerTriangulateOp::prePreparePrograms(const GrTessellationShader::ProgramArgs& args,
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
    bool forceRedbookStencilPass =
            (fPathFlags & (FillPathFlags::kStencilOnly | FillPathFlags::kWireframe));
    bool doFill = !(fPathFlags & FillPathFlags::kStencilOnly);

    bool isLinear;
    fFanTriangulator = args.fArena->make<GrInnerFanTriangulator>(fPath, args.fArena);
    fFanPolys = fFanTriangulator->pathToPolys(&fFanBreadcrumbs, &isLinear);

    // Create a pipeline for stencil passes if needed.
    const GrPipeline* pipelineForStencils = nullptr;
    if (forceRedbookStencilPass || !isLinear) {  // Curves always get stencilled.
        auto pipelineFlags = (fPathFlags & FillPathFlags::kWireframe)
                ? GrPipeline::InputFlags::kWireframe
                : GrPipeline::InputFlags::kNone;
        pipelineForStencils = GrPathTessellationShader::MakeStencilOnlyPipeline(
                args, fAAType, appliedClip.hardClip(), pipelineFlags);
    }

    // Create a pipeline for fill passes if needed.
    if (doFill) {
        fPipelineForFills = GrTessellationShader::MakePipeline(args, fAAType,
                                                               std::move(appliedClip),
                                                               std::move(fProcessors));
    }

    // Pass 1: Tessellate the outer curves into the stencil buffer.
    if (!isLinear) {
        fTessellator = PathCurveTessellator::Make(args.fArena,
                                                  args.fCaps->shaderCaps()->infinitySupport());
        auto* tessShader = GrPathTessellationShader::Make(args.fArena,
                                                          fViewMatrix,
                                                          SK_PMColor4fTRANSPARENT,
                                                          fPath.countVerbs(),
                                                          *pipelineForStencils,
                                                          fTessellator->patchAttribs(),
                                                          *args.fCaps);
        const GrUserStencilSettings* stencilPathSettings =
                GrPathTessellationShader::StencilPathSettings(GrFillRuleForSkPath(fPath));
        fStencilCurvesProgram = GrTessellationShader::MakeProgram(args,
                                                                  tessShader,
                                                                  pipelineForStencils,
                                                                  stencilPathSettings);
    }

    // Pass 2: Fill the path's inner fan with a stencil test against the curves.
    if (fFanPolys) {
        if (forceRedbookStencilPass) {
            // Use a standard Redbook "stencil then cover" algorithm instead of bypassing the
            // stencil buffer to fill the fan directly.
            const GrUserStencilSettings* stencilPathSettings =
                    GrPathTessellationShader::StencilPathSettings(GrFillRuleForSkPath(fPath));
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
        auto* hullShader = args.fArena->make<HullShader>(fViewMatrix, fColor,
                                                         *args.fCaps->shaderCaps());
        fCoverHullsProgram = GrTessellationShader::MakeProgram(
                args, hullShader, fPipelineForFills,
                GrPathTessellationShader::TestAndResetStencilSettings());
    }
}

void PathInnerTriangulateOp::onPrePrepare(GrRecordingContext* context,
                                          const GrSurfaceProxyView& writeView,
                                          GrAppliedClip* clip,
                                          const GrDstProxyView& dstProxyView,
                                          GrXferBarrierFlags renderPassXferBarriers,
                                          GrLoadOp colorLoadOp) {
    // DMSAA is not supported on DDL.
    bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;
    this->prePreparePrograms({context->priv().recordTimeAllocator(), writeView, usesMSAASurface,
                             &dstProxyView, renderPassXferBarriers, colorLoadOp,
                             context->priv().caps()},
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

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gHullVertexBufferKey);

void PathInnerTriangulateOp::onPrepare(GrOpFlushState* flushState) {
    const GrCaps& caps = flushState->caps();

    if (!fFanTriangulator) {
        this->prePreparePrograms({flushState->allocator(), flushState->writeView(),
                                 flushState->usesMSAASurface(), &flushState->dstProxyView(),
                                 flushState->renderPassBarriers(), flushState->colorLoadOp(),
                                 &caps}, flushState->detachAppliedClip());
        if (!fFanTriangulator) {
            return;
        }
    }

    if (fFanPolys) {
        GrEagerDynamicVertexAllocator alloc(flushState, &fFanBuffer, &fBaseFanVertex);
        fFanVertexCount = fFanTriangulator->polysToTriangles(fFanPolys, &alloc, &fFanBreadcrumbs);
    }

    if (fTessellator) {
        int patchPreallocCount = fFanBreadcrumbs.count() +
                                 fTessellator->patchPreallocCount(fPath.countVerbs());
        SkASSERT(patchPreallocCount);  // Otherwise fTessellator should be null.

        PatchWriter patchWriter(flushState, fTessellator, patchPreallocCount);

        // Write out breadcrumb triangles. This must be called after polysToTriangles() in order for
        // fFanBreadcrumbs to be complete.
        SkDEBUGCODE(int breadcrumbCount = 0;)
        for (const auto* tri = fFanBreadcrumbs.head(); tri; tri = tri->fNext) {
            SkDEBUGCODE(++breadcrumbCount;)
            auto p0 = float2::Load(tri->fPts);
            auto p1 = float2::Load(tri->fPts + 1);
            auto p2 = float2::Load(tri->fPts + 2);
            if (skvx::any((p0 == p1) & (p1 == p2))) {
                // Cull completely horizontal or vertical triangles. GrTriangulator can't always
                // get these breadcrumb edges right when they run parallel to the sweep
                // direction because their winding is undefined by its current definition.
                // FIXME(skia:12060): This seemed safe, but if there is a view matrix it will
                // introduce T-junctions.
                continue;
            }
            PatchWriter::TrianglePatch(patchWriter) << p0 << p1 << p2;
        }
        SkASSERT(breadcrumbCount == fFanBreadcrumbs.count());

        // Write out the curves.
        auto tessShader = &fStencilCurvesProgram->geomProc().cast<GrPathTessellationShader>();
        fTessellator->writePatches(patchWriter,
                                   tessShader->maxTessellationSegments(*caps.shaderCaps()),
                                   tessShader->viewMatrix(),
                                   {SkMatrix::I(), fPath, SK_PMColor4fTRANSPARENT});

        if (!tessShader->willUseTessellationShaders()) {
            fTessellator->prepareFixedCountBuffers(flushState);
        }
    }

    if (!caps.shaderCaps()->vertexIDSupport()) {
        constexpr static float kStripOrderIDs[4] = {0, 1, 3, 2};

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gHullVertexBufferKey);

        fHullVertexBufferIfNoIDSupport = flushState->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kStripOrderIDs), kStripOrderIDs,
                gHullVertexBufferKey);
    }
}

void PathInnerTriangulateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fCoverHullsProgram &&
        fCoverHullsProgram->geomProc().hasVertexAttributes() &&
        !fHullVertexBufferIfNoIDSupport) {
        return;
    }

    if (fStencilCurvesProgram) {
        SkASSERT(fTessellator);
        flushState->bindPipelineAndScissorClip(*fStencilCurvesProgram, this->bounds());
        fTessellator->draw(flushState,
                           fStencilCurvesProgram->geomProc().willUseTessellationShaders());
        if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
            flushState->gpu()->insertManualFramebufferBarrier();  // http://skbug.com/9739
        }
    }

    // Allocation of the fan vertex buffer may have failed but we already pushed back fan programs.
    if (fFanBuffer) {
        for (const GrProgramInfo* fanProgram : fFanPrograms) {
            flushState->bindPipelineAndScissorClip(*fanProgram, this->bounds());
            flushState->bindTextures(fanProgram->geomProc(), nullptr, fanProgram->pipeline());
            flushState->bindBuffers(nullptr, nullptr, fFanBuffer);
            flushState->draw(fFanVertexCount, fBaseFanVertex);
        }
    }

    if (fCoverHullsProgram) {
        SkASSERT(fTessellator);
        flushState->bindPipelineAndScissorClip(*fCoverHullsProgram, this->bounds());
        flushState->bindTextures(fCoverHullsProgram->geomProc(), nullptr, *fPipelineForFills);
        fTessellator->drawHullInstances(flushState, fHullVertexBufferIfNoIDSupport);
    }
}

} // namespace skgpu::v1
