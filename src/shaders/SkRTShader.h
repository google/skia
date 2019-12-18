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

struct GrFPArgs;
class GrFragmentProcessor;
class SkData;
class SkMatrix;
class SkRuntimeEffect;

namespace SkSL { class ByteCode; }

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs, const SkMatrix* localMatrix,
               sk_sp<SkShader>* children, size_t childCount, bool isOpaque);
    ~SkRTShader() override;

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
    std::vector<sk_sp<SkShader>> fChildren;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;

    typedef SkShaderBase INHERITED;
};

class SK_API SkRuntimeShaderFactory {
public:
    SkRuntimeShaderFactory(SkString sksl, bool isOpaque);
    SkRuntimeShaderFactory(const SkRuntimeShaderFactory&);
    SkRuntimeShaderFactory(SkRuntimeShaderFactory&&);

    ~SkRuntimeShaderFactory();

    SkRuntimeShaderFactory& operator=(const SkRuntimeShaderFactory&);
    SkRuntimeShaderFactory& operator=(SkRuntimeShaderFactory&&);

    sk_sp<SkShader> make(sk_sp<SkData> inputs, const SkMatrix* localMatrix) {
        return this->make(std::move(inputs), localMatrix, nullptr, 0);
    }

    sk_sp<SkShader> make(sk_sp<SkData> inputs, const SkMatrix* localMatrix,
                         sk_sp<SkShader>* children, size_t childCount);

private:
    sk_sp<SkRuntimeEffect> fEffect;
    bool fIsOpaque;
};

#endif
