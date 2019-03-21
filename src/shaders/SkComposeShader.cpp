/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlendModePriv.h"
#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkColorData.h"
#include "SkColorShader.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

sk_sp<SkShader> SkShader::MakeCompose(sk_sp<SkShader> dst, sk_sp<SkShader> src, SkBlendMode mode,
                                      float lerpT) {
    if (!src || !dst || SkScalarIsNaN(lerpT)) {
        return nullptr;
    }
    lerpT = SkScalarPin(lerpT, 0, 1);

    if (lerpT == 0) {
        return dst;
    } else if (lerpT == 1) {
        if (mode == SkBlendMode::kSrc) {
            return src;
        }
        if (mode == SkBlendMode::kDst) {
            return dst;
        }
    }
    return sk_sp<SkShader>(new SkComposeShader(std::move(dst), std::move(src), mode, lerpT));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    unsigned        mode = buffer.read32();
    float           lerp = buffer.readScalar();

    // check for valid mode before we cast to the enum type
    if (!buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
        return nullptr;
    }
    return MakeCompose(std::move(dst), std::move(src), static_cast<SkBlendMode>(mode), lerp);
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.write32((int)fMode);
    buffer.writeScalar(fLerpT);
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (!this->isJustMode()) {
        return false;
    }

    if (rec) {
        rec->fShaderA   = fDst.get();
        rec->fShaderB   = fSrc.get();
        rec->fBlendMode = fMode;
    }
    return true;
}
#endif

bool SkComposeShader::onAppendStages(const SkStageRec& rec) const {
    struct Storage {
        float   fRGBA[4 * SkRasterPipeline_kMaxStride];
        float   fAlpha;
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!as_SB(fDst)->appendStages(rec)) {
        return false;
    }
    // This outputs r,g,b,a, which we'll need later when we apply the mode, so we save it off now
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRGBA);

    if (!as_SB(fSrc)->appendStages(rec)) {
        return false;
    }
    // r,g,b,a now have the right input for the next step (lerp and/or mode), but we need to
    // reload dr,dg,db,da from memory, since we stashed that from our fDst invocation earlier.
    rec.fPipeline->append(SkRasterPipeline::load_dst, storage->fRGBA);

    if (!this->isJustLerp()) {
        SkBlendMode_AppendStages(fMode, rec.fPipeline);
    }
    if (!this->isJustMode()) {
        rec.fPipeline->append(SkRasterPipeline::lerp_1_float, &fLerpT);
    }
    return true;
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> SkComposeShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    if (this->isJustMode()) {
        SkASSERT(fMode != SkBlendMode::kSrc && fMode != SkBlendMode::kDst); // caught in factory
        if (fMode == SkBlendMode::kClear) {
            return GrConstColorProcessor::Make(SK_PMColor4fTRANSPARENT,
                                               GrConstColorProcessor::InputMode::kIgnore);
        }
    }

    std::unique_ptr<GrFragmentProcessor> fpA(as_SB(fDst)->asFragmentProcessor(args));
    if (!fpA) {
        return nullptr;
    }
    std::unique_ptr<GrFragmentProcessor> fpB(as_SB(fSrc)->asFragmentProcessor(args));
    if (!fpB) {
        return nullptr;
    }
    // TODO: account for fLerpT when it is < 1
    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                              std::move(fpA), fMode);
}
#endif
