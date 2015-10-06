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
    static SkColorMatrixFilter* Create(const SkColorMatrix& cm) {
        return new SkColorMatrixFilter(cm);
    }
    static SkColorMatrixFilter* Create(const SkScalar array[20]) {
        return new SkColorMatrixFilter(array);
    }

    void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const override;
    uint32_t getFlags() const override;
    bool asColorMatrix(SkScalar matrix[20]) const override;
    SkColorFilter* newComposed(const SkColorFilter*) const override;

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*) const override;
#endif

    struct State {
        int32_t fArray[20];
        int     fShift;
    };

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorMatrixFilter)

protected:
    explicit SkColorMatrixFilter(const SkColorMatrix&);
    explicit SkColorMatrixFilter(const SkScalar array[20]);
    void flatten(SkWriteBuffer&) const override;

private:
    SkColorMatrix   fMatrix;
    float           fTranspose[SkColorMatrix::kCount]; // for Sk4s

    typedef void (*Proc)(const State&, unsigned r, unsigned g, unsigned b,
                         unsigned a, int32_t result[4]);

    Proc        fProc;
    State       fState;
    uint32_t    fFlags;

    void initState(const SkScalar array[20]);

    typedef SkColorFilter INHERITED;
};

#endif
