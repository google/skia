/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrConstColorProcessor.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class GLConstColorProcessor : public GrGLSLFragmentProcessor {
public:
    GLConstColorProcessor() : fPrevColor(GrColor_ILLEGAL) {}

    void emitCode(EmitArgs& args) override {
        GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
        const char* colorUni;
        fColorUniform = args.fUniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
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
    void onSetData(const GrGLSLProgramDataManager& pdm, const GrProcessor& processor) override {
        GrColor color = processor.cast<GrConstColorProcessor>().color();
        // We use the "illegal" color value as an uninit sentinel. However, ut isn't inherently
        // illegal to use this processor with unpremul colors. So we correctly handle the case
        // when the "illegal" color is used but we will always upload it.
        if (GrColor_ILLEGAL == color || fPrevColor != color) {
            static const float scale = 1.f / 255.f;
            float floatColor[4] = {
                GrColorUnpackR(color) * scale,
                GrColorUnpackG(color) * scale,
                GrColorUnpackB(color) * scale,
                GrColorUnpackA(color) * scale,
            };
            pdm.set4fv(fColorUniform, 1, floatColor);
            fPrevColor = color;
        }
    }

private:
    GrGLSLProgramDataManager::UniformHandle fColorUniform;
    GrColor                               fPrevColor;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void GrConstColorProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (kIgnore_InputMode == fMode) {
        inout->setToOther(kRGBA_GrColorComponentFlags, fColor,
                          GrInvariantOutput::kWillNot_ReadInput);
    } else {
        GrColor r = GrColorUnpackR(fColor);
        bool colorIsSingleChannel = r == GrColorUnpackG(fColor) && r == GrColorUnpackB(fColor) &&
                                    r == GrColorUnpackA(fColor);
        if (kModulateRGBA_InputMode == fMode) {
            if (colorIsSingleChannel) {
                inout->mulByKnownSingleComponent(r);
            } else {
                inout->mulByKnownFourComponents(fColor);
            }
        } else {
            if (colorIsSingleChannel) {
                inout->mulAlphaByKnownSingleComponent(r);
            } else {
                inout->mulAlphaByKnownFourComponents(fColor);
            }
        }
    }
}

void GrConstColorProcessor::onGetGLSLProcessorKey(const GrGLSLCaps&,
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

const GrFragmentProcessor* GrConstColorProcessor::TestCreate(GrProcessorTestData* d) {
    GrColor color;
    int colorPicker = d->fRandom->nextULessThan(3);
    switch (colorPicker) {
        case 0: {
            uint32_t a = d->fRandom->nextULessThan(0x100);
            uint32_t r = d->fRandom->nextULessThan(a+1);
            uint32_t g = d->fRandom->nextULessThan(a+1);
            uint32_t b = d->fRandom->nextULessThan(a+1);
            color = GrColorPackRGBA(r, g, b, a);
            break;
        }
        case 1:
            color = 0;
            break;
        case 2:
            color = d->fRandom->nextULessThan(0x100);
            color = color | (color << 8) | (color << 16) | (color << 24);
            break;
    }
    InputMode mode = static_cast<InputMode>(d->fRandom->nextULessThan(kInputModeCnt));
    return GrConstColorProcessor::Create(color, mode);
}
