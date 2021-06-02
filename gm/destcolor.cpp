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
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrFillRectOp.h"
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
            : INHERITED(kTestFP_ClassID, that.optimizationFlags()) {
        this->cloneAndRegisterAllChildProcessors(that);
    }

    const char* name() const override { return "DestColorTestFP"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    class Impl : public GrGLSLFragmentProcessor {
        void emitCode(EmitArgs& args) override {
            SkString result = this->invokeChild(0, args);
            args.fFragBuilder->codeAppendf("return (half4(1) - (%s)).rgb1;", result.c_str());
        }
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& processor) override {
        }
    };

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        return std::make_unique<Impl>();
    }

    using INHERITED = GrFragmentProcessor;
};

}  // namespace

DEF_SIMPLE_GPU_GM(destcolor, ctx, rtCtx, canvas, 512, 512) {
    // This is copied from the Swizzle GM (replacing the color swizzle part with RGB).
    SkRect bounds = SkRect::MakeIWH(512, 512);

    SkBitmap bmp;
    GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bmp);
    auto view = std::get<0>(GrMakeCachedBitmapProxyView(ctx, bmp, GrMipmapped::kNo));
    if (!view) {
        return;
    }
    std::unique_ptr<GrFragmentProcessor> imgFP =
        GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
    auto fp = GrFragmentProcessor::SwizzleOutput(std::move(imgFP), GrSwizzle("rgb1"));

    GrPaint grPaint;
    grPaint.setColorFragmentProcessor(std::move(fp));

    rtCtx->addDrawOp(GrFillRectOp::MakeNonAARect(ctx, std::move(grPaint), SkMatrix(), bounds));

    // Now let's add a new FP on top. It paints purple.
    GrPaint purplePaint;
    purplePaint.setColor4f(SK_PMColor4fWHITE);
    purplePaint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    purplePaint.setColorFragmentProcessor(DestColorTestFP::Make(GrFragmentProcessor::DestColor()));
    rtCtx->addDrawOp(GrFillRectOp::MakeNonAARect(ctx, std::move(purplePaint), SkMatrix(),
                                                 SkRect::MakeLTRB(128, 128, 640, 640)));
}
