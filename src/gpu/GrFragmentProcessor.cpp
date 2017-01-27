/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrPipeline.h"
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

bool GrFragmentProcessor::isEqual(const GrFragmentProcessor& that) const {
    if (this->classID() != that.classID() ||
        !this->hasSameSamplersAndAccesses(that)) {
        return false;
    }
    if (!this->hasSameTransforms(that)) {
        return false;
    }
    if (!this->onIsEqual(that)) {
        return false;
    }
    if (this->numChildProcessors() != that.numChildProcessors()) {
        return false;
    }
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).isEqual(that.childProcessor(i))) {
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

void GrFragmentProcessor::addCoordTransform(const GrCoordTransform* transform) {
    fCoordTransforms.push_back(transform);
    fFlags |= kUsesLocalCoords_Flag;
    SkDEBUGCODE(transform->setInProcessor();)
}

int GrFragmentProcessor::registerChildProcessor(sk_sp<GrFragmentProcessor> child) {
    this->combineRequiredFeatures(*child);

    if (child->usesLocalCoords()) {
        fFlags |= kUsesLocalCoords_Flag;
    }
    if (child->usesDistanceVectorField()) {
        fFlags |= kUsesDistanceVectorField_Flag;
    }

    int index = fChildProcessors.count();
    fChildProcessors.push_back(child.release());

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
    if (this->numCoordTransforms() != that.numCoordTransforms()) {
        return false;
    }
    int count = this->numCoordTransforms();
    for (int i = 0; i < count; ++i) {
        if (!this->coordTransform(i).hasSameEffectAs(that.coordTransform(i))) {
            return false;
        }
    }
    return true;
}

sk_sp<GrFragmentProcessor> GrFragmentProcessor::MulOutputByInputAlpha(
    sk_sp<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(fp), SkBlendMode::kDstIn);
}

sk_sp<GrFragmentProcessor> GrFragmentProcessor::PremulInput(sk_sp<GrFragmentProcessor> fp) {

    class PremulInputFragmentProcessor : public GrFragmentProcessor {
    public:
        PremulInputFragmentProcessor()
                : INHERITED(kPreservesOpaqueInput_OptimizationFlag |
                            kConstantOutputForConstantInput_OptimizationFlag) {
            this->initClassID<PremulInputFragmentProcessor>();
        }

        const char* name() const override { return "PremultiplyInput"; }
    private:
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, args.fInputColor);
                    fragBuilder->codeAppendf("%s.rgb *= %s.a;",
                                             args.fOutputColor, args.fInputColor);
                }
            };
            return new GLFP;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            inout->premulFourChannelColor();
        }
        GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
            return input.premul();
        }

        typedef GrFragmentProcessor INHERITED;
    };
    if (!fp) {
        return nullptr;
    }
    sk_sp<GrFragmentProcessor> fpPipeline[] = { sk_make_sp<PremulInputFragmentProcessor>(), fp};
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}

