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
#include "include/core/SkString.h"
#include "include/private/SkOnce.h"
#include "include/private/SkSLSampleUsage.h"

#include <vector>

class GrRecordingContext;
class SkColorFilter;
class SkImage;
class SkShader;

namespace SkSL {
class FunctionDefinition;
struct Program;
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

    struct Varying {
        SkString name;
        int      width;  // 1 - 4 (floats)
    };

    struct Options {
        // Sets an upper limit on the acceptable amount of code growth from inlining.
        // By default, runtime effects don't run the inliner directly.
        int inlineThreshold = 0;
    };

    // If the effect is compiled successfully, `effect` will be non-null.
    // Otherwise, `errorText` will contain the reason for failure.
    struct Result {
        sk_sp<SkRuntimeEffect> effect;
        SkString errorText;
    };

    static Result Make(SkString sksl, const Options& options);

    // We can't use a default argument for `options` due to a bug in Clang.
    // https://bugs.llvm.org/show_bug.cgi?id=36684
    static Result Make(SkString sksl) { return Make(std::move(sksl), Options{}); }

    sk_sp<SkShader> makeShader(sk_sp<SkData> uniforms,
                               sk_sp<SkShader> children[],
                               size_t childCount,
                               const SkMatrix* localMatrix,
                               bool isOpaque);

    sk_sp<SkImage> makeImage(GrRecordingContext*,
                             sk_sp<SkData> uniforms,
                             sk_sp<SkShader> children[],
                             size_t childCount,
                             const SkMatrix* localMatrix,
                             SkImageInfo resultInfo,
                             bool mipmapped);

    sk_sp<SkColorFilter> makeColorFilter(sk_sp<SkData> uniforms);
    sk_sp<SkColorFilter> makeColorFilter(sk_sp<SkData> uniforms,
                                         sk_sp<SkColorFilter> children[],
                                         size_t childCount);

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
    ConstIterable<Varying> varyings() const { return ConstIterable<Varying>(fVaryings); }

    // Returns pointer to the named uniform variable's description, or nullptr if not found
    const Uniform* findUniform(const char* name) const;

    // Returns index of the named child, or -1 if not found
    int findChild(const char* name) const;

    static void RegisterFlattenables();
    ~SkRuntimeEffect() override;

private:
    SkRuntimeEffect(SkString sksl,
                    std::unique_ptr<SkSL::Program> baseProgram,
                    const SkSL::FunctionDefinition& main,
                    std::vector<Uniform>&& uniforms,
                    std::vector<SkString>&& children,
                    std::vector<SkSL::SampleUsage>&& sampleUsages,
                    std::vector<Varying>&& varyings,
                    bool usesSampleCoords,
                    bool allowColorFilter);

    uint32_t hash() const { return fHash; }
    bool usesSampleCoords() const { return fUsesSampleCoords; }

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
    std::vector<Varying>  fVaryings;

    SkOnce fColorFilterProgramOnce;
    std::unique_ptr<skvm::Program> fColorFilterProgram;

    bool   fUsesSampleCoords;
    bool   fAllowColorFilter;
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
class SkRuntimeShaderBuilder {
public:
    SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect>);
    ~SkRuntimeShaderBuilder();

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

        SkRuntimeShaderBuilder*         fOwner;
        const SkRuntimeEffect::Uniform* fVar;    // nullptr if the variable was not found
    };

    struct BuilderChild {
        BuilderChild& operator=(const sk_sp<SkShader>& val);

        SkRuntimeShaderBuilder* fOwner;
        int                     fIndex;  // -1 if the child was not found
    };

    const SkRuntimeEffect* effect() const { return fEffect.get(); }

    BuilderUniform uniform(const char* name) { return { this, fEffect->findUniform(name) }; }
    BuilderChild child(const char* name) { return { this, fEffect->findChild(name) }; }

    sk_sp<SkShader> makeShader(const SkMatrix* localMatrix, bool isOpaque);
    sk_sp<SkImage> makeImage(GrRecordingContext*,
                             const SkMatrix* localMatrix,
                             SkImageInfo resultInfo,
                             bool mipmapped);

private:
    void* writableUniformData();

    sk_sp<SkRuntimeEffect>       fEffect;
    sk_sp<SkData>                fUniforms;
    std::vector<sk_sp<SkShader>> fChildren;
};

#endif
