/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectPriv_DEFINED
#define SkRuntimeEffectPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkEffectPriv.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

#include "include/sksl/SkSLVersion.h"

class SkArenaAlloc;
class SkCapabilities;
class SkColorSpace;
class SkData;
class SkMatrix;
class SkReadBuffer;
class SkShader;
class SkWriteBuffer;
struct SkColorSpaceXformSteps;

namespace SkShaders {
class MatrixRec;
}

namespace SkSL {
class Context;
class Variable;
struct Program;
}

class SkRuntimeEffectPriv {
public:
    struct UniformsCallbackContext {
        const SkColorSpace* fDstColorSpace;
    };

    // Private (experimental) API for creating runtime shaders with late-bound uniforms.
    // The callback must produce a uniform data blob of the correct size for the effect.
    // It is invoked at "draw" time (essentially, when a draw call is made against the canvas
    // using the resulting shader). There are no strong guarantees about timing.
    // Serializing the resulting shader will immediately invoke the callback (and record the
    // resulting uniforms).
    using UniformsCallback = std::function<sk_sp<const SkData>(const UniformsCallbackContext&)>;
    static sk_sp<SkShader> MakeDeferredShader(const SkRuntimeEffect* effect,
                                              UniformsCallback uniformsCallback,
                                              SkSpan<const SkRuntimeEffect::ChildPtr> children,
                                              const SkMatrix* localMatrix = nullptr);

    // Helper function when creating an effect for a GrSkSLFP that verifies an effect will
    // implement the GrFragmentProcessor "constant output for constant input" optimization flag.
    static bool SupportsConstantOutputForConstantInput(const SkRuntimeEffect* effect) {
        // This optimization is only implemented for color filters without any children.
        if (!effect->allowColorFilter() || !effect->children().empty()) {
            return false;
        }
        return true;
    }

    static uint32_t Hash(const SkRuntimeEffect& effect) {
        return effect.hash();
    }

    static uint32_t StableKey(const SkRuntimeEffect& effect) {
        return effect.fStableKey;
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

    static void SetStableKey(SkRuntimeEffect::Options* options, uint32_t stableKey) {
        options->fStableKey = stableKey;
    }

    static SkRuntimeEffect::Uniform VarAsUniform(const SkSL::Variable&,
                                                 const SkSL::Context&,
                                                 size_t* offset);

    static SkRuntimeEffect::Child VarAsChild(const SkSL::Variable& var,
                                             int index);

    static const char* ChildTypeToStr(SkRuntimeEffect::ChildType type);

    // If there are layout(color) uniforms then this performs color space transformation on the
    // color values and returns a new SkData. Otherwise, the original data is returned.
    static sk_sp<const SkData> TransformUniforms(SkSpan<const SkRuntimeEffect::Uniform> uniforms,
                                                 sk_sp<const SkData> originalData,
                                                 const SkColorSpaceXformSteps&);
    static sk_sp<const SkData> TransformUniforms(SkSpan<const SkRuntimeEffect::Uniform> uniforms,
                                                 sk_sp<const SkData> originalData,
                                                 const SkColorSpace* dstCS);
    static SkSpan<const float> UniformsAsSpan(
        SkSpan<const SkRuntimeEffect::Uniform> uniforms,
        sk_sp<const SkData> originalData,
        bool alwaysCopyIntoAlloc,
        const SkColorSpace* destColorSpace,
        SkArenaAlloc* alloc);

    static bool CanDraw(const SkCapabilities*, const SkSL::Program*);
    static bool CanDraw(const SkCapabilities*, const SkRuntimeEffect*);

    static bool ReadChildEffects(SkReadBuffer& buffer,
                                 const SkRuntimeEffect* effect,
                                 skia_private::TArray<SkRuntimeEffect::ChildPtr>* children);
    static void WriteChildEffects(SkWriteBuffer& buffer,
                                  SkSpan<const SkRuntimeEffect::ChildPtr> children);

    static bool UsesColorTransform(const SkRuntimeEffect* effect) {
        return effect->usesColorTransform();
    }
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

class RuntimeEffectRPCallbacks : public SkSL::RP::Callbacks {
public:
    // SkStageRec::fPaintColor is used (strictly) to tint alpha-only image shaders with the paint
    // color. We want to suppress that behavior when they're sampled from runtime effects, so we
    // just override the paint color here. See also: SkImageShader::appendStages.
    RuntimeEffectRPCallbacks(const SkStageRec& s,
                             const SkShaders::MatrixRec& m,
                             SkSpan<const SkRuntimeEffect::ChildPtr> c,
                             SkSpan<const SkSL::SampleUsage> u)
            : fStage{s.fPipeline,
                     s.fAlloc,
                     s.fDstColorType,
                     s.fDstCS,
                     SkColors::kTransparent,
                     s.fSurfaceProps}
            , fMatrix(m)
            , fChildren(c)
            , fSampleUsages(u) {}

    bool appendShader(int index) override;
    bool appendColorFilter(int index) override;
    bool appendBlender(int index) override;

    // TODO: If an effect calls these intrinsics more than once, we could cache and re-use the steps
    // object(s), rather than re-creating them in the arena repeatedly.
    void toLinearSrgb(const void* color) override;

    void fromLinearSrgb(const void* color) override;

private:
    void applyColorSpaceXform(const SkColorSpaceXformSteps& tempXform, const void* color);

    const SkStageRec fStage;
    const SkShaders::MatrixRec& fMatrix;
    SkSpan<const SkRuntimeEffect::ChildPtr> fChildren;
    SkSpan<const SkSL::SampleUsage> fSampleUsages;
};

#endif  // SkRuntimeEffectPriv_DEFINED
