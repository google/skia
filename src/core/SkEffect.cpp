/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlendModePriv.h"
#include "SkEffectPriv.h"
#include "SkMixerBase.h"
#include "SkReadBuffer.h"
#include "SkRasterPipeline.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#endif

bool SkEffect::appendStages(const SkStageRec& rec) const {
    return this->onAppendStages(rec);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkEffect_PMColor final : public SkEffect {
    SkEffect_PMColor(const SkPMColor4f& pm) : fPM(pm) {}

    SkPMColor4f fPM;
    friend class SkEffect;
public:
    SK_FLATTENABLE_HOOKS(SkEffect_PMColor)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writePad32(&fPM, sizeof(SkPMColor4f));
    }

    bool onAppendStages(const SkStageRec& rec) const override {
        rec.fPipeline->append_constant_color(rec.fAlloc, (const float*)&fPM);
        return true;
    }
};

sk_sp<SkFlattenable> SkEffect_PMColor::CreateProc(SkReadBuffer& buffer) {
    SkPMColor4f pm;
    buffer.readPad32(&pm, sizeof(SkPMColor4f));
    return sk_sp<SkFlattenable>(new SkEffect_PMColor(pm));
}

sk_sp<SkEffect> SkEffect::Color(SkColor c) {
    return Color(SkColor4f::FromColor(c));
}
sk_sp<SkEffect> SkEffect::Color(const SkColor4f& c) {
    return sk_sp<SkEffect>(new SkEffect_PMColor(c.premul()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkEffect_LM final : public SkEffect {
    SkEffect_LM(const SkMatrix& lm, sk_sp<SkEffect> e) : fEffect(std::move(e)), fLM(lm) {
        SkASSERT(fEffect);
        SkASSERT(!fLM.isIdentity());
    }

    sk_sp<SkEffect> fEffect;
    SkMatrix        fLM;
    friend class SkEffect;
public:
    SK_FLATTENABLE_HOOKS(SkEffect_LM)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fEffect.get());
        buffer.writeMatrix(fLM);
    }

    bool onAppendStages(const SkStageRec& rec) const override {
        SkMatrix lm = fLM;
        if (rec.fLocalM) {
            lm.preConcat(*rec.fLocalM);
        }
        SkStageRec newRec = rec;
        newRec.fLocalM = &lm;
        return fEffect->appendStages(newRec);
    }
};

sk_sp<SkFlattenable> SkEffect_LM::CreateProc(SkReadBuffer& buffer) {
    auto e = buffer.readEffect();
    SkMatrix lm;
    buffer.readMatrix(&lm);
    return buffer.isValid() ? LocalMatrix(lm, std::move(e)) : nullptr;
}

sk_sp<SkEffect> SkEffect::LocalMatrix(const SkMatrix& lm, sk_sp<SkEffect> e) {
    if (!e || !lm.isFinite()) {
        return nullptr;
    }
    if (lm.isIdentity()) {
        return e;
    }
    return sk_sp<SkEffect>(new SkEffect_LM(lm, std::move(e)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool append_effect_or_paint(const SkStageRec& rec, SkEffect* effect) {
    if (effect) {
        if (!effect->appendStages(rec)) {
            return false;
        }
    } else {
        rec.fPipeline->append_constant_color(rec.fAlloc, rec.fPaint.getColor4f().premul().vec());
    }
    return true;
}

// Returns the output of e0, and leaves the output of e1 in r,g,b,a
static float* append_two_effects(const SkStageRec& rec, SkEffect* e0, SkEffect* e1) {
    struct Storage {
        float   fRes0[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!append_effect_or_paint(rec, e0)) {
        return nullptr;
    }
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRes0);

    if (!append_effect_or_paint(rec, e1)) {
        return nullptr;
    }
    return storage->fRes0;
}

class SkEffect_LerpT final : public SkEffect {
    SkEffect_LerpT(float weight, sk_sp<SkEffect> e0, sk_sp<SkEffect> e1)
        : fE0(std::move(e0))
        , fE1(std::move(e1))
        , fWeight(weight)
    {
        SkASSERT(fWeight >= 0 && fWeight <= 1);
    }

    sk_sp<SkEffect> fE0;
    sk_sp<SkEffect> fE1;
    const float fWeight;
    friend class SkEffect;

public:
    SK_FLATTENABLE_HOOKS(SkEffect_LerpT)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fE0.get());
        buffer.writeFlattenable(fE1.get());
        buffer.writeScalar(fWeight);
    }

    bool onAppendStages(const SkStageRec& rec) const override {
        float* res0 = append_two_effects(rec, fE0.get(), fE1.get());
        if (!res0) {
            return false;
        }

        rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
        rec.fPipeline->append(SkRasterPipeline::lerp_1_float, &fWeight);
        return true;
    }
};

sk_sp<SkFlattenable> SkEffect_LerpT::CreateProc(SkReadBuffer& buffer) {
    auto e0 = buffer.readEffect();
    auto e1 = buffer.readEffect();
    float t = buffer.readScalar();
    return buffer.isValid() ? SkEffect::LerpT(t, std::move(e0), std::move(e1)) : nullptr;
}

sk_sp<SkEffect> SkEffect::LerpT(float t, sk_sp<SkEffect> e0, sk_sp<SkEffect> e1) {
    if (SkScalarIsNaN(t)) {
        return nullptr;
    }
    if (t <= 0) {
        return e0;
    } else if (t >= 1) {
        return e1;
    }
    return sk_sp<SkEffect>(new SkEffect_LerpT(t, std::move(e0), std::move(e1)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkEffect_LerpEffect final : public SkEffect {
    SkEffect_LerpEffect(sk_sp<SkEffect> te, sk_sp<SkEffect> e0, sk_sp<SkEffect> e1)
        : fE0(std::move(e0))
        , fE1(std::move(e1))
        , fTE(std::move(te))
    {
        SkASSERT(fTE);
    }

    sk_sp<SkEffect> fE0;
    sk_sp<SkEffect> fE1;
    sk_sp<SkEffect> fTE;
    friend class SkEffect;

public:
    SK_FLATTENABLE_HOOKS(SkEffect_LerpEffect)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fE0.get());
        buffer.writeFlattenable(fE1.get());
        buffer.writeFlattenable(fTE.get());
    }

    bool onAppendStages(const SkStageRec& rec) const override {
        // TODO: can we tell TEffect that we only need red (for perf)?
        struct Storage {
            float   fResT[4 * SkRasterPipeline_kMaxStride]; // only actually need red-channel
        };
        auto storage = rec.fAlloc->make<Storage>();
        if (!fTE->appendStages(rec)) {
            return false;
        }
        rec.fPipeline->append(SkRasterPipeline::store_src, storage->fResT);

        float* res0 = append_two_effects(rec, fE0.get(), fE1.get());
        if (!res0) {
            return false;
        }

        rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
        rec.fPipeline->append(SkRasterPipeline::lerp_native, &storage->fResT[0]);
        return true;
    }
};

sk_sp<SkFlattenable> SkEffect_LerpEffect::CreateProc(SkReadBuffer& buffer) {
    auto e0 = buffer.readEffect();
    auto e1 = buffer.readEffect();
    auto te = buffer.readEffect();
    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkEffect::LerpEffect(std::move(te), std::move(e0), std::move(e1));
}

sk_sp<SkEffect> SkEffect::LerpEffect(sk_sp<SkEffect> te, sk_sp<SkEffect> e0, sk_sp<SkEffect> e1) {
    if (!te) {
        return nullptr;
    }
    return sk_sp<SkEffect>(new SkEffect_LerpEffect(std::move(te), std::move(e0), std::move(e1)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkEffect_Blend final : public SkEffect {
    SkEffect_Blend(SkBlendMode mode, sk_sp<SkEffect> e0, sk_sp<SkEffect> e1)
        : fE0(std::move(e0))
        , fE1(std::move(e1))
        , fMode(mode)
    {}

    sk_sp<SkEffect> fE0;
    sk_sp<SkEffect> fE1;
    SkBlendMode     fMode;
    friend class SkEffect;

public:
    SK_FLATTENABLE_HOOKS(SkEffect_Blend)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fE0.get());
        buffer.writeFlattenable(fE1.get());
        buffer.write32(static_cast<uint32_t>(fMode));
    }

    bool onAppendStages(const SkStageRec& rec) const override {
        float* res0 = append_two_effects(rec, fE0.get(), fE1.get());
        if (!res0) {
            return false;
        }

        rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
        SkBlendMode_AppendStages(fMode, rec.fPipeline);
        return true;
    }
};

sk_sp<SkFlattenable> SkEffect_Blend::CreateProc(SkReadBuffer& buffer) {
    auto e0 = buffer.readEffect();
    auto e1 = buffer.readEffect();
    unsigned mode = buffer.read32();
    if (!buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
        return nullptr;
    }
    return Blend(static_cast<SkBlendMode>(mode), std::move(e0), std::move(e1));
}

sk_sp<SkEffect> SkEffect::Blend(SkBlendMode mode, sk_sp<SkEffect> e0, sk_sp<SkEffect> e1) {
    switch (mode) {
        case SkBlendMode::kClear: return Color(0);
        case SkBlendMode::kDst:   return e0;
        case SkBlendMode::kSrc:   return e1;
        default: break;
    }
    return sk_sp<SkEffect>(new SkEffect_Blend(mode, std::move(e0), std::move(e1)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkEffect_PMColor);
    SK_REGISTER_FLATTENABLE(SkEffect_LM);
    SK_REGISTER_FLATTENABLE(SkEffect_LerpT);
    SK_REGISTER_FLATTENABLE(SkEffect_LerpEffect);
    SK_REGISTER_FLATTENABLE(SkEffect_Blend);
}
