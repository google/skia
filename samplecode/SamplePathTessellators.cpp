/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "samplecode/Sample.h"
#include "src/core/SkPathPriv.h"

#if SK_SUPPORT_GPU

#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrPathCurveTessellator.h"
#include "src/gpu/tessellate/GrPathWedgeTessellator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

namespace {

enum class Mode {
    kWedgeMiddleOut,
    kCurveMiddleOut,
    kWedgeTessellate,
    kCurveTessellate
};

static const char* ModeName(Mode mode) {
    switch (mode) {
        case Mode::kWedgeMiddleOut:
            return "MiddleOutShader (kWedges)";
        case Mode::kCurveMiddleOut:
            return "MiddleOutShader (kCurves)";
        case Mode::kWedgeTessellate:
            return "HardwareWedgeShader";
        case Mode::kCurveTessellate:
            return "HardwareCurveShader";
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
        int numVerbsToGetMiddleOut = 0;
        int numVerbsToGetTessellation = caps.minPathVerbsForHwTessellation();
        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState, std::move(fProcessors),
                                                                 fPipelineFlags);
        switch (fMode) {
            using DrawInnerFan = GrPathCurveTessellator::DrawInnerFan;
            case Mode::kWedgeMiddleOut:
                fTessellator = GrPathWedgeTessellator::Make(alloc, shaderMatrix, kCyan,
                                                            numVerbsToGetMiddleOut, *pipeline,
                                                            caps);
                break;
            case Mode::kCurveMiddleOut:
                fTessellator = GrPathCurveTessellator::Make(alloc, shaderMatrix, kCyan,
                                                            DrawInnerFan::kYes,
                                                            numVerbsToGetMiddleOut, *pipeline,
                                                            caps);
                break;
            case Mode::kWedgeTessellate:
                fTessellator = GrPathWedgeTessellator::Make(alloc, shaderMatrix, kCyan,
                                                            numVerbsToGetTessellation, *pipeline,
                                                            caps);
                break;
            case Mode::kCurveTessellate:
                fTessellator = GrPathCurveTessellator::Make(alloc, shaderMatrix, kCyan,
                                                            DrawInnerFan::kYes,
                                                            numVerbsToGetTessellation, *pipeline,
                                                            caps);
                break;
        }
        fTessellator->prepare(flushState, this->bounds(), {pathMatrix, fPath}, fPath.countVerbs());
        fProgram = GrTessellationShader::MakeProgram({alloc, flushState->writeView(),
                                                     &flushState->dstProxyView(),
                                                     flushState->renderPassBarriers(),
                                                     GrLoadOp::kClear, &flushState->caps()},
                                                     fTessellator->shader(), pipeline,
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
    GrPathTessellator* fTessellator = nullptr;
    GrProgramInfo* fProgram;
    GrProcessorSet fProcessors{SkBlendMode::kSrcOver};

    friend class GrOp;  // For ctor.
};

}  // namespace

// This sample enables wireframe and visualizes the triangles generated by path tessellators.
class SamplePathTessellators : public Sample {
public:
    SamplePathTessellators() {
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
    }

private:
    void onDrawContent(SkCanvas*) override;
    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Sample::Click*) override;
    bool onChar(SkUnichar) override;

    SkString name() override { return SkString("PathTessellators"); }

    SkPath fPath;
    GrPipeline::InputFlags fPipelineFlags = GrPipeline::InputFlags::kHWAntialias |
                                            GrPipeline::InputFlags::kWireframe;
    Mode fMode = Mode::kWedgeMiddleOut;

    float fConicWeight = .5;

    class Click;
};

void SamplePathTessellators::onDrawContent(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    auto ctx = canvas->recordingContext();
    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);

    SkString error;
    if (!sdc || !ctx) {
        error = "GPU Only.";
    } else if (!GrTessellationPathRenderer::IsSupported(*ctx->priv().caps())) {
        error = "GrTessellationPathRenderer not supported.";
    } else if (fMode >= Mode::kWedgeTessellate &&
               !ctx->priv().caps()->shaderCaps()->tessellationSupport()) {
        error.printf("%s requires hardware tessellation support.", ModeName(fMode));
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

class SamplePathTessellators::Click : public Sample::Click {
public:
    Click(int ptIdx) : fPtIdx(ptIdx) {}

    void doClick(SkPath* path) {
        SkPoint pt = path->getPoint(fPtIdx);
        SkPathPriv::UpdatePathPoint(path, fPtIdx, pt + fCurr - fPrev);
    }

private:
    int fPtIdx;
};

Sample::Click* SamplePathTessellators::onFindClickHandler(SkScalar x, SkScalar y,
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

bool SamplePathTessellators::onClick(Sample::Click* click) {
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

bool SamplePathTessellators::onChar(SkUnichar unichar) {
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
        case '3':
        case '4':
            fMode = (Mode)(unichar - '1');
            return true;
    }
    return false;
}

Sample* MakeTessellatedPathSample() { return new SamplePathTessellators; }
static SampleRegistry gTessellatedPathSample(MakeTessellatedPathSample);

#endif  // SK_SUPPORT_GPU
