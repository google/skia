/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkModeColorFilter.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////

SkModeColorFilter::SkModeColorFilter(SkColor color, SkBlendMode mode) {
    fColor = color;
    fMode = mode;
}

bool SkModeColorFilter::onAsAColorMode(SkColor* color, SkBlendMode* mode) const {
    if (color) {
        *color = fColor;
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

uint32_t SkModeColorFilter::getFlags() const {
    uint32_t flags = 0;
    switch (fMode) {
        case SkBlendMode::kDst:      //!< [Da, Dc]
        case SkBlendMode::kSrcATop:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            flags |= kAlphaUnchanged_Flag;
        default:
            break;
    }
    return flags;
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
    buffer.writeUInt((int)fMode);
}

sk_sp<SkFlattenable> SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkColor color = buffer.readColor();
    SkBlendMode mode = (SkBlendMode)buffer.readUInt();
    return SkColorFilters::Blend(color, mode);
}

bool SkModeColorFilter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipeline::move_src_dst);
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "src/gpu/GrBlend.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkModeColorFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorInfo& dstColorInfo) const {
    if (SkBlendMode::kDst == fMode) {
        return nullptr;
    }

    auto constFP = GrConstColorProcessor::Make(SkColorToPMColor4f(fColor, dstColorInfo),
                                               GrConstColorProcessor::InputMode::kIgnore);
    auto fp = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(constFP), fMode);
    if (!fp) {
        return nullptr;
    }
#ifdef SK_DEBUG
    // With a solid color input this should always be able to compute the blended color
    // (at least for coeff modes)
    if ((unsigned)fMode <= (unsigned)SkBlendMode::kLastCoeffMode) {
        SkASSERT(fp->hasConstantOutputForConstantInput());
    }
#endif
    return fp;
}

#endif

sk_sp<SkColorFilter> SkColorFilters::Blend(SkColor color, SkBlendMode mode) {
    if (!SkIsValidMode(mode)) {
        return nullptr;
    }

    unsigned alpha = SkColorGetA(color);

    // first collaps some modes if possible

    if (SkBlendMode::kClear == mode) {
        color = 0;
        mode = SkBlendMode::kSrc;
    } else if (SkBlendMode::kSrcOver == mode) {
        if (0 == alpha) {
            mode = SkBlendMode::kDst;
        } else if (255 == alpha) {
            mode = SkBlendMode::kSrc;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkBlendMode::kDst == mode ||
        (0 == alpha && (SkBlendMode::kSrcOver == mode ||
                        SkBlendMode::kDstOver == mode ||
                        SkBlendMode::kDstOut == mode ||
                        SkBlendMode::kSrcATop == mode ||
                        SkBlendMode::kXor == mode ||
                        SkBlendMode::kDarken == mode)) ||
            (0xFF == alpha && SkBlendMode::kDstIn == mode)) {
        return nullptr;
    }

    return SkModeColorFilter::Make(color, mode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

class SkBlendColorFilter : public SkColorFilter {
public:
    SkBlendColorFilter(SkBlendMode mode, sk_sp<SkColorFilter> dst, sk_sp<SkColorFilter> src)
        : fMode(mode), fDst(std::move(dst)), fSrc(std::move(src))
    {}

    uint32_t getFlags() const override {
        // Not sure how to be clever here, so will return 0 (alpha may change)
        return 0;
    }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        SkRasterPipeline* p = rec.fPipeline;

        if (fDst) {
            struct State {
                float orig_rgba[4 * SkRasterPipeline_kMaxStride];
            };
            auto state = rec.fAlloc->make<State>();
            p->append(SkRasterPipeline::store_src, state->orig_rgba);
            fDst->appendStages(rec, shaderIsOpaque);
            p->append(SkRasterPipeline::move_src_dst);
            p->append(SkRasterPipeline::load_src, state->orig_rgba);
        } else {
            p->append(SkRasterPipeline::move_src_dst);
        }
        if (fSrc) {
            fSrc->appendStages(rec, shaderIsOpaque);
        }
        SkBlendMode_AppendStages(fMode, p);
        return true;
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorInfo& dstColorInfo) const override {
        return nullptr;
    }
#endif

    SK_FLATTENABLE_HOOKS(SkBlendColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fDst.get());
        buffer.writeFlattenable(fSrc.get());
        buffer.write32(static_cast<uint32_t>(fMode));
    }

private:
    sk_sp<SkColorFilter> fDst;
    sk_sp<SkColorFilter> fSrc;
    const SkBlendMode    fMode;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkBlendColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> dst(buffer.readColorFilter());
    sk_sp<SkColorFilter> src(buffer.readColorFilter());
    const uint32_t mode = buffer.read32();
    if (buffer.validate(mode <= static_cast<uint32_t>(SkBlendMode::kLastMode))) {
        return SkColorFilters::Blend(static_cast<SkBlendMode>(mode),
                                     std::move(dst), std::move(src));
    }
    return nullptr;
}
// external symbol for registering
sk_sp<SkFlattenable> SkBlendColorFilter_CreateProc(SkReadBuffer& buffer) {
    return SkBlendColorFilter::CreateProc(buffer);
}

sk_sp<SkColorFilter> SkColorFilters::Blend(SkBlendMode mode, sk_sp<SkColorFilter> dst,
                                                             sk_sp<SkColorFilter> src) {
    switch (mode) {
        case SkBlendMode::kClear: return Blend(0, mode);
        case SkBlendMode::kSrc:   return src;
        case SkBlendMode::kDst:   return dst;
        default: break;
    }

    return sk_sp<SkColorFilter>(new SkBlendColorFilter(mode, std::move(dst), std::move(src)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

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
            GrRecordingContext* context, const GrColorInfo& dstColorInfo) const override {
        return GrMixerEffect::Make(
                fCF0->asFragmentProcessor(context, dstColorInfo),
                fCF1 ? fCF1->asFragmentProcessor(context, dstColorInfo) : nullptr,
                fWeight);
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
// external symbol for registering
sk_sp<SkFlattenable> SkLerpColorFilter_CreateProc(SkReadBuffer& buffer) {
    return SkMixerColorFilter::CreateProc(buffer);
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
