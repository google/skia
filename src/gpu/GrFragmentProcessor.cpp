/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "GrPipeline.h"
#include "GrProcessorAnalysis.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrPremulInputFragmentProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "effects/GrUnpremulInputFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

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

bool GrFragmentProcessor::instantiate(GrResourceProvider* resourceProvider) const {
    if (!INHERITED::instantiate(resourceProvider)) {
        return false;
    }

    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).instantiate(resourceProvider)) {
            return false;
        }
    }

    return true;
}

void GrFragmentProcessor::markPendingExecution() const {
    INHERITED::addPendingIOs();
    INHERITED::removeRefs();
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        this->childProcessor(i).markPendingExecution();
    }
}

int GrFragmentProcessor::registerChildProcessor(std::unique_ptr<GrFragmentProcessor> child) {
    if (child->usesLocalCoords()) {
        fFlags |= kUsesLocalCoords_Flag;
    }

    int index = fChildProcessors.count();
    fChildProcessors.push_back(std::move(child));

    return index;
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

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MulChildByInputAlpha(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(fp), SkBlendMode::kDstIn);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MulInputByChildAlpha(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(fp), SkBlendMode::kSrcIn);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::PremulInput(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    std::unique_ptr<GrFragmentProcessor> fpPipeline[] = { GrPremulInputFragmentProcessor::Make(),
                                                          std::move(fp) };
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::PremulOutput(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    std::unique_ptr<GrFragmentProcessor> fpPipeline[] = { std::move(fp),
                                                          GrPremulInputFragmentProcessor::Make() };
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::UnpremulOutput(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    std::unique_ptr<GrFragmentProcessor> fpPipeline[] = { std::move(fp),
                                                          GrUnpremulInputFragmentProcessor::Make() };
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::SwizzleOutput(
        std::unique_ptr<GrFragmentProcessor> fp, const GrSwizzle& swizzle) {
    class SwizzleFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(const GrSwizzle& swizzle) {
            return std::unique_ptr<GrFragmentProcessor>(new SwizzleFragmentProcessor(swizzle));
        }

        const char* name() const override { return "Swizzle"; }
        const GrSwizzle& swizzle() const { return fSwizzle; }

        std::unique_ptr<GrFragmentProcessor> clone() const override { return Make(fSwizzle); }

    private:
        SwizzleFragmentProcessor(const GrSwizzle& swizzle)
                : INHERITED(kSwizzleFragmentProcessor_ClassID, kAll_OptimizationFlags)
                , fSwizzle(swizzle) {
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    const SwizzleFragmentProcessor& sfp = args.fFp.cast<SwizzleFragmentProcessor>();
                    const GrSwizzle& swizzle = sfp.swizzle();
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                    fragBuilder->codeAppendf("%s = %s.%s;",
                                             args.fOutputColor, args.fInputColor, swizzle.c_str());
                }
            };
            return new GLFP;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
            b->add32(fSwizzle.asKey());
        }

        bool onIsEqual(const GrFragmentProcessor& other) const override {
            const SwizzleFragmentProcessor& sfp = other.cast<SwizzleFragmentProcessor>();
            return fSwizzle == sfp.fSwizzle;
        }

        GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
            return fSwizzle.applyTo(input);
        }

        GrSwizzle fSwizzle;

        typedef GrFragmentProcessor INHERITED;
    };

    if (!fp) {
        return nullptr;
    }
    if (GrSwizzle::RGBA() == swizzle) {
        return fp;
    }
    std::unique_ptr<GrFragmentProcessor> fpPipeline[] = { std::move(fp),
                                                          SwizzleFragmentProcessor::Make(swizzle) };
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MakeInputPremulAndMulByOutput(
        std::unique_ptr<GrFragmentProcessor> fp) {
    class PremulFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(
                std::unique_ptr<GrFragmentProcessor> processor) {
            return std::unique_ptr<GrFragmentProcessor>(
                    new PremulFragmentProcessor(std::move(processor)));
        }

        const char* name() const override { return "Premultiply"; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0).clone());
        }

    private:
        PremulFragmentProcessor(std::unique_ptr<GrFragmentProcessor> processor)
                : INHERITED(kPremulFragmentProcessor_ClassID, OptFlags(processor.get())) {
            this->registerChildProcessor(std::move(processor));
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                    this->emitChild(0, args);
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
    return PremulFragmentProcessor::Make(std::move(fp));
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::OverrideInput(
        std::unique_ptr<GrFragmentProcessor> fp, GrColor4f color) {
    class ReplaceInputFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child,
                                                         GrColor4f color) {
            return std::unique_ptr<GrFragmentProcessor>(
                    new ReplaceInputFragmentProcessor(std::move(child), color));
        }

        const char* name() const override { return "Replace Color"; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0).clone(), fColor);
        }

    private:
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                GLFP() : fHaveSetColor(false) {}
                void emitCode(EmitArgs& args) override {
                    const char* colorName;
                    fColorUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                 kHalf4_GrSLType,
                                                                 "Color", &colorName);
                    this->emitChild(0, colorName, args);
                }

            private:
                void onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrFragmentProcessor& fp) override {
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

        ReplaceInputFragmentProcessor(std::unique_ptr<GrFragmentProcessor> child, GrColor4f color)
                : INHERITED(kReplaceInputFragmentProcessor_ClassID, OptFlags(child.get(), color))
                , fColor(color) {
            this->registerChildProcessor(std::move(child));
        }

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

        GrColor4f constantOutputForConstantInput(GrColor4f) const override {
            return ConstantOutputForConstantInput(this->childProcessor(0), fColor);
        }

        GrColor4f fColor;

        typedef GrFragmentProcessor INHERITED;
    };

    if (!fp) {
        return nullptr;
    }
    return ReplaceInputFragmentProcessor::Make(std::move(fp), color);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::RunInSeries(
        std::unique_ptr<GrFragmentProcessor>* series, int cnt) {
    class SeriesFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(
                std::unique_ptr<GrFragmentProcessor>* children, int cnt) {
            return std::unique_ptr<GrFragmentProcessor>(new SeriesFragmentProcessor(children, cnt));
        }

        const char* name() const override { return "Series"; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            SkSTArray<4, std::unique_ptr<GrFragmentProcessor>> children(this->numChildProcessors());
            for (int i = 0; i < this->numChildProcessors(); ++i) {
                if (!children.push_back(this->childProcessor(i).clone())) {
                    return nullptr;
                }
            }
            return Make(children.begin(), this->numChildProcessors());
        }

    private:
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

        SeriesFragmentProcessor(std::unique_ptr<GrFragmentProcessor>* children, int cnt)
                : INHERITED(kSeriesFragmentProcessor_ClassID, OptFlags(children, cnt)) {
            SkASSERT(cnt > 1);
            for (int i = 0; i < cnt; ++i) {
                this->registerChildProcessor(std::move(children[i]));
            }
        }

        static OptimizationFlags OptFlags(std::unique_ptr<GrFragmentProcessor>* children, int cnt) {
            OptimizationFlags flags = kAll_OptimizationFlags;
            for (int i = 0; i < cnt && flags != kNone_OptimizationFlags; ++i) {
                flags &= children[i]->optimizationFlags();
            }
            return flags;
        }
        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

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
    if (1 == cnt) {
        return std::move(series[0]);
    }
    // Run the through the series, do the invariant output processing, and look for eliminations.
    GrProcessorAnalysisColor inputColor;
    inputColor.setToUnknown();
    GrColorFragmentProcessorAnalysis info(inputColor, unique_ptr_address_as_pointer_address(series),
                                          cnt);
    SkTArray<std::unique_ptr<GrFragmentProcessor>> replacementSeries;
    GrColor4f knownColor;
    int leadingFPsToEliminate = info.initialProcessorsToEliminate(&knownColor);
    if (leadingFPsToEliminate) {
        std::unique_ptr<GrFragmentProcessor> colorFP(
                GrConstColorProcessor::Make(knownColor, GrConstColorProcessor::InputMode::kIgnore));
        if (leadingFPsToEliminate == cnt) {
            return colorFP;
        }
        cnt = cnt - leadingFPsToEliminate + 1;
        replacementSeries.reserve(cnt);
        replacementSeries.emplace_back(std::move(colorFP));
        for (int i = 0; i < cnt - 1; ++i) {
            replacementSeries.emplace_back(std::move(series[leadingFPsToEliminate + i]));
        }
        series = replacementSeries.begin();
    }
    return SeriesFragmentProcessor::Make(series, cnt);
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

