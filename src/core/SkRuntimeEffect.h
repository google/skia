/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffect_DEFINED
#define SkRuntimeEffect_DEFINED

#include "include/core/SkString.h"
#include "src/sksl/SkSLCompiler.h"

#include <vector>

class GrShaderCaps;
class SkMatrix;
class SkShader;

namespace SkSL {
class ByteCode;
struct Program;
}

class SkRuntimeEffect : public SkRefCnt {
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

    const SkString& source() const { return fSkSL; }
    int index() const { return fIndex; }
    size_t inputSize() const;
    size_t childCount() const { return fChildren.size(); }

#if SK_SUPPORT_GPU
    // This re-compiles the program from scratch, using the supplied shader caps.
    // This is necessary to get the correct values of settings.
    bool toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                         SkSL::String* outCode,
                         std::vector<SkSL::Compiler::FormatArg>* outFormatArgs,
                         std::vector<SkSL::Compiler::GLSLFunction>* outFunctions);
#endif

    // [ByteCode, ErrorText]
    // If successful, ByteCode != nullptr, otherwise, ErrorText contains the reason for failure.
    using ByteCodeResult = std::tuple<std::unique_ptr<SkSL::ByteCode>, SkString>;

    ByteCodeResult toByteCode();

private:
    SkRuntimeEffect(SkString sksl, std::unique_ptr<SkSL::Compiler> compiler,
                    std::unique_ptr<SkSL::Program> baseProgram);

    int fIndex;
    SkString fSkSL;

    std::unique_ptr<SkSL::Compiler> fCompiler;
    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<Variable> fInAndUniformVars;
    std::vector<SkString> fChildren;

    friend class GrGLSLSkSLFP;
    friend class GrSkSLFP;
    friend class SkSLSlide;
};

#endif
