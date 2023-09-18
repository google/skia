/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "src/core/SkPathPriv.h"
#include "tools/viewer/ClickHandlerSlide.h"

#if defined(SK_GANESH)

#include "include/core/SkFont.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/ganesh/ops/TessellationPathRenderer.h"
#include "src/gpu/ganesh/tessellate/GrPathTessellationShader.h"
#include "src/gpu/ganesh/tessellate/PathTessellator.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

namespace skgpu::ganesh {

namespace {

enum class Mode {
    kWedgeMiddleOut,
    kCurveMiddleOut
};

static const char* ModeName(Mode mode) {
    switch (mode) {
        case Mode::kWedgeMiddleOut:
            return "MiddleOutShader (kWedges)";
        case Mode::kCurveMiddleOut:
            return "MiddleOutShader (kCurves)";
    }
    SkUNREACHABLE;
}

// Draws a path directly to the screen using a specific tessellator.
class SamplePathTessellatorOp : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    SamplePathTessellatorOp(const SkRect& drawBounds, const SkPath& path, const SkMatrix& m,
                            GrPipeline::InputFlags pipelineFlags, Mode mode)
            : GrDrawOp(ClassID())
            , fPath(path)
            , fMatrix(m)
            , fPipelineFlags(pipelineFlags)
            , fMode(mode) {
        this->setBounds(drawBounds, HasAABloat::kNo, IsHairline::kNo);
    }
    const char* name() const override { return "SamplePathTessellatorOp"; }
    void visitProxies(const GrVisitProxyFunc&) const override {}
    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kUsesHWAA;
    }
    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        SkPMColor4f color;
        return fProcessors.finalize(SK_PMColor4fWHITE, GrProcessorAnalysisCoverage::kNone, clip,
                                    nullptr, caps, clampType, &color);
    }
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override {}
    void onPrepare(GrOpFlushState* flushState) override {
        constexpr static SkPMColor4f kCyan = {0,1,1,1};
        auto alloc = flushState->allocator();
        const SkMatrix& shaderMatrix = SkMatrix::I();
        const SkMatrix& pathMatrix = fMatrix;
        const GrCaps& caps = flushState->caps();
        const GrShaderCaps& shaderCaps = *caps.shaderCaps();

        PathTessellator::PathDrawList pathList{pathMatrix, fPath, kCyan};
        if (fMode == Mode::kCurveMiddleOut) {
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
            // This emulates what PathStencilCoverOp does when using curves, except we include the
            // middle-out triangles directly in the written patches for convenience (normally they
            // use a simple triangle pipeline). But PathCurveTessellator only knows how to read
            // extra triangles from BreadcrumbTriangleList, so build on from the middle-out stack.
            SkArenaAlloc storage{256};
            GrInnerFanTriangulator::BreadcrumbTriangleList triangles;
            for (tess::PathMiddleOutFanIter it(fPath); !it.done();) {
                for (auto [p0, p1, p2] : it.nextStack()) {
                    triangles.append(&storage,
                                     pathMatrix.mapPoint(p0),
                                     pathMatrix.mapPoint(p1),
                                     pathMatrix.mapPoint(p2),
                                     /*winding=*/1);
                }
            }

            auto* tess = PathCurveTessellator::Make(alloc, shaderCaps.fInfinitySupport);
            tess->prepareWithTriangles(flushState, shaderMatrix, &triangles, pathList,
                                       fPath.countVerbs());
            fTessellator = tess;
#else
            auto* tess = PathCurveTessellator::Make(alloc, shaderCaps.fInfinitySupport);
            tess->prepareWithTriangles(flushState, shaderMatrix, nullptr, pathList,
                                       fPath.countVerbs());
            fTessellator = tess;
#endif
        } else {
            // This emulates what PathStencilCoverOp does when using wedges.
            fTessellator = PathWedgeTessellator::Make(alloc, shaderCaps.fInfinitySupport);
            fTessellator->prepare(flushState, shaderMatrix, pathList, fPath.countVerbs());
        }

        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState, std::move(fProcessors),
                                                                 fPipelineFlags);
        auto* tessShader = GrPathTessellationShader::Make(*caps.shaderCaps(),
                                                          alloc,
                                                          shaderMatrix,
                                                          kCyan,
                                                          fTessellator->patchAttribs());
        fProgram = GrTessellationShader::MakeProgram({alloc, flushState->writeView(),
                                                     flushState->usesMSAASurface(),
                                                     &flushState->dstProxyView(),
                                                     flushState->renderPassBarriers(),
                                                     GrLoadOp::kClear, &flushState->caps()},
                                                     tessShader,
                                                     pipeline,
                                                     &GrUserStencilSettings::kUnused);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        flushState->bindPipeline(*fProgram, chainBounds);
        fTessellator->draw(flushState);
    }

    const SkPath fPath;
    const SkMatrix fMatrix;
    const GrPipeline::InputFlags fPipelineFlags;
    const Mode fMode;
    PathTessellator* fTessellator = nullptr;
    GrProgramInfo* fProgram;
    GrProcessorSet fProcessors{SkBlendMode::kSrcOver};

    friend class GrOp;  // For ctor.
};

}  // namespace

