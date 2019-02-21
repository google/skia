/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorFilter.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformSteps.h"
#include "SkColorSpaceXformer.h"
#include "SkNx.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

bool SkColorFilter::asColorMode(SkColor*, SkBlendMode*) const {
    return false;
}

bool SkColorFilter::asColorMatrix(SkScalar matrix[20]) const {
    return false;
}

bool SkColorFilter::asComponentTable(SkBitmap*) const {
    return false;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkColorFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return nullptr;
}
#endif

void SkColorFilter::appendStages(SkRasterPipeline* p,
                                 SkColorSpace* dstCS,
                                 SkArenaAlloc* alloc,
                                 bool shaderIsOpaque) const {
    this->onAppendStages(p, dstCS, alloc, shaderIsOpaque);
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    const float inv255 = 1.0f / 255;
    SkColor4f c4 = this->filterColor4f({
        SkColorGetR(c) * inv255,
        SkColorGetG(c) * inv255,
        SkColorGetB(c) * inv255,
        SkColorGetA(c) * inv255,
    }, nullptr);
    return SkColorSetARGB(sk_float_round2int(c4.fA*255),
                          sk_float_round2int(c4.fR*255),
                          sk_float_round2int(c4.fG*255),
                          sk_float_round2int(c4.fB*255));
}

