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
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkRTShader.h"

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"

static inline uint32_t new_sksl_unique_id() {
    return GrSkSLFP::NewIndex();
}
#else
static inline uint32_t new_sksl_unique_id() {
    return 0;   // not used w/o GPU
}
#endif

SkRTShader::SkRTShader(SkString sksl, sk_sp<SkData> inputs, const SkMatrix* localMatrix,
                       bool isOpaque)
    : SkShaderBase(localMatrix)
    , fSkSL(std::move(sksl))
    , fInputs(std::move(inputs))
    , fUniqueID(new_sksl_unique_id())
    , fIsOpaque(isOpaque)
{}

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
        if (!fByteCode->getFunction("main")) {
            return false;
        }
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

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkRTShader::asFragmentProcessor(const GrFPArgs& args) const {
    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    return GrSkSLFP::Make(args.fContext, fUniqueID, "runtime-shader", fSkSL,
                          fInputs->data(), fInputs->size(), SkSL::Program::kPipelineStage_Kind,
                          &matrix);
}
#endif
