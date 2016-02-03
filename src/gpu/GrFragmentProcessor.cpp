
/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrProcOptInfo.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

GrFragmentProcessor::~GrFragmentProcessor() {
    // If we got here then our ref count must have reached zero, so we will have converted refs
    // to pending executions for all children.
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        fChildProcessors[i]->completedExecution();
    }
}

bool GrFragmentProcessor::isEqual(const GrFragmentProcessor& that,
                                  bool ignoreCoordTransforms) const {
    if (this->classID() != that.classID() ||
        !this->hasSameTextureAccesses(that)) {
        return false;
    }
    if (ignoreCoordTransforms) {
        if (this->numTransforms() != that.numTransforms()) {
            return false;
        }
    } else if (!this->hasSameTransforms(that)) {
        return false;
    }
    if (!this->onIsEqual(that)) {
        return false;
    }
    if (this->numChildProcessors() != that.numChildProcessors()) {
        return false;
    }
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).isEqual(that.childProcessor(i), ignoreCoordTransforms)) {
            return false;
        }
    }
    return true;
}

GrGLSLFragmentProcessor* GrFragmentProcessor::createGLSLInstance() const {
    GrGLSLFragmentProcessor* glFragProc = this->onCreateGLSLInstance();
    glFragProc->fChildProcessors.push_back_n(fChildProcessors.count());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        glFragProc->fChildProcessors[i] = fChildProcessors[i]->createGLSLInstance();
    }
    return glFragProc;
}

void GrFragmentProcessor::addTextureAccess(const GrTextureAccess* textureAccess) {
    // Can't add texture accesses after registering any children since their texture accesses have
    // already been bubbled up into our fTextureAccesses array
    SkASSERT(fChildProcessors.empty());

    INHERITED::addTextureAccess(textureAccess);
    fNumTexturesExclChildren++;
}

void GrFragmentProcessor::addCoordTransform(const GrCoordTransform* transform) {
    // Can't add transforms after registering any children since their transforms have already been
    // bubbled up into our fCoordTransforms array
    SkASSERT(fChildProcessors.empty());

    fCoordTransforms.push_back(transform);
    fUsesLocalCoords = fUsesLocalCoords || transform->sourceCoords() == kLocal_GrCoordSet;
    SkDEBUGCODE(transform->setInProcessor();)
    fNumTransformsExclChildren++;
}

int GrFragmentProcessor::registerChildProcessor(const GrFragmentProcessor* child) {
    // Append the child's transforms to our transforms array and the child's textures array to our
    // textures array
    if (!child->fCoordTransforms.empty()) {
        fCoordTransforms.push_back_n(child->fCoordTransforms.count(),
                                     child->fCoordTransforms.begin());
    }
    if (!child->fTextureAccesses.empty()) {
        fTextureAccesses.push_back_n(child->fTextureAccesses.count(),
                                     child->fTextureAccesses.begin());
    }

    int index = fChildProcessors.count();
    fChildProcessors.push_back(SkRef(child));

    if (child->willReadFragmentPosition()) {
        this->setWillReadFragmentPosition();
    }

    if (child->usesLocalCoords()) {
        fUsesLocalCoords = true;
    }

    return index;
}

void GrFragmentProcessor::notifyRefCntIsZero() const {
    // See comment above GrProgramElement for a detailed explanation of why we do this.
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        fChildProcessors[i]->addPendingExecution();
        fChildProcessors[i]->unref();
    }
}

bool GrFragmentProcessor::hasSameTransforms(const GrFragmentProcessor& that) const {
    if (this->numTransforms() != that.numTransforms()) {
        return false;
    }
    int count = this->numTransforms();
    for (int i = 0; i < count; ++i) {
        if (this->coordTransform(i) != that.coordTransform(i)) {
            return false;
        }
    }
    return true;
}

const GrFragmentProcessor* GrFragmentProcessor::MulOutputByInputAlpha(
    const GrFragmentProcessor* fp) {
    if (!fp) {
        return nullptr;
    }
    return GrXfermodeFragmentProcessor::CreateFromDstProcessor(fp, SkXfermode::kDstIn_Mode);
}