sk_sp<GrFragmentProcessor> GrFragmentProcessor::MulOutputByInputUnpremulColor(
    sk_sp<GrFragmentProcessor> fp) {

    class PremulFragmentProcessor : public GrFragmentProcessor {
    public:
        PremulFragmentProcessor(sk_sp<GrFragmentProcessor> processor)
                : INHERITED(OptFlags(processor.get())) {
            this->initClassID<PremulFragmentProcessor>();
            this->registerChildProcessor(processor);
        }

        const char* name() const override { return "Premultiply"; }

    private:
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                    this->emitChild(0, nullptr, args);
                    fragBuilder->codeAppendf("%s.rgb *= %s.rgb;", args.fOutputColor,
                                                                args.fInputColor);
                    fragBuilder->codeAppendf("%s *= %s.a;", args.fOutputColor, args.fInputColor);
                }
            };
            return new GLFP;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        static OptimizationFlags OptFlags(const GrFragmentProcessor* inner) {
            OptimizationFlags flags = kNone_OptimizationFlags;
            if (inner->preservesOpaqueInput()) {
                flags |= kPreservesOpaqueInput_OptimizationFlag;
            }
            if (inner->hasConstantOutputForConstantInput()) {
                flags |= kConstantOutputForConstantInput_OptimizationFlag;
            }
            return flags;
        }
        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            // TODO: Add a helper to GrInvariantOutput that handles multiplying by color with flags?
            if (!(inout->validFlags() & kA_GrColorComponentFlag)) {
                inout->setToUnknown();
                return;
            }

            GrInvariantOutput childOutput(GrColor_WHITE, kRGBA_GrColorComponentFlags);
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
            inout->setToOther(commonFlags, color);
        }
        GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
            GrColor4f childColor = ConstantOutputForConstantInput(this->childProcessor(0),
                                                                  GrColor4f::OpaqueWhite());
            return GrColor4f(input.fRGBA[3] * input.fRGBA[0] * childColor.fRGBA[0],
                             input.fRGBA[3] * input.fRGBA[1] * childColor.fRGBA[1],
                             input.fRGBA[3] * input.fRGBA[2] * childColor.fRGBA[2],
                             input.fRGBA[3] * childColor.fRGBA[3]);
        }

        typedef GrFragmentProcessor INHERITED;
    };
    if (!fp) {
        return nullptr;
    }
    return sk_sp<GrFragmentProcessor>(new PremulFragmentProcessor(std::move(fp)));
}

//////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrFragmentProcessor::OverrideInput(sk_sp<GrFragmentProcessor> fp,
                                                              GrColor4f color) {
    class ReplaceInputFragmentProcessor : public GrFragmentProcessor {
    public:
        ReplaceInputFragmentProcessor(sk_sp<GrFragmentProcessor> child, GrColor4f color)
                : INHERITED(OptFlags(child.get(), color)), fColor(color) {
            this->initClassID<ReplaceInputFragmentProcessor>();
            this->registerChildProcessor(std::move(child));
        }

        const char* name() const override { return "Replace Color"; }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                GLFP() : fHaveSetColor(false) {}
                void emitCode(EmitArgs& args) override {
                    const char* colorName;
                    fColorUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                 kVec4f_GrSLType,
                                                                 kDefault_GrSLPrecision,
                                                                 "Color", &colorName);
                    this->emitChild(0, colorName, args);
                }

            private:
                void onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrProcessor& fp) override {
                    GrColor4f color = fp.cast<ReplaceInputFragmentProcessor>().fColor;
                    if (!fHaveSetColor || color != fPreviousColor) {
                        pdman.set4fv(fColorUni, 1, color.fRGBA);
                        fPreviousColor = color;
                        fHaveSetColor = true;
                    }
                }

                GrGLSLProgramDataManager::UniformHandle fColorUni;
                bool      fHaveSetColor;
                GrColor4f fPreviousColor;
            };

            return new GLFP;
        }

    private:
        static OptimizationFlags OptFlags(const GrFragmentProcessor* child, GrColor4f color) {
            OptimizationFlags childFlags = child->optimizationFlags();
            OptimizationFlags flags = kNone_OptimizationFlags;
            if (childFlags & kConstantOutputForConstantInput_OptimizationFlag) {
                flags |= kConstantOutputForConstantInput_OptimizationFlag;
            }
            if ((childFlags & kPreservesOpaqueInput_OptimizationFlag) && color.isOpaque()) {
                flags |= kPreservesOpaqueInput_OptimizationFlag;
            }
            return flags;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override
        {}

        bool onIsEqual(const GrFragmentProcessor& that) const override {
            return fColor == that.cast<ReplaceInputFragmentProcessor>().fColor;
        }

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            inout->setToOther(kRGBA_GrColorComponentFlags, fColor.toGrColor());
            this->childProcessor(0).computeInvariantOutput(inout);
        }

        GrColor4f constantOutputForConstantInput(GrColor4f) const override {
            return ConstantOutputForConstantInput(this->childProcessor(0), fColor);
        }

        GrColor4f fColor;

        typedef GrFragmentProcessor INHERITED;
    };

    GrInvariantOutput childOut(0x0, kNone_GrColorComponentFlags);
    fp->computeInvariantOutput(&childOut);
    return sk_sp<GrFragmentProcessor>(new ReplaceInputFragmentProcessor(std::move(fp), color));
}

