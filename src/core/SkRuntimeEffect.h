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
    static sk_sp<SkRuntimeEffect> Make(SkString sksl);

    const SkString& source() const { return fSkSL; }
    int index() const { return fIndex; }
    bool isValid() const { return fBaseProgram != nullptr; }

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
    std::vector<const SkSL::Variable*> fInAndUniformVars;

    friend class GrGLSLSkSLFP;
    friend class GrSkSLFP;
};

#endif
