/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrBlendFragmentProcessor.h"

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

// Some of the CPU implementations of blend modes differ from the GPU enough that
// we can't use the CPU implementation to implement constantOutputForConstantInput.
static inline bool does_cpu_blend_impl_match_gpu(SkBlendMode mode) {
    // The non-separable modes differ too much. So does SoftLight. ColorBurn differs too much on our
    // test iOS device, but we just disable it across the board since it might differ on untested
    // GPUs.
    return mode <= SkBlendMode::kLastSeparableMode && mode != SkBlendMode::kSoftLight &&
           mode != SkBlendMode::kColorBurn;
}

static bool supports_shared_blend_logic(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kSrcIn:
        case SkBlendMode::kDstIn:
        case SkBlendMode::kSrcOut:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstATop:
        case SkBlendMode::kXor:
        case SkBlendMode::kPlus:
            return true;

        default:
            return false;
    }
}

//////////////////////////////////////////////////////////////////////////////

class BlendFragmentProcessor : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                                     std::unique_ptr<GrFragmentProcessor> dst,
                                                     SkBlendMode mode,
                                                     bool shareBlendLogic) {
        return std::unique_ptr<GrFragmentProcessor>(
                new BlendFragmentProcessor(std::move(src), std::move(dst), mode, shareBlendLogic));
    }

    const char* name() const override { return "Blend"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    BlendFragmentProcessor(std::unique_ptr<GrFragmentProcessor> src,
                           std::unique_ptr<GrFragmentProcessor> dst,
                           SkBlendMode mode,
                           bool shareBlendLogic)
            : INHERITED(kBlendFragmentProcessor_ClassID, OptFlags(src.get(), dst.get(), mode))
            , fMode(mode)
            , fShareBlendLogic(shareBlendLogic && supports_shared_blend_logic(mode)) {
        this->setIsBlendFunction();
        this->registerChild(std::move(src));
        this->registerChild(std::move(dst));
    }

    BlendFragmentProcessor(const BlendFragmentProcessor& that)
            : INHERITED(that)
            , fMode(that.fMode)
            , fShareBlendLogic(that.fShareBlendLogic) {}

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        return SkStringPrintf("(fMode=%s)", SkBlendMode_Name(fMode));
    }
#endif

    static OptimizationFlags OptFlags(const GrFragmentProcessor* src,
                                      const GrFragmentProcessor* dst, SkBlendMode mode) {
        OptimizationFlags flags;
        switch (mode) {
            case SkBlendMode::kClear:
            case SkBlendMode::kSrc:
            case SkBlendMode::kDst:
                SkDEBUGFAIL("Shouldn't have created a Blend FP as 'clear', 'src', or 'dst'.");
                flags = kNone_OptimizationFlags;
                break;

            // Produces opaque if both src and dst are opaque. These also will modulate the child's
            // output by either the input color or alpha. However, if the child is not compatible
            // with the coverage as alpha then it may produce a color that is not valid premul.
            case SkBlendMode::kSrcIn:
            case SkBlendMode::kDstIn:
            case SkBlendMode::kModulate:
                if (src && dst) {
                    flags = ProcessorOptimizationFlags(src) & ProcessorOptimizationFlags(dst) &
                            kPreservesOpaqueInput_OptimizationFlag;
                } else if (src) {
                    flags = ProcessorOptimizationFlags(src) &
                            ~kConstantOutputForConstantInput_OptimizationFlag;
                } else if (dst) {
                    flags = ProcessorOptimizationFlags(dst) &
                            ~kConstantOutputForConstantInput_OptimizationFlag;
                } else {
                    flags = kNone_OptimizationFlags;
                }
                break;

            // Produces zero when both are opaque, indeterminate if one is opaque.
            case SkBlendMode::kSrcOut:
            case SkBlendMode::kDstOut:
            case SkBlendMode::kXor:
                flags = kNone_OptimizationFlags;
                break;

            // Is opaque if the dst is opaque.
            case SkBlendMode::kSrcATop:
                flags = ProcessorOptimizationFlags(dst) & kPreservesOpaqueInput_OptimizationFlag;
                break;

            // DstATop is the converse of kSrcATop. Screen is also opaque if the src is a opaque.
            case SkBlendMode::kDstATop:
            case SkBlendMode::kScreen:
                flags = ProcessorOptimizationFlags(src) & kPreservesOpaqueInput_OptimizationFlag;
                break;

            // These modes are all opaque if either src or dst is opaque. All the advanced modes
            // compute alpha as src-over.
            case SkBlendMode::kSrcOver:
            case SkBlendMode::kDstOver:
            case SkBlendMode::kPlus:
            case SkBlendMode::kOverlay:
            case SkBlendMode::kDarken:
            case SkBlendMode::kLighten:
            case SkBlendMode::kColorDodge:
            case SkBlendMode::kColorBurn:
            case SkBlendMode::kHardLight:
            case SkBlendMode::kSoftLight:
            case SkBlendMode::kDifference:
            case SkBlendMode::kExclusion:
            case SkBlendMode::kMultiply:
            case SkBlendMode::kHue:
            case SkBlendMode::kSaturation:
            case SkBlendMode::kColor:
            case SkBlendMode::kLuminosity:
                flags = (ProcessorOptimizationFlags(src) | ProcessorOptimizationFlags(dst)) &
                        kPreservesOpaqueInput_OptimizationFlag;
                break;
        }
        if (does_cpu_blend_impl_match_gpu(mode) &&
            (!src || src->hasConstantOutputForConstantInput()) &&
            (!dst || dst->hasConstantOutputForConstantInput())) {
            flags |= kConstantOutputForConstantInput_OptimizationFlag;
        }
        return flags;
    }

    void onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const override {
        if (fShareBlendLogic) {
            b->add32(-1);
        } else {
            b->add32((int)fMode);
        }
    }

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const BlendFragmentProcessor& cs = other.cast<BlendFragmentProcessor>();
        return fMode == cs.fMode;
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        const auto* src = this->childProcessor(0);
        const auto* dst = this->childProcessor(1);

        SkPMColor4f srcColor = ConstantOutputForConstantInput(src, input);
        SkPMColor4f dstColor = ConstantOutputForConstantInput(dst, input);

        return SkBlendMode_Apply(fMode, srcColor, dstColor);
    }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    SkBlendMode fMode;
    bool fShareBlendLogic;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};

