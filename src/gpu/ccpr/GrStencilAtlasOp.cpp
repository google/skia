/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrStencilAtlasOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/ccpr/GrCCPerFlushResources.h"
#include "src/gpu/ccpr/GrSampleMaskProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

class StencilResolveProcessor : public GrGeometryProcessor {
public:
    StencilResolveProcessor() : GrGeometryProcessor(kStencilResolveProcessor_ClassID) {
        static constexpr Attribute kIBounds = {
                "ibounds", kShort4_GrVertexAttribType, kShort4_GrSLType};
        this->setInstanceAttributes(&kIBounds, 1);
        SkASSERT(this->instanceStride() == sizeof(GrStencilAtlasOp::ResolveRectInstance));
    }

private:
    const char* name() const override { return "GrCCPathProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;
    class Impl;
};

// This processor draws pixel-aligned rectangles directly on top of every path in the atlas.
// The caller should have set up the instance data such that "Nonzero" paths get clockwise
// rectangles (l < r) and "even/odd" paths get counter-clockwise (r < l). Its purpose
// is to convert winding counts in the stencil buffer to A8 coverage in the color buffer.
class StencilResolveProcessor::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        args.fVaryingHandler->emitAttributes(args.fGP.cast<StencilResolveProcessor>());

        GrGLSLVertexBuilder* v = args.fVertBuilder;
        v->codeAppendf("short2 devcoord;");
        v->codeAppendf("devcoord.x = (0 == (sk_VertexID & 1)) ? ibounds.x : ibounds.z;");
        v->codeAppendf("devcoord.y = (sk_VertexID < 2) ? ibounds.y : ibounds.w;");

        v->codeAppendf("float2 atlascoord = float2(devcoord);");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "atlascoord");

        // Just output "1" for coverage. This will be modulated by the MSAA stencil test.
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
        f->codeAppendf("%s = %s = half4(1);", args.fOutputColor, args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&&) override {}
};

GrGLSLPrimitiveProcessor* StencilResolveProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl();
}

}

std::unique_ptr<GrDrawOp> GrStencilAtlasOp::Make(
        GrRecordingContext* context, sk_sp<const GrCCPerFlushResources> resources,
        FillBatchID fillBatchID, StrokeBatchID strokeBatchID, int baseStencilResolveInstance,
        int endStencilResolveInstance, const SkISize& drawBounds) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrStencilAtlasOp>(
            std::move(resources), fillBatchID, strokeBatchID, baseStencilResolveInstance,
            endStencilResolveInstance, drawBounds);
}

// Increments clockwise triangles and decrements counterclockwise. We use the same incr/decr
// settings regardless of fill rule; fill rule is accounted for during the resolve step.
static constexpr GrUserStencilSettings kIncrDecrStencil(
    GrUserStencilSettings::StaticInitSeparate<
        0x0000,                        0x0000,
        GrUserStencilTest::kNever,     GrUserStencilTest::kNever,
        0xffff,                        0xffff,
        GrUserStencilOp::kIncWrap,     GrUserStencilOp::kDecWrap,
        GrUserStencilOp::kIncWrap,     GrUserStencilOp::kDecWrap,
        0xffff,                        0xffff>()
);

// Resolves stencil winding counts to A8 coverage. Leaves stencil values untouched.
static constexpr GrUserStencilSettings kResolveStencilCoverage(
    GrUserStencilSettings::StaticInitSeparate<
        0x0000,                           0x0000,
        GrUserStencilTest::kNotEqual,     GrUserStencilTest::kNotEqual,
        0xffff,                           0x1,
        GrUserStencilOp::kKeep,           GrUserStencilOp::kKeep,
        GrUserStencilOp::kKeep,           GrUserStencilOp::kKeep,
        0xffff,                           0xffff>()
);

// Same as above, but also resets stencil values to zero. This is better for non-tilers
// where we prefer to not clear the stencil buffer at the beginning of every render pass.
static constexpr GrUserStencilSettings kResolveStencilCoverageAndClearStencil(
    GrUserStencilSettings::StaticInitSeparate<
        0x0000,                           0x0000,
        GrUserStencilTest::kNotEqual,     GrUserStencilTest::kNotEqual,
        0xffff,                           0x1,
        GrUserStencilOp::kZero,           GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,           GrUserStencilOp::kKeep,
        0xffff,                           0xffff>()
);

void GrStencilAtlasOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkIRect drawBoundsRect = SkIRect::MakeWH(fDrawBounds.width(), fDrawBounds.height());

    GrPipeline pipeline(
            GrScissorTest::kEnabled, GrDisableColorXPFactory::MakeXferProcessor(),
            flushState->drawOpArgs().outputSwizzle(), GrPipeline::InputFlags::kHWAntialias,
            &kIncrDecrStencil);

    GrSampleMaskProcessor sampleMaskProc;

    fResources->filler().drawFills(
            flushState, &sampleMaskProc, pipeline, fFillBatchID, drawBoundsRect);

    fResources->stroker().drawStrokes(
            flushState, &sampleMaskProc, fStrokeBatchID, drawBoundsRect);

    auto pipelineFlags = GrPipeline::InputFlags::kNone;

    // The next draw will be the final op in the renderTargetContext, so we might be able to get
    // away with not resetting the stencil values back to zero on some platforms.
    const GrUserStencilSettings* stencilResolveSettings = &kResolveStencilCoverage;

    if (!flushState->caps().discardStencilValuesAfterRenderPass()) {
        // We aren't going to discard stencil, so Ganesh expects us to leave the stencil buffer in a
        // cleared state.
        stencilResolveSettings = &kResolveStencilCoverageAndClearStencil;
    }

    if (!flushState->caps().multisampleDisableSupport()) {
        pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
        if (1 == flushState->drawOpArgs().renderTarget()->numSamples()) {
            // We are using mixed samples but cannot disable multisample. In order to avoid a double
            // blend along the shared edge of our "resolve box" geometry, we must:
            //   1) Enable conservative raster to ensure every pixel is either fully covered or not
            //      touched at all (since MSAA will be enabled).
            //   2) Use a stencil test that blocks subsequent hits to a pixel afer the first one
            //      (since pixels on the shared edge will be rasterized by both triangles in the box
            //      geometry).
            pipelineFlags |= GrPipeline::InputFlags::kConservativeRaster;
            stencilResolveSettings = &kResolveStencilCoverageAndClearStencil;
        }
    }

    GrPipeline resolvePipeline(GrScissorTest::kEnabled, SkBlendMode::kSrc,
                               flushState->drawOpArgs().outputSwizzle(), pipelineFlags,
                               stencilResolveSettings);
    GrPipeline::FixedDynamicState scissorRectState(drawBoundsRect);

    GrMesh mesh(GrPrimitiveType::kTriangleStrip);
    mesh.setInstanced(fResources->refStencilResolveBuffer(),
                      fEndStencilResolveInstance - fBaseStencilResolveInstance,
                      fBaseStencilResolveInstance, 4);

    StencilResolveProcessor primProc;

    GrProgramInfo programInfo(flushState, resolvePipeline, primProc, &scissorRectState, nullptr, 0);

    flushState->opsRenderPass()->draw(programInfo, &mesh, 1, SkRect::Make(drawBoundsRect));
}
