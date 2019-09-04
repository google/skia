/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/effects/SkColorFixFilter.h"
#include "include/private/SkColorData.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/GrColorSpaceInfo.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

// using InvertStyle = SkColorFixConfig::InvertStyle;

class SkColorFix_Filter : public SkColorFilter {
public:
    SkColorFix_Filter(const SkColorFixConfig& config) { fConfig = config; }

    ~SkColorFix_Filter() override {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorFix_Filter)

    SkColorFixConfig fConfig;

    friend class SkColorFixFilter;

    typedef SkColorFilter INHERITED;
};

bool SkColorFix_Filter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;

    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::unpremul);
    }

    p->append(SkRasterPipeline::rgb_to_hsl);

    struct Ctx : public SkRasterPipeline_CallbackCtx {
        float R;
        float G;
    };
    // TODO: do we care about transforming to dstCS?
    auto ctx = rec.fAlloc->make<Ctx>();
    ctx->R = fConfig.fR;
    ctx->G = fConfig.fG;
    ctx->fn = [](SkRasterPipeline_CallbackCtx* arg, int active_pixels) {
        auto ctx = (Ctx*)arg;
        auto pixels = (SkPMColor4f*)ctx->rgba;
        for (int i = 0; i < active_pixels; i++) {
            float prevx = 1.0;
            float prevy = 1.0;
            float cury = 0.0;
            float curx = 0.0;

            if (pixels[i].fR < prevx) {
                curx = prevx;
                cury = prevy;
                prevx = 0.36;
                prevy = 0.36;
            }
            if (pixels[i].fR < prevx) {
                curx = prevx;
                cury = prevy;
                prevx = 0.199;
                prevy = ctx->G;
            }
            if (pixels[i].fR < prevx) {
                curx = prevx;
                cury = prevy;
                prevx = 0.1389;
                prevy = ctx->R;
            }
            if (pixels[i].fR < prevx) {
                curx = prevx;
                cury = prevy;
                prevx = 0.0;
                prevy = 0.0;
            }
            float coeff = (pixels[i].fR - prevx) / (curx - prevx);
            pixels[i].fR = (coeff * (cury - prevy) + prevy);
            prevx = 0.0;
            prevy = 0.0;
            curx = 0.2;
            cury = 0.4;
            if (pixels[i].fG > curx) {
                prevx = curx;
                prevy = cury;
                curx = 1.0;
                cury = 0.99;
            }
            coeff = (pixels[i].fG - prevx) / (curx - prevx);
            pixels[i].fG = (coeff * (cury - prevy) + prevy);
        }
    };
    rec.fPipeline->append(SkRasterPipeline::callback, ctx);

    p->append(SkRasterPipeline::hsl_to_rgb);

    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::premul);
    }
    return true;
}

void SkColorFix_Filter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fConfig.fR);
    buffer.writeScalar(fConfig.fG);
}

sk_sp<SkFlattenable> SkColorFix_Filter::CreateProc(SkReadBuffer& buffer) {
    SkColorFixConfig config;
    config.fR = buffer.readScalar();
    config.fG = buffer.readScalar();
    return SkColorFixFilter::Make(config);
}

sk_sp<SkColorFilter> SkColorFixFilter::Make(const SkColorFixConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }
    return sk_make_sp<SkColorFix_Filter>(config);
}

void SkColorFixFilter::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkColorFix_Filter); }

#if SK_SUPPORT_GPU
class ColorFixFilterEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const SkColorFixConfig& config) {
        return std::unique_ptr<GrFragmentProcessor>(new ColorFixFilterEffect(config));
    }

    const char* name() const override { return "ColorFixFilter"; }

    const SkColorFixConfig& config() const { return fConfig; }

    std::unique_ptr<GrFragmentProcessor> clone() const override { return Make(fConfig); }

private:
    ColorFixFilterEffect(const SkColorFixConfig& config)
            : INHERITED(kColorFixFilterEffect_ClassID, kNone_OptimizationFlags), fConfig(config) {}

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const ColorFixFilterEffect& that = other.cast<ColorFixFilterEffect>();
        return that.fConfig.fR == fConfig.fR && that.fConfig.fG == fConfig.fG;
    }

    SkColorFixConfig fConfig;

    typedef GrFragmentProcessor INHERITED;
};

class GLColorFixFilterEffect : public GrGLSLFragmentProcessor {
public:
    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;
    void emitCode(EmitArgs& args) override;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

GrGLSLFragmentProcessor* ColorFixFilterEffect::onCreateGLSLInstance() const {
    return new GLColorFixFilterEffect();
}

void ColorFixFilterEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                 GrProcessorKeyBuilder* b) const {
    GLColorFixFilterEffect::GenKey(*this, caps, b);
}

void GLColorFixFilterEffect::onSetData(const GrGLSLProgramDataManager& pdm,
                                       const GrFragmentProcessor& proc) {
    const ColorFixFilterEffect& hcfe = proc.cast<ColorFixFilterEffect>();
}

void GLColorFixFilterEffect::GenKey(const GrProcessor& proc, const GrShaderCaps&,
                                    GrProcessorKeyBuilder* b) {
    const ColorFixFilterEffect& hcfe = proc.cast<ColorFixFilterEffect>();
    b->add32(static_cast<uint32_t>(hcfe.config().fR));
    b->add32(static_cast<uint32_t>(hcfe.config().fG));
}

void GLColorFixFilterEffect::emitCode(EmitArgs& args) {
    const ColorFixFilterEffect& hcfe = args.fFp.cast<ColorFixFilterEffect>();
    const SkColorFixConfig& config = hcfe.config();
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->codeAppendf("half4 color = %s;", args.fInputColor);
    fragBuilder->codeAppendf("%s = color;", args.fOutputColor);
}

std::unique_ptr<GrFragmentProcessor> SkColorFix_Filter::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo& csi) const {
    return ColorFixFilterEffect::Make(fConfig);
}
#endif
