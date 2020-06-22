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
#include "src/gpu/effects/generated/GrClampFragmentProcessor.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/effects/generated/GrOverrideInputFragmentProcessor.h"
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
    for (auto [sampler, fp] : FPTextureSamplerRange(*this)) {
        bool mipped = (GrSamplerState::Filter::kMipMap == sampler.samplerState().filter());
        func(sampler.view().proxy(), GrMipMapped(mipped));
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

void GrFragmentProcessor::addCoordTransform(GrCoordTransform* transform) {
    fCoordTransforms.push_back(transform);
    fFlags |= kHasCoordTransforms_Flag;
}

void GrFragmentProcessor::setSampleMatrix(SkSL::SampleMatrix newMatrix) {
    if (newMatrix == fMatrix) {
        return;
    }
    SkASSERT(newMatrix.fKind != SkSL::SampleMatrix::Kind::kNone);
    SkASSERT(fMatrix.fKind != SkSL::SampleMatrix::Kind::kVariable);
    if (this->numCoordTransforms() == 0 &&
        (newMatrix.fKind == SkSL::SampleMatrix::Kind::kConstantOrUniform ||
         newMatrix.fKind == SkSL::SampleMatrix::Kind::kMixed)) {
        // as things stand, matrices only work when there's a coord transform, so we need to add
        // an identity transform to keep the downstream code happy
        static GrCoordTransform identity;
        this->addCoordTransform(&identity);
    }
    if (fMatrix.fKind == SkSL::SampleMatrix::Kind::kConstantOrUniform) {
        if (newMatrix.fKind == SkSL::SampleMatrix::Kind::kConstantOrUniform) {
            // need to base this transform on the one that happened in our parent
            // If we're already based on something, then we have to assume that parent is now
            // based on yet another transform, so don't update our base pointer (or we'll skip
            // the intermediate transform).
            if (!fMatrix.fBase) {
                fMatrix.fBase = newMatrix.fOwner;
            }
        } else {
            SkASSERT(newMatrix.fKind == SkSL::SampleMatrix::Kind::kVariable);
            fMatrix.fKind = SkSL::SampleMatrix::Kind::kMixed;
            fMatrix.fBase = nullptr;
        }
    } else {
        SkASSERT(fMatrix.fKind == SkSL::SampleMatrix::Kind::kNone);
        fMatrix = newMatrix;
    }
    for (auto& child : fChildProcessors) {
        child->setSampleMatrix(newMatrix);
    }
}

void GrFragmentProcessor::setSampledWithExplicitCoords() {
    fFlags |= kSampledWithExplicitCoords;
    for (auto& child : fChildProcessors) {
        child->setSampledWithExplicitCoords();
    }
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

int GrFragmentProcessor::registerChild(std::unique_ptr<GrFragmentProcessor> child,
                                       SkSL::SampleMatrix sampleMatrix,
                                       bool explicitlySampled) {
    // Configure child's sampling state first
    if (explicitlySampled) {
        child->setSampledWithExplicitCoords();
    }
    if (sampleMatrix.fKind != SkSL::SampleMatrix::Kind::kNone) {
        // FIXME(michaelludwig) - Temporary hack. Owner tracking will be moved off of SampleMatrix
        // and into FP. Currently, coord transform compilation fails on sample_matrix GMs if the
        // child isn't the owner. But the matrix effect (and expected behavior) require the owner
        // to be 'this' FP.
        if (this->classID() == kGrMatrixEffect_ClassID) {
            sampleMatrix.fOwner = this;
        } else {
            sampleMatrix.fOwner = child.get();
        }
        child->setSampleMatrix(sampleMatrix);
    }

    if (child->fFlags & kHasCoordTransforms_Flag) {
        fFlags |= kHasCoordTransforms_Flag;
    }
    fRequestedFeatures |= child->fRequestedFeatures;

    int index = fChildProcessors.count();
    fChildProcessors.push_back(std::move(child));
    SkASSERT(fMatrix.fKind == SkSL::SampleMatrix::Kind::kNone ||
             fMatrix.fKind == SkSL::SampleMatrix::Kind::kConstantOrUniform);
    return index;
}

int GrFragmentProcessor::cloneAndRegisterChildProcessor(const GrFragmentProcessor& fp) {
    std::unique_ptr<GrFragmentProcessor> clone = fp.clone();
    return this->registerChild(std::move(clone), fp.sampleMatrix(),
                               fp.isSampledWithExplicitCoords());
}

void GrFragmentProcessor::cloneAndRegisterAllChildProcessors(const GrFragmentProcessor& src) {
    for (int i = 0; i < src.numChildProcessors(); ++i) {
        this->cloneAndRegisterChildProcessor(src.childProcessor(i));
    }
}

bool GrFragmentProcessor::hasSameTransforms(const GrFragmentProcessor& that) const {
    if (this->numCoordTransforms() != that.numCoordTransforms()) {
        return false;
    }
    int count = this->numCoordTransforms();
    for (int i = 0; i < count; ++i) {
        if (!this->coordTransform(i).hasSameEffectiveMatrix(that.coordTransform(i))) {
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

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ClampPremulOutput(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrClampFragmentProcessor::Make(std::move(fp), /*clampToPremul=*/true);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::SwizzleOutput(
        std::unique_ptr<GrFragmentProcessor> fp, const GrSwizzle& swizzle) {
    class SwizzleFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                         const GrSwizzle& swizzle) {
            return std::unique_ptr<GrFragmentProcessor>(
                new SwizzleFragmentProcessor(std::move(fp), swizzle));
        }

        const char* name() const override { return "Swizzle"; }
        const GrSwizzle& swizzle() const { return fSwizzle; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0).clone(), fSwizzle);
        }

    private:
        SwizzleFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp, const GrSwizzle& swizzle)
                : INHERITED(kSwizzleFragmentProcessor_ClassID, ProcessorOptimizationFlags(fp.get()))
                , fSwizzle(swizzle) {
            this->registerChild(std::move(fp));
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    SkString childColor = this->invokeChild(0, args.fInputColor, args);

                    const SwizzleFragmentProcessor& sfp = args.fFp.cast<SwizzleFragmentProcessor>();
                    const GrSwizzle& swizzle = sfp.swizzle();
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                    fragBuilder->codeAppendf("%s = %s.%s;",
                            args.fOutputColor, childColor.c_str(), swizzle.asString().c_str());
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
    return SwizzleFragmentProcessor::Make(std::move(fp), swizzle);
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
            this->registerChild(std::move(processor));
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                    SkString temp = this->invokeChild(0, args);
                    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, temp.c_str());
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
        std::unique_ptr<GrFragmentProcessor> series[], int cnt) {
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
                    SkString result = this->invokeChild(0, args.fInputColor, args);
                    for (int i = 1; i < this->numChildProcessors(); ++i) {
                        result = this->invokeChild(i, result.c_str(), args);
                    }
                    // Copy last output to our output variable
                    args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, result.c_str());
                }
            };
            return new GLFP;
        }

        SeriesFragmentProcessor(std::unique_ptr<GrFragmentProcessor>* children, int cnt)
                : INHERITED(kSeriesFragmentProcessor_ClassID, OptFlags(children, cnt)) {
            SkASSERT(cnt > 1);
            for (int i = 0; i < cnt; ++i) {
                this->registerChild(std::move(children[i]));
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
    GrColorFragmentProcessorAnalysis info(inputColor, series, cnt);
    SkTArray<std::unique_ptr<GrFragmentProcessor>> replacementSeries;
    SkPMColor4f knownColor;
    int leadingFPsToEliminate = info.initialProcessorsToEliminate(&knownColor);
    if (leadingFPsToEliminate) {
        std::unique_ptr<GrFragmentProcessor> colorFP = GrConstColorProcessor::Make(
            /*inputFP=*/nullptr, knownColor, GrConstColorProcessor::InputMode::kIgnore);
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

GrFragmentProcessor::CIter::CIter(const GrPaint& paint) {
    for (int i = paint.numCoverageFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(paint.getCoverageFragmentProcessor(i));
    }
    for (int i = paint.numColorFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(paint.getColorFragmentProcessor(i));
    }
}

GrFragmentProcessor::CIter::CIter(const GrProcessorSet& set) {
    for (int i = set.numCoverageFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(set.coverageFragmentProcessor(i));
    }
    for (int i = set.numColorFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(set.colorFragmentProcessor(i));
    }
}

GrFragmentProcessor::CIter::CIter(const GrPipeline& pipeline) {
    for (int i = pipeline.numFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(&pipeline.getFragmentProcessor(i));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor::TextureSampler::TextureSampler(GrSurfaceProxyView view,
                                                    GrSamplerState samplerState)
        : fView(std::move(view)), fSamplerState(samplerState) {
    GrSurfaceProxy* proxy = this->proxy();
    fSamplerState.setFilterMode(
            std::min(samplerState.filter(),
                   GrTextureProxy::HighestFilterMode(proxy->backendFormat().textureType())));
}

#if GR_TEST_UTILS
void GrFragmentProcessor::TextureSampler::set(GrSurfaceProxyView view,
                                              GrSamplerState samplerState) {
    SkASSERT(view.proxy()->asTextureProxy());
    fView = std::move(view);
    fSamplerState = samplerState;

    fSamplerState.setFilterMode(
            std::min(samplerState.filter(),
                   GrTextureProxy::HighestFilterMode(this->proxy()->backendFormat().textureType())));
}
#endif
