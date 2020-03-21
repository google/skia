/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkComposeShader.h"

namespace {

sk_sp<SkShader> wrap_lm(sk_sp<SkShader> shader, const SkMatrix* lm) {
    return (shader && lm) ? shader->makeWithLocalMatrix(*lm) : shader;
}

struct LocalMatrixStageRec final : public SkStageRec {
    LocalMatrixStageRec(const SkStageRec& rec, const SkMatrix& lm)
        : INHERITED(rec) {
        if (!lm.isIdentity()) {
            if (fLocalM) {
                fStorage.setConcat(lm, *fLocalM);
                fLocalM = fStorage.isIdentity() ? nullptr : &fStorage;
            } else {
                fLocalM = &lm;
            }
        }
    }

private:
    SkMatrix fStorage;

    using INHERITED = SkStageRec;
};

} // namespace

sk_sp<SkShader> SkShaders::Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                 const SkMatrix* lm) {
    switch (mode) {
        case SkBlendMode::kClear: return Color(0);
        case SkBlendMode::kDst:   return wrap_lm(std::move(dst), lm);
        case SkBlendMode::kSrc:   return wrap_lm(std::move(src), lm);
        default: break;
    }
    return sk_sp<SkShader>(new SkShader_Blend(mode, std::move(dst), std::move(src), lm));
}

sk_sp<SkShader> SkShaders::Lerp(float weight, sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                const SkMatrix* lm) {
    if (SkScalarIsNaN(weight)) {
        return nullptr;
    }
    if (dst == src || weight <= 0) {
        return wrap_lm(std::move(dst), lm);
    }
    if (weight >= 1) {
        return wrap_lm(std::move(src), lm);
    }
    return sk_sp<SkShader>(new SkShader_Lerp(weight, std::move(dst), std::move(src), lm));
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

bool SkShader_Blend::onAppendStages(const SkStageRec& orig_rec) const {
    const LocalMatrixStageRec rec(orig_rec, this->getLocalMatrix());

    float* res0 = append_two_shaders(rec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

skvm::Color SkShader_Blend::onProgram(skvm::Builder* p, skvm::F32 x, skvm::F32 y,
                                      const SkMatrix& ctm, const SkMatrix* localM,
                                      SkFilterQuality q, SkColorSpace* cs,
                                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Color dst, src;
    if ((dst = as_SB(fDst)->program(p, x,y, ctm,localM, q, cs, uniforms, alloc)) &&
        (src = as_SB(fSrc)->program(p, x,y, ctm,localM, q, cs, uniforms, alloc)))
    {
        return p->blend(fMode, src, dst);
    }
    return {};
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

bool SkShader_Lerp::onAppendStages(const SkStageRec& orig_rec) const {
    const LocalMatrixStageRec rec(orig_rec, this->getLocalMatrix());

    float* res0 = append_two_shaders(rec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
    rec.fPipeline->append(SkRasterPipeline::lerp_1_float, &fWeight);
    return true;
}

skvm::Color SkShader_Lerp::onProgram(skvm::Builder* p, skvm::F32 x, skvm::F32 y,
                                     const SkMatrix& ctm, const SkMatrix* localM,
                                     SkFilterQuality q, SkColorSpace* cs,
                                     skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Color dst, src;
    if ((dst = as_SB(fDst)->program(p, x,y, ctm,localM, q, cs, uniforms, alloc)) &&
        (src = as_SB(fSrc)->program(p, x,y, ctm,localM, q, cs, uniforms, alloc)))
    {
        auto t = p->uniformF(uniforms->pushF(fWeight));
        return {
            p->lerp(dst.r, src.r, t),
            p->lerp(dst.g, src.g, t),
            p->lerp(dst.b, src.b, t),
            p->lerp(dst.a, src.a, t),
        };
    }
    return {};
}

#if SK_SUPPORT_GPU

#include "include/private/GrRecordingContext.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/gpu/effects/generated/GrComposeLerpEffect.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

static std::unique_ptr<GrFragmentProcessor> as_fp(const GrFPArgs& args, SkShader* shader) {
    return shader ? as_SB(shader)->asFragmentProcessor(args) : nullptr;
}

std::unique_ptr<GrFragmentProcessor> SkShader_Blend::asFragmentProcessor(
        const GrFPArgs& orig_args) const {
    const GrFPArgs::WithPreLocalMatrix args(orig_args, this->getLocalMatrix());
    auto fpA = as_fp(args, fDst.get());
    auto fpB = as_fp(args, fSrc.get());
    if (!fpA) {
        return GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(fpB), fMode);
    }
    if (!fpB) {
        return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(fpA), fMode);
    }
    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                              std::move(fpA), fMode);
}

std::unique_ptr<GrFragmentProcessor> SkShader_Lerp::asFragmentProcessor(
        const GrFPArgs& orig_args) const {
    const GrFPArgs::WithPreLocalMatrix args(orig_args, this->getLocalMatrix());
    auto fpA = as_fp(args, fDst.get());
    auto fpB = as_fp(args, fSrc.get());
    return GrComposeLerpEffect::Make(std::move(fpA), std::move(fpB), fWeight);
}
#endif
