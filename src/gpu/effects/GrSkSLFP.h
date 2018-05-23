/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFP_DEFINED
#define GrSkSLFP_DEFINED

#include "SkTypes.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "GrShaderCaps.h"
#include "SkSLCompiler.h"
#include "SkSLPipelineStageCodeGenerator.h"

#define STRINGIFY(x) #x

static const char* DITHER_SRC =
#include "GrDitherEffect.inc"
;

/**
 * Produces GrFragmentProcessors from SkSL code. As the shader code produced from the SkSL depends
 * upon the inputs to the SkSL (static if's, etc.) you must first create a factory for a given SkSL
 * string, then use that to create the actual GrFragmentProcessor.
 */
class GrSkSLFPFactory {
public:
    /**
     * Constructs a GrSkSLFPFactory for a given SkSL program. Creating a factory will preprocess
     * the SkSL and determine which of its inputs are declared "key" (meaning they cause the
     * produced shaders to differ), so it is important to reuse the same factory instance for the
     * same shader in order to avoid repeatedly re-parsing the SkSL.
     */
    GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps, const char* sksl);

    std::unique_ptr<GrFragmentProcessor> make(
                   const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs);

private:
    const char* fName;
    SkSL::Compiler fCompiler;
    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<SkSL::String> fKeyVars;
};

class GrSkSLFP : public GrFragmentProcessor {
public:
    enum class ProcessorType {
        kDither
    };

    static std::unique_ptr<GrFragmentProcessor> Make(
                   GrContext* context,
                   ProcessorType processor,
                   const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs) {
        const char* sksl;
        const char* name;
        switch (processor) {
            case ProcessorType::kDither:
                sksl = DITHER_SRC;
                name = "Dither";
                break;
        }
        const auto found = factories.find(processor);
        GrSkSLFPFactory* factory;
        if (found == factories.end()) {
            factory = new GrSkSLFPFactory(name, context->contextPriv().caps()->shaderCaps(), sksl);
            factories[processor] = std::unique_ptr<GrSkSLFPFactory>(factory);
        } else {
            factory = found->second.get();
        }
        return factory->make(inputs);
    }

    const char* name() const override { return fName.c_str(); }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    static std::unique_ptr<GrFragmentProcessor> Make(
                   const char* name,
                   SkSL::Compiler* compiler,
                   std::unique_ptr<SkSL::Program> program,
                   SkSL::String key) {
        return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(std::move(name), compiler,
                                                                 std::move(program), key));
    }

    GrSkSLFP(SkSL::String name, SkSL::Compiler* compiler, std::unique_ptr<SkSL::Program> program,
             SkSL::String key)
    : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
    , fName(std::move(name))
    , fKey(std::move(key)) {
        if (!compiler->toPipelineStage(*program, &fGLSL, &fFormatArgs)) {
            printf("%s\n", compiler->errorText().c_str());
            abort();
        }
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SkSL::String fName;

    SkSL::String fGLSL;

    SkSL::String fKey;

    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    // FIXME find somewhere for this to live
    static std::map<ProcessorType, std::unique_ptr<GrSkSLFPFactory>> factories;

    typedef GrFragmentProcessor INHERITED;

    friend class GrSkSLFPFactory;
};

#endif
