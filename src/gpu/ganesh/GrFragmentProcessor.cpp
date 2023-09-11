/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/GrFragmentProcessor.h"

#include "include/core/SkM44.h"
#include "src/base/SkVx.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

bool GrFragmentProcessor::isEqual(const GrFragmentProcessor& that) const {
    if (this->classID() != that.classID()) {
        return false;
    }
    if (this->sampleUsage() != that.sampleUsage()) {
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

void GrFragmentProcessor::visitProxies(const GrVisitProxyFunc& func) const {
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

void GrFragmentProcessor::visitWithImpls(
        const std::function<void(const GrFragmentProcessor&, ProgramImpl&)>& f,
        ProgramImpl& impl) const {
    f(*this, impl);
    SkASSERT(impl.numChildProcessors() == this->numChildProcessors());
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (const auto* child = this->childProcessor(i)) {
            child->visitWithImpls(f, *impl.childProcessor(i));
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

#if defined(GR_TEST_UTILS)
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

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrFragmentProcessor::makeProgramImpl() const {
    std::unique_ptr<ProgramImpl> impl = this->onMakeProgramImpl();
    impl->fChildProcessors.push_back_n(fChildProcessors.size());
    for (int i = 0; i < fChildProcessors.size(); ++i) {
        impl->fChildProcessors[i] = fChildProcessors[i] ? fChildProcessors[i]->makeProgramImpl()
                                                        : nullptr;
    }
    return impl;
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
    SkASSERT(sampleUsage.isSampled());

    if (!child) {
        fChildProcessors.push_back(nullptr);
        return;
    }

    // The child should not have been attached to another FP already and not had any sampling
    // strategy set on it.
    SkASSERT(!child->fParent && !child->sampleUsage().isSampled());

    // Configure child's sampling state first
    child->fUsage = sampleUsage;

    // Propagate the "will read dest-color" flag up to parent FPs.
    if (child->willReadDstColor()) {
        this->setWillReadDstColor();
    }

    // If this child receives passthrough or matrix transformed coords from its parent then note
    // that the parent's coords are used indirectly to ensure that they aren't omitted.
    if ((sampleUsage.isPassThrough() || sampleUsage.isUniformMatrix()) &&
        child->usesSampleCoords()) {
        fFlags |= kUsesSampleCoordsIndirectly_Flag;
    }

    // Record that the child is attached to us; this FP is the source of any uniform data needed
    // to evaluate the child sample matrix.
    child->fParent = this;
    fChildProcessors.push_back(std::move(child));

    // Validate: our sample strategy comes from a parent we shouldn't have yet.
    SkASSERT(!fUsage.isSampled() && !fParent);
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

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MakeColor(SkPMColor4f color) {
    // Use ColorFilter signature/factory to get the constant output for constant input optimization
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "uniform half4 color;"
        "half4 main(half4 inColor) { return color; }"
    );
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(effect, "color_fp", /*inputFP=*/nullptr,
                          color.isOpaque() ? GrSkSLFP::OptFlags::kPreservesOpaqueInput
                                           : GrSkSLFP::OptFlags::kNone,
                          "color", color);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::MulInputByChildAlpha(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrBlendFragmentProcessor::Make<SkBlendMode::kSrcIn>(/*src=*/nullptr, std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ApplyPaintAlpha(
        std::unique_ptr<GrFragmentProcessor> child) {
    SkASSERT(child);
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "uniform colorFilter fp;"
        "half4 main(half4 inColor) {"
            "return fp.eval(inColor.rgb1) * inColor.a;"
        "}"
    );
    return GrSkSLFP::Make(effect, "ApplyPaintAlpha", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput |
                          GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                          "fp", std::move(child));
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ModulateRGBA(
        std::unique_ptr<GrFragmentProcessor> inputFP, const SkPMColor4f& color) {
    auto colorFP = MakeColor(color);
    return GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(colorFP),
                                                                  std::move(inputFP));
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ClampOutput(
        std::unique_ptr<GrFragmentProcessor> fp) {
    SkASSERT(fp);
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 inColor) {"
            "return saturate(inColor);"
        "}"
    );
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(effect, "Clamp", std::move(fp),
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::SwizzleOutput(
        std::unique_ptr<GrFragmentProcessor> fp, const skgpu::Swizzle& swizzle) {
    class SwizzleFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                         const skgpu::Swizzle& swizzle) {
            return std::unique_ptr<GrFragmentProcessor>(
                    new SwizzleFragmentProcessor(std::move(fp), swizzle));
        }

        const char* name() const override { return "Swizzle"; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0)->clone(), fSwizzle);
        }

    private:
        SwizzleFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp,
                                 const skgpu::Swizzle& swizzle)
                : INHERITED(kSwizzleFragmentProcessor_ClassID, ProcessorOptimizationFlags(fp.get()))
                , fSwizzle(swizzle) {
            this->registerChild(std::move(fp));
        }

        std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
            class Impl : public ProgramImpl {
            public:
                void emitCode(EmitArgs& args) override {
                    SkString childColor = this->invokeChild(0, args);

                    const SwizzleFragmentProcessor& sfp = args.fFp.cast<SwizzleFragmentProcessor>();
                    const skgpu::Swizzle& swizzle = sfp.fSwizzle;
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                    fragBuilder->codeAppendf("return %s.%s;",
                                             childColor.c_str(), swizzle.asString().c_str());
                }
            };
            return std::make_unique<Impl>();
        }

        void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const override {
            b->add32(fSwizzle.asKey());
        }

        bool onIsEqual(const GrFragmentProcessor& other) const override {
            const SwizzleFragmentProcessor& sfp = other.cast<SwizzleFragmentProcessor>();
            return fSwizzle == sfp.fSwizzle;
        }

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
            return fSwizzle.applyTo(ConstantOutputForConstantInput(this->childProcessor(0), input));
        }

        skgpu::Swizzle fSwizzle;

        using INHERITED = GrFragmentProcessor;
    };

    if (!fp) {
        return nullptr;
    }
    if (skgpu::Swizzle::RGBA() == swizzle) {
        return fp;
    }
    return SwizzleFragmentProcessor::Make(std::move(fp), swizzle);
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::OverrideInput(
        std::unique_ptr<GrFragmentProcessor> fp, const SkPMColor4f& color) {
    if (!fp) {
        return nullptr;
    }
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "uniform colorFilter fp;"  // Declared as colorFilter so we can pass a color
        "uniform half4 color;"
        "half4 main(half4 inColor) {"
            "return fp.eval(color);"
        "}"
    );
    return GrSkSLFP::Make(effect, "OverrideInput", /*inputFP=*/nullptr,
                          color.isOpaque() ? GrSkSLFP::OptFlags::kPreservesOpaqueInput
                                           : GrSkSLFP::OptFlags::kNone,
                          "fp", std::move(fp),
                          "color", color);
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::DisableCoverageAsAlpha(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp || !fp->compatibleWithCoverageAsAlpha()) {
        return fp;
    }
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 inColor) { return inColor; }"
    );
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(effect, "DisableCoverageAsAlpha", std::move(fp),
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::DestColor() {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForBlender,
        "half4 main(half4 src, half4 dst) {"
            "return dst;"
        "}"
    );
    return GrSkSLFP::Make(effect, "DestColor", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone);
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
        std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
            class Impl : public ProgramImpl {
            public:
                void emitCode(EmitArgs& args) override {
                    SkString result = this->invokeChild(1, args);         // g(x)
                    result = this->invokeChild(0, result.c_str(), args);  // f(g(x))
                    args.fFragBuilder->codeAppendf("return %s;", result.c_str());
                }
            };
            return std::make_unique<Impl>();
        }

        ComposeProcessor(std::unique_ptr<GrFragmentProcessor> f,
                         std::unique_ptr<GrFragmentProcessor> g)
                : INHERITED(kSeriesFragmentProcessor_ClassID,
                            f->optimizationFlags() & g->optimizationFlags()) {
            this->registerChild(std::move(f));
            this->registerChild(std::move(g));
        }

        ComposeProcessor(const ComposeProcessor& that) : INHERITED(that) {}

        void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

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
    GrColorFragmentProcessorAnalysis info(inputColor, series, std::size(series));

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
                                          /*g=*/MakeColor(knownColor));
        case 2:
            // Replace the entire composition with a constant color.
            return MakeColor(knownColor);
    }
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::ColorMatrix(
        std::unique_ptr<GrFragmentProcessor> child,
        const float matrix[20],
        bool unpremulInput,
        bool clampRGBOutput,
        bool premulOutput) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "uniform half4x4 m;"
        "uniform half4 v;"
        "uniform int unpremulInput;"   // always specialized
        "uniform int clampRGBOutput;"  // always specialized
        "uniform int premulOutput;"    // always specialized
        "half4 main(half4 color) {"
            "if (bool(unpremulInput)) {"
                "color = unpremul(color);"
            "}"
            "color = m * color + v;"
            "if (bool(clampRGBOutput)) {"
                "color = saturate(color);"
            "} else {"
                "color.a = saturate(color.a);"
            "}"
            "if (bool(premulOutput)) {"
                "color.rgb *= color.a;"
            "}"
            "return color;"
        "}"
    );
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));

    SkM44 m44(matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3],
              matrix[ 5], matrix[ 6], matrix[ 7], matrix[ 8],
              matrix[10], matrix[11], matrix[12], matrix[13],
              matrix[15], matrix[16], matrix[17], matrix[18]);
    SkV4 v4 = {matrix[4], matrix[9], matrix[14], matrix[19]};
    return GrSkSLFP::Make(effect, "ColorMatrix", std::move(child), GrSkSLFP::OptFlags::kNone,
                          "m", m44,
                          "v", v4,
                          "unpremulInput",  GrSkSLFP::Specialize(unpremulInput  ? 1 : 0),
                          "clampRGBOutput", GrSkSLFP::Specialize(clampRGBOutput ? 1 : 0),
                          "premulOutput",   GrSkSLFP::Specialize(premulOutput   ? 1 : 0));
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::SurfaceColor() {
    class SurfaceColorProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make() {
            return std::unique_ptr<GrFragmentProcessor>(new SurfaceColorProcessor());
        }

        std::unique_ptr<GrFragmentProcessor> clone() const override { return Make(); }

        const char* name() const override { return "SurfaceColor"; }

    private:
        std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
            class Impl : public ProgramImpl {
            public:
                void emitCode(EmitArgs& args) override {
                    const char* dstColor = args.fFragBuilder->dstColor();
                    args.fFragBuilder->codeAppendf("return %s;", dstColor);
                }
            };
            return std::make_unique<Impl>();
        }

        SurfaceColorProcessor()
                : INHERITED(kSurfaceColorProcessor_ClassID, kNone_OptimizationFlags) {
            this->setWillReadDstColor();
        }

        void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        using INHERITED = GrFragmentProcessor;
    };

    return SurfaceColorProcessor::Make();
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::DeviceSpace(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }

    class DeviceSpace : GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp) {
            return std::unique_ptr<GrFragmentProcessor>(new DeviceSpace(std::move(fp)));
        }

    private:
        DeviceSpace(std::unique_ptr<GrFragmentProcessor> fp)
                : GrFragmentProcessor(kDeviceSpace_ClassID, fp->optimizationFlags()) {
            // Passing FragCoord here is the reason this is a subclass and not a runtime-FP.
            this->registerChild(std::move(fp), SkSL::SampleUsage::FragCoord());
        }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            auto child = this->childProcessor(0)->clone();
            return std::unique_ptr<GrFragmentProcessor>(new DeviceSpace(std::move(child)));
        }

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& f) const override {
            return this->childProcessor(0)->constantOutputForConstantInput(f);
        }

        std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
            class Impl : public ProgramImpl {
            public:
                Impl() = default;
                void emitCode(ProgramImpl::EmitArgs& args) override {
                    auto child = this->invokeChild(0, args.fInputColor, args, "sk_FragCoord.xy");
                    args.fFragBuilder->codeAppendf("return %s;", child.c_str());
                }
            };
            return std::make_unique<Impl>();
        }

        void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor& processor) const override { return true; }

        const char* name() const override { return "DeviceSpace"; }
    };

    return DeviceSpace::Make(std::move(fp));
}

