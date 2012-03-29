
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkKernel33MaskFilter_DEFINED
#define SkKernel33MaskFilter_DEFINED

#include "SkMaskFilter.h"

class SkKernel33ProcMaskFilter : public SkMaskFilter {
public:
    SkKernel33ProcMaskFilter(unsigned percent256 = 256)
        : fPercent256(percent256) {}

    virtual uint8_t computeValue(uint8_t* const* srcRows) = 0;
    
    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat();
    virtual bool filterMask(SkMask*, const SkMask&, const SkMatrix&, SkIPoint*);

protected:
    SkKernel33ProcMaskFilter(SkFlattenableReadBuffer& rb);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    int fPercent256;
    
    typedef SkMaskFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkKernel33MaskFilter : public SkKernel33ProcMaskFilter {
public:
    SkKernel33MaskFilter(const int coeff[3][3], int shift, int percent256 = 256)
            : SkKernel33ProcMaskFilter(percent256) {
        memcpy(fKernel, coeff, 9 * sizeof(int));
        fShift = shift;
    }
    
    // override from SkKernel33ProcMaskFilter
    virtual uint8_t computeValue(uint8_t* const* srcRows);
    
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkKernel33MaskFilter)
    
private:
    int fKernel[3][3];
    int fShift;

    SkKernel33MaskFilter(SkFlattenableReadBuffer& rb);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    
    typedef SkKernel33ProcMaskFilter INHERITED;
};

#endif
