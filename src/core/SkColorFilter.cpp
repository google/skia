/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorFilter.h"
#include "SkColorSpaceXformer.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"
#include "../jumper/SkJumper.h"

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
sk_sp<GrFragmentProcessor> SkColorFilter::asFragmentProcessor(GrContext*, SkColorSpace*) const {
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
    });
    return SkColorSetARGB(sk_float_round2int(c4.fA*255),
                          sk_float_round2int(c4.fR*255),
                          sk_float_round2int(c4.fG*255),
                          sk_float_round2int(c4.fB*255));
}

#include "SkRasterPipeline.h"
SkColor4f SkColorFilter::filterColor4f(const SkColor4f& c) const {
    SkPM4f dst, src = c.premul();

    SkSTArenaAlloc<128> alloc;
    SkRasterPipeline    pipeline(&alloc);

    pipeline.append_uniform_color(&alloc, src);
    this->onAppendStages(&pipeline, nullptr, &alloc, c.fA == 1);
    SkPM4f* dstPtr = &dst;
    pipeline.append(SkRasterPipeline::store_f32, &dstPtr);
    pipeline.run(0,0, 1);

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
        // Can only claim alphaunchanged and SkPM4f support if both our proxys do.
        return fOuter->getFlags() & fInner->getFlags();
    }

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override {
        SkString outerS, innerS;
        fOuter->toString(&outerS);
        fInner->toString(&innerS);
        // These strings can be long.  SkString::appendf has limitations.
        str->append(SkStringPrintf("SkComposeColorFilter: outer(%s) inner(%s)", outerS.c_str(),
                                   innerS.c_str()));
    }
#endif

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
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext* context,
                                                   SkColorSpace* dstColorSpace) const override {
        sk_sp<GrFragmentProcessor> innerFP(fInner->asFragmentProcessor(context, dstColorSpace));
        sk_sp<GrFragmentProcessor> outerFP(fOuter->asFragmentProcessor(context, dstColorSpace));
        if (!innerFP || !outerFP) {
            return nullptr;
        }
        sk_sp<GrFragmentProcessor> series[] = { std::move(innerFP), std::move(outerFP) };
        return GrFragmentProcessor::RunInSeries(series, 2);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fOuter.get());
        buffer.writeFlattenable(fInner.get());
    }

private:
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
            return SkColorFilter::MakeComposeFilter(outer, inner);
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
    return MakeComposeFilter(std::move(outer), std::move(inner));
}

sk_sp<SkColorFilter> SkColorFilter::MakeComposeFilter(sk_sp<SkColorFilter> outer,
                                                      sk_sp<SkColorFilter> inner) {
    if (!outer) {
        return inner;
    }
    if (!inner) {
        return outer;
    }

    // Give the subclass a shot at a more optimal composition...
    auto composition = outer->makeComposed(inner);
    if (composition) {
        return composition;
    }

    int count = inner->privateComposedFilterCount() + outer->privateComposedFilterCount();
    if (count > SK_MAX_COMPOSE_COLORFILTER_COUNT) {
        return nullptr;
    }
    return sk_sp<SkColorFilter>(new SkComposeColorFilter(std::move(outer), std::move(inner),count));
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
    SkSRGBGammaColorFilter(Direction dir) : fDir(dir) {}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext* x, SkColorSpace* cs) const override {
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

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSRGBGammaColorFilter)

    void onAppendStages(SkRasterPipeline* p, SkColorSpace*, SkArenaAlloc* alloc,
                        bool shaderIsOpaque) const override {
        if (!shaderIsOpaque) {
            p->append(SkRasterPipeline::unpremul);
        }
        switch (fDir) {
            case Direction::kLinearToSRGB:
                p->append(SkRasterPipeline::to_srgb);
                break;
            case Direction::kSRGBToLinear:
                p->append_from_srgb(shaderIsOpaque ? kOpaque_SkAlphaType : kUnpremul_SkAlphaType);
                break;
        }
        if (!shaderIsOpaque) {
            p->append(SkRasterPipeline::premul);
        }
    }

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.write32(static_cast<uint32_t>(fDir));
    }

private:
    const Direction fDir;

    friend class SkColorFilter;
    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkSRGBGammaColorFilter::CreateProc(SkReadBuffer& buffer) {
    uint32_t dir = buffer.read32();
    if (dir <= 1) {
        return sk_sp<SkFlattenable>(new SkSRGBGammaColorFilter(static_cast<Direction>(dir)));
    }
    buffer.validate(false);
    return nullptr;
}

#ifndef SK_IGNORE_TO_STRING
void SkSRGBGammaColorFilter::toString(SkString* str) const {
    str->append("srgbgamma");
}
#endif

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

#include "SkModeColorFilter.h"

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkModeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSRGBGammaColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