//////////////////////////////////////////////////////////////////////////////

#define CLIP_EDGE_SKSL              \
    "const int kFillBW = 0;"        \
    "const int kFillAA = 1;"        \
    "const int kInverseFillBW = 2;" \
    "const int kInverseFillAA = 3;"

static_assert(static_cast<int>(GrClipEdgeType::kFillBW) == 0);
static_assert(static_cast<int>(GrClipEdgeType::kFillAA) == 1);
static_assert(static_cast<int>(GrClipEdgeType::kInverseFillBW) == 2);
static_assert(static_cast<int>(GrClipEdgeType::kInverseFillAA) == 3);

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::Rect(
        std::unique_ptr<GrFragmentProcessor> inputFP, GrClipEdgeType edgeType, SkRect rect) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
    CLIP_EDGE_SKSL
        "uniform int edgeType;"  // GrClipEdgeType, specialized
        "uniform float4 rectUniform;"

        "half4 main(float2 xy) {"
            "half coverage;"
            "if (edgeType == kFillBW || edgeType == kInverseFillBW) {"
                // non-AA
                "coverage = half(all(greaterThan(float4(sk_FragCoord.xy, rectUniform.zw),"
                                                "float4(rectUniform.xy, sk_FragCoord.xy))));"
            "} else {"
                // compute coverage relative to left and right edges, add, then subtract 1 to
                // account for double counting. And similar for top/bottom.
                "half4 dists4 = saturate(half4(1, 1, -1, -1) *"
                                        "half4(sk_FragCoord.xyxy - rectUniform));"
                "half2 dists2 = dists4.xy + dists4.zw - 1;"
                "coverage = dists2.x * dists2.y;"
            "}"

            "if (edgeType == kInverseFillBW || edgeType == kInverseFillAA) {"
                "coverage = 1.0 - coverage;"
            "}"

            "return half4(coverage);"
        "}"
    );

    SkASSERT(rect.isSorted());
    // The AA math in the shader evaluates to 0 at the uploaded coordinates, so outset by 0.5
    // to interpolate from 0 at a half pixel inset and 1 at a half pixel outset of rect.
    SkRect rectUniform = GrClipEdgeTypeIsAA(edgeType) ? rect.makeOutset(.5f, .5f) : rect;

    auto rectFP = GrSkSLFP::Make(effect, "Rect", /*inputFP=*/nullptr,
                                 GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                                "edgeType", GrSkSLFP::Specialize(static_cast<int>(edgeType)),
                                "rectUniform", rectUniform);
    return GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(rectFP),
                                                                  std::move(inputFP));
}

