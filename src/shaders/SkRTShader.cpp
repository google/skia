/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"
#endif

#include "include/private/GrRecordingContext.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/sksl/SkSLByteCode.h"

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(SkString sksl, sk_sp<SkData> inputs);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    SkRTShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkRTShader)

    SkString fSkSL;
    sk_sp<SkData> fInputs;

//    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;

    typedef SkShaderBase INHERITED;
};

SkRTShader::SkRTShader(SkString sksl, sk_sp<SkData> inputs)
    : fSkSL(std::move(sksl))
    , fInputs(std::move(inputs))
{}

bool SkRTShader::onAppendStages(const SkStageRec& rec) const {
    struct InterpreterCtx : public SkRasterPipeline_CallbackCtx {
        SkSL::ByteCode* byteCode;
        SkSL::ByteCodeFunction* main;
        const void* inputs;
        int ninputs;
    };
    auto ctx = rec.fAlloc->make<InterpreterCtx>();
    ctx->inputs = fInputs->data();
    ctx->ninputs = fInputs->size() / 4;

 //   SkAutoMutexExclusive ama(fByteCodeMutex);
    if (!fByteCode) {
        SkSL::Compiler c;
        auto prog = c.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                     SkSL::String(fSkSL.c_str()),
                                     SkSL::Program::Settings());
        if (c.errorCount()) {
            SkDebugf("%s\n", c.errorText().c_str());
            return false;
        }
        fByteCode = c.toByteCode(*prog);
    }
    ctx->byteCode = fByteCode.get();
    ctx->main = ctx->byteCode->fFunctions[0].get();
    ctx->fn = [](SkRasterPipeline_CallbackCtx* arg, int active_pixels) {
        auto ctx = (InterpreterCtx*)arg;
        ctx->byteCode->run(ctx->main, ctx->rgba, nullptr, active_pixels,
                           (float*)ctx->inputs, ctx->ninputs);
    };
    rec.fPipeline->append(SkRasterPipeline::callback, ctx);
    return true;
}

void SkRTShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeString(fSkSL.c_str());
    if (fInputs) {
        buffer.writeDataAsByteArray(fInputs.get());
    } else {
        buffer.writeByteArray(nullptr, 0);
    }
}

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    return sk_sp<SkFlattenable>(new SkRTShader(std::move(sksl), std::move(inputs)));
}

sk_sp<SkShader> SkRuntimeShaderMaker(SkString sksl, sk_sp<SkData> inputs) {
    return sk_sp<SkShader>(new SkRTShader(std::move(sksl), std::move(inputs)));
}
