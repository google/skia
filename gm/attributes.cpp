/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/gpu/ProxyUtils.h"

#include <memory>
#include <vector>

class GrAppliedClip;
class GrGLSLProgramDataManager;

namespace {

enum class AttrMode {
    kAuto,
    kManual,
    kWacky
};

class AttributeTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, AttrMode mode) {
        return arena->make([&](void* ptr) { return new (ptr) AttributeTestProcessor(mode); });
    }

    const char* name() const final { return "AttributeTestProcessor"; }

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const final {
        b->add32(static_cast<uint32_t>(fMode));
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

private:
    AttributeTestProcessor(AttrMode mode)
            : GrGeometryProcessor(kAttributeTestProcessor_ClassID), fMode(mode) {
        switch (fMode) {
            case AttrMode::kAuto:
                fAttributes.emplace_back("pos", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
                fAttributes.emplace_back("color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType);
                this->setVertexAttributesWithImplicitOffsets(fAttributes.data(),
                                                             fAttributes.size());
                break;
            case AttrMode::kManual:
                // Same result as kAuto but with explicitly specified offsets and stride.
                fAttributes.emplace_back("pos", kFloat2_GrVertexAttribType, kFloat2_GrSLType, 0);
                fAttributes.emplace_back(
                        "color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType, 8);
                this->setVertexAttributes(fAttributes.data(), fAttributes.size(), 12);
                break;
            case AttrMode::kWacky:
                //  0 thru  7 : float2 aliased to "pos0" and "pos1"
                //  8 thru 11: pad
                // 12 thru 15: unorm4 "color"
                // 16 thru 19: pad
                fAttributes.emplace_back("pos0", kFloat2_GrVertexAttribType, kFloat2_GrSLType, 0);
                fAttributes.emplace_back("pos1", kFloat2_GrVertexAttribType, kFloat2_GrSLType, 0);
                fAttributes.emplace_back(
                        "color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType, 12);
                this->setVertexAttributes(fAttributes.data(), fAttributes.size(), 20);
                break;
        }
    }

    const AttrMode fMode;

    std::vector<Attribute> fAttributes;

    using INHERITED = GrGeometryProcessor;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> AttributeTestProcessor::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager&,
                     const GrShaderCaps&,
                     const GrGeometryProcessor&) override {}

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const AttributeTestProcessor& proc = args.fGeomProc.cast<AttributeTestProcessor>();
            args.fVaryingHandler->emitAttributes(proc);
            if (proc.fMode == AttrMode::kWacky) {
                args.fVertBuilder->codeAppend("float2 pos = pos0 + pos1;");
            }
            args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
            args.fVaryingHandler->addPassThroughAttribute(GrShaderVar("color", kHalf4_GrSLType),
                                                          args.fOutputColor);
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "pos");
            args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
        }
    };

    return std::make_unique<Impl>();
}

class AttributeTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context, AttrMode mode, const SkRect& r) {
        return GrOp::Make<AttributeTestOp>(context, mode, r);
    }