const GrFragmentProcessor* GrFragmentProcessor::MulOutputByInputUnpremulColor(
    const GrFragmentProcessor* fp) {

    class PremulFragmentProcessor : public GrFragmentProcessor {
    public:
        PremulFragmentProcessor(const GrFragmentProcessor* processor) {
            this->initClassID<PremulFragmentProcessor>();
            this->registerChildProcessor(processor);
        }

        const char* name() const override { return "Premultiply"; }

    private:
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                GLFP() {}

                void emitCode(EmitArgs& args) override {
                    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
                    this->emitChild(0, nullptr, args);
                    fragBuilder->codeAppendf("%s.rgb *= %s.rgb;", args.fOutputColor,
                                                                args.fInputColor);
                    fragBuilder->codeAppendf("%s *= %s.a;", args.fOutputColor, args.fInputColor);
                }
            };
            return new GLFP;
        }

        void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            // TODO: Add a helper to GrInvariantOutput that handles multiplying by color with flags?
            if (!(inout->validFlags() & kA_GrColorComponentFlag)) {
                inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
                return;
            }

            GrInvariantOutput childOutput(GrColor_WHITE, kRGBA_GrColorComponentFlags, false);
            this->childProcessor(0).computeInvariantOutput(&childOutput);

            if (0 == GrColorUnpackA(inout->color()) || 0 == GrColorUnpackA(childOutput.color())) {
                inout->mulByKnownFourComponents(0x0);
                return;
            }
            GrColorComponentFlags commonFlags = childOutput.validFlags() & inout->validFlags();
            GrColor c0 = GrPremulColor(inout->color());
            GrColor c1 = childOutput.color();
            GrColor color = 0x0;
            if (commonFlags & kR_GrColorComponentFlag) {
                color |= SkMulDiv255Round(GrColorUnpackR(c0), GrColorUnpackR(c1)) <<
                    GrColor_SHIFT_R;
            }
            if (commonFlags & kG_GrColorComponentFlag) {
                color |= SkMulDiv255Round(GrColorUnpackG(c0), GrColorUnpackG(c1)) <<
                    GrColor_SHIFT_G;
            }
            if (commonFlags & kB_GrColorComponentFlag) {
                color |= SkMulDiv255Round(GrColorUnpackB(c0), GrColorUnpackB(c1)) <<
                    GrColor_SHIFT_B;
            }
            inout->setToOther(commonFlags, color, GrInvariantOutput::kWill_ReadInput);
        }
    };
    if (!fp) {
        return nullptr;
    }
    return new PremulFragmentProcessor(fp);
}

//////////////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* GrFragmentProcessor::OverrideInput(const GrFragmentProcessor* fp,
                                                              GrColor color) {
    class ReplaceInputFragmentProcessor : public GrFragmentProcessor {
    public:
        ReplaceInputFragmentProcessor(const GrFragmentProcessor* child, GrColor color)
            : fColor(color) {
            this->initClassID<ReplaceInputFragmentProcessor>();
            this->registerChildProcessor(child);
        }

        const char* name() const override { return "Replace Color"; }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                GLFP() : fHaveSetColor(false) {}
                void emitCode(EmitArgs& args) override {
                    const char* colorName;
                    fColorUni = args.fUniformHandler->addUniform(
                                                         GrGLSLUniformHandler::kFragment_Visibility,
                                                         kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                         "Color", &colorName);
                    this->emitChild(0, colorName, args);
                }

            private:
                void onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrProcessor& fp) override {
                    GrColor color = fp.cast<ReplaceInputFragmentProcessor>().fColor;
                    if (!fHaveSetColor || color != fPreviousColor) {
                        static const float scale = 1.f / 255.f;
                        float floatColor[4] = {
                            GrColorUnpackR(color) * scale,
                            GrColorUnpackG(color) * scale,
                            GrColorUnpackB(color) * scale,
                            GrColorUnpackA(color) * scale,
                        };
                        pdman.set4fv(fColorUni, 1, floatColor);
                        fPreviousColor = color;
                        fHaveSetColor = true;
                    }
                }

                GrGLSLProgramDataManager::UniformHandle fColorUni;
                bool    fHaveSetColor;
                GrColor fPreviousColor;
            };

            return new GLFP;
        }

    private:
        void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override
        {}

        bool onIsEqual(const GrFragmentProcessor& that) const override {
            return fColor == that.cast<ReplaceInputFragmentProcessor>().fColor;
        }

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            inout->setToOther(kRGBA_GrColorComponentFlags, fColor,
                              GrInvariantOutput::kWillNot_ReadInput);
            this->childProcessor(0).computeInvariantOutput(inout);
        }

        GrColor fColor;
    };

    GrInvariantOutput childOut(0x0, kNone_GrColorComponentFlags, false);
    fp->computeInvariantOutput(&childOut);
    if (childOut.willUseInputColor()) {
        return new ReplaceInputFragmentProcessor(fp, color);
    } else {
        return SkRef(fp);
    }
}

