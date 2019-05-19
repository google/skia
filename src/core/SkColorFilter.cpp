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
#include "include/private/SkArenaAlloc.h"
#include "include/private/SkNx.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
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
std::unique_ptr<GrFragmentProcessor> SkColorFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return nullptr;
}
#endif

bool SkColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    return this->onAppendStages(rec, shaderIsOpaque);
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    return this->filterColor4f(SkColor4f::FromColor(c), nullptr)
        .toSkColor();
}

#include "src/core/SkRasterPipeline.h"
SkColor4f SkColorFilter::filterColor4f(const SkColor4f& c, SkColorSpace* colorSpace) const {
    SkPMColor4f dst, src = c.premul();

    SkSTArenaAlloc<128> alloc;
    SkRasterPipeline    pipeline(&alloc);

    pipeline.append_constant_color(&alloc, src.vec());

    SkPaint dummyPaint;
    SkStageRec rec = {
        &pipeline, &alloc, kRGBA_F32_SkColorType, colorSpace, dummyPaint, nullptr, SkMatrix::I()
    };
    this->onAppendStages(rec, c.fA == 1);
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

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        bool innerIsOpaque = shaderIsOpaque;
        if (!(fInner->getFlags() & kAlphaUnchanged_Flag)) {
            innerIsOpaque = false;
        }
        return fInner->appendStages(rec, shaderIsOpaque) &&
               fOuter->appendStages(rec, innerIsOpaque);
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

    int count = inner->privateComposedFilterCount() + this->privateComposedFilterCount();
    if (count > SK_MAX_COMPOSE_COLORFILTER_COUNT) {
        return nullptr;
    }
    return sk_sp<SkColorFilter>(new SkComposeColorFilter(sk_ref_sp(this), std::move(inner), count));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "src/gpu/effects/GrSRGBEffect.h"
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

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorSpaceInfo& dstColorSpaceInfo) const override {
        return GrMixerEffect::Make(
                fCF0->asFragmentProcessor(context, dstColorSpaceInfo),
                fCF1 ? fCF1->asFragmentProcessor(context, dstColorSpaceInfo) : nullptr,
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

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLInterpreter.h"

class SkRuntimeColorFilter : public SkColorFilter {
public:
    SkRuntimeColorFilter(int index, SkString sksl, sk_sp<SkData> inputs,
                         void (*cpuFunction)(float[4], const void*))
        : fIndex(index)
        , fSkSL(std::move(sksl))
        , fInputs(std::move(inputs))
        , fCpuFunction(cpuFunction) {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorSpaceInfo&) const override {
        return GrSkSLFP::Make(context, fIndex, "Runtime Color Filter", fSkSL,
                              fInputs ? fInputs->data() : nullptr,
                              fInputs ? fInputs->size() : 0);
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        if (fCpuFunction) {
            struct CpuFuncCtx : public SkRasterPipeline_CallbackCtx {
                SkRuntimeColorFilterFn cpuFn;
                const void* inputs;
            };
            auto ctx = rec.fAlloc->make<CpuFuncCtx>();
            ctx->inputs = fInputs->data();
            ctx->cpuFn = fCpuFunction;
            ctx->fn = [](SkRasterPipeline_CallbackCtx* arg, int active_pixels) {
                auto ctx = (CpuFuncCtx*)arg;
                for (int i = 0; i < active_pixels; i++) {
                    ctx->cpuFn(ctx->rgba + i * 4, ctx->inputs);
                }
            };
            rec.fPipeline->append(SkRasterPipeline::callback, ctx);
        } else {
            struct InterpreterCtx : public SkRasterPipeline_CallbackCtx {
                SkSL::ByteCodeFunction* main;
                std::unique_ptr<SkSL::Interpreter> interpreter;
                const void* inputs;
            };
            auto ctx = rec.fAlloc->make<InterpreterCtx>();
            ctx->inputs = fInputs->data();
            SkSL::Compiler c;
            std::unique_ptr<SkSL::Program> prog =
                                                c.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                                                 SkSL::String(fSkSL.c_str()),
                                                                 SkSL::Program::Settings());
            if (c.errorCount()) {
                SkDebugf("%s\n", c.errorText().c_str());
                SkASSERT(false);
            }
            std::unique_ptr<SkSL::ByteCode> byteCode = c.toByteCode(*prog);
            ctx->main = byteCode->fFunctions[0].get();
            ctx->interpreter.reset(new SkSL::Interpreter(std::move(prog), std::move(byteCode),
                                                         (SkSL::Interpreter::Value*) ctx->inputs));
            ctx->fn = [](SkRasterPipeline_CallbackCtx* arg, int active_pixels) {
                auto ctx = (InterpreterCtx*)arg;
                for (int i = 0; i < active_pixels; i++) {
                    ctx->interpreter->run(*ctx->main,
                                          (SkSL::Interpreter::Value*) (ctx->rgba + i * 4),
                                          nullptr);
                }
            };
            rec.fPipeline->append(SkRasterPipeline::callback, ctx);
        }
        return true;
    }

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        // the client is responsible for ensuring that the indices match up between flattening and
        // unflattening; we don't have a reasonable way to enforce that at the moment
        buffer.writeInt(fIndex);
        buffer.writeString(fSkSL.c_str());
        if (fInputs) {
            buffer.writeDataAsByteArray(fInputs.get());
        } else {
            buffer.writeByteArray(nullptr, 0);
        }
    }

private:
    SK_FLATTENABLE_HOOKS(SkRuntimeColorFilter)

    int fIndex;
    SkString fSkSL;
    sk_sp<SkData> fInputs;
    SkRuntimeColorFilterFn fCpuFunction;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    int index = buffer.readInt();
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    return sk_sp<SkFlattenable>(new SkRuntimeColorFilter(index, std::move(sksl), std::move(inputs),
                                                         nullptr));
}

SkRuntimeColorFilterFactory::SkRuntimeColorFilterFactory(SkString sksl,
                                                         SkRuntimeColorFilterFn cpuFunc)
    : fIndex(GrSkSLFP::NewIndex())
    , fSkSL(std::move(sksl))
    , fCpuFunc(cpuFunc) {}

sk_sp<SkColorFilter> SkRuntimeColorFilterFactory::make(sk_sp<SkData> inputs) {
    return sk_sp<SkColorFilter>(new SkRuntimeColorFilter(fIndex, fSkSL, std::move(inputs),
                                                         fCpuFunc));
}

#endif // SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/core/SkModeColorFilter.h"

void SkColorFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeColorFilter);
    SK_REGISTER_FLATTENABLE(SkModeColorFilter);
    SK_REGISTER_FLATTENABLE(SkSRGBGammaColorFilter);
    SK_REGISTER_FLATTENABLE(SkMixerColorFilter);
#if SK_SUPPORT_GPU
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
#endif
}
