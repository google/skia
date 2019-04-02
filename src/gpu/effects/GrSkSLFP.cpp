/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSkSLFP.h"

#include "GrBaseContextPriv.h"
#include "GrContext_Base.h"
#include "GrTexture.h"
#include "SkSLUtil.h"

#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"

GrSkSLFPFactory::GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps, const char* sksl,
                                 SkSL::Program::Kind kind)
        : fKind(kind)
        , fName(name) {
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    fBaseProgram = fCompiler.convertProgram(fKind, SkSL::String(sksl), settings);
    if (fCompiler.errorCount()) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
    }
    SkASSERT(fBaseProgram);
    SkASSERT(!fCompiler.errorCount());
    for (const auto& e : *fBaseProgram) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) *varStatement).fVar;
                if (var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                    fInputVars.push_back(&var);
                }
                if (var.fModifiers.fLayout.fKey) {
                    fKeyVars.push_back(&var);
                }
            }
        }
    }
}

const SkSL::Program* GrSkSLFPFactory::getSpecialization(const SkSL::String& key, const void* inputs,
                                                        size_t inputSize) {
    const auto& found = fSpecializations.find(key);
    if (found != fSpecializations.end()) {
        return found->second.get();
    }

    std::unordered_map<SkSL::String, SkSL::Program::Settings::Value> inputMap;
    size_t offset = 0;
    for (const auto& v : fInputVars) {
        SkSL::String name(v->fName);
        if (&v->fType == fCompiler.context().fInt_Type.get()) {
            offset = SkAlign4(offset);
            int32_t v = *(int32_t*) (((uint8_t*) inputs) + offset);
            inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            offset += sizeof(int32_t);
        } else if (&v->fType == fCompiler.context().fFloat_Type.get()) {
            offset = SkAlign4(offset);
            float v = *(float*) (((uint8_t*) inputs) + offset);
            inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            offset += sizeof(float);
        } else if (&v->fType == fCompiler.context().fBool_Type.get()) {
            bool v = *(((bool*) inputs) + offset);
            inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            offset += sizeof(bool);
        } else if (&v->fType == fCompiler.context().fFloat4_Type.get() ||
                   &v->fType == fCompiler.context().fHalf4_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 4;
        } else if (&v->fType == fCompiler.context().fFragmentProcessor_Type.get()) {
            // do nothing
        } else {
            printf("can't handle input var: %s\n", SkSL::String(v->fType.fName).c_str());
            SkASSERT(false);
        }
    }

    std::unique_ptr<SkSL::Program> specialized = fCompiler.specialize(*fBaseProgram, inputMap);
    bool optimized = fCompiler.optimize(*specialized);
    if (!optimized) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
        SkASSERT(false);
    }
    const SkSL::Program* result = specialized.get();
    fSpecializations.insert(std::make_pair(key, std::move(specialized)));
    return result;
}

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    GrGLSLSkSLFP(const SkSL::Context* context, const std::vector<const SkSL::Variable*>* inputVars,
                 SkSL::String glsl, std::vector<SkSL::Compiler::FormatArg> formatArgs)
            : fContext(*context)
            , fInputVars(*inputVars)
            , fGLSL(glsl)
            , fFormatArgs(formatArgs) {}

    GrSLType uniformType(const SkSL::Type& type) {
        if (type == *fContext.fFloat_Type) {
            return kFloat_GrSLType;
        } else if (type == *fContext.fHalf_Type) {
            return kHalf_GrSLType;
        } else if (type == *fContext.fFloat2_Type) {
            return kFloat2_GrSLType;
        } else if (type == *fContext.fHalf2_Type) {
            return kHalf2_GrSLType;
        } else if (type == *fContext.fFloat4_Type) {
            return kFloat4_GrSLType;
        } else if (type == *fContext.fHalf4_Type) {
            return kHalf4_GrSLType;
        } else if (type == *fContext.fFloat4x4_Type) {
            return kFloat4x4_GrSLType;
        } else if (type == *fContext.fHalf4x4_Type) {
            return kHalf4x4_GrSLType;
        } else if (type == *fContext.fBool_Type) {
            return kBool_GrSLType;
        } else if (type == *fContext.fInt_Type) {
            return kInt_GrSLType;
        }
        printf("%s\n", SkSL::String(type.fName).c_str());
        SK_ABORT("unsupported uniform type");
        return kFloat_GrSLType;
    }

    void emitCode(EmitArgs& args) override {
        for (const auto& v : fInputVars) {
            if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag && v->fType !=
                                                                *fContext.fFragmentProcessor_Type) {
                fUniformHandles.push_back(args.fUniformHandler->addUniform(
                                                                   kFragment_GrShaderFlag,
                                                                   this->uniformType(v->fType),
                                                                   SkSL::String(v->fName).c_str()));
            }
        }
        std::vector<SkString> childNames;
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            childNames.push_back(SkStringPrintf("_child%d", i));
            this->emitChild(i, &childNames[i], args);
        }
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        int substringStartIndex = 0;
        int formatArgIndex = 0;
        for (size_t i = 0; i < fGLSL.length(); ++i) {
            char c = fGLSL[i];
            if (c == '%') {
                fragBuilder->codeAppend(fGLSL.c_str() + substringStartIndex,
                                        i - substringStartIndex);
                ++i;
                c = fGLSL[i];
                switch (c) {
                    case 's': {
                        SkSL::Compiler::FormatArg& arg = fFormatArgs[formatArgIndex++];
                        switch (arg.fKind) {
                            case SkSL::Compiler::FormatArg::Kind::kInput:
                                fragBuilder->codeAppend(args.fInputColor);
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kOutput:
                                fragBuilder->codeAppend(args.fOutputColor);
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kUniform:
                                fragBuilder->codeAppend(args.fUniformHandler->getUniformCStr(
                                                                      fUniformHandles[arg.fIndex]));
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kChildProcessor:
                                fragBuilder->codeAppend(childNames[arg.fIndex].c_str());
                                break;
                        }
                        break;
                    }
                    default:
                        fragBuilder->codeAppendf("%c", c);
                }
                substringStartIndex = i + 1;
            }
        }
        fragBuilder->codeAppend(fGLSL.c_str() + substringStartIndex,
                                fGLSL.length() - substringStartIndex);
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        size_t uniformIndex = 0;
        size_t offset = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        char* inputs = (char*) outer.fInputs.get();
        const SkSL::Context& context = outer.fFactory->fCompiler.context();
        for (const auto& v : outer.fFactory->fInputVars) {
            if (&v->fType == context.fFloat4_Type.get() ||
                &v->fType == context.fHalf4_Type.get()) {
                float f1, f2, f3, f4;
                switch (v->fModifiers.fLayout.fCType) {
                    case SkSL::Layout::CType::kSkPMColor:
                        f1 = ((uint8_t*) inputs)[offset++] / 255.0;
                        f2 = ((uint8_t*) inputs)[offset++] / 255.0;
                        f3 = ((uint8_t*) inputs)[offset++] / 255.0;
                        f4 = ((uint8_t*) inputs)[offset++] / 255.0;
                        break;
                    case SkSL::Layout::CType::kSkRect: // fall through
                    case SkSL::Layout::CType::kDefault:
                        offset = SkAlign4(offset);
                        f1 = *(float*) (inputs + offset);
                        offset += sizeof(float);
                        f2 = *(float*) (inputs + offset);
                        offset += sizeof(float);
                        f3 = *(float*) (inputs + offset);
                        offset += sizeof(float);
                        f4 = *(float*) (inputs + offset);
                        offset += sizeof(float);
                        break;
                    default:
                        SK_ABORT("unsupported uniform ctype");
                }
                if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                    pdman.set4f(fUniformHandles[uniformIndex++], f1, f2, f3, f4);
                }
            } else if (&v->fType == context.fInt_Type.get()) {
                int32_t i = *(int32_t*) (inputs + offset);
                offset += sizeof(int32_t);
                if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                    pdman.set1i(fUniformHandles[uniformIndex++], i);
                }
            } else if (&v->fType == context.fFloat_Type.get()) {
                float f = *(float*) (inputs + offset);
                offset += sizeof(float);
                if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                    pdman.set1f(fUniformHandles[uniformIndex++], f);
                }
            } else if (&v->fType == context.fBool_Type.get()) {
                SkASSERT(!(v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag));
                ++offset;
            } else {
                SkASSERT(&v->fType == context.fFragmentProcessor_Type.get());
            }
        }
    }

    const SkSL::Context& fContext;
    const std::vector<const SkSL::Variable*>& fInputVars;
    // nearly-finished GLSL; still contains printf-style "%s" format tokens
    const SkSL::String fGLSL;
    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;
    std::vector<UniformHandle> fUniformHandles;
};

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, int index, const char* name,
                                         const char* sksl, const void* inputs,
                                         size_t inputSize, SkSL::Program::Kind kind) {
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().fpFactoryCache(),
                                                  context->priv().caps()->shaderCaps(),
                                                  kind, index, name, sksl, SkString(),
                                                  inputs, inputSize));
}

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, int index, const char* name,
                                         SkString sksl, const void* inputs, size_t inputSize,
                                         SkSL::Program::Kind kind) {
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().fpFactoryCache(),
                                                  context->priv().caps()->shaderCaps(),
                                                  kind, index, name, nullptr, std::move(sksl),
                                                  inputs, inputSize));
}

