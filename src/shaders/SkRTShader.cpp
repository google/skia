/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffect.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkRTShader.h"

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"

#include "src/gpu/GrFPArgs.h"
#include "src/gpu/effects/GrSkSLFP.h"
#endif

SkRTShader::SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs,
                       const SkMatrix* localMatrix, bool isOpaque)
        : SkShaderBase(localMatrix)
        , fEffect(std::move(effect))
        , fIsOpaque(isOpaque)
        , fInputs(std::move(inputs)) {
}

bool SkRTShader::onAppendStages(const SkStageRec& rec) const {
    SkMatrix inverse;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &inverse)) {
        return false;
    }

    auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
    ctx->paintColor = rec.fPaint.getColor4f();
    ctx->inputs = fInputs->data();
    ctx->ninputs = fInputs->size() / 4;
    ctx->shaderConvention = true;

    SkAutoMutexExclusive ama(fByteCodeMutex);
    if (!fByteCode) {
        auto [byteCode, errorCount, errorText] = fEffect->toByteCode();
        if (errorCount) {
            SkDebugf("%s\n", errorText.c_str());
            return false;
        }
        SkASSERT(byteCode);
        fByteCode = std::move(byteCode);
    }
    ctx->byteCode = fByteCode.get();
    ctx->fn = ctx->byteCode->getFunction("main");

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

    buffer.writeString(fEffect->source().c_str());
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

    // We don't have a way to ensure that indices are consistent and correct when deserializing.
    // Perhaps we should have a hash table to map strings to indices? For now, all shaders get a
    // new unique ID after serialization.
    return sk_sp<SkFlattenable>(new SkRTShader(SkRuntimeEffect::Make(std::move(sksl)),
                                               std::move(inputs), localMPtr, isOpaque));
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkRTShader::asFragmentProcessor(const GrFPArgs& args) const {
    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    return GrSkSLFP::Make(args.fContext, fEffect, "runtime-shader",
                          fInputs->data(), fInputs->size(), &matrix);
}
#endif

SkRuntimeShaderFactory::SkRuntimeShaderFactory(SkString sksl, bool isOpaque)
    : fEffect(SkRuntimeEffect::Make(std::move(sksl)))
    , fIsOpaque(isOpaque) {}

sk_sp<SkShader> SkRuntimeShaderFactory::make(sk_sp<SkData> inputs, const SkMatrix* localMatrix) {
    return fEffect && fEffect->isValid()
        ? sk_sp<SkShader>(new SkRTShader(fEffect, std::move(inputs), localMatrix, fIsOpaque))
        : nullptr;
}
