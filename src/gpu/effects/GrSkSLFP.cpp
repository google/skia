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

static size_t align(size_t offset, size_t alignment) {
    size_t delta = offset % alignment;
    if (delta) {
        return offset + alignment - delta;
    }
    return offset;
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFPFactory::make(const void* inputs) {
    SkASSERT(!fCompiler.errorCount());
    std::unordered_map<SkSL::String, SkSL::Program::Settings::Value> inputMap;
    size_t index = 0;
    for (const auto& v : fInputVars) {
        SkSL::String name(v->fName);
        if (&v->fType == fCompiler.context().fInt_Type.get()) {
            index = align(index, sizeof(int32_t));
            int32_t v = *(int32_t*) (((int8_t*) inputs) + index);
            inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            index += sizeof(int32_t);
        }
    }
    std::unique_ptr<SkSL::Program> specialized = fCompiler.specialize(*fBaseProgram, inputMap);
    SkSL::String key;
    for (const auto& v : fKeyVars) {
        const auto& found = inputMap.find(SkSL::String(v->fName));
        SkASSERT(found != inputMap.end());
        switch (found->second.fKind) {
            case SkSL::Program::Settings::Value::kBool_Kind:
                key += (char) found->second.fValue;
                break;
            case SkSL::Program::Settings::Value::kInt_Kind:
                key += (char) ((found->second.fValue >> 24) & 0xFF);
                key += (char) ((found->second.fValue >> 16) & 0xFF);
                key += (char) ((found->second.fValue >>  8) & 0xFF);
                key += (char) ((found->second.fValue)       & 0xFF);
                break;
        }
    }
    return GrSkSLFP::Make(fName, &fCompiler, std::move(specialized), std::move(key));
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

    const SkSL::String fGLSL;
    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;
};

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::Make(GrContext* context, int index, const char* name,
                                                    const char* sksl, const void* inputs) {
    GrSkSLFPFactory* factory = context->contextPriv().getFPFactory(index);
    if (!factory) {
        factory = new GrSkSLFPFactory(name, context->contextPriv().caps()->shaderCaps(), sksl);
        context->contextPriv().setFPFactory(index, std::unique_ptr<GrSkSLFPFactory>(factory));
    }
    return factory->make(inputs);
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    return new GrGLSLSkSLFP(fGLSL, fFormatArgs);
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    const char* current = fKey.data();
    size_t length = fKey.length();
    for (size_t i = 0; i < length / 4; ++i) {
        b->add32(*(int32_t*) current);
        current += 4;
    }
    size_t excessCount = length % 4;
    if (excessCount) {
        int32_t excess = 0;
        for (size_t i = 0; i < excessCount; ++i) {
            excess <<= 8;
            excess += *current;
            ++current;
        }
        b->add32(excess);
    }
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    return fGLSL == sk.fGLSL && fFormatArgs == sk.fFormatArgs;
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(*this));
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
            return GrSkSLFP::Make(d->context(), ditherIndex, "Dither", SKSL_DITHER_SRC, &rangeType);
        }
    }
    SK_ABORT("unreachable");
    return nullptr;
}

#endif
