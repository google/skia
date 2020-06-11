/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffect_DEFINED
#define SkRuntimeEffect_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkString.h"

#include <vector>

#if SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "include/private/GrTypesPriv.h"
#endif

class GrShaderCaps;
class SkColorFilter;
class SkMatrix;
class SkShader;

namespace SkSL {
class ByteCode;
struct PipelineStageArgs;
struct Program;
class SharedCompiler;
}

/*
 * SkRuntimeEffect supports creating custom SkShader and SkColorFilter objects using Skia's SkSL
 * shading language.
 *
 * NOTE: This API is experimental and subject to change.
 */
class SK_API SkRuntimeEffect : public SkRefCnt {
public:
    struct Variable {
        enum class Qualifier {
            kUniform,
            kIn,
        };

        enum class Type {
            kBool,
            kInt,
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

        SkString  fName;
        size_t    fOffset;
        Qualifier fQualifier;
        Type      fType;
        int       fCount;
        uint32_t  fFlags;
        uint32_t  fMarker;

#if SK_SUPPORT_GPU
        GrSLType fGPUType;
#endif

        bool isArray() const { return SkToBool(fFlags & kArray_Flag); }
        size_t sizeInBytes() const;
    };

    struct Varying {
        SkString fName;
        int      fWidth;  // 1 - 4 (floats)
    };

    // [Effect, ErrorText]
    // If successful, Effect != nullptr, otherwise, ErrorText contains the reason for failure.
    using EffectResult = std::tuple<sk_sp<SkRuntimeEffect>, SkString>;
    static EffectResult Make(SkString sksl);

    sk_sp<SkShader> makeShader(sk_sp<SkData> inputs,
                               sk_sp<SkShader> children[],
                               size_t childCount,
                               const SkMatrix* localMatrix,
                               bool isOpaque);

    sk_sp<SkColorFilter> makeColorFilter(sk_sp<SkData> inputs);

    const SkString& source() const { return fSkSL; }
    uint32_t hash() const { return fHash; }

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

    // Combined size of all 'in' and 'uniform' variables. When calling makeColorFilter or
    // makeShader, provide an SkData of this size, containing values for all of those variables.
    size_t inputSize() const;

    ConstIterable<Variable> inputs() const { return ConstIterable<Variable>(fInAndUniformVars); }
    ConstIterable<SkString> children() const { return ConstIterable<SkString>(fChildren); }
    ConstIterable<Varying> varyings() const { return ConstIterable<Varying>(fVaryings); }

    // Returns pointer to the named in/uniform variable's description, or nullptr if not found
    const Variable* findInput(const char* name) const;

    // Returns index of the named child, or -1 if not found
    int findChild(const char* name) const;

    static void RegisterFlattenables();
    ~SkRuntimeEffect();

private:
    SkRuntimeEffect(SkString sksl, std::unique_ptr<SkSL::Program> baseProgram,
                    std::vector<Variable>&& inAndUniformVars, std::vector<SkString>&& children,
                    std::vector<Varying>&& varyings, size_t uniformSize);

    using SpecializeResult = std::tuple<std::unique_ptr<SkSL::Program>, SkString>;
    SpecializeResult specialize(SkSL::Program& baseProgram, const void* inputs,
                                const SkSL::SharedCompiler&) const;

#if SK_SUPPORT_GPU
    friend class GrSkSLFP;  // toPipelineStage

    // This re-compiles the program from scratch, using the supplied shader caps.
    // This is necessary to get the correct values of settings.
    bool toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                         GrContextOptions::ShaderErrorHandler* errorHandler,
                         SkSL::PipelineStageArgs* outArgs);
#endif

    friend class SkRTShader;            // toByteCode & uniformSize
    friend class SkRuntimeColorFilter;  //

    // [ByteCode, ErrorText]
    // If successful, ByteCode != nullptr, otherwise, ErrorText contains the reason for failure.
    using ByteCodeResult = std::tuple<std::unique_ptr<SkSL::ByteCode>, SkString>;
    ByteCodeResult toByteCode(const void* inputs) const;

    // Combined size of just the 'uniform' variables.
    size_t uniformSize() const { return fUniformSize; }


    uint32_t fHash;
    SkString fSkSL;

    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<Variable> fInAndUniformVars;
    std::vector<SkString> fChildren;
    std::vector<Varying>  fVaryings;

    size_t fUniformSize;
};

/**
 * SkRuntimeShaderBuilder is a utility to simplify creating SkShader objects from SkRuntimeEffects.
 *
 * NOTE: Like SkRuntimeEffect, this API is experimental and subject to change!
 *
 * Given an SkRuntimeEffect, the SkRuntimeShaderBuilder manages creating an input data block and
 * provides named access to the 'in' and 'uniform' variables in that block, as well as named access
 * to a list of child shader slots. Usage:
 *
 *   sk_sp<SkRuntimeEffect> effect = ...;
 *   SkRuntimeShaderBuilder builder(effect);
 *   builder.input("some_uniform_float")  = 3.14f;
 *   builder.input("some_uniform_matrix") = SkM44::Rotate(...);
 *   builder.child("some_child_effect")   = mySkImage->makeShader(...);
 *   ...
 *   sk_sp<SkShader> shader = builder.makeShader(nullptr, false);
 *
 * Note that SkRuntimeShaderBuilder is built entirely on the public API of SkRuntimeEffect,
 * so can be used as-is or serve as inspiration for other interfaces or binding techniques.
 */
struct SkRuntimeShaderBuilder {
    SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect>);
    ~SkRuntimeShaderBuilder();

    struct BuilderInput {
        // Copy 'val' to this variable. No type conversion is performed - 'val' must be same
        // size as expected by the effect. Information about the variable can be queried by
        // looking at fVar. If the size is incorrect, no copy will be performed, and debug
        // builds will abort. If this is the result of querying a missing variable, fVar will
        // be nullptr, and assigning will also do nothing (and abort in debug builds).
        template <typename T>
        std::enable_if_t<std::is_trivially_copyable<T>::value, BuilderInput&> operator=(
                const T& val) {
            if (!fVar) {
                SkDEBUGFAIL("Assigning to missing variable");
            } else if (sizeof(val) != fVar->sizeInBytes()) {
                SkDEBUGFAIL("Incorrect value size");
            } else {
                memcpy(SkTAddOffset<void>(fOwner->fInputs->writable_data(), fVar->fOffset),
                        &val, sizeof(val));
            }
            return *this;
        }

        SkRuntimeShaderBuilder*          fOwner;
        const SkRuntimeEffect::Variable* fVar;    // nullptr if the variable was not found
    };

    struct BuilderChild {
        BuilderChild& operator=(const sk_sp<SkShader>& val);

        SkRuntimeShaderBuilder* fOwner;
        int                     fIndex;  // -1 if the child was not found
    };

    BuilderInput input(const char* name) { return { this, fEffect->findInput(name) }; }
    BuilderChild child(const char* name) { return { this, fEffect->findChild(name) }; }

    sk_sp<SkShader> makeShader(const SkMatrix* localMatrix, bool isOpaque);

    sk_sp<SkRuntimeEffect>       fEffect;
    sk_sp<SkData>                fInputs;
    std::vector<sk_sp<SkShader>> fChildren;
};

#endif
