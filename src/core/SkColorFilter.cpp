/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkNx.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"
#endif

bool SkColorFilter::asColorMode(SkColor* color, SkBlendMode* mode) const {
    return as_CFB(this)->onAsAColorMode(color, mode);
}

bool SkColorFilter::asAColorMode(SkColor* color, SkBlendMode* mode) const {
    return as_CFB(this)->onAsAColorMode(color, mode);
}

bool SkColorFilter::asAColorMatrix(float matrix[20]) const {
    return as_CFB(this)->onAsAColorMatrix(matrix);
}

uint32_t SkColorFilter::getFlags() const { return as_CFB(this)->onGetFlags(); }

bool SkColorFilter::isAlphaUnchanged() const {
    return SkToBool(this->getFlags() & kAlphaUnchanged_Flag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkColorFilterBase::onAsAColorMode(SkColor*, SkBlendMode*) const {
    return false;
}

bool SkColorFilterBase::onAsAColorMatrix(float matrix[20]) const {
    return false;
}

#if SK_SUPPORT_GPU
GrFPResult SkColorFilterBase::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                  GrRecordingContext* context,
                                                  const GrColorInfo& dstColorInfo) const {
    // This color filter doesn't implement `asFragmentProcessor`.
    return GrFPFailure(std::move(inputFP));
}
#endif

bool SkColorFilterBase::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    return this->onAppendStages(rec, shaderIsOpaque);
}

skvm::Color SkColorFilterBase::program(skvm::Builder* p, skvm::Color c,
                                       SkColorSpace* dstCS,
                                       skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::F32 original = c.a;
    if ((c = this->onProgram(p,c, dstCS, uniforms,alloc))) {
        if (this->isAlphaUnchanged()) {
            c.a = original;
        }
        return c;
    }
    //SkDebugf("cannot onProgram %s\n", this->getTypeName());
    return {};
}

// This should look familiar... see just above.
skvm::HalfColor SkColorFilterBase::program(skvm::Builder* p, skvm::HalfColor c,
                                           SkColorSpace* dstCS,
                                           skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Half original = c.a;
    if ((c = this->onProgram(p,c, dstCS, uniforms,alloc))) {
        if (this->isAlphaUnchanged()) {
            c.a = original;
        }
        return c;
    }
    //SkDebugf("cannot onProgram %s\n", this->getTypeName());
    return {};
}

skvm::Color SkColorFilterBase::onProgram(skvm::Builder* p, skvm::Color c,
                                         SkColorSpace* dstCS,
                                         skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    if (skvm::HalfColor hc = this->onProgram(p, to_Half(c), dstCS, uniforms, alloc)) {
        return to_F32(hc);
    }
    return {};
}
skvm::HalfColor SkColorFilterBase::onProgram(skvm::Builder* p, skvm::HalfColor hc,
                                             SkColorSpace* dstCS,
                                             skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    if (skvm::Color c = this->onProgram(p, to_F32(hc), dstCS, uniforms, alloc)) {
        return to_Half(c);
    }
    return {};
}


SkColor SkColorFilter::filterColor(SkColor c) const {
    // This is mostly meaningless. We should phase-out this call entirely.
    SkColorSpace* cs = nullptr;
    return this->filterColor4f(SkColor4f::FromColor(c), cs, cs).toSkColor();
}

SkColor4f SkColorFilter::filterColor4f(const SkColor4f& origSrcColor, SkColorSpace* srcCS,
                                       SkColorSpace* dstCS) const {
    SkColor4f color = origSrcColor;
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType,
                           dstCS, kPremul_SkAlphaType).apply(color.vec());

    constexpr size_t kEnoughForCommonFilters = 512; // big enough for compose+colormatrix
    SkSTArenaAlloc<kEnoughForCommonFilters> alloc;
    SkRasterPipeline    pipeline(&alloc);
    pipeline.append_constant_color(&alloc, color.vec());
    SkPaint dummyPaint;
    SkSimpleMatrixProvider matrixProvider(SkMatrix::I());
    SkStageRec rec = {
        &pipeline, &alloc, kRGBA_F32_SkColorType, dstCS, dummyPaint, nullptr, matrixProvider
    };
    as_CFB(this)->onAppendStages(rec, color.fA == 1);

    SkPMColor4f dst;
    SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
    pipeline.append(SkRasterPipeline::store_f32, &dstPtr);
    pipeline.run(0,0, 1,1);
    return dst.unpremul();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkComposeColorFilter : public SkColorFilterBase {
public:
    uint32_t onGetFlags() const override {
        // Can only claim alphaunchanged support if both our proxys do.
        return fOuter->onGetFlags() & fInner->onGetFlags();
    }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        bool innerIsOpaque = shaderIsOpaque;
        if (!fInner->isAlphaUnchanged()) {
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
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo) const override {
        GrFragmentProcessor* originalInputFP = inputFP.get();

        auto [innerSuccess, innerFP] =
                fInner->asFragmentProcessor(std::move(inputFP), context, dstColorInfo);
        if (!innerSuccess) {
            return GrFPFailure(std::move(innerFP));
        }

        auto [outerSuccess, outerFP] =
                fOuter->asFragmentProcessor(std::move(innerFP), context, dstColorInfo);
        if (!outerSuccess) {
            // In the rare event that the outer FP cannot be built, we have no good way of
            // separating the inputFP from the innerFP, so we need to return a cloned inputFP.
            // This could hypothetically be expensive, but failure here should be extremely rare.
            return GrFPFailure(originalInputFP->clone());
        }

        return GrFPSuccess(std::move(outerFP));
    }
#endif

    SK_FLATTENABLE_HOOKS(SkComposeColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fOuter.get());
        buffer.writeFlattenable(fInner.get());
    }

