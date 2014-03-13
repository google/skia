/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKernel33MaskFilter_DEFINED
#define SkKernel33MaskFilter_DEFINED

#include "SkMaskFilter.h"

class SK_API SkKernel33ProcMaskFilter : public SkMaskFilter {
public:
    virtual uint8_t computeValue(uint8_t* const* srcRows) const = 0;

    virtual SkMask::Format getFormat() const SK_OVERRIDE;
    virtual bool filterMask(SkMask*, const SkMask&, const SkMatrix&,
                            SkIPoint*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()

protected:
    SkKernel33ProcMaskFilter(unsigned percent256 = 256)
        : fPercent256(percent256) {}
    SkKernel33ProcMaskFilter(SkReadBuffer& rb);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    int fPercent256;

    typedef SkMaskFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SK_API SkKernel33MaskFilter : public SkKernel33ProcMaskFilter {
public:
    static SkKernel33MaskFilter* Create(const int coeff[3][3], int shift, int percent256 = 256) {
        return SkNEW_ARGS(SkKernel33MaskFilter, (coeff, shift, percent256));
    }

    // override from SkKernel33ProcMaskFilter
    virtual uint8_t computeValue(uint8_t* const* srcRows) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkKernel33MaskFilter)

protected:
#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkKernel33MaskFilter(const int coeff[3][3], int shift, int percent256 = 256)
            : SkKernel33ProcMaskFilter(percent256) {
        memcpy(fKernel, coeff, 9 * sizeof(int));
        fShift = shift;
    }

private:
    int fKernel[3][3];
    int fShift;

    SkKernel33MaskFilter(SkReadBuffer& rb);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    typedef SkKernel33ProcMaskFilter INHERITED;
};

#endif
