/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrConstColorProcessor.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/builders/GrGLProgramBuilder.h"

class GLConstColorProcessor : public GrGLFragmentProcessor {
public:
    GLConstColorProcessor() : fPrevColor(GrColor_ILLEGAL) {}

    void emitCode(GrGLFPBuilder* builder,
                  const GrFragmentProcessor& fp,
                  const char* outputColor,
                  const char* inputColor,
                  const TransformedCoordsArray& coords,
                  const TextureSamplerArray& samplers) override {
        GrGLFragmentBuilder* fsBuilder = builder->getFragmentShaderBuilder();
        const char* colorUni;
        fColorUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                            kVec4f_GrSLType, kMedium_GrSLPrecision, "constantColor",
                                            &colorUni);
        switch (fp.cast<GrConstColorProcessor>().inputMode()) {
            case GrConstColorProcessor::kIgnore_InputMode:
                fsBuilder->codeAppendf("%s = %s;", outputColor, colorUni);
                break;
            case GrConstColorProcessor::kModulateRGBA_InputMode:
                fsBuilder->codeAppendf("%s = %s * %s;", outputColor, inputColor, colorUni);
                break;
            case GrConstColorProcessor::kModulateA_InputMode:
                fsBuilder->codeAppendf("%s = %s.a * %s;", outputColor, inputColor, colorUni);
                break;
        }
    }

    void setData(const GrGLProgramDataManager& pdm, const GrProcessor& processor) override {
        GrColor color = processor.cast<GrConstColorProcessor>().color();
        // We use the "illegal" color value as an uninit sentinel. However, ut isn't inherently
        // illegal to use this processor with unpremul colors. So we correctly handle the case
        // when the "illegal" color is used but we will always upload it.
        if (GrColor_ILLEGAL == color || fPrevColor != color) {
            static const GrGLfloat scale = 1.f / 255.f;
            GrGLfloat floatColor[4] = {
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
    GrGLProgramDataManager::UniformHandle fColorUniform;
    GrColor                               fPrevColor;

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void GrConstColorProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (kIgnore_InputMode == fMode) {
        inout->setToOther(kRGBA_GrColorComponentFlags, fColor, GrInvariantOutput::kWill_ReadInput);
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

void GrConstColorProcessor::getGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder* b) const {
    b->add32(fMode);
}

GrGLFragmentProcessor* GrConstColorProcessor::createGLInstance() const  {
    return SkNEW(GLConstColorProcessor);
}

bool GrConstColorProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConstColorProcessor& that = other.cast<GrConstColorProcessor>();
    return fMode == that.fMode && fColor == that.fColor;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConstColorProcessor);

GrFragmentProcessor* GrConstColorProcessor::TestCreate(SkRandom* random,
                                                       GrContext*,
                                                       const GrDrawTargetCaps&,
                                                       GrTexture*[]) {
    GrColor color;
    int colorPicker = random->nextULessThan(3);
    switch (colorPicker) {
        case 0: {
            uint32_t a = random->nextULessThan(0x100);
            uint32_t r = random->nextULessThan(a+1);
            uint32_t g = random->nextULessThan(a+1);
            uint32_t b = random->nextULessThan(a+1);
            color = GrColorPackRGBA(r, g, b, a);
            break;
        }
        case 1:
            color = 0;
            break;
        case 2:
            color = random->nextULessThan(0x100);
            color = color | (color << 8) | (color << 16) | (color << 24);
            break;
    }
    InputMode mode = static_cast<InputMode>(random->nextULessThan(kInputModeCnt));
    return GrConstColorProcessor::Create(color, mode);
}