GrSkSLFP::GrSkSLFP(sk_sp<GrSkSLFPFactoryCache> factoryCache, const GrShaderCaps* shaderCaps,
                   SkSL::Program::Kind kind, int index, const char* name, const char* sksl,
                   SkString skslString, const void* inputs, size_t inputSize)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fFactoryCache(factoryCache)
        , fShaderCaps(sk_ref_sp(shaderCaps))
        , fKind(kind)
        , fIndex(index)
        , fName(name)
        , fSkSLString(skslString)
        , fSkSL(sksl ? sksl : fSkSLString.c_str())
        , fInputs(new int8_t[inputSize])
        , fInputSize(inputSize) {
    if (fInputSize) {
        memcpy(fInputs.get(), inputs, inputSize);
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fFactoryCache(other.fFactoryCache)
        , fShaderCaps(other.fShaderCaps)
        , fFactory(other.fFactory)
        , fKind(other.fKind)
        , fIndex(other.fIndex)
        , fName(other.fName)
        , fSkSLString(other.fSkSLString)
        , fSkSL(other.fSkSL)
        , fInputs(new int8_t[other.fInputSize])
        , fInputSize(other.fInputSize) {
    if (fInputSize) {
        memcpy(fInputs.get(), other.fInputs.get(), fInputSize);
    }
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::createFactory() const {
    if (!fFactory) {
        fFactory = fFactoryCache->get(fIndex);
        if (!fFactory) {
            fFactory = sk_sp<GrSkSLFPFactory>(new GrSkSLFPFactory(fName, fShaderCaps.get(), fSkSL,
                                                                  fKind));
            fFactoryCache->set(fIndex, fFactory);
        }
    }
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child) {
    this->registerChildProcessor(std::move(child));
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    this->createFactory();
    const SkSL::Program* specialized = fFactory->getSpecialization(fKey, fInputs.get(), fInputSize);
    SkSL::String glsl;
    std::vector<SkSL::Compiler::FormatArg> formatArgs;
    if (!fFactory->fCompiler.toPipelineStage(*specialized, &glsl, &formatArgs)) {
        printf("%s\n", fFactory->fCompiler.errorText().c_str());
        SkASSERT(false);
    }
    return new GrGLSLSkSLFP(specialized->fContext.get(), &fFactory->fInputVars, glsl, formatArgs);
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    this->createFactory();
    b->add32(fIndex);
    size_t offset = 0;
    char* inputs = (char*) fInputs.get();
    const SkSL::Context& context = fFactory->fCompiler.context();
    for (const auto& v : fFactory->fInputVars) {
        if (&v->fType == context.fInt_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fLayout.fKey) {
                fKey += inputs[offset + 0];
                fKey += inputs[offset + 1];
                fKey += inputs[offset + 2];
                fKey += inputs[offset + 3];
                b->add32(*(int32_t*) (inputs + offset));
            }
            offset += sizeof(int32_t);
        } else if (&v->fType == context.fFloat_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fLayout.fKey) {
                fKey += inputs[offset + 0];
                fKey += inputs[offset + 1];
                fKey += inputs[offset + 2];
                fKey += inputs[offset + 3];
                b->add32(*(float*) (inputs + offset));
            }
            offset += sizeof(float);
        } else if (&v->fType == context.fFloat4_Type.get() ||
                   &v->fType == context.fHalf4_Type.get()) {
            if (v->fModifiers.fLayout.fKey) {
                for (size_t i = 0; i < sizeof(float) * 4; ++i) {
                    fKey += inputs[offset + i];
                }
                b->add32(*(int32_t*) (inputs + offset));
                offset += sizeof(float);
                b->add32(*(int32_t*) (inputs + offset));
                offset += sizeof(float);
                b->add32(*(int32_t*) (inputs + offset));
                offset += sizeof(float);
                b->add32(*(int32_t*) (inputs + offset));
                offset += sizeof(float);
            } else {
                offset += sizeof(float) * 4;
            }
        } else if (&v->fType == context.fBool_Type.get()) {
            if (v->fModifiers.fLayout.fKey) {
                fKey += inputs[offset];
                b->add32(inputs[offset]);
            }
            ++offset;
        } else if (&v->fType == context.fFragmentProcessor_Type.get()) {
            continue;
        } else {
            // unsupported input var type
            printf("%s\n", SkSL::String(v->fType.fName).c_str());
            SkASSERT(false);
        }
    }
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    SkASSERT(fIndex != sk.fIndex || fInputSize == sk.fInputSize);
    return fIndex == sk.fIndex &&
            !memcmp(fInputs.get(), sk.fInputs.get(), fInputSize);
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    std::unique_ptr<GrSkSLFP> result(new GrSkSLFP(*this));
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        result->registerChildProcessor(this->childProcessor(i).clone());
    }
    return std::unique_ptr<GrFragmentProcessor>(result.release());
}

