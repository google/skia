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

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Const final : public SkMixerBase {
    SkMixer_Const(const SkPMColor4f& pm) : fPM(pm) {}
    const SkPMColor4f fPM;
    friend class SkMixer;
public:
    SK_FLATTENABLE_HOOKS(SkMixer_Const)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writePad32(&fPM, sizeof(SkPMColor4f));
    }

    bool appendStages(const SkStageRec& rec) const override {
        rec.fPipeline->append_constant_color(rec.fAlloc, (const float*)&fPM);
        return true;
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

sk_sp<SkFlattenable> SkMixer_Const::CreateProc(SkReadBuffer& buffer) {
    SkPMColor4f pm;
    buffer.readPad32(&pm, sizeof(SkPMColor4f));
    return sk_sp<SkFlattenable>(new SkMixer_Const(pm));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Reverse final : public SkMixerBase {
    SkMixer_Reverse(sk_sp<SkMixer> proxy) : fProxy(std::move(proxy)) {
        SkASSERT(fProxy);
    }
    sk_sp<SkMixer> fProxy;
    friend class SkMixer;
public:
    SK_FLATTENABLE_HOOKS(SkMixer_Reverse)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fProxy.get());
    }

    bool appendStages(const SkStageRec& rec) const override {
        struct Storage {
            float   fRGBA[4 * SkRasterPipeline_kMaxStride];
        };
        auto storage = rec.fAlloc->make<Storage>();
        SkRasterPipeline* pipeline = rec.fPipeline;

        // swap src,dst
        pipeline->append(SkRasterPipeline::store_dst, storage->fRGBA);
        pipeline->append(SkRasterPipeline::move_src_dst);
        pipeline->append(SkRasterPipeline::load_src, storage->fRGBA);

        return as_MB(fProxy)->appendStages(rec);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

sk_sp<SkFlattenable> SkMixer_Reverse::CreateProc(SkReadBuffer& buffer) {
    auto orig = buffer.readMixer();
    return orig ? orig->makeReverse() : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Blend final : public SkMixerBase {
    SkMixer_Blend(SkBlendMode mode) : fMode(mode) {}
    const SkBlendMode fMode;
    friend class SkMixer;
public:
    SK_FLATTENABLE_HOOKS(SkMixer_Blend)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.write32(static_cast<int>(fMode));
    }

    bool appendStages(const SkStageRec& rec) const override {
        SkBlendMode_AppendStages(fMode, rec.fPipeline);
        return true;
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

sk_sp<SkFlattenable> SkMixer_Blend::CreateProc(SkReadBuffer& buffer) {
    unsigned mode = buffer.read32();
    if (!buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
        return nullptr;
    }
    return MakeBlend(static_cast<SkBlendMode>(mode));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Lerp final : public SkMixerBase {
    SkMixer_Lerp(float weight) : fWeight(weight) {
        SkASSERT(fWeight >= 0 && fWeight <= 1);
    }
    const float fWeight;
    friend class SkMixer;
public:
    SK_FLATTENABLE_HOOKS(SkMixer_Lerp)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeScalar(fWeight);
    }

    bool appendStages(const SkStageRec& rec) const override {
        rec.fPipeline->append(SkRasterPipeline::lerp_1_float, &fWeight);
        return true;
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

sk_sp<SkFlattenable> SkMixer_Lerp::CreateProc(SkReadBuffer& buffer) {
    return MakeLerp(buffer.readScalar());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_ShaderLerp final : public SkMixerBase {
    SkMixer_ShaderLerp(sk_sp<SkShader> shader) : fShader(std::move(shader)) {
        SkASSERT(fShader);
    }
    sk_sp<SkShader> fShader;
    friend class SkMixer;
public:
    SK_FLATTENABLE_HOOKS(SkMixer_ShaderLerp)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fShader.get());
    }

    bool appendStages(const SkStageRec& rec) const override {
        struct Storage {
            float   fSrc[4 * SkRasterPipeline_kMaxStride];
            float   fDst[4 * SkRasterPipeline_kMaxStride];
            float   fShaderOutput[4 * SkRasterPipeline_kMaxStride];
        };
        auto storage = rec.fAlloc->make<Storage>();

        // we've been given our inputs as (drdgdbda, rgba)
        rec.fPipeline->append(SkRasterPipeline::store_dst, storage->fDst);
        rec.fPipeline->append(SkRasterPipeline::store_src, storage->fSrc);

        if (!as_SB(fShader)->appendStages(rec)) {
            return false;
        }
        // the shader's output is in rgba. We need to store "r" as our "t" values
        rec.fPipeline->append(SkRasterPipeline::store_src, storage->fShaderOutput);

        // now we need to reload the original dst and src so we can run our stage (lerp)
        rec.fPipeline->append(SkRasterPipeline::load_dst, storage->fDst);
        rec.fPipeline->append(SkRasterPipeline::load_src, storage->fSrc);

        // we use the first channel (e.g. R) as our T values
        rec.fPipeline->append(SkRasterPipeline::lerp_native, &storage->fShaderOutput[0]);
        return true;
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        return nullptr;
    }
#endif
};

sk_sp<SkFlattenable> SkMixer_ShaderLerp::CreateProc(SkReadBuffer& buffer) {
    return MakeShaderLerp(buffer.readShader());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixer_Merge final : public SkMixerBase {
    SkMixer_Merge(sk_sp<SkMixer> m0, sk_sp<SkMixer> m1, sk_sp<SkMixer> combine)
        : fM0(std::move(m0))
        , fM1(std::move(m1))
        , fCombine(std::move(combine))
    {
        SkASSERT(fCombine);
        SkASSERT(fM0 || fM1);   // need at least one. If not, the caller just wants combine
    }
    sk_sp<SkMixer> fM0, fM1, fCombine;
    friend class SkMixer;
public:
    SK_FLATTENABLE_HOOKS(SkMixer_Merge)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fM0.get());   // could be null
        buffer.writeFlattenable(fM1.get());   // could be null
        buffer.writeFlattenable(fCombine.get());
    }

    bool appendStages(const SkStageRec& rec) const override {
        struct Storage {
            float   fDst[4 * SkRasterPipeline_kMaxStride];
            float   fSrc[4 * SkRasterPipeline_kMaxStride];
            float   fM0Output[4 * SkRasterPipeline_kMaxStride];
        };
        auto storage = rec.fAlloc->make<Storage>();
        SkRasterPipeline* pipeline = rec.fPipeline;

        // Need to save off dr,dg,db,da and r,g,b,a so we can use them twice (for fM0 and fM1)
        pipeline->append(SkRasterPipeline::store_dst, storage->fDst);
        pipeline->append(SkRasterPipeline::store_src, storage->fSrc);

        if (!as_MB(fM0)->appendStages(rec)) {
            return false;
        }
        // This outputs r,g,b,a, which we'll need later when we apply the mixer, but we save it off
        // now since fM1 will overwrite them.
        pipeline->append(SkRasterPipeline::store_src, storage->fM0Output);

        // Now restore the original colors to call the first mixer
        pipeline->append(SkRasterPipeline::load_dst, storage->fDst);
        pipeline->append(SkRasterPipeline::load_src, storage->fSrc);
        if (!as_MB(fM1)->appendStages(rec)) {
            return false;
        }

        // M1's output is in r,g,b,a, which is the 2nd argument to fCombine, so we just need
        // to load M0's output back into dr,dg,db,da
        pipeline->append(SkRasterPipeline::load_dst, storage->fM0Output);

        // 1st color in dr,dg,db,da     <-- M0's output
        // 2nd color in  r, g, b, a     <-- M1's output
        return as_MB(fCombine)->appendStages(rec);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        //    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
        return nullptr;
    }
#endif
};

sk_sp<SkFlattenable> SkMixer_Merge::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkMixer> m0 = buffer.readMixer();
    sk_sp<SkMixer> m1 = buffer.readMixer();
    sk_sp<SkMixer> combine = buffer.readMixer();
    return combine ? combine->makeMerge(std::move(m0), std::move(m1)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkMixer> SkMixer::MakeFirst() {
    return MakeBlend(SkBlendMode::kDst);
}

sk_sp<SkMixer> SkMixer::MakeSecond() {
    return MakeBlend(SkBlendMode::kSrc);
}

sk_sp<SkMixer> SkMixer::MakeConst(const SkColor4f& c) {
    return sk_sp<SkMixer>(new SkMixer_Const(c.premul()));
}

sk_sp<SkMixer> SkMixer::MakeConst(SkColor c) {
    return MakeConst(SkColor4f::FromColor(c));
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

sk_sp<SkMixer> SkMixer::MakeShaderLerp(sk_sp<SkShader> shader) {
    if (!shader) {
        return MakeFirst();
    }
    return sk_sp<SkMixer>(new SkMixer_ShaderLerp(std::move(shader)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkMixer> SkMixer::makeReverse() const {
    return sk_sp<SkMixer>(new SkMixer_Reverse(sk_ref_sp(this)));
}

sk_sp<SkMixer> SkMixer::makeMerge(sk_sp<SkMixer> m0, sk_sp<SkMixer> m1) const {
    auto self = sk_ref_sp(this);
    if (!m0 && !m1) {
        return self;
    }
    return sk_sp<SkMixer>(new SkMixer_Merge(std::move(m0), std::move(m1), self));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkPMColor4f SkMixerBase::test_mix(const SkPMColor4f& a, const SkPMColor4f& b) const {
    SkPMColor4f dst = a,
                src = b;

    SkSTArenaAlloc<128> alloc;
    SkRasterPipeline    pipeline(&alloc);
    SkPaint             dummyPaint;
    SkStageRec rec = {
        &pipeline, &alloc, kRGBA_F32_SkColorType, nullptr, dummyPaint, nullptr, SkMatrix::I()
    };

    SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
    SkRasterPipeline_MemoryCtx srcPtr = { &src, 0 };

    pipeline.append(SkRasterPipeline::load_f32, &dstPtr);
    pipeline.append(SkRasterPipeline::move_src_dst);        // dst is our 1st arg
    pipeline.append(SkRasterPipeline::load_f32, &srcPtr);   // src is our 2nd arg
    as_MB(this)->appendStages(rec);
    pipeline.append(SkRasterPipeline::store_f32, &dstPtr);
    pipeline.run(0,0, 1,1);

    return dst;
}

void SkMixerBase::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMixer_Const);
    SK_REGISTER_FLATTENABLE(SkMixer_Reverse);
    SK_REGISTER_FLATTENABLE(SkMixer_Blend);
    SK_REGISTER_FLATTENABLE(SkMixer_Lerp);
    SK_REGISTER_FLATTENABLE(SkMixer_ShaderLerp);
    SK_REGISTER_FLATTENABLE(SkMixer_Merge);
}