private:
    AttributeTestOp(AttrMode mode, SkRect rect) : GrDrawOp(ClassID()), fMode(mode), fRect(rect) {
        this->setBounds(fRect, HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "AttributeTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

    GrProgramInfo* createProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& writeView,
                                     bool usesMSAASurface,
                                     GrAppliedClip&& appliedClip,
                                     const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) const {
        GrGeometryProcessor* geomProc = AttributeTestProcessor::Make(arena, fMode);

        return sk_gpu_test::CreateProgramInfo(caps,
                                              arena,
                                              writeView,
                                              usesMSAASurface,
                                              std::move(appliedClip),
                                              dstProxyView,
                                              geomProc,
                                              SkBlendMode::kSrcOver,
                                              GrPrimitiveType::kTriangleStrip,
                                              renderPassXferBarriers,
                                              colorLoadOp);
    }

    GrProgramInfo* createProgramInfo(GrOpFlushState* flushState) const {
        return this->createProgramInfo(&flushState->caps(),
                                       flushState->allocator(),
                                       flushState->writeView(),
                                       flushState->usesMSAASurface(),
                                       flushState->detachAppliedClip(),
                                       flushState->dstProxyView(),
                                       flushState->renderPassBarriers(),
                                       flushState->colorLoadOp());
    }

    void onPrePrepare(GrRecordingContext* context,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip* clip,
                      const GrDstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) final {
        SkArenaAlloc* arena = context->priv().recordTimeAllocator();

        // DMSAA is not supported on DDL.
        bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;

        // This is equivalent to a GrOpFlushState::detachAppliedClip
        GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip::Disabled();

        fProgramInfo = this->createProgramInfo(context->priv().caps(),
                                               arena,
                                               writeView,
                                               usesMSAASurface,
                                               std::move(appliedClip),
                                               dstProxyView,
                                               renderPassXferBarriers,
                                               colorLoadOp);

        context->priv().recordProgramInfo(fProgramInfo);
    }

    template <typename V> void makeVB(GrOpFlushState* flushState, const SkRect rect) {
        V v[4];
        v[0].p = {rect.left() , rect.top()   };
        v[1].p = {rect.right(), rect.top()   };
        v[2].p = {rect.left() , rect.bottom()};
        v[3].p = {rect.right(), rect.bottom()};
        v[0].color = SK_ColorRED;
        v[1].color = SK_ColorGREEN;
        v[2].color = SK_ColorYELLOW;
        v[3].color = SK_ColorMAGENTA;
        fVertexBuffer = flushState->resourceProvider()->createBuffer(
                sizeof(v), GrGpuBufferType::kVertex, kStatic_GrAccessPattern, v);
    }

    void onPrepare(GrOpFlushState* flushState) override {
        if (fMode == AttrMode::kWacky) {
            struct V {
                SkPoint p;
                uint32_t pad0;
                uint32_t color;
                uint32_t pad1;
            };
            SkRect rect {fRect.fLeft/2.f, fRect.fTop/2.f, fRect.fRight/2.f, fRect.fBottom/2.f};
            this->makeVB<V>(flushState, rect);
        } else {
            struct V {
                SkPoint p;
                uint32_t color;
            };
            this->makeVB<V>(flushState, fRect);
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fVertexBuffer) {
            return;
        }

        if (!fProgramInfo) {
            fProgramInfo = this->createProgramInfo(flushState);
        }

        flushState->bindPipeline(*fProgramInfo, fRect);
        flushState->bindBuffers(nullptr, nullptr, std::move(fVertexBuffer));
        flushState->draw(4, 0);
    }

    sk_sp<GrBuffer> fVertexBuffer;
    const AttrMode  fMode;
    const SkRect    fRect;

    // The program info (and both the GrPipeline and GrGeometryProcessor it relies on), when
    // allocated, are allocated in either the ddl-record-time or flush-time arena. It is the
    // arena's job to free up their memory so we just have a bare programInfo pointer here. We
    // don't even store the GrPipeline and GrGeometryProcessor pointers here bc they are
    // guaranteed to have the same lifetime as the program info.
    GrProgramInfo* fProgramInfo = nullptr;

    friend class ::GrOp;  // for ctor

    using INHERITED = GrDrawOp;
};

}  // namespace

namespace skiagm {

/**
 * This is a GPU-backend specific test that exercises explicit and implicit attribute offsets and
 * strides.
 */
class AttributesGM : public GpuGM {
    SkString onShortName() override { return SkString("attributes"); }
    SkISize onISize() override { return {120, 340}; }
    DrawResult onDraw(GrRecordingContext*, SkCanvas*, SkString* errorMsg) override;
};

DrawResult AttributesGM::onDraw(GrRecordingContext* rc, SkCanvas* canvas, SkString* errorMsg) {
    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    sdc->clear(SK_PMColor4fBLACK);

    // Draw the test directly to the frame buffer.
    auto r = SkRect::MakeXYWH(10, 10, 100, 100);
    for (AttrMode m : {AttrMode::kAuto, AttrMode::kManual, AttrMode::kWacky}) {
        sdc->addDrawOp(AttributeTestOp::Make(rc, m, r));
        r.offset(0, 110);
    }

    return DrawResult::kOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DEF_GM( return new AttributesGM(); )

}  // namespace skiagm
