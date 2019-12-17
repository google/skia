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

namespace SkSL {
class ByteCode;
struct Program;
struct Variable;
}

class SkRuntimeEffect : public SkRefCnt {
public:
    enum class CPUType {
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

    struct Variable {
        SkString fName;
        size_t   fOffset;
        bool     fUniform;  // Otherwise, 'in'
        CPUType  fCPUType;

#if SK_SUPPORT_GPU
        GrSLType fGPUType;
#endif
        int      fArrayCount;  // 0 means scalar

        size_t sizeInBytes() const;
    };

    static sk_sp<SkRuntimeEffect> Make(SkString sksl);

    const SkString& source() const { return fSkSL; }
    int index() const { return fIndex; }
    bool isValid() const { return fBaseProgram != nullptr; }
    size_t inputSize() const;

#if SK_SUPPORT_GPU
    // This re-compiles the program from scratch, using the supplied shader caps.
    // This is necessary to get the correct values of settings.
    bool toPipelineStage(const void* inputs, size_t inputSize, const GrShaderCaps* shaderCaps,
                         SkSL::String* outCode,
                         std::vector<SkSL::Compiler::FormatArg>* outFormatArgs,
                         std::vector<SkSL::Compiler::GLSLFunction>* outFunctions);
#endif

    // Returns ByteCode, ErrorCount, ErrorText
    std::tuple<std::unique_ptr<SkSL::ByteCode>, int, SkString> toByteCode();

private:
    SkRuntimeEffect(SkString sksl);

    int fIndex;
    SkString fSkSL;

    SkSL::Compiler fCompiler;
    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<Variable> fInAndUniformVars;

    friend class GrGLSLSkSLFP;
    friend class GrSkSLFP;
};

#endif