GrFPResult GrFragmentProcessor::Circle(std::unique_ptr<GrFragmentProcessor> inputFP,
                                       GrClipEdgeType edgeType,
                                       SkPoint center,
                                       float radius) {
    // A radius below half causes the implicit insetting done by this processor to become
    // inverted. We could handle this case by making the processor code more complicated.
    if (radius < .5f && GrClipEdgeTypeIsInverseFill(edgeType)) {
        return GrFPFailure(std::move(inputFP));
    }

    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
    CLIP_EDGE_SKSL
        "uniform int edgeType;"  // GrClipEdgeType, specialized
        // The circle uniform is (center.x, center.y, radius + 0.5, 1 / (radius + 0.5)) for regular
        // fills and (..., radius - 0.5, 1 / (radius - 0.5)) for inverse fills.
        "uniform float4 circle;"

        "half4 main(float2 xy) {"
            // TODO: Right now the distance to circle calculation is performed in a space normalized
            // to the radius and then denormalized. This is to mitigate overflow on devices that
            // don't have full float.
            "half d;"
            "if (edgeType == kInverseFillBW || edgeType == kInverseFillAA) {"
                "d = half((length((circle.xy - sk_FragCoord.xy) * circle.w) - 1.0) * circle.z);"
            "} else {"
                "d = half((1.0 - length((circle.xy - sk_FragCoord.xy) * circle.w)) * circle.z);"
            "}"
            "return half4((edgeType == kFillAA || edgeType == kInverseFillAA)"
                              "? saturate(d)"
                              ": (d > 0.5 ? 1 : 0));"
        "}"
    );

    SkScalar effectiveRadius = radius;
    if (GrClipEdgeTypeIsInverseFill(edgeType)) {
        effectiveRadius -= 0.5f;
        // When the radius is 0.5 effectiveRadius is 0 which causes an inf * 0 in the shader.
        effectiveRadius = std::max(0.001f, effectiveRadius);
    } else {
        effectiveRadius += 0.5f;
    }
    SkV4 circle = {center.fX, center.fY, effectiveRadius, SkScalarInvert(effectiveRadius)};

    auto circleFP = GrSkSLFP::Make(effect, "Circle", /*inputFP=*/nullptr,
                                   GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                                   "edgeType", GrSkSLFP::Specialize(static_cast<int>(edgeType)),
                                   "circle", circle);
    return GrFPSuccess(GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(inputFP),
                                                                              std::move(circleFP)));
}