private:
    SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner)
        : fOuter(as_CFB_sp(std::move(outer)))
        , fInner(as_CFB_sp(std::move(inner)))
    {}

    sk_sp<SkColorFilterBase> fOuter;
    sk_sp<SkColorFilterBase> fInner;

    friend class SkColorFilter;

    using INHERITED = SkColorFilter;
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

class SkSRGBGammaColorFilter : public SkColorFilterBase {
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
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo) const override {
        // wish our caller would let us know if our input was opaque...
        constexpr SkAlphaType alphaType = kPremul_SkAlphaType;
        switch (fDir) {
            case Direction::kLinearToSRGB:
                return GrFPSuccess(GrColorSpaceXformEffect::Make(
                                       std::move(inputFP),
                                       sk_srgb_linear_singleton(), alphaType,
                                       sk_srgb_singleton(),        alphaType));
            case Direction::kSRGBToLinear:
                return GrFPSuccess(GrColorSpaceXformEffect::Make(
                                       std::move(inputFP),
                                       sk_srgb_singleton(),        alphaType,
                                       sk_srgb_linear_singleton(), alphaType));
        }
        SkUNREACHABLE;
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipeline::unpremul);
        }

        fSteps.apply(rec.fPipeline);

        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipeline::premul);
        }
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        return premul(fSteps.program(p, uniforms, unpremul(c)));
    }

    SK_FLATTENABLE_HOOKS(SkSRGBGammaColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.write32(static_cast<uint32_t>(fDir));
    }

private:
    const Direction fDir;
    SkColorSpaceXformSteps fSteps;

    friend class SkColorFilter;
    using INHERITED = SkColorFilterBase;
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

class SkMixerColorFilter : public SkColorFilterBase {
public:
    SkMixerColorFilter(sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1, float weight)
        : fCF0(as_CFB_sp(std::move(cf0)))
        , fCF1(as_CFB_sp(std::move(cf1)))
        , fWeight(weight)
    {
        SkASSERT(fCF0);
        SkASSERT(fWeight >= 0 && fWeight <= 1);
    }

    uint32_t onGetFlags() const override {
        uint32_t f0 = fCF0->onGetFlags();
        uint32_t f1 = fCF1 ? fCF1->onGetFlags() : ~0U;
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
            if (!fCF0->appendStages(rec, shaderIsOpaque)) {
                return false;
            }
            p->append(SkRasterPipeline::move_src_dst);
            p->append(SkRasterPipeline::load_src, state->orig_rgba);
        } else {
            if (!fCF0->appendStages(rec, shaderIsOpaque)) {
                return false;
            }
            p->append(SkRasterPipeline::store_src, state->filtered_rgba);
            p->append(SkRasterPipeline::load_src, state->orig_rgba);
            if (!fCF1->appendStages(rec, shaderIsOpaque)) {
                return false;
            }
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
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo) const override {
        bool success;
        std::unique_ptr<GrFragmentProcessor> fp0, fp1;

        std::tie(success, fp0) = fCF0->asFragmentProcessor(/*inputFP=*/nullptr,
                                                           context, dstColorInfo);
        if (!success) {
            return GrFPFailure(std::move(inputFP));
        }

        if (fCF1) {
            std::tie(success, fp1) = fCF1->asFragmentProcessor(/*inputFP=*/nullptr,
                                                               context, dstColorInfo);
            if (!success) {
                return GrFPFailure(std::move(inputFP));
            }
        }

        return GrFPSuccess(GrMixerEffect::Make(std::move(inputFP), std::move(fp0),
                                               std::move(fp1), fWeight));
    }
#endif

    SK_FLATTENABLE_HOOKS(SkMixerColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fCF0.get());
        buffer.writeFlattenable(fCF1.get());
        buffer.writeScalar(fWeight);
    }

private:
    sk_sp<SkColorFilterBase> fCF0;
    sk_sp<SkColorFilterBase> fCF1;
    const float              fWeight;

    friend class SkColorFilter;

    using INHERITED = SkColorFilterBase;
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

void SkColorFilterBase::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeColorFilter);
    SK_REGISTER_FLATTENABLE(SkModeColorFilter);
    SK_REGISTER_FLATTENABLE(SkSRGBGammaColorFilter);
    SK_REGISTER_FLATTENABLE(SkMixerColorFilter);
}
