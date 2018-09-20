/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFP_DEFINED
#define GrSkSLFP_DEFINED

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrShaderCaps.h"
#include "SkSLCompiler.h"
#include "SkSLPipelineStageCodeGenerator.h"
#include "SkRefCnt.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "../private/GrSkSLFPFactoryCache.h"

#if GR_TEST_UTILS
#define GR_FP_SRC_STRING const char*
#else
#define GR_FP_SRC_STRING static const char*
#endif

class GrContext;
class GrSkSLFPFactory;
class GrGLSLSkSLFP;

class GrSkSLFP : public GrFragmentProcessor {
public:
    using UniformHandle = GrGLSLUniformHandler::UniformHandle;

    /**
     * Returns a new unique identifier. Each different SkSL fragment processor should call
     * NewIndex once, statically, and use this index for all calls to Make.
     */
    static int NewIndex() {
        static int index = 0;
        return sk_atomic_inc(&index);
    }

    /**
     * Creates a new fragment processor from an SkSL source string and a struct of inputs to the
     * program. The input struct's type is derived from the 'in' variables in the SkSL source, so
     * e.g. the shader:
     *
     *    in bool dither;
     *    in float x;
     *    in float y;
     *    ....
     *
     * would expect a pointer to a struct set up like:
     *
     * struct {
     *     bool dither;
     *     float x;
     *     float y;
     * };
     *
     * As turning SkSL into GLSL / SPIR-V / etc. is fairly expensive, and the output may differ
     * based on the inputs, internally the process is divided into two steps: we first parse and
     * semantically analyze the SkSL into an internal representation, and then "specialize" this
     * internal representation based on the inputs. The unspecialized internal representation of
     * the program is cached, so further specializations of the same code are much faster than the
     * first call.
     *
     * This caching is based on the 'index' parameter, which should be derived by statically calling
     * 'NewIndex()'. Each given SkSL string should have a single, statically defined index
     * associated with it.
     */
    template<typename Inputs>
    static std::unique_ptr<GrSkSLFP> Make(GrContext* context, int index, const char* name,
                                          const char* sksl, const Inputs& inputs) {
        void* resultBytes = GrSkSLFP::operator new(sizeof(GrSkSLFP) + sizeof(Inputs));
        void* inputBytes = (int8_t*) resultBytes + sizeof(GrSkSLFP);
        memcpy(inputBytes, &inputs, sizeof(Inputs));
        GrSkSLFP* result = new (resultBytes) GrSkSLFP(context->contextPriv().getFPFactoryCache(),
                                                      context->contextPriv().caps()->shaderCaps(),
                                                      index, name, sksl, inputBytes,
                                                      sizeof(Inputs), nullptr, 0);
        return std::unique_ptr<GrSkSLFP>(result);
    }

    /**
     * As the above overload of Make, but also tracks an additional extraData struct which is shared
     * between all instances of the FP. The extraData struct is copied into the GrGLSLSkSLFP and is
     * accessible from within the setData hook function.
     */
    template<typename Inputs, typename ExtraData>
    static std::unique_ptr<GrSkSLFP> Make(GrContext* context, int index, const char* name,
                                          const char* sksl, const Inputs& inputs,
                                          const ExtraData& extraData) {
        void* resultBytes = GrSkSLFP::operator new(sizeof(GrSkSLFP) + sizeof(Inputs) +
                                                   sizeof(ExtraData));
        void* inputBytes = (int8_t*) resultBytes + sizeof(GrSkSLFP);
        memcpy(inputBytes, &inputs, sizeof(Inputs));
        void* extraDataBytes = (int8_t*) resultBytes + sizeof(GrSkSLFP) + sizeof(Inputs);
        memcpy(extraDataBytes, &extraData, sizeof(ExtraData));
        return std::unique_ptr<GrSkSLFP>(new (resultBytes) GrSkSLFP(
                                                        context->contextPriv().getFPFactoryCache(),
                                                        context->contextPriv().caps()->shaderCaps(),
                                                        index, name, sksl, inputBytes,
                                                        sizeof(Inputs), extraDataBytes,
                                                        sizeof(ExtraData)));
    }