GrFPResult GrFragmentProcessor::Ellipse(std::unique_ptr<GrFragmentProcessor> inputFP,
                                        GrClipEdgeType edgeType,
                                        SkPoint center,
                                        SkPoint radii,
                                        const GrShaderCaps& caps) {
    const bool medPrecision = !caps.fFloatIs32Bits;

    // Small radii produce bad results on devices without full float.
    if (medPrecision && (radii.fX < 0.5f || radii.fY < 0.5f)) {
        return GrFPFailure(std::move(inputFP));
    }
    // Very narrow ellipses produce bad results on devices without full float
    if (medPrecision && (radii.fX > 255*radii.fY || radii.fY > 255*radii.fX)) {
        return GrFPFailure(std::move(inputFP));
    }
    // Very large ellipses produce bad results on devices without full float
    if (medPrecision && (radii.fX > 16384 || radii.fY > 16384)) {
        return GrFPFailure(std::move(inputFP));
    }

    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
    CLIP_EDGE_SKSL
        "uniform int edgeType;"      // GrClipEdgeType, specialized
        "uniform int medPrecision;"  // !sk_Caps.floatIs32Bits, specialized

        "uniform float4 ellipse;"
        "uniform float2 scale;"    // only for medPrecision

        "half4 main(float2 xy) {"
            // d is the offset to the ellipse center
            "float2 d = sk_FragCoord.xy - ellipse.xy;"
            // If we're on a device with a "real" mediump then we'll do the distance computation in
            // a space that is normalized by the larger radius or 128, whichever is smaller. The
            // scale uniform will be scale, 1/scale. The inverse squared radii uniform values are
            // already in this normalized space. The center is not.
            "if (bool(medPrecision)) {"
                "d *= scale.y;"
            "}"
            "float2 Z = d * ellipse.zw;"
            // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
            "float implicit = dot(Z, d) - 1;"
            // grad_dot is the squared length of the gradient of the implicit.
            "float grad_dot = 4 * dot(Z, Z);"
            // Avoid calling inversesqrt on zero.
            "if (bool(medPrecision)) {"
                "grad_dot = max(grad_dot, 6.1036e-5);"
            "} else {"
                "grad_dot = max(grad_dot, 1.1755e-38);"
            "}"
            "float approx_dist = implicit * inversesqrt(grad_dot);"
            "if (bool(medPrecision)) {"
                "approx_dist *= scale.x;"
            "}"

            "half alpha;"
            "if (edgeType == kFillBW) {"
                "alpha = approx_dist > 0.0 ? 0.0 : 1.0;"
            "} else if (edgeType == kFillAA) {"
                "alpha = saturate(0.5 - half(approx_dist));"
            "} else if (edgeType == kInverseFillBW) {"
                "alpha = approx_dist > 0.0 ? 1.0 : 0.0;"
            "} else {"  // edgeType == kInverseFillAA
                "alpha = saturate(0.5 + half(approx_dist));"
            "}"
            "return half4(alpha);"
        "}"
    );

    float invRXSqd;
    float invRYSqd;
    SkV2 scale = {1, 1};
    // If we're using a scale factor to work around precision issues, choose the larger radius as
    // the scale factor. The inv radii need to be pre-adjusted by the scale factor.
    if (medPrecision) {
        if (radii.fX > radii.fY) {
            invRXSqd = 1.f;
            invRYSqd = (radii.fX * radii.fX) / (radii.fY * radii.fY);
            scale = {radii.fX, 1.f / radii.fX};
        } else {
            invRXSqd = (radii.fY * radii.fY) / (radii.fX * radii.fX);
            invRYSqd = 1.f;
            scale = {radii.fY, 1.f / radii.fY};
        }
    } else {
        invRXSqd = 1.f / (radii.fX * radii.fX);
        invRYSqd = 1.f / (radii.fY * radii.fY);
    }
    SkV4 ellipse = {center.fX, center.fY, invRXSqd, invRYSqd};

    auto ellipseFP = GrSkSLFP::Make(effect, "Ellipse", /*inputFP=*/nullptr,
                                    GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                                    "edgeType", GrSkSLFP::Specialize(static_cast<int>(edgeType)),
                                    "medPrecision",  GrSkSLFP::Specialize<int>(medPrecision),
                                    "ellipse", ellipse,
                                    "scale", scale);
    return GrFPSuccess(GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(ellipseFP),
                                                                              std::move(inputFP)));
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrFragmentProcessor::HighPrecision(
        std::unique_ptr<GrFragmentProcessor> fp) {
    class HighPrecisionFragmentProcessor : public GrFragmentProcessor {
    public:
        static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp) {
            return std::unique_ptr<GrFragmentProcessor>(
                    new HighPrecisionFragmentProcessor(std::move(fp)));
        }

        const char* name() const override { return "HighPrecision"; }

        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0)->clone());
        }

    private:
        HighPrecisionFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp)
                : INHERITED(kHighPrecisionFragmentProcessor_ClassID,
                            ProcessorOptimizationFlags(fp.get())) {
            this->registerChild(std::move(fp));
        }

        std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
            class Impl : public ProgramImpl {
            public:
                void emitCode(EmitArgs& args) override {
                    SkString childColor = this->invokeChild(0, args);

                    args.fFragBuilder->forceHighPrecision();
                    args.fFragBuilder->codeAppendf("return %s;", childColor.c_str());
                }
            };
            return std::make_unique<Impl>();
        }

        void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}
        bool onIsEqual(const GrFragmentProcessor& other) const override { return true; }

        SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
            return ConstantOutputForConstantInput(this->childProcessor(0), input);
        }

        using INHERITED = GrFragmentProcessor;
    };

    return HighPrecisionFragmentProcessor::Make(std::move(fp));
}

