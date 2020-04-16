/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkNx.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"
#endif

bool SkColorFilter::onAsAColorMode(SkColor*, SkBlendMode*) const {
    return false;
}

bool SkColorFilter::onAsAColorMatrix(float matrix[20]) const {
    return false;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkColorFilter::asFragmentProcessor(GrRecordingContext*,
                                                                        const GrColorInfo&) const {
    return nullptr;
}
#endif

bool SkColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    return this->onAppendStages(rec, shaderIsOpaque);
}

skvm::Color SkColorFilter::program(skvm::Builder* p, skvm::Color c,
                                   SkColorSpace* dstCS,
                                   skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::F32 original = c.a;
    if ((c = this->onProgram(p,c, dstCS, uniforms,alloc))) {
        if (this->getFlags() & kAlphaUnchanged_Flag) {
            c.a = original;
        }
        return c;
    }
    //SkDebugf("cannot onProgram %s\n", this->getTypeName());
    return {};
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    // This is mostly meaningless. We should phase-out this call entirely.
    SkColorSpace* cs = nullptr;
    return this->filterColor4f(SkColor4f::FromColor(c), cs, cs).toSkColor();
}

SkColor4f SkColorFilter::filterColor4f(const SkColor4f& origSrcColor, SkColorSpace* srcCS,
                                       SkColorSpace* dstCS) const {
#ifdef SK_SUPPORT_LEGACY_COLORFILTER_NO_SHADER
    SkPMColor4f src = origSrcColor.premul();
    SkColor4f color = *(SkColor4f*)&src;
#else
    SkColor4f color = origSrcColor;
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType,
                           dstCS, kPremul_SkAlphaType).apply(color.vec());
#endif

    constexpr size_t kEnoughForCommonFilters = 512; // big enough for compose+colormatrix
    SkSTArenaAlloc<kEnoughForCommonFilters> alloc;
    SkRasterPipeline    pipeline(&alloc);
    pipeline.append_constant_color(&alloc, color.vec());
    SkPaint dummyPaint;
    SkStageRec rec = {
        &pipeline, &alloc, kRGBA_F32_SkColorType, dstCS, dummyPaint, nullptr, SkMatrix::I()
    };
    this->onAppendStages(rec, color.fA == 1);

    SkPMColor4f dst;
    SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
    pipeline.append(SkRasterPipeline::store_f32, &dstPtr);
    pipeline.run(0,0, 1,1);
    return dst.unpremul();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkComposeColorFilter : public SkColorFilter {
public:
    uint32_t getFlags() const override {
        // Can only claim alphaunchanged support if both our proxys do.
        return fOuter->getFlags() & fInner->getFlags();
    }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        bool innerIsOpaque = shaderIsOpaque;
        if (!(fInner->getFlags() & kAlphaUnchanged_Flag)) {
            innerIsOpaque = false;
        }
        return fInner->appendStages(rec, shaderIsOpaque) &&
               fOuter->appendStages(rec, innerIsOpaque);
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
               c = fInner->program(p, c, dstCS, uniforms, alloc);
        return c ? fOuter->program(p, c, dstCS, uniforms, alloc) : skvm::Color{};
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorInfo& dstColorInfo) const override {
        auto innerFP = fInner->asFragmentProcessor(context, dstColorInfo);
        auto outerFP = fOuter->asFragmentProcessor(context, dstColorInfo);
        if (!innerFP || !outerFP) {
            return nullptr;
        }
        std::unique_ptr<GrFragmentProcessor> series[] = { std::move(innerFP), std::move(outerFP) };
        return GrFragmentProcessor::RunInSeries(series, 2);
    }
#endif

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fOuter.get());
        buffer.writeFlattenable(fInner.get());
    }

private:
    SK_FLATTENABLE_HOOKS(SkComposeColorFilter)

    SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner)
        : fOuter(std::move(outer))
        , fInner(std::move(inner)) {}

    sk_sp<SkColorFilter> fOuter;
    sk_sp<SkColorFilter> fInner;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkComposeColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> outer(buffer.readColorFilter());
    sk_sp<SkColorFilter> inner(buffer.readColorFilter());
    return outer ? outer->makeComposed(std::move(inner)) : inner;
}

sk_sp<SkColorFilter> SkColorFilter::makeComposed(sk_sp<SkColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return sk_sp<SkColorFilter>(new SkComposeColorFilter(sk_ref_sp(this), std::move(inner)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkSRGBGammaColorFilter : public SkColorFilter {
public:
    enum class Direction {
        kLinearToSRGB,
        kSRGBToLinear,
    };
    SkSRGBGammaColorFilter(Direction dir) : fDir(dir), fSteps([&]{
        // We handle premul/unpremul separately, so here just always upm->upm.
        if (dir == Direction::kLinearToSRGB) {
            return SkColorSpaceXformSteps{sk_srgb_linear_singleton(), kUnpremul_SkAlphaType,
                                          sk_srgb_singleton(),        kUnpremul_SkAlphaType};
        } else {
            return SkColorSpaceXformSteps{sk_srgb_singleton(),        kUnpremul_SkAlphaType,
                                          sk_srgb_linear_singleton(), kUnpremul_SkAlphaType};
        }
    }()) {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             const GrColorInfo&) const override {
        // wish our caller would let us know if our input was opaque...
        SkAlphaType at = kPremul_SkAlphaType;
        switch (fDir) {
            case Direction::kLinearToSRGB:
                return GrColorSpaceXformEffect::Make(sk_srgb_linear_singleton(), at,
                                                     sk_srgb_singleton(),        at);
            case Direction::kSRGBToLinear:
                return GrColorSpaceXformEffect::Make(sk_srgb_singleton(),        at,
                                                     sk_srgb_linear_singleton(), at);
        }
        return nullptr;
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipeline::unpremul);
        }

        // TODO: is it valuable to thread this through appendStages()?
        bool shaderIsNormalized = false;
        fSteps.apply(rec.fPipeline, shaderIsNormalized);

        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipeline::premul);
        }
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        return premul(fSteps.program(p, uniforms, unpremul(c)));
    }

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.write32(static_cast<uint32_t>(fDir));
    }

