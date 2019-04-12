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

sk_sp<SkShader> SkShaders::Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    switch (mode) {
        case SkBlendMode::kClear: return Color(0);
        case SkBlendMode::kDst:   return dst;
        case SkBlendMode::kSrc:   return src;
        default: break;
    }
    return sk_sp<SkShader>(new SkShader_Blend(mode, std::move(dst), std::move(src)));
}

sk_sp<SkShader> SkShaders::Lerp(float weight, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (SkScalarIsNaN(weight)) {
        return nullptr;
    }
    if (dst == src) {
        return dst;
    }
    if (weight <= 0) {
        return dst;
    } else if (weight >= 1) {
        return src;
    }
    return sk_sp<SkShader>(new SkShader_Lerp(weight, std::move(dst), std::move(src)));
}

sk_sp<SkShader> SkShaders::Lerp(sk_sp<SkShader> red, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!red) {
        return nullptr;
    }
    if (dst == src) {
        return dst;
    }
    return sk_sp<SkShader>(new SkShader_LerpRed(std::move(red), std::move(dst), std::move(src)));
}

///////////////////////////////////////////////////////////////////////////////

static bool append_shader_or_paint(const SkStageRec& rec, SkShader* shader) {
    if (shader) {
        if (!as_SB(shader)->appendStages(rec)) {
            return false;
        }
    } else {
        rec.fPipeline->append_constant_color(rec.fAlloc, rec.fPaint.getColor4f().premul().vec());
    }
    return true;
}

// Returns the output of e0, and leaves the output of e1 in r,g,b,a
static float* append_two_shaders(const SkStageRec& rec, SkShader* s0, SkShader* s1) {
    struct Storage {
        float   fRes0[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!append_shader_or_paint(rec, s0)) {
        return nullptr;
    }
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRes0);

    if (!append_shader_or_paint(rec, s1)) {
        return nullptr;
    }
    return storage->fRes0;
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkShader_Blend::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    unsigned        mode = buffer.read32();

    // check for valid mode before we cast to the enum type
    if (!buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
        return nullptr;
    }
    return SkShaders::Blend(static_cast<SkBlendMode>(mode), std::move(dst), std::move(src));
}

void SkShader_Blend::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.write32((int)fMode);
}

bool SkShader_Blend::onAppendStages(const SkStageRec& rec) const {
    float* res0 = append_two_shaders(rec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

sk_sp<SkFlattenable> SkShader_Lerp::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    float t = buffer.readScalar();
    return buffer.isValid() ? SkShaders::Lerp(t, std::move(dst), std::move(src)) : nullptr;
}

void SkShader_Lerp::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.writeScalar(fWeight);
}

bool SkShader_Lerp::onAppendStages(const SkStageRec& rec) const {
    float* res0 = append_two_shaders(rec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
    rec.fPipeline->append(SkRasterPipeline::lerp_1_float, &fWeight);
    return true;
}

sk_sp<SkFlattenable> SkShader_LerpRed::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    sk_sp<SkShader> red(buffer.readShader());
    return buffer.isValid() ?
           SkShaders::Lerp(std::move(red), std::move(dst), std::move(src)) : nullptr;
}

void SkShader_LerpRed::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.writeFlattenable(fRed.get());
}

bool SkShader_LerpRed::onAppendStages(const SkStageRec& rec) const {
    struct Storage {
        float   fRed[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();
    if (!as_SB(fRed)->appendStages(rec)) {
        return false;
    }
    // actually, we just need the first (red) channel, but for now we store rgba
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRed);

    float* res0 = append_two_shaders(rec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
    rec.fPipeline->append(SkRasterPipeline::lerp_native, &storage->fRed[0]);
    return true;
}

#if SK_SUPPORT_GPU

#include "effects/generated/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "GrRecordingContext.h"
#include "effects/generated/GrComposeLerpEffect.h"
#include "effects/generated/GrComposeLerpRedEffect.h"

static std::unique_ptr<GrFragmentProcessor> as_fp(const GrFPArgs& args, SkShader* shader) {
    return shader ? as_SB(shader)->asFragmentProcessor(args) : nullptr;
}

std::unique_ptr<GrFragmentProcessor> SkShader_Blend::asFragmentProcessor(const GrFPArgs& args) const {
    auto fpA = as_fp(args, fDst.get());
    auto fpB = as_fp(args, fSrc.get());
    if (!fpA || !fpB) {
        return nullptr;
    }
    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                              std::move(fpA), fMode);
}

std::unique_ptr<GrFragmentProcessor> SkShader_Lerp::asFragmentProcessor(const GrFPArgs& args) const {
    auto fpA = as_fp(args, fDst.get());
    auto fpB = as_fp(args, fSrc.get());
    return GrComposeLerpEffect::Make(std::move(fpA), std::move(fpB), fWeight);
}

std::unique_ptr<GrFragmentProcessor> SkShader_LerpRed::asFragmentProcessor(const GrFPArgs& args) const {
    auto fpA = as_fp(args, fDst.get());
    auto fpB = as_fp(args, fSrc.get());
    auto red = as_SB(fRed)->asFragmentProcessor(args);
    if (!red) {
        return nullptr;
    }
    return GrComposeLerpRedEffect::Make(std::move(fpA), std::move(fpB), std::move(red));
}
#endif
