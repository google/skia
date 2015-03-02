/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"

bool SkColorFilter::asColorMode(SkColor* color, SkXfermode::Mode* mode) const {
    return false;
}

bool SkColorFilter::asColorMatrix(SkScalar matrix[20]) const {
    return false;
}

bool SkColorFilter::asComponentTable(SkBitmap*) const {
    return false;
}

void SkColorFilter::filterSpan16(const uint16_t s[], int count, uint16_t d[]) const {
    SkASSERT(this->getFlags() & SkColorFilter::kHasFilter16_Flag);
    SkDEBUGFAIL("missing implementation of SkColorFilter::filterSpan16");

    if (d != s) {
        memcpy(d, s, count * sizeof(uint16_t));
    }
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    SkPMColor dst, src = SkPreMultiplyColor(c);
    this->filterSpan(&src, 1, &dst);
    return SkUnPreMultiply::PMColorToColor(dst);
}

GrFragmentProcessor* SkColorFilter::asFragmentProcessor(GrContext*) const {
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkComposeColorFilter : public SkColorFilter {
public:
    SkComposeColorFilter(SkColorFilter* outer, SkColorFilter* inner)
        : fOuter(SkRef(outer))
        , fInner(SkRef(inner))
    {}
    
    uint32_t getFlags() const SK_OVERRIDE {
        // Can only claim alphaunchanged and 16bit support if both our proxys do.
        return fOuter->getFlags() & fInner->getFlags();
    }
    
    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const SK_OVERRIDE {
        fInner->filterSpan(shader, count, result);
        fOuter->filterSpan(result, count, result);
    }
    
    void filterSpan16(const uint16_t shader[], int count, uint16_t result[]) const SK_OVERRIDE {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        fInner->filterSpan16(shader, count, result);
        fOuter->filterSpan16(result, count, result);
    }
    
#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const SK_OVERRIDE {
        SkString outerS, innerS;
        fOuter->toString(&outerS);
        fInner->toString(&innerS);
        str->appendf("SkComposeColorFilter: outer(%s) inner(%s)", outerS.c_str(), innerS.c_str());
    }
#endif

#if 0   // TODO: should we support composing the fragments?
#if SK_SUPPORT_GPU
    GrFragmentProcessor* asFragmentProcessor(GrContext*) const SK_OVERRIDE;
#endif
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeColorFilter)
    
protected:
    void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE {
        buffer.writeFlattenable(fOuter);
        buffer.writeFlattenable(fInner);
    }
    
private:
    SkAutoTUnref<SkColorFilter> fOuter;
    SkAutoTUnref<SkColorFilter> fInner;

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
    if (NULL == composition) {
        composition = SkNEW_ARGS(SkComposeColorFilter, (outer, inner));
    }
    return composition;
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

