/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkPerlinNoiseShaderImpl.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkPerlinNoiseShaderType.h"

#include <optional>

SkPerlinNoiseShader::SkPerlinNoiseShader(SkPerlinNoiseShaderType type,
                                         SkScalar baseFrequencyX,
                                         SkScalar baseFrequencyY,
                                         int numOctaves,
                                         SkScalar seed,
                                         const SkISize* tileSize)
        : fType(type)
        , fBaseFrequencyX(baseFrequencyX)
        , fBaseFrequencyY(baseFrequencyY)
        , fNumOctaves(numOctaves > kMaxOctaves ? kMaxOctaves
                                               : numOctaves)  // [0,255] octaves allowed
        , fSeed(seed)
        , fTileSize(nullptr == tileSize ? SkISize::Make(0, 0) : *tileSize)
        , fStitchTiles(!fTileSize.isEmpty()) {
    SkASSERT(numOctaves >= 0 && numOctaves <= kMaxOctaves);
    SkASSERT(fBaseFrequencyX >= 0);
    SkASSERT(fBaseFrequencyY >= 0);

    // If kBlockSize changes then it must be changed in the SkSL noise_function
    // implementation and the graphite backend
    static_assert(SkPerlinNoiseShader::kBlockSize == 256);
}

sk_sp<SkFlattenable> SkPerlinNoiseShader::CreateProc(SkReadBuffer& buffer) {
    SkPerlinNoiseShaderType type = buffer.read32LE(SkPerlinNoiseShaderType::kLast);

    SkScalar freqX = buffer.readScalar();
    SkScalar freqY = buffer.readScalar();
    int octaves = buffer.read32LE<int>(kMaxOctaves);

    SkScalar seed = buffer.readScalar();
    SkISize tileSize;
    tileSize.fWidth = buffer.readInt();
    tileSize.fHeight = buffer.readInt();

    switch (type) {
        case SkPerlinNoiseShaderType::kFractalNoise:
            return SkShaders::MakeFractalNoise(freqX, freqY, octaves, seed, &tileSize);
        case SkPerlinNoiseShaderType::kTurbulence:
            return SkShaders::MakeTurbulence(freqX, freqY, octaves, seed, &tileSize);
        default:
            // Really shouldn't get here b.c. of earlier check on type
            buffer.validate(false);
            return nullptr;
    }
}

void SkPerlinNoiseShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int)fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

bool SkPerlinNoiseShader::appendStages(const SkStageRec& rec,
                                       const SkShaders::MatrixRec& mRec) const {
    std::optional<SkShaders::MatrixRec> newMRec = mRec.apply(rec);
    if (!newMRec.has_value()) {
        return false;
    }

    fInitPaintingDataOnce([&] {
        const_cast<SkPerlinNoiseShader*>(this)->fPaintingData = this->getPaintingData();
    });

    auto* ctx = rec.fAlloc->make<SkRasterPipeline_PerlinNoiseCtx>();
    ctx->noiseType = fType;
    ctx->baseFrequencyX = fPaintingData->fBaseFrequency.fX;
    ctx->baseFrequencyY = fPaintingData->fBaseFrequency.fY;
    ctx->stitchDataInX = fPaintingData->fStitchDataInit.fWidth;
    ctx->stitchDataInY = fPaintingData->fStitchDataInit.fHeight;
    ctx->stitching = fStitchTiles;
    ctx->numOctaves = fNumOctaves;
    ctx->latticeSelector = fPaintingData->fLatticeSelector;
    ctx->noiseData = &fPaintingData->fNoise[0][0][0];

    rec.fPipeline->append(SkRasterPipelineOp::perlin_noise, ctx);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool valid_input(
        SkScalar baseX, SkScalar baseY, int numOctaves, const SkISize* tileSize, SkScalar seed) {
    if (!(baseX >= 0 && baseY >= 0)) {
        return false;
    }
    if (!(numOctaves >= 0 && numOctaves <= SkPerlinNoiseShader::kMaxOctaves)) {
        return false;
    }
    if (tileSize && !(tileSize->width() >= 0 && tileSize->height() >= 0)) {
        return false;
    }
    if (!SkIsFinite(seed)) {
        return false;
    }
    return true;
}

void SkRegisterPerlinNoiseShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkPerlinNoiseShader);
    // Previous name
    SkFlattenable::Register("SkPerlinNoiseShaderImpl", SkPerlinNoiseShader::CreateProc);
}

namespace SkShaders {
sk_sp<SkShader> MakeFractalNoise(SkScalar baseFrequencyX,
                                 SkScalar baseFrequencyY,
                                 int numOctaves,
                                 SkScalar seed,
                                 const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }

    if (0 == numOctaves) {
        // For kFractalNoise, w/o any octaves, the entire shader collapses to:
        //    [0,0,0,0] * 0.5 + 0.5
        constexpr SkColor4f kTransparentGray = {0.5f, 0.5f, 0.5f, 0.5f};

        return SkShaders::Color(kTransparentGray, /* colorSpace= */ nullptr);
    }

    return sk_sp<SkShader>(new SkPerlinNoiseShader(SkPerlinNoiseShaderType::kFractalNoise,
                                                   baseFrequencyX,
                                                   baseFrequencyY,
                                                   numOctaves,
                                                   seed,
                                                   tileSize));
}

sk_sp<SkShader> MakeTurbulence(SkScalar baseFrequencyX,
                               SkScalar baseFrequencyY,
                               int numOctaves,
                               SkScalar seed,
                               const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }

    if (0 == numOctaves) {
        // For kTurbulence, w/o any octaves, the entire shader collapses to: [0,0,0,0]
        return SkShaders::Color(SkColors::kTransparent, /* colorSpace= */ nullptr);
    }

    return sk_sp<SkShader>(new SkPerlinNoiseShader(SkPerlinNoiseShaderType::kTurbulence,
                                                   baseFrequencyX,
                                                   baseFrequencyY,
                                                   numOctaves,
                                                   seed,
                                                   tileSize));
}

}  // namespace SkShaders
