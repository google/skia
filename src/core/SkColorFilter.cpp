/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

bool SkColorFilter::asColorMode(SkColor* color, SkXfermode::Mode* mode) const {
    return false;
}

bool SkColorFilter::asColorMatrix(SkScalar matrix[20]) const {
    return false;
}

bool SkColorFilter::asComponentTable(SkBitmap*) const {
    return false;
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    SkPMColor dst, src = SkPreMultiplyColor(c);
    this->filterSpan(&src, 1, &dst);
    return SkUnPreMultiply::PMColorToColor(dst);
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
        // Can only claim alphaunchanged and 16bit support if both our proxys do.
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
        str->appendf("SkComposeColorFilter: outer(%s) inner(%s)", outerS.c_str(), innerS.c_str());
    }
#endif

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext* context) const override {
        SkAutoTUnref<const GrFragmentProcessor> innerFP(fInner->asFragmentProcessor(context));
        SkAutoTUnref<const GrFragmentProcessor> outerFP(fOuter->asFragmentProcessor(context));
        if (!innerFP || !outerFP) {
            return nullptr;
        }
        const GrFragmentProcessor* series[] = { innerFP, outerFP };
        return GrFragmentProcessor::RunInSeries(series, 2);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeColorFilter)
    
protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fOuter);
        buffer.writeFlattenable(fInner);
    }
    
private:
    SkComposeColorFilter(SkColorFilter* outer, SkColorFilter* inner, int composedFilterCount)
        : fOuter(SkRef(outer))
        , fInner(SkRef(inner))
        , fComposedFilterCount(composedFilterCount)
    {
        SkASSERT(composedFilterCount >= 2);
        SkASSERT(composedFilterCount <= SK_MAX_COMPOSE_COLORFILTER_COUNT);
    }

    int privateComposedFilterCount() const override {
        return fComposedFilterCount;
    }

    SkAutoTUnref<SkColorFilter> fOuter;
    SkAutoTUnref<SkColorFilter> fInner;
    const int                   fComposedFilterCount;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

SkFlattenable* SkComposeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkAutoTUnref<SkColorFilter> outer(buffer.readColorFilter());
    SkAutoTUnref<SkColorFilter> inner(buffer.readColorFilter());
    return CreateComposeFilter(outer, inner);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkColorFilter::CreateComposeFilter(SkColorFilter* outer, SkColorFilter* inner) {
    if (!outer) {
        return SkSafeRef(inner);
    }
    if (!inner) {
        return SkSafeRef(outer);
    }

    // Give the subclass a shot at a more optimal composition...
    SkColorFilter* composition = outer->newComposed(inner);
    if (composition) {
        return composition;
    }

    int count = inner->privateComposedFilterCount() + outer->privateComposedFilterCount();
    if (count > SK_MAX_COMPOSE_COLORFILTER_COUNT) {
        return nullptr;
    }
    return new SkComposeColorFilter(outer, inner, count);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