/////////////////////////////////////////////////////////////////////


GR_DEFINE_FRAGMENT_PROCESSOR_TEST(BlendFragmentProcessor);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> BlendFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    // Create one or two random fragment processors.
    std::unique_ptr<GrFragmentProcessor> src(GrProcessorUnitTest::MakeOptionalChildFP(d));
    std::unique_ptr<GrFragmentProcessor> dst(GrProcessorUnitTest::MakeChildFP(d));
    if (d->fRandom->nextBool()) {
        std::swap(src, dst);
    }
    bool shareLogic = d->fRandom->nextBool();

    SkBlendMode mode;
    do {
        mode = static_cast<SkBlendMode>(d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
    } while (SkBlendMode::kClear == mode || SkBlendMode::kSrc == mode || SkBlendMode::kDst == mode);
    return std::unique_ptr<GrFragmentProcessor>(
            new BlendFragmentProcessor(std::move(src), std::move(dst), mode, shareLogic));
}
#endif

std::unique_ptr<GrFragmentProcessor> BlendFragmentProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new BlendFragmentProcessor(*this));
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> BlendFragmentProcessor::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            const BlendFragmentProcessor& bfp = args.fFp.cast<BlendFragmentProcessor>();
            const SkBlendMode mode = bfp.fMode;

            // Invoke src/dst with our input color (or substitute input color if no child FP)
            SkString srcColor = this->invokeChild(0, args);
            SkString dstColor = this->invokeChild(1, args);

            if (bfp.fShareBlendLogic) {
                // Handle basic Porter-Duff blend ops by multiplying with uniforms.
                const char* blendOp;
                fBlendOpUniform = args.fUniformHandler->addUniform(&args.fFp,
                                                                   kFragment_GrShaderFlag,
                                                                   SkSLType::kHalf4,
                                                                   "blendOp", &blendOp);
                fragBuilder->codeAppendf(
                        "half4 src = %s;"
                        "half4 dst = %s;"
                        "half2 coeff = %s.xy + (%s.zw * (half2(dst.a, src.a) + min(%s.zw, 0)));"
                        "return min(half4(1), src * coeff.x + dst * coeff.y);",
                        srcColor.c_str(), dstColor.c_str(),
                        blendOp, blendOp, blendOp);
            } else {
                // Blend src and dst colors together using a built-in blend function.
                fragBuilder->codeAppendf("return %s(%s, %s);",
                                         GrGLSLBlend::BlendFuncName(mode),
                                         srcColor.c_str(),
                                         dstColor.c_str());
            }
        }

        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const BlendFragmentProcessor& bfp = fp.cast<BlendFragmentProcessor>();
            const SkBlendMode mode = bfp.fMode;

            if (bfp.fShareBlendLogic) {
                switch (mode) {
                    case SkBlendMode::kSrcOver: pdman.set4f(fBlendOpUniform,  1, 0,  0, -1); break;
                    case SkBlendMode::kDstOver: pdman.set4f(fBlendOpUniform,  0, 1, -1,  0); break;
                    case SkBlendMode::kSrcIn:   pdman.set4f(fBlendOpUniform,  0, 0,  1,  0); break;
                    case SkBlendMode::kDstIn:   pdman.set4f(fBlendOpUniform,  0, 0,  0,  1); break;
                    case SkBlendMode::kSrcOut:  pdman.set4f(fBlendOpUniform,  0, 0, -1,  0); break;
                    case SkBlendMode::kDstOut:  pdman.set4f(fBlendOpUniform,  0, 0,  0, -1); break;
                    case SkBlendMode::kSrcATop: pdman.set4f(fBlendOpUniform,  0, 0,  1, -1); break;
                    case SkBlendMode::kDstATop: pdman.set4f(fBlendOpUniform,  0, 0, -1,  1); break;
                    case SkBlendMode::kXor:     pdman.set4f(fBlendOpUniform,  0, 0, -1, -1); break;
                    case SkBlendMode::kPlus:    pdman.set4f(fBlendOpUniform,  1, 1,  0,  0); break;
                    default:                    SkDEBUGFAIL("unexpected blend mode"); break;
                }
            }
        }

        UniformHandle fBlendOpUniform;
    };

    return std::make_unique<Impl>();
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrBlendFragmentProcessor::Make(
        std::unique_ptr<GrFragmentProcessor> src,
        std::unique_ptr<GrFragmentProcessor> dst,
        SkBlendMode mode,
        bool shareBlendLogic) {
    switch (mode) {
        case SkBlendMode::kClear:
            return GrFragmentProcessor::MakeColor(SK_PMColor4fTRANSPARENT);
        case SkBlendMode::kSrc:
            return src;
        case SkBlendMode::kDst:
            return dst;
        default:
            return BlendFragmentProcessor::Make(
                    std::move(src), std::move(dst), mode, shareBlendLogic);
    }
}
