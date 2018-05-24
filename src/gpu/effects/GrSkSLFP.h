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

/**
 * Produces GrFragmentProcessors from SkSL code. As the shader code produced from the SkSL depends
 * upon the inputs to the SkSL (static if's, etc.) we first create a factory for a given SkSL
 * string, then use that to create the actual GrFragmentProcessor.
 */
class GrSkSLFPFactory {
    /**
     * Constructs a GrSkSLFPFactory for a given SkSL source string. Creating a factory will
     * preprocess the SkSL and determine which of its inputs are declared "key" (meaning they cause
     * the produced shaders to differ), so it is important to reuse the same factory instance for
     * the same shader in order to avoid repeatedly re-parsing the SkSL.
     */
    GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps, const char* sksl);

    std::unique_ptr<GrFragmentProcessor> make(
                   const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs);

    const char* fName;
    SkSL::Compiler fCompiler;
    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<SkSL::String> fKeyVars;

    friend class GrSkSLFP;
};

class GrSkSLFP : public GrFragmentProcessor {
public:
    /**
     * Creates a new fragment processor from an SkSL source string and a map of inputs to the
     * program.
     *
     * As turning SkSL into GLSL / SPIR-V / etc. is fairly expensive, and the output may differ
     * based on the inputs map, internally the process is divided into two steps: we first parse and
     * semantically analyze the SkSL into an internal representation, and then "specialize" this
     * internal representation based on the inputs map. The unspecialized internal representation of
     * the program is cached, so further specializations of the same code are much faster than the
     * first call.
     *
     * This caching is based on the actual pointer value of the source code string, as opposed to
     * its contents, so for performance it is important that the sksl pointer have the same value
     * for the same source code.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(
                   GrContext* context,
                   const char* name,
                   const char* sksl,
                   const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs) {
        const auto found = factories.find(sksl);
        GrSkSLFPFactory* factory;
        if (found == factories.end()) {
            factory = new GrSkSLFPFactory(name, context->contextPriv().caps()->shaderCaps(), sksl);
            factories[sksl] = std::unique_ptr<GrSkSLFPFactory>(factory);
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
    static std::unordered_map<const char*, std::unique_ptr<GrSkSLFPFactory>> factories;

    typedef GrFragmentProcessor INHERITED;

    friend class GrSkSLFPFactory;
};

#endif
