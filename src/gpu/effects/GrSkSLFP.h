/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFP_DEFINED
#define GrSkSLFP_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLPipelineStageCodeGenerator.h"
#include <atomic>

#if GR_TEST_UTILS
#define GR_FP_SRC_STRING const char*
#else
#define GR_FP_SRC_STRING static const char*
#endif

class GrContext_Base;
class GrShaderCaps;
class SkRuntimeEffect;

class GrSkSLFP : public GrFragmentProcessor {
public:
    /**
     * Returns a new unique identifier. Each different SkSL fragment processor should call
     * NewIndex once, statically, and use this index for all calls to Make.
     */
    static int NewIndex() {
        static std::atomic<int> nextIndex{0};
        return nextIndex++;
    }

    /**
     * Creates a new fragment processor from an SkSL source string and a struct of inputs to the
     * program. The input struct's type is derived from the 'in' and 'uniform' variables in the SkSL
     * source, so e.g. the shader:
     *
     *    in bool dither;
     *    uniform float x;
     *    uniform float y;
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
     * While both 'in' and 'uniform' variables go into this struct, the difference between them is
     * that 'in' variables are statically "baked in" to the generated code, becoming literals,
     * whereas uniform variables may be changed from invocation to invocation without having to
     * recompile the shader.
     *
     * As the decision of whether to create a new shader or just upload new uniforms all happens
     * behind the scenes, the difference between the two from an end-user perspective is primarily
     * in performance: on the one hand, changing the value of an 'in' variable is very expensive
     * (requiring the compiler to regenerate the code, upload a new shader to the GPU, and so
     * forth), but on the other hand the compiler can optimize around its value because it is known
     * at compile time. 'in' variables are therefore suitable for things like flags, where there are
     * only a few possible values and a known-in-advance value can cause entire chunks of code to
     * become dead (think static @ifs), while 'uniform's are used for continuous values like colors
     * and coordinates, where it would be silly to create a separate shader for each possible set of
     * values. Other than the (significant) performance implications, the only difference between
     * the two is that 'in' variables can be used in static @if / @switch tests. When in doubt, use
     * 'uniform'.
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
    static std::unique_ptr<GrSkSLFP> Make(GrContext_Base* context,
                                          sk_sp<SkRuntimeEffect> effect,
                                          const char* name,
                                          const void* inputs,
                                          size_t inputSize,
                                          const SkMatrix* matrix = nullptr);

    const char* name() const override;

    void addChild(std::unique_ptr<GrFragmentProcessor> child);

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    GrSkSLFP(sk_sp<const GrShaderCaps> shaderCaps, sk_sp<SkRuntimeEffect> effect,
             const char* name, const void* inputs, size_t inputSize, const SkMatrix* matrix);

    GrSkSLFP(const GrSkSLFP& other);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    sk_sp<const GrShaderCaps> fShaderCaps;

    sk_sp<SkRuntimeEffect> fEffect;
    const char*            fName;

    const std::unique_ptr<int8_t[]> fInputs;
    size_t                          fInputSize;

    GrCoordTransform fCoordTransform;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;

    friend class GrGLSLSkSLFP;

    friend class GrSkSLFPFactory;
};

#endif
