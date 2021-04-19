/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffect_DEFINED
#define SkRuntimeEffect_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/private/SkOnce.h"
#include "include/private/SkSLSampleUsage.h"

#include <vector>

class GrFragmentProcessor;
class GrRecordingContext;
class SkColorFilter;
class SkImage;
class SkShader;

namespace SkSL {
class FunctionDefinition;
struct Program;
enum class ProgramKind : int8_t;
}  // namespace SkSL

namespace skvm {
class Program;
}  // namespace skvm

/*
 * SkRuntimeEffect supports creating custom SkShader and SkColorFilter objects using Skia's SkSL
 * shading language.
 *
 * NOTE: This API is experimental and subject to change.
 */
class SK_API SkRuntimeEffect : public SkRefCnt {
public:
    struct Uniform {
        enum class Type {
            kFloat,
            kFloat2,
            kFloat3,
            kFloat4,
            kFloat2x2,
            kFloat3x3,
            kFloat4x4,
            kInt,
            kInt2,
            kInt3,
            kInt4,
        };

        enum Flags {
            kArray_Flag         = 0x1,
            kMarker_Flag        = 0x2,
            kMarkerNormals_Flag = 0x4,
            kSRGBUnpremul_Flag  = 0x8,
        };

        SkString  name;
        size_t    offset;
        Type      type;
        int       count;
        uint32_t  flags;
        uint32_t  marker;

        bool isArray() const { return SkToBool(this->flags & kArray_Flag); }
        size_t sizeInBytes() const;
    };

    struct Options {
        // For testing purposes, completely disable the inliner. (Normally, Runtime Effects don't
        // run the inliner directly, but they still get an inlining pass once they are painted.)
        bool forceNoInline = false;
    };

    // If the effect is compiled successfully, `effect` will be non-null.
    // Otherwise, `errorText` will contain the reason for failure.
    struct Result {
        sk_sp<SkRuntimeEffect> effect;
        SkString errorText;
    };

    // MakeForColorFilter and MakeForShader verify that the SkSL code is valid for those stages of
    // the Skia pipeline. In all of the signatures described below, color parameters and return
    // values are flexible. They are listed as being 'vec4', but they can also be 'half4' or
    // 'float4'. ('vec4' is an alias for 'float4').

    // Color filter SkSL requires an entry point that looks like:
    //     vec4 main(vec4 inColor) { ... }
    static Result MakeForColorFilter(SkString sksl, const Options&);

    // Shader SkSL requires an entry point that looks like:
    //     vec4 main(vec2 inCoords) { ... }
    //   -or-
    //     vec4 main(vec2 inCoords, vec4 inColor) { ... }
    //
    // Most shaders don't use the input color, so that parameter is optional.
    static Result MakeForShader(SkString sksl, const Options&);

    // [DEPRECATED] Make supports SkSL that is legal as either an SkShader or SkColorFilter.
    // makeColorFilter might return nullptr, if the effect is dependent on position in any way.
    static Result Make(SkString sksl, const Options&);

    // We can't use a default argument for `options` due to a bug in Clang.
    // https://bugs.llvm.org/show_bug.cgi?id=36684
    static Result MakeForColorFilter(SkString sksl) {
        return MakeForColorFilter(std::move(sksl), Options{});
    }
    static Result MakeForShader(SkString sksl) {
        return MakeForShader(std::move(sksl), Options{});
    }
    static Result Make(SkString sksl) {
        return Make(std::move(sksl), Options{});
    }

    static Result MakeForColorFilter(std::unique_ptr<SkSL::Program> program);

    static Result MakeForShader(std::unique_ptr<SkSL::Program> program);

    sk_sp<SkShader> makeShader(sk_sp<SkData> uniforms,
                               sk_sp<SkShader> children[],
                               size_t childCount,
                               const SkMatrix* localMatrix,
                               bool isOpaque) const;

    sk_sp<SkImage> makeImage(GrRecordingContext*,
                             sk_sp<SkData> uniforms,
                             sk_sp<SkShader> children[],
                             size_t childCount,
                             const SkMatrix* localMatrix,
                             SkImageInfo resultInfo,
                             bool mipmapped) const;

    sk_sp<SkColorFilter> makeColorFilter(sk_sp<SkData> uniforms) const;
    sk_sp<SkColorFilter> makeColorFilter(sk_sp<SkData> uniforms,
                                         sk_sp<SkColorFilter> children[],
                                         size_t childCount) const;

    const SkString& source() const { return fSkSL; }

    template <typename T>
    class ConstIterable {
    public:
        ConstIterable(const std::vector<T>& vec) : fVec(vec) {}

        using const_iterator = typename std::vector<T>::const_iterator;

