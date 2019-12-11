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

namespace SkSL {
struct Program;
struct Variable;
}

class SkRuntimeEffect : public SkRefCnt {
public:
    static sk_sp<SkRuntimeEffect> Make(SkString sksl);

    bool isValid() const { return fBaseProgram != nullptr; }

    std::unique_ptr<SkSL::Program> getSpecialization(const void* inputs, size_t inputSize);

private:
    SkRuntimeEffect(SkString sksl);

    int fIndex;
    SkString fSkSL;

    SkSL::Compiler fCompiler;
    std::unique_ptr<SkSL::Program> fBaseProgram;
    std::vector<const SkSL::Variable*> fInAndUniformVars;

    friend class GrGLSLSkSLFP;
    friend class GrSkSLFP;
    friend class SkRTShader;
    friend class SkRuntimeColorFilter;
    friend class SkRuntimeColorFilterFactory;
};

#endif
