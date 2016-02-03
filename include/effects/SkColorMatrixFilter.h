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
    static SkColorFilter* Create(const SkColorMatrix& cm) {
        return new SkColorMatrixFilter(cm);
    }
    static SkColorFilter* Create(const SkScalar array[20]) {
        return new SkColorMatrixFilter(array);
    }

    /**
     *  Create a colorfilter that multiplies the RGB channels by one color, and
     *  then adds a second color, pinning the result for each component to
     *  [0..255]. The alpha components of the mul and add arguments
     *  are ignored.
     */
    static SkColorFilter* CreateLightingFilter(SkColor mul, SkColor add);

    void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const override;
    uint32_t getFlags() const override;
    bool asColorMatrix(SkScalar matrix[20]) const override;
    SkColorFilter* newComposed(const SkColorFilter*) const override;

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*) const override;
#endif

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorMatrixFilter)

protected:
    explicit SkColorMatrixFilter(const SkColorMatrix&);
    explicit SkColorMatrixFilter(const SkScalar array[20]);
    void flatten(SkWriteBuffer&) const override;

private:
    SkColorMatrix   fMatrix;
    float           fTranspose[SkColorMatrix::kCount]; // for Sk4s
    uint32_t        fFlags;

    void initState(const SkScalar array[20]);

    typedef SkColorFilter INHERITED;
};

#endif
