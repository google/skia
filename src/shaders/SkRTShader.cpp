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

#if SK_SUPPORT_GPU
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/effects/GrSkSLFP.h"
#endif

SkRTShader::SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs,
                       const SkMatrix* localMatrix, sk_sp<SkShader>* children, size_t childCount,
                       bool isOpaque)
        : SkShaderBase(localMatrix)
        , fEffect(std::move(effect))
        , fIsOpaque(isOpaque)
        , fInputs(std::move(inputs))
        , fChildren(children, children + childCount) {
}

SkRTShader::~SkRTShader() = default;

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
        auto [byteCode, errorText] = fEffect->toByteCode();
        if (!byteCode) {
            SkDebugf("%s\n", errorText.c_str());
            return false;
        }
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
    buffer.write32(fChildren.size());
    for (const auto& child : fChildren) {
        buffer.writeFlattenable(child.get());
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

    std::vector<sk_sp<SkShader>> children;
    children.resize(buffer.read32());
    for (size_t i = 0; i < children.size(); ++i) {
        children[i] = buffer.readShader();
    }

    // We don't have a way to ensure that indices are consistent and correct when deserializing.
    // Perhaps we should have a hash table to map strings to indices? For now, all shaders get a
    // new unique ID after serialization.
    auto effect = std::get<0>(SkRuntimeEffect::Make(std::move(sksl)));
    return sk_sp<SkFlattenable>(new SkRTShader(std::move(effect), std::move(inputs), localMPtr,
                                               children.data(), children.size(), isOpaque));
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkRTShader::asFragmentProcessor(const GrFPArgs& args) const {
    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    auto fp = GrSkSLFP::Make(args.fContext, fEffect, "runtime-shader",
                             fInputs->data(), fInputs->size(), &matrix);
    for (const auto& child : fChildren) {
        auto childFP = child ? as_SB(child)->asFragmentProcessor(args) : nullptr;
        if (!childFP) {
            // TODO: This is the case that should eventually mean "the original input color"
            return nullptr;
        }
        fp->addChild(std::move(childFP));
    }
    return fp;
}
#endif

SkRuntimeShaderFactory::SkRuntimeShaderFactory(SkString sksl, bool isOpaque)
    : fEffect(std::get<0>(SkRuntimeEffect::Make(std::move(sksl))))
    , fIsOpaque(isOpaque) {}

SkRuntimeShaderFactory::SkRuntimeShaderFactory(const SkRuntimeShaderFactory&) = default;
SkRuntimeShaderFactory::SkRuntimeShaderFactory(SkRuntimeShaderFactory&&) = default;

SkRuntimeShaderFactory::~SkRuntimeShaderFactory() = default;

SkRuntimeShaderFactory& SkRuntimeShaderFactory::operator=(const SkRuntimeShaderFactory&) = default;
SkRuntimeShaderFactory& SkRuntimeShaderFactory::operator=(SkRuntimeShaderFactory&&) = default;

sk_sp<SkShader> SkRuntimeShaderFactory::make(sk_sp<SkData> inputs, const SkMatrix* localMatrix,
                                             sk_sp<SkShader>* children, size_t childCount) {
    return fEffect
        && inputs->size() >= fEffect->inputSize()
        && childCount >= fEffect->childCount()
        ? sk_sp<SkShader>(new SkRTShader(fEffect, std::move(inputs), localMatrix,
                                         children, childCount, fIsOpaque))
        : nullptr;
}
