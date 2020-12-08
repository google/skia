/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSkSLFP.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/GrContext_Base.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrTexture.h"
#include "src/sksl/SkSLUtil.h"

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    GrGLSLSkSLFP(SkSL::PipelineStageArgs&& args) : fArgs(std::move(args)) {}

    SkSL::String expandFormatArgs(const SkSL::String& raw,
                                  EmitArgs& args,
                                  const char* sampleCoords,
                                  std::vector<SkSL::Compiler::FormatArg>::const_iterator& fmtArg) {
        SkSL::String result;
        int substringStartIndex = 0;
        for (size_t i = 0; i < raw.length(); ++i) {
            char c = raw[i];
            if (c == SkSL::Compiler::kFormatArgPlaceholder) {
                result += SkSL::StringFragment(raw.c_str() + substringStartIndex,
                                               i - substringStartIndex);
                const SkSL::Compiler::FormatArg& arg = *fmtArg++;
                switch (arg.fKind) {
                    case SkSL::Compiler::FormatArg::Kind::kCoords:
                        // See note about helper functions in emitCode
                        SkASSERT(sampleCoords);
                        result += sampleCoords ? sampleCoords : "float2(0)";
                        break;
                    case SkSL::Compiler::FormatArg::Kind::kUniform:
                        result += args.fUniformHandler->getUniformCStr(fUniformHandles[arg.fIndex]);
                        break;
                    case SkSL::Compiler::FormatArg::Kind::kChildProcessor: {
                        SkSL::String coords =
                                this->expandFormatArgs(arg.fCoords, args, sampleCoords, fmtArg);
                        result += this->invokeChild(arg.fIndex, args, coords).c_str();
                        break;
                    }
                    case SkSL::Compiler::FormatArg::Kind::kChildProcessorWithMatrix: {
                        const auto& fp(args.fFp.cast<GrSkSLFP>());
                        const auto& sampleUsages(fp.fEffect->fSampleUsages);

                        SkASSERT((size_t)arg.fIndex < sampleUsages.size());
                        const SkSL::SampleUsage& sampleUsage(sampleUsages[arg.fIndex]);

                        SkSL::String coords =
                                this->expandFormatArgs(arg.fCoords, args, sampleCoords, fmtArg);
                        result += this->invokeChildWithMatrix(
                                              arg.fIndex, args,
                                              sampleUsage.hasUniformMatrix() ? "" : coords)
                                          .c_str();
                        break;
                    }
                    case SkSL::Compiler::FormatArg::Kind::kFunctionName:
                        SkASSERT((int) fFunctionNames.size() > arg.fIndex);
                        result += fFunctionNames[arg.fIndex].c_str();
                        break;
                }
                substringStartIndex = i + 1;
            }
        }
        result += SkSL::StringFragment(raw.c_str() + substringStartIndex,
                                       raw.length() - substringStartIndex);
        return result;
    }

    void emitCode(EmitArgs& args) override {
        const GrSkSLFP& fp = args.fFp.cast<GrSkSLFP>();
        for (const auto& v : fp.fEffect->uniforms()) {
            auto handle = args.fUniformHandler->addUniformArray(&fp,
                                                                kFragment_GrShaderFlag,
                                                                v.fGPUType,
                                                                v.fName.c_str(),
                                                                v.isArray() ? v.fCount : 0);
            fUniformHandles.push_back(handle);
        }
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        std::vector<SkString> childNames;
        // We need to ensure that we emit each child's helper function at least once.
        // Any child FP that isn't sampled won't trigger a call otherwise, leading to asserts later.
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            if (this->childProcessor(i)) {
                this->emitChildFunction(i, args);
            }
        }
        for (const auto& f : fArgs.fFunctions) {
            fFunctionNames.push_back(fragBuilder->getMangledFunctionName(f.fName.c_str()));
            auto fmtArgIter = f.fFormatArgs.cbegin();
            // Helper functions can't refer to sample-coords directly (they're a parameter to main)
            SkSL::String body =
                    this->expandFormatArgs(f.fBody, args, /*sampleCoords=*/nullptr, fmtArgIter);
            SkASSERT(fmtArgIter == f.fFormatArgs.cend());
            fragBuilder->emitFunction(f.fReturnType,
                                      fFunctionNames.back().c_str(),
                                      {f.fParameters.data(), f.fParameters.size()},
                                      body.c_str());
        }
        SkString coordsVarName = fragBuilder->newTmpVarName("coords");
        const char* coords = nullptr;
        if (fp.referencesSampleCoords()) {
            coords = coordsVarName.c_str();
            fragBuilder->codeAppendf("float2 %s = %s;\n", coords, args.fSampleCoord);
        }
        auto fmtArgIter = fArgs.fFormatArgs.cbegin();
        fragBuilder->codeAppend(
                this->expandFormatArgs(fArgs.fCode, args, coords, fmtArgIter).c_str());
        SkASSERT(fmtArgIter == fArgs.fFormatArgs.cend());
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        size_t uniIndex = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        const uint8_t* uniformData = outer.fUniforms->bytes();
        for (const auto& v : outer.fEffect->uniforms()) {
            const float* data = reinterpret_cast<const float*>(uniformData + v.fOffset);
            switch (v.fType) {
                case SkRuntimeEffect::Uniform::Type::kFloat:
                    pdman.set1fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat2:
                    pdman.set2fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat3:
                    pdman.set3fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat4:
                    pdman.set4fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat2x2:
                    pdman.setMatrix2fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat3x3:
                    pdman.setMatrix3fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat4x4:
                    pdman.setMatrix4fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                default:
                    SkDEBUGFAIL("Unsupported uniform type");
                    break;
            }
        }
    }

    // nearly-finished GLSL; still contains printf-style "%s" format tokens
    SkSL::PipelineStageArgs fArgs;
    std::vector<UniformHandle> fUniformHandles;
    std::vector<SkString> fFunctionNames;
};

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, sk_sp<SkRuntimeEffect> effect,
                                         const char* name, sk_sp<SkData> uniforms) {
    if (uniforms->size() != effect->uniformSize()) {
        return nullptr;
    }
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().getShaderErrorHandler(),
                                                  std::move(effect), name, std::move(uniforms)));
}