//////////////////////////////////////////////////////////////////////////////

using ProgramImpl = GrFragmentProcessor::ProgramImpl;

void ProgramImpl::setData(const GrGLSLProgramDataManager& pdman,
                          const GrFragmentProcessor& processor) {
    this->onSetData(pdman, processor);
}

SkString ProgramImpl::invokeChild(int childIndex,
                                  const char* inputColor,
                                  const char* destColor,
                                  EmitArgs& args,
                                  std::string_view skslCoords) {
    SkASSERT(childIndex >= 0);

    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        // If no child processor is provided, return the input color as-is.
        return SkString(inputColor);
    }

    auto invocation = SkStringPrintf("%s(%s", this->childProcessor(childIndex)->functionName(),
                                     inputColor);

    if (childProc->isBlendFunction()) {
        if (!destColor) {
            destColor = args.fFp.isBlendFunction() ? args.fDestColor : "half4(1)";
        }
        invocation.appendf(", %s", destColor);
    }

    // Assert that the child has no sample matrix. A uniform matrix sample call would go through
    // invokeChildWithMatrix, not here.
    SkASSERT(!childProc->sampleUsage().isUniformMatrix());

    if (args.fFragBuilder->getProgramBuilder()->fragmentProcessorHasCoordsParam(childProc)) {
        SkASSERT(!childProc->sampleUsage().isFragCoord() || skslCoords == "sk_FragCoord.xy");
        // The child's function takes a half4 color and a float2 coordinate
        if (!skslCoords.empty()) {
            invocation.appendf(", %.*s", (int)skslCoords.size(), skslCoords.data());
        } else {
            invocation.appendf(", %s", args.fSampleCoord);
        }
    }

    invocation.append(")");
    return invocation;
}