const GrFragmentProcessor* GrFragmentProcessor::RunInSeries(const GrFragmentProcessor* series[],
                                                            int cnt) {
    class SeriesFragmentProcessor : public GrFragmentProcessor {
    public:
        SeriesFragmentProcessor(const GrFragmentProcessor* children[], int cnt){
            SkASSERT(cnt > 1);
            this->initClassID<SeriesFragmentProcessor>();
            for (int i = 0; i < cnt; ++i) {
                this->registerChildProcessor(children[i]);
            }
        }

        const char* name() const override { return "Series"; }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                GLFP() {}
                void emitCode(EmitArgs& args) override {
                    SkString input(args.fInputColor);
                    for (int i = 0; i < this->numChildProcessors() - 1; ++i) {
                        SkString temp;
                        temp.printf("out%d", i);
                        this->emitChild(i, input.c_str(), &temp, args);
                        input = temp;
                    }
                    // Last guy writes to our output variable.
                    this->emitChild(this->numChildProcessors() - 1, input.c_str(), args);
                }
            };
            return new GLFP;
        }

    private:
        void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            GrProcOptInfo info;
            SkTDArray<const GrFragmentProcessor*> children;
            children.setCount(this->numChildProcessors());
            for (int i = 0; i < children.count(); ++i) {
                children[i] = &this->childProcessor(i);
            }
            info.calcWithInitialValues(children.begin(), children.count(), inout->color(),
                                       inout->validFlags(), false, false);
            for (int i = 0; i < this->numChildProcessors(); ++i) {
                this->childProcessor(i).computeInvariantOutput(inout);
            }
        }
    };

    if (!cnt) {
        return nullptr;
    }

    // Run the through the series, do the invariant output processing, and look for eliminations.
    SkTDArray<const GrFragmentProcessor*> replacementSeries;
    SkAutoTUnref<const GrFragmentProcessor> colorFP;
    GrProcOptInfo info;

    info.calcWithInitialValues(series, cnt, 0x0, kNone_GrColorComponentFlags, false, false);
    if (kRGBA_GrColorComponentFlags == info.validFlags()) {
        return GrConstColorProcessor::Create(info.color(),
                                             GrConstColorProcessor::kIgnore_InputMode);
    } else {
        int firstIdx = info.firstEffectiveProcessorIndex();
        cnt -= firstIdx;
        if (firstIdx > 0 && info.inputColorIsUsed()) {
            colorFP.reset(GrConstColorProcessor::Create(info.inputColorToFirstEffectiveProccesor(),
                                                        GrConstColorProcessor::kIgnore_InputMode));
            cnt += 1;
            replacementSeries.setCount(cnt);
            replacementSeries[0] = colorFP;
            for (int i = 0; i < cnt - 1; ++i) {
                replacementSeries[i + 1] = series[firstIdx + i];
            }
            series = replacementSeries.begin();
        } else {
            series += firstIdx;
            cnt -= firstIdx;
        }
    }

    if (1 == cnt) {
        return SkRef(series[0]);
    } else {
        return new SeriesFragmentProcessor(series, cnt);
    }
}