        const_iterator begin() const { return fVec.begin(); }
        const_iterator end() const { return fVec.end(); }
        size_t count() const { return fVec.size(); }

    private:
        const std::vector<T>& fVec;
    };

    // Combined size of all 'uniform' variables. When calling makeColorFilter or makeShader,
    // provide an SkData of this size, containing values for all of those variables.
    size_t uniformSize() const;

    ConstIterable<Uniform> uniforms() const { return ConstIterable<Uniform>(fUniforms); }
    ConstIterable<SkString> children() const { return ConstIterable<SkString>(fChildren); }

    // Returns pointer to the named uniform variable's description, or nullptr if not found
    const Uniform* findUniform(const char* name) const;

    // Returns index of the named child, or -1 if not found
    int findChild(const char* name) const;

    static void RegisterFlattenables();
    ~SkRuntimeEffect() override;

#if SK_SUPPORT_GPU
    // For internal use.
    std::unique_ptr<GrFragmentProcessor> makeFP(sk_sp<SkData> uniforms,
                                                std::unique_ptr<GrFragmentProcessor> children[],
                                                size_t childCount) const;
#endif

private:
    enum Flags {
        kUsesSampleCoords_Flag = 0x1,
        kAllowColorFilter_Flag = 0x2,
        kAllowShader_Flag      = 0x4,
    };

    SkRuntimeEffect(SkString sksl,
                    std::unique_ptr<SkSL::Program> baseProgram,
                    const Options& options,
                    const SkSL::FunctionDefinition& main,
                    std::vector<Uniform>&& uniforms,
                    std::vector<SkString>&& children,
                    std::vector<SkSL::SampleUsage>&& sampleUsages,
                    uint32_t flags);

    static Result Make(std::unique_ptr<SkSL::Program> program, SkSL::ProgramKind kind);

    static Result Make(SkString sksl, const Options& options, SkSL::ProgramKind kind);

    static Result Make(SkString sksl, std::unique_ptr<SkSL::Program> program,
                       const Options& options, SkSL::ProgramKind kind);

    uint32_t hash() const { return fHash; }
    bool usesSampleCoords() const { return (fFlags & kUsesSampleCoords_Flag); }
    bool allowShader()      const { return (fFlags & kAllowShader_Flag);      }
    bool allowColorFilter() const { return (fFlags & kAllowColorFilter_Flag); }

    struct FilterColorInfo {
        const skvm::Program& program;
        bool                 alphaUnchanged;
    };
    FilterColorInfo getFilterColorInfo();

#if SK_SUPPORT_GPU
    friend class GrSkSLFP;      // fBaseProgram, fSampleUsages
    friend class GrGLSLSkSLFP;  //
#endif

    friend class SkRTShader;            // fBaseProgram, fMain
    friend class SkRuntimeColorFilter;  //

    uint32_t fHash;
    SkString fSkSL;

    std::unique_ptr<SkSL::Program> fBaseProgram;
    const SkSL::FunctionDefinition& fMain;
    std::vector<Uniform> fUniforms;
    std::vector<SkString> fChildren;
    std::vector<SkSL::SampleUsage> fSampleUsages;

    SkOnce fColorFilterProgramOnce;
    std::unique_ptr<skvm::Program> fColorFilterProgram;
    bool fColorFilterProgramLeavesAlphaUnchanged;

    uint32_t fFlags;  // Flags
};

