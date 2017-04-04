/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrConstColorProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class GLConstColorProcessor : public GrGLSLFragmentProcessor {
public:
    GLConstColorProcessor() : fPrevColor(GrColor4f::kIllegalConstructor) {}

    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const char* colorUni;
        fColorUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                         kVec4f_GrSLType, kMedium_GrSLPrecision,
                                                         "constantColor",
                                                         &colorUni);
        GrConstColorProcessor::InputMode mode = args.fFp.cast<GrConstColorProcessor>().inputMode();
        if (!args.fInputColor) {
            mode = GrConstColorProcessor::kIgnore_InputMode;
        }
        switch (mode) {
            case GrConstColorProcessor::kIgnore_InputMode:
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, colorUni);
                break;
            case GrConstColorProcessor::kModulateRGBA_InputMode:
                fragBuilder->codeAppendf("%s = %s * %s;", args.fOutputColor, args.fInputColor,
                                       colorUni);
                break;
            case GrConstColorProcessor::kModulateA_InputMode:
                fragBuilder->codeAppendf("%s = %s.a * %s;", args.fOutputColor, args.fInputColor,
                                       colorUni);
                break;
        }
    }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdm,
                   const GrFragmentProcessor& processor) override {
        GrColor4f color = processor.cast<GrConstColorProcessor>().color();
        // We use the "illegal" color value as an uninit sentinel. With GrColor4f, the "illegal"
        // color is *really* illegal (not just unpremultiplied), so this check is simple.
        if (fPrevColor != color) {
            pdm.set4fv(fColorUniform, 1, color.fRGBA);
            fPrevColor = color;
        }
    }

private:
    GrGLSLProgramDataManager::UniformHandle fColorUniform;
    GrColor4f                               fPrevColor;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrColor4f GrConstColorProcessor::constantOutputForConstantInput(GrColor4f input) const {
    switch (fMode) {
        case kIgnore_InputMode:
            return fColor;
        case kModulateA_InputMode:
            return fColor.mulByScalar(input.fRGBA[3]);
        case kModulateRGBA_InputMode:
            return fColor.modulate(input);
    }
    SkFAIL("Unexpected mode");
    return GrColor4f::TransparentBlack();
}

void GrConstColorProcessor::onGetGLSLProcessorKey(const GrShaderCaps&,
                                                  GrProcessorKeyBuilder* b) const {
    b->add32(fMode);
}

GrGLSLFragmentProcessor* GrConstColorProcessor::onCreateGLSLInstance() const  {
    return new GLConstColorProcessor;
}

bool GrConstColorProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConstColorProcessor& that = other.cast<GrConstColorProcessor>();
    return fMode == that.fMode && fColor == that.fColor;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConstColorProcessor);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrConstColorProcessor::TestCreate(GrProcessorTestData* d) {
    GrColor4f color;
    int colorPicker = d->fRandom->nextULessThan(3);
    switch (colorPicker) {
        case 0: {
            uint32_t a = d->fRandom->nextULessThan(0x100);
            uint32_t r = d->fRandom->nextULessThan(a+1);
            uint32_t g = d->fRandom->nextULessThan(a+1);
            uint32_t b = d->fRandom->nextULessThan(a+1);
            color = GrColor4f::FromGrColor(GrColorPackRGBA(r, g, b, a));
            break;
        }
        case 1:
            color = GrColor4f::TransparentBlack();
            break;
        case 2:
            uint32_t c = d->fRandom->nextULessThan(0x100);
            color = GrColor4f::FromGrColor(c | (c << 8) | (c << 16) | (c << 24));
            break;
    }
    InputMode mode = static_cast<InputMode>(d->fRandom->nextULessThan(kInputModeCnt));
    return GrConstColorProcessor::Make(color, mode);
}
#endif
