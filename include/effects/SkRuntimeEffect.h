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

#include <algorithm>
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
 * *
 * This API is experimental and subject to change.
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
        bool hasMarker() const { return SkToBool(fFlags & kMarker_Flag); }
        bool hasNormalsMarker() const { return SkToBool(fFlags & kMarkerNormals_Flag); }
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

    sk_sp<SkShader> makeShader(sk_sp<SkData> inputs, sk_sp<SkShader> children[], size_t childCount,
                               const SkMatrix* localMatrix, bool isOpaque);

    sk_sp<SkColorFilter> makeColorFilter(sk_sp<SkData> inputs, sk_sp<SkColorFilter> children[],
                                         size_t childCount);
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

    // Combined size of just the 'uniform' variables.
    size_t uniformSize() const { return fUniformSize; }

    ConstIterable<Variable> inputs() const { return ConstIterable<Variable>(fInAndUniformVars); }
    ConstIterable<SkString> children() const { return ConstIterable<SkString>(fChildren); }
    ConstIterable<Varying> varyings() const { return ConstIterable<Varying>(fVaryings); }

#if SK_SUPPORT_GPU
    // This re-compiles the program from scratch, using the supplied shader caps.
    // This is necessary to get the correct values of settings.
    bool toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                         GrContextOptions::ShaderErrorHandler* errorHandler,
                         SkSL::PipelineStageArgs* outArgs);
#endif

    // [ByteCode, ErrorText]
    // If successful, ByteCode != nullptr, otherwise, ErrorText contains the reason for failure.
    using ByteCodeResult = std::tuple<std::unique_ptr<SkSL::ByteCode>, SkString>;

    ByteCodeResult toByteCode(const void* inputs) const;

    static void RegisterFlattenables();

    ~SkRuntimeEffect();

    // Simple utility to create an input data block for a particular effect. Usage:
    //   sk_sp<SkRuntimeEffect> effect = ...;
    //   SkRuntimeEffect::InputBuilder inputs(effect);
    //   inputs["some_uniform_float"]  = 3.14f;
    //   inputs["some_uniform_matrix"] = SkM44::Rotate(...);
    //   ...
    //   sk_sp<SkShader> shader = effect->makeShader(inputs.fData, ...);
    struct InputBuilder {
        InputBuilder(sk_sp<SkRuntimeEffect> effect)
            : fEffect(std::move(effect))
            , fData(SkData::MakeUninitialized(fEffect->inputSize())) {}

        struct InputBuilderVar {
            // Copy 'val' to this variable. No type conversion is performed - 'val' must be same
            // size as expected by the effect. Information about the variable can be queried by
            // looking at fVar. If the size is incorrect, no copy will be performed, and debug
            // builds will abort. If this is the result of querying a missing variable, fVar will
            // be nullptr, and assigning will also do nothing (and abort in debug builds).
            template <typename T> InputBuilderVar& operator=(const T& val) {
                if (!fVar) {
                    SkDEBUGFAIL("Assigning to missing variable");
                } else if (sizeof(val) != fVar->sizeInBytes()) {
                    SkDEBUGFAIL("Incorrect value size");
                } else {
                    memcpy(SkTAddOffset<void>(fOwner->fData->writable_data(), fVar->fOffset), &val,
                           sizeof(val));
                }
                return *this;
            }

            InputBuilder* fOwner;
            const SkRuntimeEffect::Variable* fVar;  // nullptr if the variable was not found
        };

        InputBuilderVar operator[](const char* name) {
            auto varIter = std::find_if(fEffect->inputs().begin(), fEffect->inputs().end(),
                                        [name](const auto& v) { return v.fName.equals(name); });
            const SkRuntimeEffect::Variable* varPtr =
                    (varIter == fEffect->inputs().end()) ? nullptr : &(*varIter);
            return { this, varPtr };
        }

        sk_sp<SkRuntimeEffect> fEffect;
        sk_sp<SkData>          fData;
    };

private:
    SkRuntimeEffect(SkString sksl, std::unique_ptr<SkSL::Program> baseProgram,
                    std::vector<Variable>&& inAndUniformVars, std::vector<SkString>&& children,
                    std::vector<Varying>&& varyings, size_t uniformSize);

    using SpecializeResult = std::tuple<std::unique_ptr<SkSL::Program>, SkString>;
    SpecializeResult specialize(SkSL::Program& baseProgram, const void* inputs,
                                const SkSL::SharedCompiler&) const;

    uint32_t fHash;
    SkString fSkSL;

    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<Variable> fInAndUniformVars;
    std::vector<SkString> fChildren;
    std::vector<Varying>  fVaryings;

    size_t fUniformSize;
};

#endif