// We have to do a bit of manual refcounting in the cache methods below. Ideally, we could just
// define fFactories to contain sk_sp<GrSkSLFPFactory> rather than GrSkSLFPFactory*, but that would
// require GrContext to include GrSkSLFP, which creates much bigger headaches than a few manual
// refcounts.

sk_sp<GrSkSLFPFactory> GrSkSLFPFactoryCache::get(int index) {
    if (index >= (int) fFactories.size()) {
        return nullptr;
    }
    GrSkSLFPFactory* result = fFactories[index];
    SkSafeRef(result);
    return sk_sp<GrSkSLFPFactory>(result);
}

void GrSkSLFPFactoryCache::set(int index, sk_sp<GrSkSLFPFactory> factory) {
    while (index >= (int) fFactories.size()) {
        fFactories.emplace_back();
    }
    factory->ref();
    SkASSERT(!fFactories[index]);
    fFactories[index] = factory.get();
}

GrSkSLFPFactoryCache::~GrSkSLFPFactoryCache() {
    for (GrSkSLFPFactory* factory : fFactories) {
        if (factory) {
            factory->unref();
        }
    }
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSkSLFP);

#if GR_TEST_UTILS

#include "GrConstColorProcessor.h"
#include "GrContext.h"
#include "SkArithmeticImageFilter.h"

