/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkOverdrawColorFilter.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "../jumper/SkJumper.h"

void SkOverdrawColorFilter::onAppendStages(SkRasterPipeline* p,
                                           SkColorSpace* dstCS,
                                           SkArenaAlloc* alloc,
                                           bool shader_is_opaque) const {
    struct Ctx : public SkJumper_CallbackCtx {
        const SkPMColor* colors;
    };
    // TODO: do we care about transforming to dstCS?
    auto ctx = alloc->make<Ctx>();
    ctx->colors = fColors;
    ctx->fn = [](SkJumper_CallbackCtx* arg, int active_pixels) {
        auto ctx = (Ctx*)arg;
        auto pixels = (SkPM4f*)ctx->rgba;
        for (int i = 0; i < active_pixels; i++) {
            uint8_t alpha = (int)(pixels[i].a() * 255);
            if (alpha >= kNumColors) {
                alpha = kNumColors - 1;
            }
            pixels[i] = SkPM4f::FromPMColor(ctx->colors[alpha]);
        }
    };
    p->append(SkRasterPipeline::callback, ctx);
}

void SkOverdrawColorFilter::toString(SkString* str) const {
    str->append("SkOverdrawColorFilter (");
    for (int i = 0; i < kNumColors; i++) {
        str->appendf("%d: %x\n", i, fColors[i]);
    }
    str->append(")");
}

void SkOverdrawColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeByteArray(fColors, kNumColors * sizeof(SkPMColor));
}

sk_sp<SkFlattenable> SkOverdrawColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkPMColor colors[kNumColors];
    size_t size = buffer.getArrayCount();
    if (!buffer.validate(size == sizeof(colors))) {
        return nullptr;
    }
    if (!buffer.readByteArray(colors, sizeof(colors))) {
        return nullptr;
    }

    return SkOverdrawColorFilter::Make(colors);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkOverdrawColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOverdrawColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

#if SK_SUPPORT_GPU

#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class OverdrawFragmentProcessor : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(const SkPMColor* colors);

    const char* name() const override { return "Overdraw"; }
private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override;

    OverdrawFragmentProcessor(const GrColor4f* colors);

    GrColor4f fColors[SkOverdrawColorFilter::kNumColors];

    typedef GrFragmentProcessor INHERITED;
};

class GLOverdrawFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GLOverdrawFragmentProcessor(const GrColor4f* colors);

    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override {}

private:
    GrColor4f fColors[SkOverdrawColorFilter::kNumColors];

    typedef GrGLSLFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> SkOverdrawColorFilter::asFragmentProcessor(GrContext*,
                                                                      SkColorSpace*) const {
    return OverdrawFragmentProcessor::Make(fColors);
}

sk_sp<GrFragmentProcessor> OverdrawFragmentProcessor::Make(const SkPMColor* colors) {
    GrColor4f grColors[SkOverdrawColorFilter::kNumColors];
    for (int i = 0; i < SkOverdrawColorFilter::kNumColors; i++) {
        grColors[i] = GrColor4f::FromGrColor(GrColorPackRGBA(SkGetPackedR32(colors[i]),
                                                             SkGetPackedG32(colors[i]),
                                                             SkGetPackedB32(colors[i]),
                                                             SkGetPackedA32(colors[i])));
    }

    return sk_sp<OverdrawFragmentProcessor>(new OverdrawFragmentProcessor(grColors));
}

// This could implement the constant input -> constant output optimization, but we don't really
// care given how this is used.
OverdrawFragmentProcessor::OverdrawFragmentProcessor(const GrColor4f* colors)
        : INHERITED(kNone_OptimizationFlags) {
    this->initClassID<OverdrawFragmentProcessor>();
    memcpy(fColors, colors, SkOverdrawColorFilter::kNumColors * sizeof(GrColor4f));
}

GrGLSLFragmentProcessor* OverdrawFragmentProcessor::onCreateGLSLInstance() const {
    return new GLOverdrawFragmentProcessor(fColors);
}

bool OverdrawFragmentProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const OverdrawFragmentProcessor& that = other.cast<OverdrawFragmentProcessor>();
    return 0 == memcmp(fColors, that.fColors,
                       sizeof(GrColor4f) * SkOverdrawColorFilter::kNumColors);
}

GLOverdrawFragmentProcessor::GLOverdrawFragmentProcessor(const GrColor4f* colors) {
    memcpy(fColors, colors, SkOverdrawColorFilter::kNumColors * sizeof(GrColor4f));
}

void GLOverdrawFragmentProcessor::emitCode(EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    if (nullptr == args.fInputColor) {
        fragBuilder->codeAppendf("%s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                    fColors[5].fRGBA[0],
                                                                    fColors[5].fRGBA[1],
                                                                    fColors[5].fRGBA[2],
                                                                    fColors[5].fRGBA[3]);
    } else {
        fragBuilder->codeAppendf("float alpha = 255.0 * %s.a;", args.fInputColor);
        fragBuilder->codeAppendf("if (alpha < 0.5) {");
        fragBuilder->codeAppendf("    %s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                        fColors[0].fRGBA[0],
                                                                        fColors[0].fRGBA[1],
                                                                        fColors[0].fRGBA[2],
                                                                        fColors[0].fRGBA[3]);
        fragBuilder->codeAppendf("} else if (alpha < 1.5) {");
        fragBuilder->codeAppendf("    %s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                        fColors[1].fRGBA[0],
                                                                        fColors[1].fRGBA[1],
                                                                        fColors[1].fRGBA[2],
                                                                        fColors[1].fRGBA[3]);
        fragBuilder->codeAppendf("} else if (alpha < 2.5) {");
        fragBuilder->codeAppendf("    %s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                        fColors[2].fRGBA[0],
                                                                        fColors[2].fRGBA[1],
                                                                        fColors[2].fRGBA[2],
                                                                        fColors[2].fRGBA[3]);
        fragBuilder->codeAppendf("} else if (alpha < 3.5) {");
        fragBuilder->codeAppendf("    %s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                        fColors[3].fRGBA[0],
                                                                        fColors[3].fRGBA[1],
                                                                        fColors[3].fRGBA[2],
                                                                        fColors[3].fRGBA[3]);
        fragBuilder->codeAppendf("} else if (alpha < 4.5) {");
        fragBuilder->codeAppendf("    %s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                        fColors[4].fRGBA[0],
                                                                        fColors[4].fRGBA[1],
                                                                        fColors[4].fRGBA[2],
                                                                        fColors[4].fRGBA[3]);
        fragBuilder->codeAppendf("} else {");
        fragBuilder->codeAppendf("    %s.rgba = vec4(%f, %f, %f, %f);", args.fOutputColor,
                                                                        fColors[5].fRGBA[0],
                                                                        fColors[5].fRGBA[1],
                                                                        fColors[5].fRGBA[2],
                                                                        fColors[5].fRGBA[3]);
        fragBuilder->codeAppendf("}");
    }
}

#endif
