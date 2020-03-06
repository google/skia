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
                                  std::vector<SkSL::Compiler::FormatArg>::const_iterator& fmtArg,
                                  const char* coordsName) {
        SkSL::String result;
        int substringStartIndex = 0;
        for (size_t i = 0; i < raw.length(); ++i) {
            char c = raw[i];
            if (c == '%') {
                result += SkSL::StringFragment(raw.c_str() + substringStartIndex,
                                               i - substringStartIndex);
                ++i;
                c = raw[i];
                switch (c) {
                    case 's': {
                        const SkSL::Compiler::FormatArg& arg = *fmtArg++;
                        switch (arg.fKind) {
                            case SkSL::Compiler::FormatArg::Kind::kInput:
                                result += args.fInputColor;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kOutput:
                                result += args.fOutputColor;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kCoords:
                                result += coordsName;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kUniform:
                                result += args.fUniformHandler->getUniformCStr(
                                                                       fUniformHandles[arg.fIndex]);
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kChildProcessor: {
                                SkSL::String coords = this->expandFormatArgs(arg.fCoords, args,
                                                                             fmtArg, coordsName);
                                result += this->invokeChild(arg.fIndex, args, coords).c_str();
                                break;
                            }
                            case SkSL::Compiler::FormatArg::Kind::kFunctionName:
                                SkASSERT((int) fFunctionNames.size() > arg.fIndex);
                                result += fFunctionNames[arg.fIndex].c_str();
                                break;
                        }
                        break;
                    }
                    default:
                        result += c;
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
        for (const auto& v : fp.fEffect->inputs()) {
            if (v.fQualifier == SkRuntimeEffect::Variable::Qualifier::kUniform) {
                auto handle = args.fUniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                                    v.fGPUType,
                                                                    v.fName.c_str(),
                                                                    v.isArray() ? v.fCount : 0);
                fUniformHandles.push_back(handle);
            }
        }
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkASSERT(args.fTransformedCoords.count() == 1);
        SkString coords = fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);
        std::vector<SkString> childNames;
        // We need to ensure that we call invokeChild on each child FP at least once.
        // Any child FP that isn't sampled won't trigger a call otherwise, leading to asserts later.
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            (void)this->invokeChild(i, args, SkSL::String("_coords"));
        }
        for (const auto& f : fArgs.fFunctions) {
            fFunctionNames.emplace_back();
            auto fmtArgIter = f.fFormatArgs.cbegin();
            SkSL::String body =
                    this->expandFormatArgs(f.fBody.c_str(), args, fmtArgIter, coords.c_str());
            SkASSERT(fmtArgIter == f.fFormatArgs.cend());
            fragBuilder->emitFunction(f.fReturnType,
                                      f.fName.c_str(),
                                      f.fParameters.size(),
                                      f.fParameters.data(),
                                      body.c_str(),
                                      &fFunctionNames.back());
        }
        auto fmtArgIter = fArgs.fFormatArgs.cbegin();
        fragBuilder->codeAppend(this->expandFormatArgs(fArgs.fCode.c_str(), args, fmtArgIter,
                                                       coords.c_str()).c_str());
        SkASSERT(fmtArgIter == fArgs.fFormatArgs.cend());
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        size_t uniIndex = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        const uint8_t* inputs = outer.fInputs->bytes();
        for (const auto& v : outer.fEffect->inputs()) {
            if (v.fQualifier != SkRuntimeEffect::Variable::Qualifier::kUniform) {
                continue;
            }

            const float* data = reinterpret_cast<const float*>(inputs + v.fOffset);
            switch (v.fType) {
                case SkRuntimeEffect::Variable::Type::kFloat:
                    pdman.set1fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Variable::Type::kFloat2:
                    pdman.set2fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Variable::Type::kFloat3:
                    pdman.set3fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Variable::Type::kFloat4:
                    pdman.set4fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Variable::Type::kFloat2x2:
                    pdman.setMatrix2fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Variable::Type::kFloat3x3:
                    pdman.setMatrix3fv(fUniformHandles[uniIndex++], v.fCount, data);
                    break;
                case SkRuntimeEffect::Variable::Type::kFloat4x4:
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
                                         const char* name, sk_sp<SkData> inputs,
                                         const SkMatrix* matrix) {
    if (inputs->size() != effect->inputSize()) {
        return nullptr;
    }
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(
            context->priv().caps()->refShaderCaps(), context->priv().getShaderErrorHandler(),
            std::move(effect), name, std::move(inputs), matrix));
}

GrSkSLFP::GrSkSLFP(sk_sp<const GrShaderCaps> shaderCaps, ShaderErrorHandler* shaderErrorHandler,
                   sk_sp<SkRuntimeEffect> effect, const char* name, sk_sp<SkData> inputs,
                   const SkMatrix* matrix)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderCaps(std::move(shaderCaps))
        , fShaderErrorHandler(shaderErrorHandler)
        , fEffect(std::move(effect))
        , fName(name)
        , fInputs(std::move(inputs)) {
    if (matrix) {
        fCoordTransform = GrCoordTransform(*matrix);
    }
    this->addCoordTransform(&fCoordTransform);
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderCaps(other.fShaderCaps)
        , fShaderErrorHandler(other.fShaderErrorHandler)
        , fEffect(other.fEffect)
        , fName(other.fName)
        , fInputs(other.fInputs) {
    SkASSERT(other.numCoordTransforms() == 1);
    fCoordTransform = other.fCoordTransform;
    this->addCoordTransform(&fCoordTransform);
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child) {
    child->setSampledWithExplicitCoords(true);
    this->registerChildProcessor(std::move(child));
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    // Note: This is actually SkSL (again) but with inline format specifiers.
    SkSL::PipelineStageArgs args;
    fEffect->toPipelineStage(fInputs->data(), fShaderCaps.get(), fShaderErrorHandler, &args);
    return new GrGLSLSkSLFP(std::move(args));
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    // In the unlikely event of a hash collision, we also include the input size in the key.
    // That ensures that we will (at worst) use the wrong program, but one that expects the same
    // amount of input data.
    b->add32(fEffect->hash());
    b->add32(SkToU32(fInputs->size()));
    const uint8_t* inputs = fInputs->bytes();
    for (const auto& v : fEffect->inputs()) {
        if (v.fQualifier != SkRuntimeEffect::Variable::Qualifier::kIn) {
            continue;
        }
        // 'in' arrays are not supported
        SkASSERT(!v.isArray());
        switch (v.fType) {
            case SkRuntimeEffect::Variable::Type::kBool:
                b->add32(inputs[v.fOffset]);
                break;
            case SkRuntimeEffect::Variable::Type::kInt:
            case SkRuntimeEffect::Variable::Type::kFloat:
                b->add32(*(int32_t*)(inputs + v.fOffset));
                break;
            default:
                SkDEBUGFAIL("Unsupported input variable type");
                break;
        }
    }
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    return fEffect->hash() == sk.fEffect->hash() && fInputs->equals(sk.fInputs.get());
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    std::unique_ptr<GrSkSLFP> result(new GrSkSLFP(*this));
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        result->addChild(this->childProcessor(i).clone());
    }
    return std::unique_ptr<GrFragmentProcessor>(result.release());
}

/**************************************************************************************************/

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSkSLFP);

