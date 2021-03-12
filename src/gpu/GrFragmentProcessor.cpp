/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
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
    if (this->usesVaryingCoordsDirectly() != that.usesVaryingCoordsDirectly()) {
        return false;
    }
    if (!this->onIsEqual(that)) {
        return false;
    }
    if (this->numChildProcessors() != that.numChildProcessors()) {
        return false;
    }
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        auto thisChild = this->childProcessor(i),
             thatChild = that .childProcessor(i);
        if (SkToBool(thisChild) != SkToBool(thatChild)) {
            return false;
        }
        if (thisChild && !thisChild->isEqual(*thatChild)) {
            return false;
        }
    }
    return true;
}

void GrFragmentProcessor::visitProxies(const GrOp::VisitProxyFunc& func) const {
    this->visitTextureEffects([&func](const GrTextureEffect& te) {
        func(te.view().proxy(), te.samplerState().mipmapped());
    });
}

void GrFragmentProcessor::visitTextureEffects(
        const std::function<void(const GrTextureEffect&)>& func) const {
    if (auto* te = this->asTextureEffect()) {
        func(*te);
    }
    for (auto& child : fChildProcessors) {
        if (child) {
            child->visitTextureEffects(func);
        }
    }
}

GrTextureEffect* GrFragmentProcessor::asTextureEffect() {
    if (this->classID() == kGrTextureEffect_ClassID) {
        return static_cast<GrTextureEffect*>(this);
    }
    return nullptr;
}

const GrTextureEffect* GrFragmentProcessor::asTextureEffect() const {
    if (this->classID() == kGrTextureEffect_ClassID) {
        return static_cast<const GrTextureEffect*>(this);
    }
    return nullptr;
}

#if GR_TEST_UTILS
static void recursive_dump_tree_info(const GrFragmentProcessor& fp,
                                     SkString indent,
                                     SkString* text) {
    for (int index = 0; index < fp.numChildProcessors(); ++index) {
        text->appendf("\n%s(#%d) -> ", indent.c_str(), index);
        if (const GrFragmentProcessor* childFP = fp.childProcessor(index)) {
            text->append(childFP->dumpInfo());
            indent.append("\t");
            recursive_dump_tree_info(*childFP, indent, text);
        } else {
            text->append("null");
        }
    }
}

SkString GrFragmentProcessor::dumpTreeInfo() const {
    SkString text = this->dumpInfo();
    recursive_dump_tree_info(*this, SkString("\t"), &text);
    text.append("\n");
    return text;
}
#endif

std::unique_ptr<GrGLSLFragmentProcessor> GrFragmentProcessor::makeProgramImpl() const {
    std::unique_ptr<GrGLSLFragmentProcessor> glFragProc = this->onMakeProgramImpl();
    glFragProc->fChildProcessors.push_back_n(fChildProcessors.count());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        glFragProc->fChildProcessors[i] = fChildProcessors[i]
                                                  ? fChildProcessors[i]->makeProgramImpl()
                                                  : nullptr;
    }
    return glFragProc;
}

void GrFragmentProcessor::addAndPushFlagToChildren(PrivateFlags flag) {
    // This propagates down, so if we've already marked it, all our children should have it too
    if (!(fFlags & flag)) {
        fFlags |= flag;
        for (auto& child : fChildProcessors) {
            if (child) {
                child->addAndPushFlagToChildren(flag);
            }
        }
    }
#ifdef SK_DEBUG
    for (auto& child : fChildProcessors) {
        SkASSERT(!child || (child->fFlags & flag));
    }
#endif
}

int GrFragmentProcessor::numNonNullChildProcessors() const {
    return std::count_if(fChildProcessors.begin(), fChildProcessors.end(),
                         [](const auto& c) { return c != nullptr; });
}

#ifdef SK_DEBUG
bool GrFragmentProcessor::isInstantiated() const {
    bool result = true;
    this->visitTextureEffects([&result](const GrTextureEffect& te) {
        if (!te.texture()) {
            result = false;
        }
    });
    return result;
}
#endif