/** Base class for SkRuntimeShaderBuilder, defined below. */
template <typename Child> class SkRuntimeEffectBuilder {
public:
    struct BuilderUniform {
        // Copy 'val' to this variable. No type conversion is performed - 'val' must be same
        // size as expected by the effect. Information about the variable can be queried by
        // looking at fVar. If the size is incorrect, no copy will be performed, and debug
        // builds will abort. If this is the result of querying a missing variable, fVar will
        // be nullptr, and assigning will also do nothing (and abort in debug builds).
        template <typename T>
        std::enable_if_t<std::is_trivially_copyable<T>::value, BuilderUniform&> operator=(
                const T& val) {
            if (!fVar) {
                SkDEBUGFAIL("Assigning to missing variable");
            } else if (sizeof(val) != fVar->sizeInBytes()) {
                SkDEBUGFAIL("Incorrect value size");
            } else {
                memcpy(SkTAddOffset<void>(fOwner->writableUniformData(), fVar->offset),
                       &val, sizeof(val));
            }
            return *this;
        }

        BuilderUniform& operator=(const SkMatrix& val) {
            if (!fVar) {
                SkDEBUGFAIL("Assigning to missing variable");
            } else if (fVar->sizeInBytes() != 9 * sizeof(float)) {
                SkDEBUGFAIL("Incorrect value size");
            } else {
                float* data = SkTAddOffset<float>(fOwner->writableUniformData(), fVar->offset);
                data[0] = val.get(0); data[1] = val.get(3); data[2] = val.get(6);
                data[3] = val.get(1); data[4] = val.get(4); data[5] = val.get(7);
                data[6] = val.get(2); data[7] = val.get(5); data[8] = val.get(8);
            }
            return *this;
        }

        template <typename T>
        bool set(const T val[], const int count) {
            static_assert(std::is_trivially_copyable<T>::value, "Value must be trivial copyable");
            if (!fVar) {
                SkDEBUGFAIL("Assigning to missing variable");
                return false;
            } else if (sizeof(T) * count != fVar->sizeInBytes()) {
                SkDEBUGFAIL("Incorrect value size");
                return false;
            } else {
                memcpy(SkTAddOffset<void>(fOwner->writableUniformData(), fVar->offset),
                       val, sizeof(T) * count);
            }
            return true;
        }

        SkRuntimeEffectBuilder*         fOwner;
        const SkRuntimeEffect::Uniform* fVar;    // nullptr if the variable was not found
    };

    struct BuilderChild {
        template <typename C> BuilderChild& operator=(C&& val) {
            if (fIndex < 0) {
                SkDEBUGFAIL("Assigning to missing child");
            } else {
                fOwner->fChildren[fIndex] = std::forward<C>(val);
            }
            return *this;
        }

        SkRuntimeEffectBuilder* fOwner;
        int                     fIndex;  // -1 if the child was not found
    };

    const SkRuntimeEffect* effect() const { return fEffect.get(); }

    BuilderUniform uniform(const char* name) { return { this, fEffect->findUniform(name) }; }
    BuilderChild child(const char* name) { return { this, fEffect->findChild(name) }; }

protected:
    SkRuntimeEffectBuilder() = delete;
    explicit SkRuntimeEffectBuilder(sk_sp<SkRuntimeEffect> effect)
            : fEffect(std::move(effect))
            , fUniforms(SkData::MakeUninitialized(fEffect->uniformSize()))
            , fChildren(fEffect->children().count()) {}

    SkRuntimeEffectBuilder(SkRuntimeEffectBuilder&&) = default;
    SkRuntimeEffectBuilder(const SkRuntimeEffectBuilder&) = default;

    SkRuntimeEffectBuilder& operator=(SkRuntimeEffectBuilder&&) = delete;
    SkRuntimeEffectBuilder& operator=(const SkRuntimeEffectBuilder&) = delete;

    sk_sp<SkData> uniforms() { return fUniforms; }
    Child* children() { return fChildren.data(); }
    size_t numChildren() { return fChildren.size(); }

private:
    void* writableUniformData() {
        if (!fUniforms->unique()) {
            fUniforms = SkData::MakeWithCopy(fUniforms->data(), fUniforms->size());
        }
        return fUniforms->writable_data();
    }

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData>          fUniforms;
    std::vector<Child>     fChildren;
};

/**
 * SkRuntimeShaderBuilder is a utility to simplify creating SkShader objects from SkRuntimeEffects.
 *
 * NOTE: Like SkRuntimeEffect, this API is experimental and subject to change!
 *
 * Given an SkRuntimeEffect, the SkRuntimeShaderBuilder manages creating an input data block and
 * provides named access to the 'uniform' variables in that block, as well as named access
 * to a list of child shader slots. Usage:
 *
 *   sk_sp<SkRuntimeEffect> effect = ...;
 *   SkRuntimeShaderBuilder builder(effect);
 *   builder.uniform("some_uniform_float")  = 3.14f;
 *   builder.uniform("some_uniform_matrix") = SkM44::Rotate(...);
 *   builder.child("some_child_effect")     = mySkImage->makeShader(...);
 *   ...
 *   sk_sp<SkShader> shader = builder.makeShader(nullptr, false);
 *
 * Note that SkRuntimeShaderBuilder is built entirely on the public API of SkRuntimeEffect,
 * so can be used as-is or serve as inspiration for other interfaces or binding techniques.
 */
class SK_API SkRuntimeShaderBuilder : public SkRuntimeEffectBuilder<sk_sp<SkShader>> {
public:
    explicit SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect>);
    // This is currently required by Android Framework but may go away if that dependency
    // can be removed.
    SkRuntimeShaderBuilder(const SkRuntimeShaderBuilder&) = default;
    ~SkRuntimeShaderBuilder();

    sk_sp<SkShader> makeShader(const SkMatrix* localMatrix, bool isOpaque);
    sk_sp<SkImage> makeImage(GrRecordingContext*,
                             const SkMatrix* localMatrix,
                             SkImageInfo resultInfo,
                             bool mipmapped);

private:
    using INHERITED = SkRuntimeEffectBuilder<sk_sp<SkShader>>;
};

#endif
