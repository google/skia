/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "src/shaders/SkRTShader.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#endif

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

SkRTShader::SkRTShader(SkString sksl, sk_sp<SkData> inputs, const SkMatrix* localMatrix,
                       bool isOpaque)
    : SkShaderBase(localMatrix)
    , fSkSL(std::move(sksl))
    , fInputs(std::move(inputs))
    , fIsOpaque(isOpaque)
{}

bool SkRTShader::onAppendStages(const SkStageRec& rec) const {
    SkMatrix inverse;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &inverse)) {
        return false;
    }

    auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
    ctx->inputs = fInputs->data();
    ctx->ninputs = fInputs->size() / 4;
    ctx->shader_convention = true;

    SkAutoMutexExclusive ama(fByteCodeMutex);
    if (!fByteCode) {
        SkSL::Compiler c;
        auto prog = c.convertProgram(SkSL::Program::kGeneric_Kind,
                                     SkSL::String(fSkSL.c_str()),
                                     SkSL::Program::Settings());
        if (c.errorCount()) {
            SkDebugf("%s\n", c.errorText().c_str());
            return false;
        }
        fByteCode = c.toByteCode(*prog);
        if (c.errorCount()) {
            SkDebugf("%s\n", c.errorText().c_str());
            return false;
        }
        SkASSERT(fByteCode);
    }
    ctx->byteCode = fByteCode.get();
    ctx->fn = ctx->byteCode->fFunctions[0].get();

    rec.fPipeline->append(SkRasterPipeline::seed_shader);
    rec.fPipeline->append_matrix(rec.fAlloc, inverse);
    rec.fPipeline->append(SkRasterPipeline::interpreter, ctx);
    return true;
}

enum Flags {
    kIsOpaque_Flag          = 1 << 0,
    kHasLocalMatrix_Flag    = 1 << 1,
};

void SkRTShader::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fIsOpaque) {
        flags |= kIsOpaque_Flag;
    }
    if (!this->getLocalMatrix().isIdentity()) {
        flags |= kHasLocalMatrix_Flag;
    }

    buffer.writeString(fSkSL.c_str());
    if (fInputs) {
        buffer.writeDataAsByteArray(fInputs.get());
    } else {
        buffer.writeByteArray(nullptr, 0);
    }
    buffer.write32(flags);
    if (flags & kHasLocalMatrix_Flag) {
        buffer.writeMatrix(this->getLocalMatrix());
    }
}

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    uint32_t flags = buffer.read32();

    bool isOpaque = SkToBool(flags & kIsOpaque_Flag);
    SkMatrix localM, *localMPtr = nullptr;
    if (flags & kHasLocalMatrix_Flag) {
        buffer.readMatrix(&localM);
        localMPtr = &localM;
    }

    return sk_sp<SkFlattenable>(new SkRTShader(std::move(sksl), std::move(inputs),
                                               localMPtr, isOpaque));
}

sk_sp<SkShader> SkRuntimeShaderMaker(SkString sksl, sk_sp<SkData> inputs,
                                     const SkMatrix* localMatrix, bool isOpaque) {
    return sk_sp<SkShader>(new SkRTShader(std::move(sksl), std::move(inputs),
                                          localMatrix, isOpaque));
}
