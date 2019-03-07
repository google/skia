/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMixerBase.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Blend final : public SkMixerBase {
    const SkBlendMode fMode;
public:
    sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        unsigned mode = buffer.read32();
        if (!buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
            return nullptr;
        }
        return MakeBlend(static_cast<SkBlendMode>(mode));
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.write32(static_cast<int>(fMode));
    }

    bool onAppendStages(const StageRec& rec) const {
        SkBlendMode_AppendStages(fMode, rec.fPipeline);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Lerp final : public SkMixerBbase {
    const float fWeight;
public:
    sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        return MakeLerp(buffer.readScalar());
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeScalar(fWeight);
    }

    bool onAppendStages(const StageRec& rec) const {
        rec.fPipeline->append(SkRasterPipeline::lerp_1_float, &fLerpT);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkMixer> SkMixer::MakeFirst() {
    return MakeBlend(SkBlendMode::kSrc);
}

sk_sp<SkMixer> SkMixer::MakeSecond() {
    return MakeBlend(SkBlendMode::kDst);
}

sk_sp<SkMixer> SkMixer::MakeReverse(sk_sp<SkMixer> proxy) {
    return sk_sp<SkMixer>(new SkMixer_Reverse(std::move(proxy)));
}

sk_sp<SkMixer> SkMixer::MakeBlend(SkBlendMode mode) {
    return sk_sp<SkMixer>(new SkMixer_Blend(mode));
}

sk_sp<SkMixer> SkMixer::MakeLerp(float t) {
    if (SkScalarIsNaN(t)) {
        t = 0;  // is some other value better? return null?
    }
    if (t <= 0) {
        return MakeFirst();
    }
    if (t >= 1) {
        return MakeSecond();
    }
    return sk_sp<SkMixer>(new SkMixer_Lerp(t));
}

sk_sp<SkMixer> SkMixer::MakeArithmetic(float k1, float k2, float k3, float k4) {
    return nullptr; // TODO
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkColorFilter::RegisterFlattenables() {
//    SK_REGISTER_FLATTENABLE(SkMixer_Reverse);
    SK_REGISTER_FLATTENABLE(SkMixer_Blend);
    SK_REGISTER_FLATTENABLE(SkMixer_Lerp);
}
