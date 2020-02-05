/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffect_DEFINED
#define SkRuntimeEffect_DEFINED

#include "include/core/SkString.h"

#include <vector>

#if SK_SUPPORT_GPU
#include "include/private/GrTypesPriv.h"
#endif

class GrShaderCaps;
class SkColorFilter;
class SkMatrix;
class SkShader;

namespace SkSL {
class ByteCode;
class Compiler;
struct PipelineStageArgs;
struct Program;
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
            kArray_Flag = 0x1,
        };

        SkString  fName;
        size_t    fOffset;
        Qualifier fQualifier;
        Type      fType;
        int       fCount;
        uint32_t  fFlags;

#if SK_SUPPORT_GPU
        GrSLType fGPUType;
#endif

        bool isArray() const { return SkToBool(fFlags & kArray_Flag); }
        size_t sizeInBytes() const;
    };

    // [Effect, ErrorText]
    // If successful, Effect != nullptr, otherwise, ErrorText contains the reason for failure.
    using EffectResult = std::tuple<sk_sp<SkRuntimeEffect>, SkString>;

    static EffectResult Make(SkString sksl);

    sk_sp<SkShader> makeShader(sk_sp<SkData> inputs, sk_sp<SkShader> children[], size_t childCount,
                               const SkMatrix* localMatrix, bool isOpaque);

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

#if SK_SUPPORT_GPU
    // This re-compiles the program from scratch, using the supplied shader caps.
    // This is necessary to get the correct values of settings.
    bool toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                         SkSL::PipelineStageArgs* outArgs);
#endif

    // [ByteCode, ErrorText]
    // If successful, ByteCode != nullptr, otherwise, ErrorText contains the reason for failure.
    using ByteCodeResult = std::tuple<std::unique_ptr<SkSL::ByteCode>, SkString>;

    ByteCodeResult toByteCode(const void* inputs);

private:
    SkRuntimeEffect(SkString sksl, std::unique_ptr<SkSL::Compiler> compiler,
                    std::unique_ptr<SkSL::Program> baseProgram,
                    std::vector<Variable>&& inAndUniformVars, std::vector<SkString>&& children,
                    size_t uniformSize);

    using SpecializeResult = std::tuple<std::unique_ptr<SkSL::Program>, SkString>;
    SpecializeResult specialize(SkSL::Program& baseProgram, const void* inputs);

    uint32_t fHash;
    SkString fSkSL;

    std::unique_ptr<SkSL::Compiler> fCompiler;
    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<Variable> fInAndUniformVars;
    std::vector<SkString> fChildren;

    size_t fUniformSize;
};

#endif
