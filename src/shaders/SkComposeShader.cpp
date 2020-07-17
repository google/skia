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
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkComposeShader.h"

namespace {

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
    if (dst == src || weight <= 0) {
        return dst;
    }
    if (weight >= 1) {
        return src;
    }
    return sk_sp<SkShader>(new SkShader_Lerp(weight, std::move(dst), std::move(src)));
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

static skvm::Color program_or_paint(const sk_sp<SkShader>& sh, skvm::Builder* p,
                                    skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                    const SkMatrixProvider& mats, const SkMatrix* localM,
                                    SkFilterQuality q, const SkColorInfo& dst,
                                    skvm::Uniforms* uniforms, SkArenaAlloc* alloc) {
    return sh ? as_SB(sh)->program(p, device,local, paint, mats,localM, q,dst, uniforms,alloc)
              : p->premul(paint);
}

skvm::Color SkShader_Blend::onProgram(skvm::Builder* p,
                                      skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                      const SkMatrixProvider& mats, const SkMatrix* localM,
                                      SkFilterQuality q, const SkColorInfo& dst,
                                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Color d,s;
    if ((d = program_or_paint(fDst, p, device,local, paint, mats,localM, q,dst, uniforms,alloc)) &&
        (s = program_or_paint(fSrc, p, device,local, paint, mats,localM, q,dst, uniforms,alloc)))
    {
        return p->blend(fMode, s,d);
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

skvm::Color SkShader_Lerp::onProgram(skvm::Builder* p,
                                     skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                     const SkMatrixProvider& mats, const SkMatrix* localM,
                                     SkFilterQuality q, const SkColorInfo& dst,
                                     skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Color d,s;
    if ((d = program_or_paint(fDst, p, device,local, paint, mats,localM, q,dst, uniforms,alloc)) &&
        (s = program_or_paint(fSrc, p, device,local, paint, mats,localM, q,dst, uniforms,alloc)))
    {
        auto t = p->uniformF(uniforms->pushF(fWeight));
        return {
            p->lerp(d.r, s.r, t),
            p->lerp(d.g, s.g, t),
            p->lerp(d.b, s.b, t),
            p->lerp(d.a, s.a, t),
        };
    }
    return {};
}

sk_sp<SkShader> SkShaders::Multisample(sk_sp<SkShader> child,
                                       const SkPoint samples[], int nsamples,
                                       const SkScalar weights[]) {
    return child ? sk_make_sp<SkShader_Multisample>(std::move(child), samples,nsamples, weights)
                 : nullptr;
}

SkShader_Multisample::SkShader_Multisample(sk_sp<SkShader> child,
                                           const SkPoint samples[], int nsamples,
                                           const SkScalar weights[])
    : fChild  (std::move(child))
    , fSamples(samples, nsamples)
{
    fWeights.reset(nsamples);
    for (SkScalar& weight : fWeights) {
        weight = weights ? *weights++
                         : 1.0f/nsamples;
    }
}


void SkShader_Multisample::flatten(SkWriteBuffer& b) const {
    b.writeFlattenable(fChild.get());
    b.writePointArray (fSamples.data(), fSamples.count());
    b.writeScalarArray(fWeights.data(), fSamples.count());
}

sk_sp<SkFlattenable> SkShader_Multisample::CreateProc(SkReadBuffer& b) {
    sk_sp<SkShader> child = b.readShader();
    std::vector<SkPoint>  samples(b.getArrayCount());
    std::vector<SkScalar> weights(samples.size());
    if (child
            && b.readPointArray (samples.data(), samples.size())
            && b.readScalarArray(weights.data(), weights.size())) {
        return SkShaders::Multisample(child, samples.data(), samples.size(), weights.data());
    }
    return nullptr;
}

skvm::Color SkShader_Multisample::onProgram(
        skvm::Builder* p, skvm::Coord device, skvm::Coord local, skvm::Color paint,
        const SkMatrixProvider& matrices, const SkMatrix* localM,
        SkFilterQuality quality, const SkColorInfo& dst,
        skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {

    skvm::Color accum = {
        p->splat(0.0f),
        p->splat(0.0f),
        p->splat(0.0f),
        p->splat(0.0f),
    };

    // We can save a bunch of math if all the weights are the same.
    bool weights_vary = false;
    for (SkScalar w : fWeights) {
        if (w != fWeights[0]) {
            weights_vary = true;
            break;
        }
    }

    // Push up all our uniforms now.
    std::vector<skvm::Coord> samples(fSamples.count());
    std::vector<skvm::F32>   weights(fSamples.count());
    for (int i = 0; i < fSamples.count(); i++) {
        samples[i] = {p->uniformF(uniforms->pushF(fSamples[i].x())),
                      p->uniformF(uniforms->pushF(fSamples[i].y()))};
        if (i == 0 || weights_vary) {
            weights[i] = p->uniformF(uniforms->pushF(fWeights[i]));
        }
    }

    // Remember where our uniforms ended, so we can reset there between child->program() calls,
    // strongly assuming each call to program() will generate identical uniforms.
    // (Only a varying `skvm::Coord local` is changing between calls, so this _should_ work.)
    size_t saved = uniforms->buf.size();
    for (int i = 0; i < fSamples.count(); i++) {
        uniforms->buf.resize(saved);

        skvm::Coord s = samples[i];
        skvm::Color c = as_SB(fChild)->program(p, device,{local.x+s.x,local.y+s.y}, paint,
                                               matrices,localM, quality,dst, uniforms,alloc);
        if (!c) {
            return {};
        }

        if (weights_vary) {
            accum.r = accum.r + c.r*weights[i];
            accum.g = accum.g + c.g*weights[i];
            accum.b = accum.b + c.b*weights[i];
            accum.a = accum.a + c.a*weights[i];
        } else {
            accum.r = accum.r + c.r;
            accum.g = accum.g + c.g;
            accum.b = accum.b + c.b;
            accum.a = accum.a + c.a;
        }
    }
    if (!weights_vary) {
        accum.r = accum.r * weights[0];
        accum.g = accum.g * weights[0];
        accum.b = accum.b * weights[0];
        accum.a = accum.a * weights[0];
    }
    return accum;
}

#if SK_SUPPORT_GPU

#include "include/gpu/GrRecordingContext.h"
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
    return GrXfermodeFragmentProcessor::Make(std::move(fpB), std::move(fpA), fMode);
}

std::unique_ptr<GrFragmentProcessor> SkShader_Lerp::asFragmentProcessor(
        const GrFPArgs& orig_args) const {
    const GrFPArgs::WithPreLocalMatrix args(orig_args, this->getLocalMatrix());
    auto fpA = as_fp(args, fDst.get());
    auto fpB = as_fp(args, fSrc.get());
    return GrComposeLerpEffect::Make(std::move(fpA), std::move(fpB), fWeight);
}

// TODO: SkShader_Multisample

#endif