    const char* name() const override;

    void addChild(std::unique_ptr<GrFragmentProcessor> child);

    /**
     * Sets a function to be called after the automatic handling of 'in uniform' variables is
     * completed. This function should ensure that data is provided for all non-'in' uniform
     * variables.
     */
    void setSetDataHook(void (*hook)(const GrGLSLProgramDataManager& pdman,
                                     const GrGLSLSkSLFP& glslProc,
                                     const GrSkSLFP& proc));

    const void* inputs() const { return fInputs; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    GrSkSLFP(sk_sp<GrSkSLFPFactoryCache> factoryCache, const GrShaderCaps* shaderCaps, int fIndex,
             const char* name, const char* sksl, const void* inputs, size_t inputSize,
             const void* extraData, size_t extraDataSize);

    GrSkSLFP(const GrSkSLFP& other);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void createFactory() const;

    sk_sp<GrSkSLFPFactoryCache> fFactoryCache;

    const sk_sp<GrShaderCaps> fShaderCaps;

    mutable sk_sp<GrSkSLFPFactory> fFactory;

    int fIndex;

    const char* fName;

    const char* fSkSL;

    const void* fInputs;

    size_t fInputSize;

    const void* fExtraData;

    size_t fExtraDataSize;

    void (*fSetDataHook)(const GrGLSLProgramDataManager&, const GrGLSLSkSLFP&,
                         const GrSkSLFP&) = nullptr;

    mutable SkSL::String fKey;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;

    friend class GrGLSLSkSLFP;

    friend class GrSkSLFPFactory;
};

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    GrGLSLSkSLFP(SkSL::Context* context, const std::vector<const SkSL::Variable*>* uniformVars,
                 SkSL::String glsl, std::vector<SkSL::Compiler::FormatArg> formatArgs,
                 void* extraData);

    GrSLType uniformType(const SkSL::Type& type);

    /**
     * Returns the extraData pointer.
     */
    void* extraData() const;

    void emitCode(EmitArgs& args) override;

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override;

    UniformHandle uniformHandle(int index) const { return fUniformHandles[index]; }

private:
    const SkSL::Context& fContext;

    const std::vector<const SkSL::Variable*>& fUniformVars;

    // nearly-finished GLSL; still contains printf-style "%s" format tokens
    const SkSL::String fGLSL;

    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;

    std::vector<UniformHandle> fUniformHandles;

    void* fExtraData;
};

/**
 * Produces GrFragmentProcessors from SkSL code. As the shader code produced from the SkSL depends
 * upon the inputs to the SkSL (static if's, etc.) we first create a factory for a given SkSL
 * string, then use that to create the actual GrFragmentProcessor.
 */
class GrSkSLFPFactory : public SkNVRefCnt<GrSkSLFPFactory> {
public:
    /**
     * Constructs a GrSkSLFPFactory for a given SkSL source string. Creating a factory will
     * preprocess the SkSL and determine which of its inputs are declared "key" (meaning they cause
     * the produced shaders to differ), so it is important to reuse the same factory instance for
     * the same shader in order to avoid repeatedly re-parsing the SkSL.
     */
    GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps, const char* sksl);

    const SkSL::Program* getSpecialization(const SkSL::String& key, const void* inputs,
                                           size_t inputSize);

    const char* fName;

    SkSL::Compiler fCompiler;

    std::shared_ptr<SkSL::Program> fBaseProgram;

    std::vector<const SkSL::Variable*> fInputVars;

    std::vector<const SkSL::Variable*> fUniformVars;

    std::vector<const SkSL::Variable*> fKeyVars;

    std::unordered_map<SkSL::String, std::unique_ptr<const SkSL::Program>> fSpecializations;

    friend class GrSkSLFP;
};

#endif
