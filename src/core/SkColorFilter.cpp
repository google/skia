/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkArenaAlloc.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"
#include "SkPM4f.h"
#include "SkNx.h"

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

bool SkColorFilter::appendStages(SkRasterPipeline* pipeline,
                                 SkColorSpace* dst,
                                 SkArenaAlloc* scratch,
                                 bool shaderIsOpaque) const {
    return this->onAppendStages(pipeline, dst, scratch, shaderIsOpaque);
}

bool SkColorFilter::onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*, bool) const {
    return false;
}

void SkColorFilter::filterSpan4f(const SkPM4f src[], int count, SkPM4f result[]) const {
    const int N = 128;
    SkPMColor tmp[N];
    while (count > 0) {
        int n = SkTMin(count, N);
        for (int i = 0; i < n; ++i) {
            tmp[i] = src[i].toPMColor();
        }
        this->filterSpan(tmp, n, tmp);
        for (int i = 0; i < n; ++i) {
            result[i] = SkPM4f::FromPMColor(tmp[i]);
        }
        src += n;
        result += n;
        count -= n;
    }
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    SkPMColor dst, src = SkPreMultiplyColor(c);
    this->filterSpan(&src, 1, &dst);
    return SkUnPreMultiply::PMColorToColor(dst);
}

SkColor4f SkColorFilter::filterColor4f(const SkColor4f& c) const {
    SkPM4f dst, src = c.premul();
    this->filterSpan4f(&src, 1, &dst);
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

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        fInner->filterSpan(shader, count, result);
        fOuter->filterSpan(result, count, result);
    }

    void filterSpan4f(const SkPM4f shader[], int count, SkPM4f result[]) const override {
        fInner->filterSpan4f(shader, count, result);
        fOuter->filterSpan4f(result, count, result);
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

///////////////////////////////////////////////////////////////////////////////////////////////////

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

class SkMixerColorFilter : public SkColorFilter {
public:
    SkMixerColorFilter(sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1, float weight)
        : fCF0(std::move(cf0)), fCF1(std::move(cf1)), fWeight(weight)
    {
        SkASSERT(fCF0 || fCF1);
        SkASSERT(fWeight >= 0 && fWeight <= 1);
    }

    uint32_t getFlags() const override {
        return fCF0->getFlags() & fCF1->getFlags();
    }

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        for (int i = 0; i < count; ++i) {
            SkPMColor r0, r1;
            fCF0->filterSpan(&shader[i], 1, &r0);
            fCF1->filterSpan(&shader[i], 1, &r1);
            result[i] = SkFastFourByteInterp256(r0, r1, (unsigned)(fWeight * 256));
        }
    }

    void filterSpan4f(const SkPM4f shader[], int count, SkPM4f result[]) const override {
        Sk4f w(fWeight);
        for (int i = 0; i < count; ++i) {
            SkPM4f r0, r1;
            fCF0->filterSpan4f(&shader[i], 1, &r0);
            fCF1->filterSpan4f(&shader[i], 1, &r1);
            result[i] = SkPM4f::From4f(r0.to4f() * w + r1.to4f() * (Sk4f(1) - w));
        }
    }

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override { str->append("mixer"); }
#endif

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext* context,
                                                   SkColorSpace* dstColorSpace) const override {
        return nullptr;
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMixerColorFilter)

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
    return MakeMixer(std::move(cf0), std::move(cf1), weight);
}

sk_sp<SkColorFilter> SkColorFilter::MakeMixer(sk_sp<SkColorFilter> cf0,
                                              sk_sp<SkColorFilter> cf1,
                                              float weight) {
    if (!cf0 && !cf1) {
        return nullptr;
    }
    if (!SkScalarIsFinite(weight)) {
        return nullptr;
    }

    weight = SkTMin(SkTMax(weight, 0.f), 1.f);
    return sk_sp<SkColorFilter>(new SkMixerColorFilter(std::move(cf0), std::move(cf1), weight));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkModeColorFilter.h"

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMixerColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkModeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
