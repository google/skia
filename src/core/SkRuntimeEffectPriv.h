/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectPriv_DEFINED
#define SkRuntimeEffectPriv_DEFINED

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkVM.h"

#include <functional>

#ifdef SK_ENABLE_SKSL

namespace SkSL {
class Context;
class Variable;
struct Program;
}

class SkCapabilities;
struct SkColorSpaceXformSteps;

class SkRuntimeEffectPriv {
public:
    // Helper function when creating an effect for a GrSkSLFP that verifies an effect will
    // implement the constant output for constant input optimization flag.
    static bool SupportsConstantOutputForConstantInput(const SkRuntimeEffect* effect) {
        return effect->getFilterColorProgram();
    }

    static uint32_t Hash(const SkRuntimeEffect& effect) {
        return effect.hash();
    }

    static const SkSL::Program& Program(const SkRuntimeEffect& effect) {
        return *effect.fBaseProgram;
    }

    static SkRuntimeEffect::Options ES3Options() {
        SkRuntimeEffect::Options options;
        options.maxVersionAllowed = SkSL::Version::k300;
        return options;
    }

    static void AllowPrivateAccess(SkRuntimeEffect::Options* options) {
        options->allowPrivateAccess = true;
    }

    static SkRuntimeEffect::Uniform VarAsUniform(const SkSL::Variable&,
                                                 const SkSL::Context&,
                                                 size_t* offset);

    // If there are layout(color) uniforms then this performs color space transformation on the
    // color values and returns a new SkData. Otherwise, the original data is returned.
    static sk_sp<const SkData> TransformUniforms(SkSpan<const SkRuntimeEffect::Uniform> uniforms,
                                                 sk_sp<const SkData> originalData,
                                                 const SkColorSpaceXformSteps&);
    static sk_sp<const SkData> TransformUniforms(SkSpan<const SkRuntimeEffect::Uniform> uniforms,
                                                 sk_sp<const SkData> originalData,
                                                 const SkColorSpace* dstCS);

    static bool CanDraw(const SkCapabilities*, const SkSL::Program*);
    static bool CanDraw(const SkCapabilities*, const SkRuntimeEffect*);
};

// These internal APIs for creating runtime effects vary from the public API in two ways:
//
//     1) they're used in contexts where it's not useful to receive an error message;
//     2) they're cached.
//
// Users of the public SkRuntimeEffect::Make*() can of course cache however they like themselves;
// keeping these APIs private means users will not be forced into our cache or cache policy.

sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::Result (*make)(SkString sksl, const SkRuntimeEffect::Options&),
        SkString sksl);

inline sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::Result (*make)(SkString, const SkRuntimeEffect::Options&),
        const char* sksl) {
    return SkMakeCachedRuntimeEffect(make, SkString{sksl});
}

// Internal API that assumes (and asserts) that the shader code is valid, but does no internal
// caching. Used when the caller will cache the result in a static variable. Ownership is passed to
// the caller; the effect will be leaked if it the pointer is not stored or explicitly deleted.
inline SkRuntimeEffect* SkMakeRuntimeEffect(
        SkRuntimeEffect::Result (*make)(SkString, const SkRuntimeEffect::Options&),
        const char* sksl,
        SkRuntimeEffect::Options options = SkRuntimeEffect::Options{}) {
#if defined(SK_DEBUG)
    // Our SKSL snippets we embed in Skia should not have comments or excess indentation.
    // Removing them helps trim down code size and speeds up parsing
    if (SkStrContains(sksl, "//") || SkStrContains(sksl, "    ")) {
        SkDEBUGFAILF("Found SkSL snippet that can be minified: \n %s\n", sksl);
    }
#endif
    SkRuntimeEffectPriv::AllowPrivateAccess(&options);
    auto result = make(SkString{sksl}, options);
    if (!result.effect) {
        SK_ABORT("%s", result.errorText.c_str());
    }
    return result.effect.release();
}

/**
 * Runtime effects are often long lived & cached. Individual color filters or FPs created from them
 * and are often short-lived. However, color filters and FPs may need to operate on a single color
 * (on the CPU). This may be done at the paint level (eg, filter the paint color), or as part of
 * FP tree analysis.
 *
 * SkFilterColorProgram is an skvm program representing a (color filter) SkRuntimeEffect. It can
 * process a single color, without knowing the details of a particular instance (uniform values or
 * children).
 */
class SkFilterColorProgram {
public:
    static std::unique_ptr<SkFilterColorProgram> Make(const SkRuntimeEffect* effect);

    SkPMColor4f eval(const SkPMColor4f& inColor,
                     const void* uniformData,
                     std::function<SkPMColor4f(int, SkPMColor4f)> evalChild) const;

    bool isAlphaUnchanged() const { return fAlphaUnchanged; }

private:
    struct SampleCall {
        enum class Kind {
            kInputColor,  // eg child.eval(inputColor)
            kImmediate,   // eg child.eval(half4(1))
            kPrevious,    // eg child1.eval(child2.eval(...))
            kUniform,     // eg uniform half4 color; ... child.eval(color)
        };

        int  fChild;
        Kind fKind;
        union {
            SkPMColor4f fImm;       // for kImmediate
            int         fPrevious;  // for kPrevious
            int         fOffset;    // for kUniform
        };
    };

    SkFilterColorProgram(skvm::Program program,
                         std::vector<SampleCall> sampleCalls,
                         bool alphaUnchanged);

    skvm::Program           fProgram;
    std::vector<SampleCall> fSampleCalls;
    bool                    fAlphaUnchanged;
};

#endif  // SK_ENABLE_SKSL

#endif  // SkRuntimeEffectPriv_DEFINED
