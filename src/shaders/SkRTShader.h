/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRTShader_DEFINED
#define SkRTShader_DEFINED

#include "include/core/SkString.h"
#include "include/private/SkMutex.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

struct GrFPArgs;
class GrFragmentProcessor;
class SkData;
class SkMatrix;

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

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs, const SkMatrix* localMatrix,
               bool isOpaque);

    bool isOpaque() const override { return fIsOpaque; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec& rec) const override;

private:
    SK_FLATTENABLE_HOOKS(SkRTShader)

    sk_sp<SkRuntimeEffect> fEffect;
    bool fIsOpaque;

    sk_sp<SkData> fInputs;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;

    typedef SkShaderBase INHERITED;
};

class SK_API SkRuntimeShaderFactory {
public:
    SkRuntimeShaderFactory(SkString sksl, bool isOpaque);

    sk_sp<SkShader> make(sk_sp<SkData> inputs, const SkMatrix* localMatrix);

private:
    sk_sp<SkRuntimeEffect> fEffect;
    bool fIsOpaque;
};

#endif
