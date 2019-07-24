/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/effects/generated/GrOverrideInputFragmentProcessor.h"
#include "src/gpu/effects/generated/GrPremulInputFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

bool GrFragmentProcessor::isEqual(const GrFragmentProcessor& that) const {
    if (this->classID() != that.classID()) {
        return false;
    }
    if (this->numTextureSamplers() != that.numTextureSamplers()) {
        return false;
    }
    for (int i = 0; i < this->numTextureSamplers(); ++i) {
        if (this->textureSampler(i) != that.textureSampler(i)) {
            return false;
        }
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

void GrFragmentProcessor::visitProxies(const GrOp::VisitProxyFunc& func) {
    GrFragmentProcessor::TextureAccessIter iter(this);
    while (const TextureSampler* sampler = iter.next()) {
        bool mipped = (GrSamplerState::Filter::kMipMap == sampler->samplerState().filter());
        func(sampler->proxy(), GrMipMapped(mipped));
    }
}

GrGLSLFragmentProcessor* GrFragmentProcessor::createGLSLInstance() const {
    GrGLSLFragmentProcessor* glFragProc = this->onCreateGLSLInstance();
    glFragProc->fChildProcessors.push_back_n(fChildProcessors.count());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        glFragProc->fChildProcessors[i] = fChildProcessors[i]->createGLSLInstance();
    }
    return glFragProc;
}

const GrFragmentProcessor::TextureSampler& GrFragmentProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < fTextureSamplerCnt);
    return this->onTextureSampler(i);
}

void GrFragmentProcessor::addCoordTransform(const GrCoordTransform* transform) {
    fCoordTransforms.push_back(transform);
    fFlags |= kUsesLocalCoords_Flag;
    SkDEBUGCODE(transform->setInProcessor();)
}

#ifdef SK_DEBUG
bool GrFragmentProcessor::isInstantiated() const {
    for (int i = 0; i < fTextureSamplerCnt; ++i) {
        if (!this->textureSampler(i).isInstantiated()) {
            return false;
        }
    }

    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).isInstantiated()) {
            return false;
        }
    }

    return true;
}
#endif

int GrFragmentProcessor::registerChildProcessor(std::unique_ptr<GrFragmentProcessor> child) {
    if (child->usesLocalCoords()) {
        fFlags |= kUsesLocalCoords_Flag;
    }
    fRequestedFeatures |= child->fRequestedFeatures;

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
                , fSwizzle(swizzle) {}

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

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
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
                    this->invokeChild(0, args);
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

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
            SkPMColor4f childColor = ConstantOutputForConstantInput(this->childProcessor(0),
                                                                    SK_PMColor4fWHITE);
            SkPMColor4f premulInput = SkColor4f{ input.fR, input.fG, input.fB, input.fA }.premul();
            return premulInput * childColor;
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
        std::unique_ptr<GrFragmentProcessor> fp, const SkPMColor4f& color, bool useUniform) {
    if (!fp) {
        return nullptr;
    }
    return GrOverrideInputFragmentProcessor::Make(std::move(fp), color, useUniform);
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
                    this->invokeChild(0, args.fInputColor, &temp, args);
                    SkString input = temp;
                    for (int i = 1; i < this->numChildProcessors() - 1; ++i) {
                        temp.printf("out%d", i);
                        this->invokeChild(i, input.c_str(), &temp, args);
                        input = temp;
                    }
                    // Last guy writes to our output variable.
                    this->invokeChild(this->numChildProcessors() - 1, input.c_str(), args);
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

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
            SkPMColor4f color = inColor;
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
    SkPMColor4f knownColor;
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

GrFragmentProcessor::Iter::Iter(const GrPaint& paint) {
    for (int i = paint.numCoverageFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(paint.getCoverageFragmentProcessor(i));
    }
    for (int i = paint.numColorFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(paint.getColorFragmentProcessor(i));
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

///////////////////////////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor::TextureSampler::TextureSampler(sk_sp<GrTextureProxy> proxy,
                                                    const GrSamplerState& samplerState) {
    this->reset(std::move(proxy), samplerState);
}

GrFragmentProcessor::TextureSampler::TextureSampler(sk_sp<GrTextureProxy> proxy,
                                                    GrSamplerState::Filter filterMode,
                                                    GrSamplerState::WrapMode wrapXAndY) {
    this->reset(std::move(proxy), filterMode, wrapXAndY);
}

void GrFragmentProcessor::TextureSampler::reset(sk_sp<GrTextureProxy> proxy,
                                                const GrSamplerState& samplerState) {
    fProxy = std::move(proxy);
    fSamplerState = samplerState;
    fSamplerState.setFilterMode(SkTMin(samplerState.filter(), this->proxy()->highestFilterMode()));
}

void GrFragmentProcessor::TextureSampler::reset(sk_sp<GrTextureProxy> proxy,
                                                GrSamplerState::Filter filterMode,
                                                GrSamplerState::WrapMode wrapXAndY) {
    fProxy = std::move(proxy);
    filterMode = SkTMin(filterMode, this->proxy()->highestFilterMode());
    fSamplerState = GrSamplerState(wrapXAndY, filterMode);
}