extern const char* SKSL_ARITHMETIC_SRC;
extern const char* SKSL_DITHER_SRC;
extern const char* SKSL_OVERDRAW_SRC;

using Value = SkSL::Program::Settings::Value;

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::TestCreate(GrProcessorTestData* d) {
    int type = d->fRandom->nextULessThan(3);
    switch (type) {
        case 0: {
            static int ditherIndex = NewIndex();
            int rangeType = d->fRandom->nextULessThan(3);
            std::unique_ptr<GrSkSLFP> result = GrSkSLFP::Make(d->context(), ditherIndex, "Dither",
                                                              SKSL_DITHER_SRC, &rangeType,
                                                              sizeof(rangeType));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 1: {
            static int arithmeticIndex = NewIndex();
            ArithmeticFPInputs inputs;
            inputs.k[0] = d->fRandom->nextF();
            inputs.k[1] = d->fRandom->nextF();
            inputs.k[2] = d->fRandom->nextF();
            inputs.k[3] = d->fRandom->nextF();
            inputs.enforcePMColor = d->fRandom->nextBool();
            std::unique_ptr<GrSkSLFP> result = GrSkSLFP::Make(d->context(), arithmeticIndex,
                                                              "Arithmetic", SKSL_ARITHMETIC_SRC,
                                                              &inputs, sizeof(inputs));
            result->addChild(GrConstColorProcessor::Make(
                                                        SK_PMColor4fWHITE,
                                                        GrConstColorProcessor::InputMode::kIgnore));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 2: {
            static int overdrawIndex = NewIndex();
            SkPMColor inputs[6];
            for (int i = 0; i < 6; ++i) {
                inputs[i] = d->fRandom->nextU();
            }
            std::unique_ptr<GrSkSLFP> result = GrSkSLFP::Make(d->context(), overdrawIndex,
                                                              "Overdraw", SKSL_OVERDRAW_SRC,
                                                              &inputs, sizeof(inputs));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
    }
    SK_ABORT("unreachable");
    return nullptr;
}

#endif