#include "SkRasterPipeline.h"
SkColor4f SkColorFilter::filterColor4f(const SkColor4f& c, SkColorSpace* colorSpace) const {
    SkPMColor4f dst, src = c.premul();

    SkSTArenaAlloc<128> alloc;
    SkRasterPipeline    pipeline(&alloc);

    pipeline.append_constant_color(&alloc, src.vec());
    this->onAppendStages(&pipeline, colorSpace, &alloc, c.fA == 1);
    SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
    pipeline.append(SkRasterPipeline::store_f32, &dstPtr);
    pipeline.run(0,0, 1,1);

    return dst.unpremul();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  Since colorfilters may be used on the GPU backend, and in that case we may string together
 *  many GrFragmentProcessors, we might exceed some internal instruction/resource limit.
 *
 *  Since we don't yet know *what* those limits might be when we construct the final shader,
 *  we just set an arbitrary limit during construction. If later we find smarter ways to know what
 *  the limnits are, we can change this constant (or remove it).
 */
#define SK_MAX_COMPOSE_COLORFILTER_COUNT    4

class SkComposeColorFilter : public SkColorFilter {
public:
    uint32_t getFlags() const override {
        // Can only claim alphaunchanged support if both our proxys do.
        return fOuter->getFlags() & fInner->getFlags();
    }

    void onAppendStages(SkRasterPipeline* p, SkColorSpace* dst, SkArenaAlloc* scratch,
                        bool shaderIsOpaque) const override {
        bool innerIsOpaque = shaderIsOpaque;
        if (!(fInner->getFlags() & kAlphaUnchanged_Flag)) {
            innerIsOpaque = false;
        }
        fInner->appendStages(p, dst, scratch, shaderIsOpaque);
        fOuter->appendStages(p, dst, scratch, innerIsOpaque);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        auto innerFP = fInner->asFragmentProcessor(context, dstColorSpaceInfo);
        auto outerFP = fOuter->asFragmentProcessor(context, dstColorSpaceInfo);
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

    SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner,
                         int composedFilterCount)
        : fOuter(std::move(outer))
        , fInner(std::move(inner))
        , fComposedFilterCount(composedFilterCount)
    {
        SkASSERT(composedFilterCount >= 2);
        SkASSERT(composedFilterCount <= SK_MAX_COMPOSE_COLORFILTER_COUNT);
    }

    int privateComposedFilterCount() const override {
        return fComposedFilterCount;
    }

    sk_sp<SkColorFilter> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        auto outer = xformer->apply(fOuter.get());
        auto inner = xformer->apply(fInner.get());
        if (outer != fOuter || inner != fInner) {
            return outer->makeComposed(inner);
        }
        return this->INHERITED::onMakeColorSpace(xformer);
    }

    sk_sp<SkColorFilter> fOuter;
    sk_sp<SkColorFilter> fInner;
    const int            fComposedFilterCount;

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

    // Give the subclass a shot at a more optimal composition...
    auto composition = this->onMakeComposed(inner);
    if (composition) {
        return composition;
    }

    int count = inner->privateComposedFilterCount() + this->privateComposedFilterCount();
    if (count > SK_MAX_COMPOSE_COLORFILTER_COUNT) {
        return nullptr;
    }
    return sk_sp<SkColorFilter>(new SkComposeColorFilter(sk_ref_sp(this), std::move(inner), count));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "../gpu/effects/GrSRGBEffect.h"
#endif

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
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override {
        // wish our caller would let us know if our input was opaque...
        GrSRGBEffect::Alpha alpha = GrSRGBEffect::Alpha::kPremul;
        switch (fDir) {
            case Direction::kLinearToSRGB:
                return GrSRGBEffect::Make(GrSRGBEffect::Mode::kLinearToSRGB, alpha);
            case Direction::kSRGBToLinear:
                return GrSRGBEffect::Make(GrSRGBEffect::Mode::kSRGBToLinear, alpha);
        }
        return nullptr;
    }
#endif

    void onAppendStages(SkRasterPipeline* p, SkColorSpace*, SkArenaAlloc* alloc,
                        bool shaderIsOpaque) const override {
        if (!shaderIsOpaque) {
            p->append(SkRasterPipeline::unpremul);
        }

        // TODO: is it valuable to thread this through appendStages()?
        bool shaderIsNormalized = false;
        fSteps.apply(p, shaderIsNormalized);

        if (!shaderIsOpaque) {
            p->append(SkRasterPipeline::premul);
        }
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

sk_sp<SkColorFilter> SkColorFilter::MakeLinearToSRGBGamma() {
    return MakeSRGBGammaCF<SkSRGBGammaColorFilter::Direction::kLinearToSRGB>();
}

sk_sp<SkColorFilter> SkColorFilter::MakeSRGBToLinearGamma() {
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

    void onAppendStages(SkRasterPipeline* p, SkColorSpace* dst, SkArenaAlloc* alloc,
                        bool shaderIsOpaque) const override {
        // want cf0 * (1 - w) + cf1 * w == lerp(w)
        // which means
        //      dr,dg,db,da <-- cf0
        //      r,g,b,a     <-- cf1
        struct State {
            float     orig_rgba[4 * SkRasterPipeline_kMaxStride];
            float filtered_rgba[4 * SkRasterPipeline_kMaxStride];
        };
        auto state = alloc->make<State>();

        p->append(SkRasterPipeline::store_rgba, state->orig_rgba);
        if (!fCF1) {
            fCF0->appendStages(p, dst, alloc, shaderIsOpaque);
            p->append(SkRasterPipeline::move_src_dst);
            p->append(SkRasterPipeline::load_rgba, state->orig_rgba);
        } else {
            fCF1->appendStages(p, dst, alloc, shaderIsOpaque);
            p->append(SkRasterPipeline::store_rgba, state->filtered_rgba);
            p->append(SkRasterPipeline::load_rgba, state->orig_rgba);
            fCF0->appendStages(p, dst, alloc, shaderIsOpaque);
            p->append(SkRasterPipeline::move_src_dst);
            p->append(SkRasterPipeline::load_rgba, state->filtered_rgba);
        }
        float* storage = alloc->make<float>(fWeight);
        p->append(SkRasterPipeline::lerp_1_float, storage);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorSpaceInfo&) const override {
        return nullptr;
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
    return MakeMixer(std::move(cf0), std::move(cf1), weight);
}

sk_sp<SkColorFilter> SkColorFilter::MakeMixer(sk_sp<SkColorFilter> cf0,
                                              sk_sp<SkColorFilter> cf1,
                                              float weight) {
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

#include "SkModeColorFilter.h"

void SkColorFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeColorFilter);
    SK_REGISTER_FLATTENABLE(SkModeColorFilter);
    SK_REGISTER_FLATTENABLE(SkSRGBGammaColorFilter);
    SK_REGISTER_FLATTENABLE(SkMixerColorFilter);
}
