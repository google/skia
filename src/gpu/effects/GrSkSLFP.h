/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFP_DEFINED
#define GrSkSLFP_DEFINED

#include "SkTypes.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "SkSLCompiler.h"
#include "SkSLPipelineStageCodeGenerator.h"

class GrSkSLFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const char* sksl) {
        return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(sksl));
    }

    const char* name() const override { return "SkSLFP"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    GrSkSLFP(const char* sksl)
    : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags) {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;
        sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
        settings.fCaps = caps.get();
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                                 SkSL::Program::kPipelineStage_Kind,
                                                                 SkSL::String(sksl),
                                                                 settings);
        if (!program || !compiler.toPipelineStage(*program, &fGLSL, &fFormatArgs)) {
            printf("SkSL error:\n%s\n", sksl);
            printf("%s\n", compiler.errorText().c_str());
            abort();
        }
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SkSL::String fGLSL;

    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
