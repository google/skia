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

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#endif

class SkData;
class SkMatrix;

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(SkString sksl, sk_sp<SkData> inputs, const SkMatrix* localMatrix, bool isOpaque);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override {
        // TODO
        return nullptr;
    }
#endif

    bool isOpaque() const override { return fIsOpaque; }

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec& rec) const override;

private:
    SK_FLATTENABLE_HOOKS(SkRTShader)

    SkString fSkSL;
    sk_sp<SkData> fInputs;
    const bool fIsOpaque;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;

    typedef SkShaderBase INHERITED;
};

#endif
