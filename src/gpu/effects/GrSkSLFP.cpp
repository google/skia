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
#include "GrTexture.h"
#include "SkSLUtil.h"

std::map<GrSkSLFP::ProcessorType, std::unique_ptr<GrSkSLFPFactory>> GrSkSLFP::factories;

GrSkSLFPFactory::GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps,
                                 const char* sksl)
    : fName(name) {
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    fBaseProgram = fCompiler.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(sksl),
                                            settings);
    if (fCompiler.errorCount()) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
        SkASSERT(false);
    }
    SkASSERT(fBaseProgram);
    SkASSERT(!fCompiler.errorCount());
    for (const auto& e : *fBaseProgram) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) *varStatement).fVar;
                if (var.fModifiers.fLayout.fKey) {
                    fKeyVars.push_back(SkSL::String(var.fName));
                }
            }
        }
    }
}


std::unique_ptr<GrFragmentProcessor> GrSkSLFPFactory::make(
               const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs) {
    SkASSERT(!fCompiler.errorCount());
    std::unique_ptr<SkSL::Program> specialized = fCompiler.specialize(*fBaseProgram, inputs);
    SkSL::String key;
    for (const auto& s : fKeyVars) {
        const auto& found = inputs.find(s);
        SkASSERT(found != inputs.end());
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
        int fragIndex = 0;
        for (size_t i = 0; i < fGLSL.length(); ++i) {
            char c = fGLSL[i];
            if (c == '%') {
                ++i;
                c = fGLSL[i];
                switch (c) {
                    case 's':
                        switch (fFormatArgs[fragIndex++]) {
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
            } else {
                fragBuilder->codeAppendf("%c", c);
            }
        }
    }

    const SkSL::String fGLSL;
    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;
};

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
    return true;
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    abort();
//    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(*this));
}