sk_sp<GrFragmentProcessor> GrFragmentProcessor::RunInSeries(sk_sp<GrFragmentProcessor>* series,
                                                            int cnt) {
    class SeriesFragmentProcessor : public GrFragmentProcessor {
    public:
        SeriesFragmentProcessor(sk_sp<GrFragmentProcessor>* children, int cnt)
                : INHERITED(OptFlags(children, cnt)) {
            SkASSERT(cnt > 1);
            this->initClassID<SeriesFragmentProcessor>();
            for (int i = 0; i < cnt; ++i) {
                this->registerChildProcessor(std::move(children[i]));
            }
        }

        const char* name() const override { return "Series"; }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    // First guy's input might be nil.
                    SkString temp("out0");
                    this->emitChild(0, args.fInputColor, &temp, args);
                    SkString input = temp;
                    for (int i = 1; i < this->numChildProcessors() - 1; ++i) {
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
        static OptimizationFlags OptFlags(sk_sp<GrFragmentProcessor>* children, int cnt) {
            OptimizationFlags flags = kAll_OptimizationFlags;
            for (int i = 0; i < cnt && flags != kNone_OptimizationFlags; ++i) {
                flags &= children[i]->optimizationFlags();
            }
            return flags;
        }
        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            for (int i = 0; i < this->numChildProcessors(); ++i) {
                this->childProcessor(i).computeInvariantOutput(inout);
            }
        }
        GrColor4f constantOutputForConstantInput(GrColor4f color) const override {
            int childCnt = this->numChildProcessors();
            for (int i = 0; i < childCnt; ++i) {
                color = ConstantOutputForConstantInput(this->childProcessor(i), color);
            }
            return color;
        }

        typedef GrFragmentProcessor INHERITED;
    };

    if (!cnt) {
        return nullptr;
    }

    // Run the through the series, do the invariant output processing, and look for eliminations.
    GrProcOptInfo info(0x0, kNone_GrColorComponentFlags);
    info.analyzeProcessors(sk_sp_address_as_pointer_address(series), cnt);
    if (kRGBA_GrColorComponentFlags == info.validFlags()) {
        // TODO: We need to preserve 4f and color spaces during invariant processing. This color
        // has definitely lost precision, and could easily be in the wrong gamut (or have been
        // built from colors in multiple spaces).
        return GrConstColorProcessor::Make(GrColor4f::FromGrColor(info.color()),
                                           GrConstColorProcessor::kIgnore_InputMode);
    }

    SkTArray<sk_sp<GrFragmentProcessor>> replacementSeries;

    int firstIdx = info.firstEffectiveProcessorIndex();
    cnt -= firstIdx;
    if (firstIdx > 0) {
        // See comment above - need to preserve 4f and color spaces during invariant processing.
        sk_sp<GrFragmentProcessor> colorFP(GrConstColorProcessor::Make(
            GrColor4f::FromGrColor(info.inputColorToFirstEffectiveProccesor()),
            GrConstColorProcessor::kIgnore_InputMode));
        cnt += 1;
        replacementSeries.reserve(cnt);
        replacementSeries.emplace_back(std::move(colorFP));
        for (int i = 0; i < cnt - 1; ++i) {
            replacementSeries.emplace_back(std::move(series[firstIdx + i]));
        }
        series = replacementSeries.begin();
    }

    if (1 == cnt) {
        return series[0];
    }
    return sk_sp<GrFragmentProcessor>(new SeriesFragmentProcessor(series, cnt));
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor::Iter::Iter(const GrPipeline& pipeline) {
    for (int i = pipeline.numFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(&pipeline.getFragmentProcessor(i));
    }
}

const GrFragmentProcessor* GrFragmentProcessor::Iter::next() {
    if (fFPStack.empty()) {
        return nullptr;
    }
    const GrFragmentProcessor* back = fFPStack.back();
    fFPStack.pop_back();
    for (int i = back->numChildProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(&back->childProcessor(i));
    }
    return back;
}