GrSkSLFP::GrSkSLFP(ShaderErrorHandler* shaderErrorHandler, sk_sp<SkRuntimeEffect> effect,
                   const char* name, sk_sp<SkData> uniforms)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderErrorHandler(shaderErrorHandler)
        , fEffect(std::move(effect))
        , fName(name)
        , fUniforms(std::move(uniforms)) {
    if (fEffect->usesSampleCoords()) {
        this->setUsesSampleCoordsDirectly();
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderErrorHandler(other.fShaderErrorHandler)
        , fEffect(other.fEffect)
        , fName(other.fName)
        , fUniforms(other.fUniforms) {
    if (fEffect->usesSampleCoords()) {
        this->setUsesSampleCoordsDirectly();
    }

    this->cloneAndRegisterAllChildProcessors(other);
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child) {
    int childIndex = this->numChildProcessors();
    SkASSERT((size_t)childIndex < fEffect->fSampleUsages.size());
    this->registerChild(std::move(child), fEffect->fSampleUsages[childIndex]);
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    // Note: This is actually SkSL (again) but with inline format specifiers.
    SkSL::PipelineStageArgs args;
    SkAssertResult(fEffect->toPipelineStage(fShaderErrorHandler, &args));
    return new GrGLSLSkSLFP(std::move(args));
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    // In the unlikely event of a hash collision, we also include the uniform size in the key.
    // That ensures that we will (at worst) use the wrong program, but one that expects the same
    // amount of uniform data.
    b->add32(fEffect->hash());
    b->add32(SkToU32(fUniforms->size()));
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    return fEffect->hash() == sk.fEffect->hash() && fUniforms->equals(sk.fUniforms.get());
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(*this));
}

/**************************************************************************************************/

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSkSLFP);

#if GR_TEST_UTILS

#include "include/effects/SkArithmeticImageFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "src/core/SkColorFilterBase.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

extern const char* SKSL_OVERDRAW_SRC;

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::TestCreate(GrProcessorTestData* d) {
    SkColor colors[SkOverdrawColorFilter::kNumColors];
    for (SkColor& c : colors) {
        c = d->fRandom->nextU();
    }
    auto filter = SkOverdrawColorFilter::MakeWithSkColors(colors);
    auto [success, fp] = as_CFB(filter)->asFragmentProcessor(/*inputFP=*/nullptr, d->context(),
                                                             GrColorInfo{});
    SkASSERT(success);
    return std::move(fp);
}

#endif
