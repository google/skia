/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrixFilter_DEFINED
#define SkColorMatrixFilter_DEFINED

#include "SkColorFilter.h"
#include "SkColorMatrix.h"

class SK_API SkColorMatrixFilter : public SkColorFilter {
public:
    explicit SkColorMatrixFilter(const SkColorMatrix&);
    SkColorMatrixFilter(const SkScalar array[20]);

    // overrides from SkColorFilter
    virtual void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const SK_OVERRIDE;
    virtual void filterSpan16(const uint16_t src[], int count, uint16_t[]) const SK_OVERRIDE;
    virtual uint32_t getFlags() const SK_OVERRIDE;
    virtual bool asColorMatrix(SkScalar matrix[20]) const SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual GrEffectRef* asNewEffect(GrContext*) const SK_OVERRIDE;
#endif

    struct State {
        int32_t fArray[20];
        int     fShift;
    };

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorMatrixFilter)

protected:
    SkColorMatrixFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkColorMatrix fMatrix;

    typedef void (*Proc)(const State&, unsigned r, unsigned g, unsigned b,
                         unsigned a, int32_t result[4]);

    Proc        fProc;
    State       fState;
    uint32_t    fFlags;

    void initState(const SkScalar array[20]);

    typedef SkColorFilter INHERITED;
};

#endif
