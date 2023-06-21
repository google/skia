/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/shaders/SkLocalMatrixShader.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

class SkImage;
enum class SkTileMode;
struct SkStageRec;

SkShaderBase::GradientType SkLocalMatrixShader::asGradient(GradientInfo* info,
                                                           SkMatrix* localMatrix) const {
    GradientType type = as_SB(fWrappedShader)->asGradient(info, localMatrix);
    if (type != SkShaderBase::GradientType::kNone && localMatrix) {
        *localMatrix = ConcatLocalMatrices(fLocalMatrix, *localMatrix);
    }
    return type;
}

#if defined(SK_GRAPHITE)
void SkLocalMatrixShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                   skgpu::graphite::PaintParamsKeyBuilder* builder,
                                   skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    LocalMatrixShaderBlock::LMShaderData lmShaderData(fLocalMatrix);

    KeyContextWithLocalMatrix newContext(keyContext, fLocalMatrix);

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, &lmShaderData);

    as_SB(fWrappedShader)->addToKey(newContext, builder, gatherer);

    builder->endBlock();
}
#endif

sk_sp<SkFlattenable> SkLocalMatrixShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    auto baseShader(buffer.readShader());
    if (!baseShader) {
        return nullptr;
    }
    return baseShader->makeWithLocalMatrix(lm);
}

void SkLocalMatrixShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(fLocalMatrix);
    buffer.writeFlattenable(fWrappedShader.get());
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkLocalMatrixShader::onMakeContext(const ContextRec& rec,
                                                          SkArenaAlloc* alloc) const {
    return as_SB(fWrappedShader)->makeContext(ContextRec::Concat(rec, fLocalMatrix), alloc);
}
#endif

SkImage* SkLocalMatrixShader::onIsAImage(SkMatrix* outMatrix, SkTileMode* mode) const {
    SkMatrix imageMatrix;
    SkImage* image = fWrappedShader->isAImage(&imageMatrix, mode);
    if (image && outMatrix) {
        *outMatrix = ConcatLocalMatrices(fLocalMatrix, imageMatrix);
    }

    return image;
}

bool SkLocalMatrixShader::appendStages(const SkStageRec& rec,
                                       const SkShaders::MatrixRec& mRec) const {
    return as_SB(fWrappedShader)->appendStages(rec, mRec.concat(fLocalMatrix));
}

////////////////////////////////////////////////////////////////////

SkCTMShader::SkCTMShader(sk_sp<SkShader> proxy, const SkMatrix& ctm)
        : fProxyShader(std::move(proxy)), fCTM(ctm) {}

SkShaderBase::GradientType SkCTMShader::asGradient(GradientInfo* info,
                                                   SkMatrix* localMatrix) const {
    return as_SB(fProxyShader)->asGradient(info, localMatrix);
}

bool SkCTMShader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const {
    return as_SB(fProxyShader)->appendRootStages(rec, fCTM);
}

sk_sp<SkFlattenable> SkCTMShader::CreateProc(SkReadBuffer& buffer) {
    SkASSERT(false);
    return nullptr;
}