#if GR_TEST_UTILS

#include "include/effects/SkArithmeticImageFilter.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

extern const char* SKSL_ARITHMETIC_SRC;
extern const char* SKSL_DITHER_SRC;
extern const char* SKSL_OVERDRAW_SRC;

using Value = SkSL::Program::Settings::Value;

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::TestCreate(GrProcessorTestData* d) {
    int type = d->fRandom->nextULessThan(3);
    switch (type) {
        case 0: {
            static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_DITHER_SRC)));
            int rangeType = d->fRandom->nextULessThan(3);
            auto result = GrSkSLFP::Make(d->context(), effect, "Dither",
                                         SkData::MakeWithCopy(&rangeType, sizeof(rangeType)));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 1: {
            static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_ARITHMETIC_SRC)));
            ArithmeticFPInputs inputs{d->fRandom->nextF(), d->fRandom->nextF(), d->fRandom->nextF(),
                                      d->fRandom->nextF(), d->fRandom->nextBool()};
            auto result = GrSkSLFP::Make(d->context(), effect, "Arithmetic",
                                         SkData::MakeWithCopy(&inputs, sizeof(inputs)));
            result->addChild(GrConstColorProcessor::Make(
                    SK_PMColor4fWHITE, GrConstColorProcessor::InputMode::kIgnore));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 2: {
            static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_OVERDRAW_SRC)));
            SkColor4f inputs[6];
            for (int i = 0; i < 6; ++i) {
                inputs[i] = SkColor4f::FromBytes_RGBA(d->fRandom->nextU());
            }
            auto result = GrSkSLFP::Make(d->context(), effect, "Overdraw",
                                         SkData::MakeWithCopy(&inputs, sizeof(inputs)));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
    }
    SK_ABORT("unreachable");
}

#endif
