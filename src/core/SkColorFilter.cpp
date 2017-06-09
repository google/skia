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
    SkPMColor dst, src = SkPreMultiplyColor(c);
    this->filterSpan(&src, 1, &dst);
    return SkUnPreMultiply::PMColorToColor(dst);
}

#include "SkRasterPipeline.h"
SkColor4f SkColorFilter::filterColor4f(const SkColor4f& c) const {
    SkPM4f dst, src = c.premul();

    SkSTArenaAlloc<128> alloc;
    SkRasterPipeline    pipeline(&alloc);

    pipeline.append(SkRasterPipeline::constant_color, &src);
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

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        fInner->filterSpan(shader, count, result);
        fOuter->filterSpan(result, count, result);
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
        return SkColorFilter::MakeComposeFilter(xformer->apply(fOuter.get()),
                                                xformer->apply(fInner.get()));
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

#include "SkModeColorFilter.h"

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkModeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