void GrFragmentProcessor::registerChild(std::unique_ptr<GrFragmentProcessor> child,
                                        SkSL::SampleUsage sampleUsage) {
    if (!child) {
        fChildProcessors.push_back(nullptr);
        return;
    }

    // The child should not have been attached to another FP already and not had any sampling
    // strategy set on it.
    SkASSERT(!child->fParent && !child->sampleUsage().isSampled() &&
             !child->isSampledWithExplicitCoords() && !child->hasPerspectiveTransform());

    // If a child is sampled directly (sample(child)), and with a single uniform matrix, we need to
    // treat it as if it were sampled with multiple matrices (eg variable).
    bool variableMatrix = sampleUsage.hasVariableMatrix() ||
                          (sampleUsage.fPassThrough && sampleUsage.hasUniformMatrix());

    // Configure child's sampling state first
    child->fUsage = sampleUsage;

    // When an FP is sampled using variable matrix expressions, it is effectively being sampled
    // explicitly, except that the call site will automatically evaluate the matrix expression to
    // produce the float2 passed into this FP.
    if (sampleUsage.fExplicitCoords || variableMatrix) {
        child->addAndPushFlagToChildren(kSampledWithExplicitCoords_Flag);
    }

    // Push perspective matrix type to children
    if (sampleUsage.fHasPerspective) {
        child->addAndPushFlagToChildren(kNetTransformHasPerspective_Flag);
    }

    // If the child is sampled with a variable matrix expression, auto-generated code in
    // invokeChildWithMatrix() for this FP will refer to the local coordinates.
    if (variableMatrix) {
        this->setUsesSampleCoordsDirectly();
    }

    // If the child is not sampled explicitly and not already accessing sample coords directly
    // (through reference or variable matrix expansion), then mark that this FP tree relies on
    // coordinates at a lower level. If the child is sampled with explicit coordinates and
    // there isn't any other direct reference to the sample coords, we halt the upwards propagation
    // because it means this FP is determining coordinates on its own.
    if (!child->isSampledWithExplicitCoords()) {
        if ((child->fFlags & kUsesSampleCoordsDirectly_Flag ||
             child->fFlags & kUsesSampleCoordsIndirectly_Flag)) {
            fFlags |= kUsesSampleCoordsIndirectly_Flag;
        }
    }

    fRequestedFeatures |= child->fRequestedFeatures;

    // Record that the child is attached to us; this FP is the source of any uniform data needed
    // to evaluate the child sample matrix.
    child->fParent = this;
    fChildProcessors.push_back(std::move(child));

    // Validate: our sample strategy comes from a parent we shouldn't have yet.
    SkASSERT(!this->isSampledWithExplicitCoords() && !this->hasPerspectiveTransform() &&
             !fUsage.isSampled() && !fParent);
}

