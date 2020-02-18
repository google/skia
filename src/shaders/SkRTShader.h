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

namespace SkSL {
    class ByteCodeFunction;

    template<int width>
    class Interpreter;
}

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
    static constexpr int VECTOR_WIDTH = 8;

    SK_FLATTENABLE_HOOKS(SkRTShader)

    sk_sp<SkRuntimeEffect> fEffect;
    bool fIsOpaque;

    sk_sp<SkData> fInputs;
    std::vector<sk_sp<SkShader>> fChildren;

    mutable SkMutex fInterpreterMutex;
    mutable std::unique_ptr<SkSL::Interpreter<VECTOR_WIDTH>> fInterpreter;
    mutable const SkSL::ByteCodeFunction* fMain;

    typedef SkShaderBase INHERITED;
};

#endif
