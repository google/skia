/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSkSLFP.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrTexture.h"
#include "SkSLUtil.h"

GrSkSLFPFactory::GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps, const char* sksl)
        : fName(name) {
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    fBaseProgram = fCompiler.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(sksl),
                                            settings);
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
        }
    }
    SkASSERT(offset == inputSize);

    std::unique_ptr<SkSL::Program> specialized = fCompiler.specialize(*fBaseProgram, inputMap);
    SkAssertResult(fCompiler.optimize(*specialized));
    const SkSL::Program* result = specialized.get();
    fSpecializations.insert(std::make_pair(key, std::move(specialized)));
    return result;
}

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    GrGLSLSkSLFP(SkSL::String glsl, std::vector<SkSL::Compiler::FormatArg> formatArgs)
            : fGLSL(glsl)
            , fFormatArgs(formatArgs) {}

    void emitCode(EmitArgs& args) override {
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
                    case 's':
                        switch (fFormatArgs[formatArgIndex++]) {
                            case SkSL::Compiler::FormatArg::kInput:
                                fragBuilder->codeAppend(args.fInputColor ? args.fInputColor
                                                                         : "half4(1)");
                                break;
                            case SkSL::Compiler::FormatArg::kOutput:
                                fragBuilder->codeAppend(args.fOutputColor);
                                break;
                        }
                        break;
                    default:
                        fragBuilder->codeAppendf("%c", c);
                }
                substringStartIndex = i + 1;
            }
        }
        fragBuilder->codeAppend(fGLSL.c_str() + substringStartIndex,
                                fGLSL.length() - substringStartIndex);
    }

    // nearly-finished GLSL; still contains printf-style "%s" format tokens
    const SkSL::String fGLSL;
    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;
};

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::Make(GrContext* context, int index, const char* name,
                                                    const char* sksl, const void* inputs,
                                                    size_t inputSize) {
    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(
                                                        context->contextPriv().getFPFactoryCache(),
                                                        context->contextPriv().caps()->shaderCaps(),
                                                        index, name, sksl, inputs, inputSize));
}


GrSkSLFP::GrSkSLFP(sk_sp<GrSkSLFPFactoryCache> factoryCache, const GrShaderCaps* shaderCaps,
                   int index, const char* name, const char* sksl, const void* inputs,
                   size_t inputSize)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fFactoryCache(factoryCache)
        , fShaderCaps(sk_ref_sp(shaderCaps))
        , fIndex(index)
        , fName(name)
        , fSkSL(sksl)
        , fInputs(new int8_t[inputSize])
        , fInputSize(inputSize) {
    memcpy(fInputs.get(), inputs, inputSize);
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fFactoryCache(other.fFactoryCache)
        , fShaderCaps(other.fShaderCaps)
        , fFactory(other.fFactory)
        , fIndex(other.fIndex)
        , fName(other.fName)
        , fSkSL(other.fSkSL)
        , fInputs(new int8_t[other.fInputSize])
        , fInputSize(other.fInputSize) {
    memcpy(fInputs.get(), other.fInputs.get(), fInputSize);
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::createFactory() const {
    if (!fFactory) {
        fFactory = fFactoryCache->get(fIndex);
        if (!fFactory) {
            fFactory = sk_sp<GrSkSLFPFactory>(new GrSkSLFPFactory(fName, fShaderCaps.get(), fSkSL));
            fFactoryCache->set(fIndex, fFactory);
        }
    }
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    this->createFactory();
    const SkSL::Program* specialized = fFactory->getSpecialization(fKey, fInputs.get(), fInputSize);
    SkSL::String glsl;
    std::vector<SkSL::Compiler::FormatArg> formatArgs;
     if (!fFactory->fCompiler.toPipelineStage(*specialized, &glsl, &formatArgs)) {
        printf("%s\n", fFactory->fCompiler.errorText().c_str());
        abort();
    }
    return new GrGLSLSkSLFP(glsl, formatArgs);
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    this->createFactory();
    size_t offset = 0;
    char* inputs = (char*) fInputs.get();
    for (const auto& v : fFactory->fInputVars) {
        if (&v->fType == fFactory->fCompiler.context().fInt_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fLayout.fKey) {
                fKey += inputs[offset + 0];
                fKey += inputs[offset + 1];
                fKey += inputs[offset + 2];
                fKey += inputs[offset + 3];
                b->add32(*(int32_t*) (inputs + offset));
            }
            offset += sizeof(int32_t);
        }
        else {
            // unsupported input var type
            SkASSERT(false);
        }
    }
    SkASSERT(offset == fInputSize);
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    SkASSERT(fIndex != sk.fIndex || fInputSize == sk.fInputSize);
    return fIndex == sk.fIndex &&
            !memcmp(fInputs.get(), sk.fInputs.get(), fInputSize);
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(*this));
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
    result->ref();
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

#include "SkGr.h"

using Value = SkSL::Program::Settings::Value;

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::TestCreate(GrProcessorTestData* d) {
    int type = d->fRandom->nextULessThan(1);
    switch (type) {
        case 0: {
            static int ditherIndex = NewIndex();
            int rangeType = d->fRandom->nextULessThan(3);
            return GrSkSLFP::Make(d->context(), ditherIndex, "Dither", SKSL_DITHER_SRC, &rangeType,
                                  sizeof(rangeType));
        }
    }
    SK_ABORT("unreachable");
    return nullptr;
}

#endif
