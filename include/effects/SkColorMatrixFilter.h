
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

class SkColorMatrixFilter : public SkColorFilter {
public:
    SkColorMatrixFilter();
    explicit SkColorMatrixFilter(const SkColorMatrix&);
    SkColorMatrixFilter(const SkScalar array[20]);

    void setMatrix(const SkColorMatrix&);
    void setArray(const SkScalar array[20]);

    // overrides from SkColorFilter
    virtual void filterSpan(const SkPMColor src[], int count, SkPMColor[]) SK_OVERRIDE;
    virtual void filterSpan16(const uint16_t src[], int count, uint16_t[]) SK_OVERRIDE;
    virtual uint32_t getFlags() SK_OVERRIDE;
    virtual bool asColorMatrix(SkScalar matrix[20]) SK_OVERRIDE;

    // overrides for SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer& buffer) SK_OVERRIDE;

    struct State {
        int32_t fArray[20];
        int     fShift;
        int32_t fResult[4];
    };

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer);

    SK_DECLARE_FLATTENABLE_REGISTRAR()

protected:
    // overrides for SkFlattenable
    virtual Factory getFactory();

    SkColorMatrixFilter(SkFlattenableReadBuffer& buffer);

private:

    typedef void (*Proc)(State*, unsigned r, unsigned g, unsigned b,
                         unsigned a);

    Proc        fProc;
    State       fState;
    uint32_t    fFlags;

    void setup(const SkScalar array[20]);

    typedef SkColorFilter INHERITED;
};

#endif