void GrFragmentProcessor::cloneAndRegisterAllChildProcessors(const GrFragmentProcessor& src) {
    for (int i = 0; i < src.numChildProcessors(); ++i) {
        if (auto fp = src.childProcessor(i)) {
            this->registerChild(fp->clone(), fp->sampleUsage());
        } else {
            this->registerChild(nullptr);
        }
    }
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MulChildByInputAlpha(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrBlendFragmentProcessor::Make(/*src=*/nullptr, std::move(fp), SkBlendMode::kDstIn);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MulInputByChildAlpha(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrBlendFragmentProcessor::Make(/*src=*/nullptr, std::move(fp), SkBlendMode::kSrcIn);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ModulateAlpha(
        std::unique_ptr<GrFragmentProcessor> inputFP, const SkPMColor4f& color) {
    auto colorFP = GrConstColorProcessor::Make(color);
    return GrBlendFragmentProcessor::Make(
            std::move(colorFP), std::move(inputFP), SkBlendMode::kSrcIn,
            GrBlendFragmentProcessor::BlendBehavior::kSkModeBehavior);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ModulateRGBA(
        std::unique_ptr<GrFragmentProcessor> inputFP, const SkPMColor4f& color) {
    auto colorFP = GrConstColorProcessor::Make(color);
    return GrBlendFragmentProcessor::Make(
            std::move(colorFP), std::move(inputFP), SkBlendMode::kModulate,
            GrBlendFragmentProcessor::BlendBehavior::kSkModeBehavior);
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
            return Make(this->childProcessor(0)->clone(), fSwizzle);
        }

    private:
        SwizzleFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp, const GrSwizzle& swizzle)
                : INHERITED(kSwizzleFragmentProcessor_ClassID, ProcessorOptimizationFlags(fp.get()))
                , fSwizzle(swizzle) {
            this->registerChild(std::move(fp));
        }

        std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    SkString childColor = this->invokeChild(0, args);

                    const SwizzleFragmentProcessor& sfp = args.fFp.cast<SwizzleFragmentProcessor>();
                    const GrSwizzle& swizzle = sfp.swizzle();
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                    fragBuilder->codeAppendf("return %s.%s;",
                                             childColor.c_str(), swizzle.asString().c_str());
                }
            };
            return std::make_unique<GLFP>();
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

        using INHERITED = GrFragmentProcessor;
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
            return Make(this->childProcessor(0)->clone());
        }

    private:
        PremulFragmentProcessor(std::unique_ptr<GrFragmentProcessor> processor)
                : INHERITED(kPremulFragmentProcessor_ClassID, OptFlags(processor.get())) {
            this->registerChild(std::move(processor));
        }

        std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                    SkString temp = this->invokeChild(/*childIndex=*/0, "half4(1)", args);
                    fragBuilder->codeAppendf("half4 color = %s;", temp.c_str());
                    fragBuilder->codeAppendf("color.rgb *= %s.rgb;", args.fInputColor);
                    fragBuilder->codeAppendf("return color * %s.a;", args.fInputColor);
                }
            };
            return std::make_unique<GLFP>();
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

        using INHERITED = GrFragmentProcessor;
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

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::Compose(
        std::unique_ptr<GrFragmentProcessor> f, std::unique_ptr<GrFragmentProcessor> g) {
    class ComposeProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> f,
                                                         std::unique_ptr<GrFragmentProcessor> g) {
            return std::unique_ptr<GrFragmentProcessor>(new ComposeProcessor(std::move(f),
                                                                             std::move(g)));
        }

        const char* name() const override { return "Compose"; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return std::unique_ptr<GrFragmentProcessor>(new ComposeProcessor(*this));
        }

    private:
        std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    SkString result = this->invokeChild(1, args);         // g(x)
                    result = this->invokeChild(0, result.c_str(), args);  // f(g(x))
                    args.fFragBuilder->codeAppendf("return %s;", result.c_str());
                }
            };
            return std::make_unique<GLFP>();
        }

        ComposeProcessor(std::unique_ptr<GrFragmentProcessor> f,
                         std::unique_ptr<GrFragmentProcessor> g)
                : INHERITED(kSeriesFragmentProcessor_ClassID,
                            f->optimizationFlags() & g->optimizationFlags()) {
            this->registerChild(std::move(f));
            this->registerChild(std::move(g));
        }

        ComposeProcessor(const ComposeProcessor& that)
                : INHERITED(kSeriesFragmentProcessor_ClassID, that.optimizationFlags()) {
            this->cloneAndRegisterAllChildProcessors(that);
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
            SkPMColor4f color = inColor;
            color = ConstantOutputForConstantInput(this->childProcessor(1), color);
            color = ConstantOutputForConstantInput(this->childProcessor(0), color);
            return color;
        }

        using INHERITED = GrFragmentProcessor;
    };

    // Allow either of the composed functions to be null.
    if (f == nullptr) {
        return g;
    }
    if (g == nullptr) {
        return f;
    }

    // Run an optimization pass on this composition.
    GrProcessorAnalysisColor inputColor;
    inputColor.setToUnknown();

    std::unique_ptr<GrFragmentProcessor> series[2] = {std::move(g), std::move(f)};
    GrColorFragmentProcessorAnalysis info(inputColor, series, SK_ARRAY_COUNT(series));

    SkPMColor4f knownColor;
    int leadingFPsToEliminate = info.initialProcessorsToEliminate(&knownColor);
    switch (leadingFPsToEliminate) {
        default:
            // We shouldn't eliminate more than we started with.
            SkASSERT(leadingFPsToEliminate <= 2);
            [[fallthrough]];
        case 0:
            // Compose the two processors as requested.
            return ComposeProcessor::Make(/*f=*/std::move(series[1]), /*g=*/std::move(series[0]));
        case 1:
            // Replace the first processor with a constant color.
            return ComposeProcessor::Make(/*f=*/std::move(series[1]),
                                          /*g=*/GrConstColorProcessor::Make(knownColor));
        case 2:
            // Replace the entire composition with a constant color.
            return GrConstColorProcessor::Make(knownColor);
    }
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor::CIter::CIter(const GrPaint& paint) {
    if (paint.hasCoverageFragmentProcessor()) {
        fFPStack.push_back(paint.getCoverageFragmentProcessor());
    }
    if (paint.hasColorFragmentProcessor()) {
        fFPStack.push_back(paint.getColorFragmentProcessor());
    }
}

GrFragmentProcessor::CIter::CIter(const GrPipeline& pipeline) {
    for (int i = pipeline.numFragmentProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(&pipeline.getFragmentProcessor(i));
    }
}

GrFragmentProcessor::CIter& GrFragmentProcessor::CIter::operator++() {
    SkASSERT(!fFPStack.empty());
    const GrFragmentProcessor* back = fFPStack.back();
    fFPStack.pop_back();
    for (int i = back->numChildProcessors() - 1; i >= 0; --i) {
        if (auto child = back->childProcessor(i)) {
            fFPStack.push_back(child);
        }
    }
    return *this;
}