SkString ProgramImpl::invokeChildWithMatrix(int childIndex,
                                            const char* inputColor,
                                            const char* destColor,
                                            EmitArgs& args) {
    SkASSERT(childIndex >= 0);

    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        // If no child processor is provided, return the input color as-is.
        return SkString(inputColor);
    }

    SkASSERT(childProc->sampleUsage().isUniformMatrix());

    // Every uniform matrix has the same (initial) name. Resolve that into the mangled name:
    GrShaderVar uniform = args.fUniformHandler->getUniformMapping(
            args.fFp, SkString(SkSL::SampleUsage::MatrixUniformName()));
    SkASSERT(uniform.getType() == SkSLType::kFloat3x3);
    const SkString& matrixName(uniform.getName());

    auto invocation = SkStringPrintf("%s(%s", this->childProcessor(childIndex)->functionName(),
                                     inputColor);

    if (childProc->isBlendFunction()) {
        if (!destColor) {
            destColor = args.fFp.isBlendFunction() ? args.fDestColor : "half4(1)";
        }
        invocation.appendf(", %s", destColor);
    }

    // Produce a string containing the call to the helper function. We have a uniform variable
    // containing our transform (matrixName). If the parent coords were produced by uniform
    // transforms, then the entire expression (matrixName * coords) is lifted to a vertex shader
    // and is stored in a varying. In that case, childProc will not be sampled explicitly, so its
    // function signature will not take in coords.
    //
    // In all other cases, we need to insert sksl to compute matrix * parent coords and then invoke
    // the function.
    if (args.fFragBuilder->getProgramBuilder()->fragmentProcessorHasCoordsParam(childProc)) {
        // Only check perspective for this specific matrix transform, not the aggregate FP property.
        // Any parent perspective will have already been applied when evaluated in the FS.
        if (childProc->sampleUsage().hasPerspective()) {
            invocation.appendf(", proj((%s) * %s.xy1)", matrixName.c_str(), args.fSampleCoord);
        } else if (args.fShaderCaps->fNonsquareMatrixSupport) {
            invocation.appendf(", float3x2(%s) * %s.xy1", matrixName.c_str(), args.fSampleCoord);
        } else {
            invocation.appendf(", ((%s) * %s.xy1).xy", matrixName.c_str(), args.fSampleCoord);
        }
    }

    invocation.append(")");
    return invocation;
}
