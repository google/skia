/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {

class DestColorTestFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child) {
        return std::unique_ptr<GrFragmentProcessor>(new DestColorTestFP(std::move(child)));
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new DestColorTestFP(*this));
    }

private:
    DestColorTestFP(std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child));
    }

    explicit DestColorTestFP(const DestColorTestFP& that)
            : INHERITED(that) {}

    const char* name() const override { return "DestColorTestFP"; }
    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
            void emitCode(EmitArgs& args) override {
                SkString result = this->invokeChild(0, args);
                args.fFragBuilder->codeAppendf("return (half4(1) - (%s)).rgb1;", result.c_str());
            }
        };

        return std::make_unique<Impl>();
    }

    using INHERITED = GrFragmentProcessor;
};

}  // namespace

namespace skiagm {

DEF_SIMPLE_GPU_GM_CAN_FAIL(destcolor, rContext, canvas, errorMsg, 640, 640) {
    SkRect bounds = SkRect::MakeIWH(512, 512);

    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    // Draw the mandrill.
    SkPaint p;
    p.setImageFilter(SkImageFilters::Image(GetResourceAsImage("images/mandrill_512.png"),
                                           bounds, bounds, SkSamplingOptions()));
    canvas->drawPaint(p);

    // Now let's add our test FP on top. It reads back the original image and inverts it.
    GrPaint invertPaint;
    invertPaint.setColor4f(SK_PMColor4fWHITE);
    invertPaint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    invertPaint.setColorFragmentProcessor(
            DestColorTestFP::Make(GrFragmentProcessor::SurfaceColor()));
    sdc->drawOval(/*clip*/ nullptr, std::move(invertPaint), GrAA::kYes, SkMatrix::I(),
                  SkRect::MakeLTRB(128, 128, 640, 640), GrStyle::SimpleFill());

    return DrawResult::kOk;
}

} // namespace skiagm
