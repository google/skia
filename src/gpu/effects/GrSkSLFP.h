/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFP_DEFINED
#define GrSkSLFP_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrContextOptions.h"
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
class SkData;
class SkRuntimeEffect;

class GrSkSLFP : public GrFragmentProcessor {
public:
    /**
     * Creates a new fragment processor from an SkRuntimeEffect and a struct of inputs to the
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
     */
    static std::unique_ptr<GrSkSLFP> Make(GrContext_Base* context,
                                          sk_sp<SkRuntimeEffect> effect,
                                          const char* name,
                                          sk_sp<SkData> inputs,
                                          const SkMatrix* matrix = nullptr);

    const char* name() const override;

    void addChild(std::unique_ptr<GrFragmentProcessor> child);

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    using ShaderErrorHandler = GrContextOptions::ShaderErrorHandler;

    GrSkSLFP(sk_sp<const GrShaderCaps> shaderCaps, ShaderErrorHandler* shaderErrorHandler,
             sk_sp<SkRuntimeEffect> effect, const char* name, sk_sp<SkData> inputs,
             const SkMatrix* matrix);

    GrSkSLFP(const GrSkSLFP& other);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    sk_sp<const GrShaderCaps> fShaderCaps;
    ShaderErrorHandler*       fShaderErrorHandler;

    sk_sp<SkRuntimeEffect> fEffect;
    const char*            fName;
    sk_sp<SkData>          fInputs;

    GrCoordTransform fCoordTransform;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;

    friend class GrGLSLSkSLFP;

    friend class GrSkSLFPFactory;
};

#endif