private:
    SK_FLATTENABLE_HOOKS(SkSRGBGammaColorFilter)

    const Direction fDir;
    SkColorSpaceXformSteps fSteps;

    friend class SkColorFilter;
    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkSRGBGammaColorFilter::CreateProc(SkReadBuffer& buffer) {
    uint32_t dir = buffer.read32();
    if (!buffer.validate(dir <= 1)) {
        return nullptr;
    }
    return sk_sp<SkFlattenable>(new SkSRGBGammaColorFilter(static_cast<Direction>(dir)));
}

template <SkSRGBGammaColorFilter::Direction dir>
sk_sp<SkColorFilter> MakeSRGBGammaCF() {
    static SkColorFilter* gSingleton = new SkSRGBGammaColorFilter(dir);
    return sk_ref_sp(gSingleton);
}

sk_sp<SkColorFilter> SkColorFilters::LinearToSRGBGamma() {
    return MakeSRGBGammaCF<SkSRGBGammaColorFilter::Direction::kLinearToSRGB>();
}

sk_sp<SkColorFilter> SkColorFilters::SRGBToLinearGamma() {
    return MakeSRGBGammaCF<SkSRGBGammaColorFilter::Direction::kSRGBToLinear>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMixerColorFilter : public SkColorFilter {
public:
    SkMixerColorFilter(sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1, float weight)
        : fCF0(std::move(cf0)), fCF1(std::move(cf1)), fWeight(weight)
    {
        SkASSERT(fCF0);
        SkASSERT(fWeight >= 0 && fWeight <= 1);
    }

    uint32_t getFlags() const override {
        uint32_t f0 = fCF0->getFlags();
        uint32_t f1 = fCF1 ? fCF1->getFlags() : ~0U;
        return f0 & f1;
    }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        // want cf0 * (1 - w) + cf1 * w == lerp(w)
        // which means
        //      dr,dg,db,da <-- cf0
        //      r,g,b,a     <-- cf1
        struct State {
            float     orig_rgba[4 * SkRasterPipeline_kMaxStride];
            float filtered_rgba[4 * SkRasterPipeline_kMaxStride];
        };
        auto state = rec.fAlloc->make<State>();
        SkRasterPipeline* p = rec.fPipeline;

        p->append(SkRasterPipeline::store_src, state->orig_rgba);
        if (!fCF1) {
            fCF0->appendStages(rec, shaderIsOpaque);
            p->append(SkRasterPipeline::move_src_dst);
            p->append(SkRasterPipeline::load_src, state->orig_rgba);
        } else {
            fCF0->appendStages(rec, shaderIsOpaque);
            p->append(SkRasterPipeline::store_src, state->filtered_rgba);
            p->append(SkRasterPipeline::load_src, state->orig_rgba);
            fCF1->appendStages(rec, shaderIsOpaque);
            p->append(SkRasterPipeline::load_dst, state->filtered_rgba);
        }
        float* storage = rec.fAlloc->make<float>(fWeight);
        p->append(SkRasterPipeline::lerp_1_float, storage);
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        skvm::Color c0 =        fCF0->program(p, c, dstCS, uniforms, alloc);
        skvm::Color c1 = fCF1 ? fCF1->program(p, c, dstCS, uniforms, alloc) : c;
        return (c0 && c1)
               ? lerp(c0, c1, p->uniformF(uniforms->pushF(fWeight)))
               : skvm::Color{};
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorInfo& dstColorInfo) const override {
        return GrMixerEffect::Make(
                fCF0->asFragmentProcessor(context, dstColorInfo),
                fCF1 ? fCF1->asFragmentProcessor(context, dstColorInfo) : nullptr,
                fWeight);
    }
#endif

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fCF0.get());
        buffer.writeFlattenable(fCF1.get());
        buffer.writeScalar(fWeight);
    }

private:
    SK_FLATTENABLE_HOOKS(SkMixerColorFilter)

    sk_sp<SkColorFilter> fCF0;
    sk_sp<SkColorFilter> fCF1;
    const float          fWeight;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkMixerColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> cf0(buffer.readColorFilter());
    sk_sp<SkColorFilter> cf1(buffer.readColorFilter());
    const float weight = buffer.readScalar();
    return SkColorFilters::Lerp(weight, std::move(cf0), std::move(cf1));
}

sk_sp<SkColorFilter> SkColorFilters::Lerp(float weight, sk_sp<SkColorFilter> cf0,
                                                        sk_sp<SkColorFilter> cf1) {
    if (!cf0 && !cf1) {
        return nullptr;
    }
    if (SkScalarIsNaN(weight)) {
        return nullptr;
    }

    if (cf0 == cf1) {
        return cf0; // or cf1
    }

    if (weight <= 0) {
        return cf0;
    }
    if (weight >= 1) {
        return cf1;
    }

    return sk_sp<SkColorFilter>(cf0
            ? new SkMixerColorFilter(std::move(cf0), std::move(cf1), weight)
            : new SkMixerColorFilter(std::move(cf1), nullptr, 1 - weight));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/core/SkModeColorFilter.h"

void SkColorFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeColorFilter);
    SK_REGISTER_FLATTENABLE(SkModeColorFilter);
    SK_REGISTER_FLATTENABLE(SkSRGBGammaColorFilter);
    SK_REGISTER_FLATTENABLE(SkMixerColorFilter);
}
