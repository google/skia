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
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    return true;
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    abort();
//    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(*this));
}