// This slide enables wireframe and visualizes the triangles generated by path tessellators.
class PathTessellatorsSlide : public ClickHandlerSlide {
public:
    PathTessellatorsSlide() {
#if 0
        // For viewing middle-out triangulations of the inner fan.
        fPath.moveTo(1, 0);
        int numSides = 32 * 3;
        for (int i = 1; i < numSides; ++i) {
            float theta = 2*3.1415926535897932384626433832785 * i / numSides;
            fPath.lineTo(std::cos(theta), std::sin(theta));
        }
        fPath.transform(SkMatrix::Scale(200, 200));
        fPath.transform(SkMatrix::Translate(300, 300));
#else
        fPath.moveTo(100, 500);
        fPath.cubicTo(300, 400, -100, 300, 100, 200);
        fPath.quadTo(250, 0, 400, 200);
        fPath.conicTo(600, 350, 400, 500, fConicWeight);
        fPath.close();
#endif
        fName = "PathTessellators";
    }

    bool onChar(SkUnichar) override;

    void draw(SkCanvas*) override;

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Click*) override;

    SkPath fPath;
    GrPipeline::InputFlags fPipelineFlags = GrPipeline::InputFlags::kWireframe;
    Mode fMode = Mode::kWedgeMiddleOut;

    float fConicWeight = .5;

    class Click;
};

void PathTessellatorsSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    auto ctx = canvas->recordingContext();
    auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);

    SkString error;
    if (!sdc || !ctx) {
        error = "GPU Only.";
    } else if (!skgpu::ganesh::TessellationPathRenderer::IsSupported(*ctx->priv().caps())) {
        error = "TessellationPathRenderer not supported.";
    }
    if (!error.isEmpty()) {
        canvas->clear(SK_ColorRED);
        SkFont font(nullptr, 20);
        SkPaint captionPaint;
        captionPaint.setColor(SK_ColorWHITE);
        canvas->drawString(error.c_str(), 10, 30, font, captionPaint);
        return;
    }

    sdc->addDrawOp(GrOp::Make<SamplePathTessellatorOp>(ctx,
                                                       sdc->asRenderTargetProxy()->getBoundsRect(),
                                                       fPath, canvas->getTotalMatrix(),
                                                       fPipelineFlags, fMode));

    // Draw the path points.
    SkPaint pointsPaint;
    pointsPaint.setColor(SK_ColorBLUE);
    pointsPaint.setStrokeWidth(8);
    SkPath devPath = fPath;
    devPath.transform(canvas->getTotalMatrix());
    {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->setMatrix(SkMatrix::I());
        SkString caption(ModeName(fMode));
        caption.appendf(" (w=%g)", fConicWeight);
        SkFont font(nullptr, 20);
        SkPaint captionPaint;
        captionPaint.setColor(SK_ColorWHITE);
        canvas->drawString(caption, 10, 30, font, captionPaint);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, devPath.countPoints(),
                           SkPathPriv::PointData(devPath), pointsPaint);
    }
}

class PathTessellatorsSlide::Click : public ClickHandlerSlide::Click {
public:
    Click(int ptIdx) : fPtIdx(ptIdx) {}

    void doClick(SkPath* path) {
        SkPoint pt = path->getPoint(fPtIdx);
        SkPathPriv::UpdatePathPoint(path, fPtIdx, pt + fCurr - fPrev);
    }

private:
    int fPtIdx;
};

ClickHandlerSlide::Click* PathTessellatorsSlide::onFindClickHandler(SkScalar x, SkScalar y,
                                                                    skui::ModifierKey) {
    const SkPoint* pts = SkPathPriv::PointData(fPath);
    float fuzz = 30;
    for (int i = 0; i < fPath.countPoints(); ++i) {
        if (fabs(x - pts[i].x()) < fuzz && fabsf(y - pts[i].y()) < fuzz) {
            return new Click(i);
        }
    }
    return nullptr;
}

bool PathTessellatorsSlide::onClick(ClickHandlerSlide::Click* click) {
    Click* myClick = (Click*)click;
    myClick->doClick(&fPath);
    return true;
}

static SkPath update_weight(const SkPath& path, float w) {
    SkPath path_;
    for (auto [verb, pts, _] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                path_.moveTo(pts[0]);
                break;
            case SkPathVerb::kLine:
                path_.lineTo(pts[1]);
                break;
            case SkPathVerb::kQuad:
                path_.quadTo(pts[1], pts[2]);
                break;
            case SkPathVerb::kCubic:
                path_.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPathVerb::kConic:
                path_.conicTo(pts[1], pts[2], (w != 1) ? w : .99f);
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
    return path_;
}

bool PathTessellatorsSlide::onChar(SkUnichar unichar) {
    switch (unichar) {
        case 'w':
            fPipelineFlags = (GrPipeline::InputFlags)(
                    (int)fPipelineFlags ^ (int)GrPipeline::InputFlags::kWireframe);
            return true;
        case 'D': {
            fPath.dump();
            return true;
        }
        case '+':
            fConicWeight *= 2;
            fPath = update_weight(fPath, fConicWeight);
            return true;
        case '=':
            fConicWeight *= 5/4.f;
            fPath = update_weight(fPath, fConicWeight);
            return true;
        case '_':
            fConicWeight *= .5f;
            fPath = update_weight(fPath, fConicWeight);
            return true;
        case '-':
            fConicWeight *= 4/5.f;
            fPath = update_weight(fPath, fConicWeight);
            return true;
        case '1':
        case '2':
            fMode = (Mode)(unichar - '1');
            return true;
    }
    return false;
}

DEF_SLIDE( return new PathTessellatorsSlide; )

}  // namespace skgpu::ganesh

#endif  // defined(SK_GANESH)
