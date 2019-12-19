/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSkSLFP.h"

#include "include/gpu/GrTexture.h"
#include "include/private/GrContext_Base.h"
#include "src/core/SkRuntimeEffect.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/sksl/SkSLUtil.h"

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    GrGLSLSkSLFP(SkSL::String glsl, std::vector<SkSL::Compiler::FormatArg> formatArgs,
                 std::vector<SkSL::Compiler::GLSLFunction> functions)
            : fGLSL(glsl)
            , fFormatArgs(std::move(formatArgs))
            , fFunctions(std::move(functions)) {}

    SkSL::String expandFormatArgs(const SkSL::String& raw,
                                  const EmitArgs& args,
                                  const std::vector<SkSL::Compiler::FormatArg> formatArgs,
                                  const char* coordsName,
                                  const std::vector<SkString>& childNames) {
        SkSL::String result;
        int substringStartIndex = 0;
        int formatArgIndex = 0;
        for (size_t i = 0; i < raw.length(); ++i) {
            char c = raw[i];
            if (c == '%') {
                result += SkSL::StringFragment(raw.c_str() + substringStartIndex,
                                               i - substringStartIndex);
                ++i;
                c = raw[i];
                switch (c) {
                    case 's': {
                        const SkSL::Compiler::FormatArg& arg = formatArgs[formatArgIndex++];
                        switch (arg.fKind) {
                            case SkSL::Compiler::FormatArg::Kind::kInput:
                                result += args.fInputColor;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kOutput:
                                result += args.fOutputColor;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kCoordX:
                                result += coordsName;
                                result += ".x";
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kCoordY:
                                result += coordsName;
                                result += ".y";
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kUniform:
                                result += args.fUniformHandler->getUniformCStr(
                                                                       fUniformHandles[arg.fIndex]);
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kChildProcessor:
                                result += childNames[arg.fIndex].c_str();
                                break;
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
        for (const auto& v : fp.fEffect->fInAndUniformVars) {
            if (v.fQualifier == SkRuntimeEffect::Variable::Qualifier::kUniform) {
                auto handle = args.fUniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                                    v.fGPUType,
                                                                    v.fName.c_str(),
                                                                    v.isArray() ? v.fCount : 0);
                fUniformHandles.push_back(handle);
            }
        }
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coords = args.fTransformedCoords.count()
            ? fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint)
            : SkString("sk_FragCoord");
        std::vector<SkString> childNames;
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            childNames.push_back(SkStringPrintf("_child%d", i));
            this->invokeChild(i, &childNames[i], args);
        }
        for (const auto& f : fFunctions) {
            fFunctionNames.emplace_back();
            SkSL::String body = this->expandFormatArgs(f.fBody.c_str(), args, f.fFormatArgs,
                                                       coords.c_str(), childNames);
            fragBuilder->emitFunction(f.fReturnType,
                                      f.fName.c_str(),
                                      f.fParameters.size(),
                                      f.fParameters.data(),
                                      body.c_str(),
                                      &fFunctionNames.back());
        }
        fragBuilder->codeAppend(this->expandFormatArgs(fGLSL.c_str(), args, fFormatArgs,
                                                       coords.c_str(), childNames).c_str());
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        size_t uniIndex = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        char* inputs = (char*) outer.fInputs.get();
        for (const auto& v : outer.fEffect->fInAndUniformVars) {
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
    const SkSL::String fGLSL;
    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;
    std::vector<SkSL::Compiler::GLSLFunction> fFunctions;
    std::vector<UniformHandle> fUniformHandles;
    std::vector<SkString> fFunctionNames;
};

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, sk_sp<SkRuntimeEffect> effect,
                                         const char* name, const void* inputs, size_t inputSize,
                                         const SkMatrix* matrix) {
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().caps()->refShaderCaps(),
                                                  std::move(effect), name, inputs, inputSize,
                                                  matrix));
}

GrSkSLFP::GrSkSLFP(sk_sp<const GrShaderCaps> shaderCaps, sk_sp<SkRuntimeEffect> effect,
                   const char* name, const void* inputs, size_t inputSize, const SkMatrix* matrix)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderCaps(std::move(shaderCaps))
        , fEffect(std::move(effect))
        , fName(name)
        , fInputs(new int8_t[inputSize])
        , fInputSize(inputSize) {
    if (fInputSize) {
        memcpy(fInputs.get(), inputs, inputSize);
    }
    if (matrix) {
        fCoordTransform = GrCoordTransform(*matrix);
        this->addCoordTransform(&fCoordTransform);
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderCaps(other.fShaderCaps)
        , fEffect(other.fEffect)
        , fName(other.fName)
        , fInputs(new int8_t[other.fInputSize])
        , fInputSize(other.fInputSize) {
    if (fInputSize) {
        memcpy(fInputs.get(), other.fInputs.get(), fInputSize);
    }
    if (other.numCoordTransforms()) {
        fCoordTransform = other.fCoordTransform;
        this->addCoordTransform(&fCoordTransform);
    }
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child) {
    this->registerChildProcessor(std::move(child));
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    // Note: This is actually SkSL (again) but with inline format specifiers.
    SkSL::String code;
    std::vector<SkSL::Compiler::FormatArg> formatArgs;
    std::vector<SkSL::Compiler::GLSLFunction> functions;
    SkAssertResult(fEffect->toPipelineStage(fInputs.get(), fShaderCaps.get(),
                                            &code, &formatArgs, &functions));
    return new GrGLSLSkSLFP(code, formatArgs, functions);
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    b->add32(fEffect->index());
    char* inputs = (char*) fInputs.get();
    for (const auto& v : fEffect->fInAndUniformVars) {
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
    SkASSERT(fEffect->index() != sk.fEffect->index() || fInputSize == sk.fInputSize);
    return fEffect->index() == sk.fEffect->index() &&
            !memcmp(fInputs.get(), sk.fInputs.get(), fInputSize);
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    std::unique_ptr<GrSkSLFP> result(new GrSkSLFP(*this));
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        result->registerChildProcessor(this->childProcessor(i).clone());
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
                                         &rangeType, sizeof(rangeType));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 1: {
            static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_ARITHMETIC_SRC)));
            ArithmeticFPInputs inputs;
            inputs.k[0] = d->fRandom->nextF();
            inputs.k[1] = d->fRandom->nextF();
            inputs.k[2] = d->fRandom->nextF();
            inputs.k[3] = d->fRandom->nextF();
            inputs.enforcePMColor = d->fRandom->nextBool();
            auto result = GrSkSLFP::Make(d->context(), effect, "Arithmetic",
                                         &inputs, sizeof(inputs));
            result->addChild(GrConstColorProcessor::Make(
                                                        SK_PMColor4fWHITE,
                                                        GrConstColorProcessor::InputMode::kIgnore));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 2: {
            static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_OVERDRAW_SRC)));
            SkColor4f inputs[6];
            for (int i = 0; i < 6; ++i) {
                inputs[i] = SkColor4f::FromBytes_RGBA(d->fRandom->nextU());
            }
            auto result = GrSkSLFP::Make(d->context(), effect, "Overdraw",
                                         &inputs, sizeof(inputs));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
    }
    SK_ABORT("unreachable");
}

#endif
