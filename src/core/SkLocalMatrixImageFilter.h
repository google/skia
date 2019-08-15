/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixImageFilter_DEFINED
#define SkLocalMatrixImageFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "src/core/SkImageFilter_Base.h"

/**
 *  Wraps another imagefilter + matrix, such that using this filter will give the same result
 *  as using the wrapped filter with the matrix applied to its context.
 */
class SkLocalMatrixImageFilter : public SkImageFilter_Base {
public:
    static sk_sp<SkImageFilter> Make(const SkMatrix& localM, sk_sp<SkImageFilter> input);

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLocalMatrixImageFilter)

    SkLocalMatrixImageFilter(const SkMatrix& localM, sk_sp<SkImageFilter> input);

    // LocalMatrixImageFilter takes over the entire bounds process to properly fiddle with the
    // context's layer matrix.
    skif::IRect<In::kLayer, For::kInput> onFilterLayerBounds(
            const skif::IRect<In::kLayer, For::kOutput>& targetOutputBounds,
            const SkMatrix& layerMatrix,
            const skif::IRect<In::kLayer, For::kInput>& originalInput) const override;

    skif::IRect<In::kLayer, For::kOutput> onFilterOutputBounds(
            const skif::IRect<In::kLayer, For::kInput>& contentBounds,
            const SkMatrix& layerMatrix) const override;

    bool onCanHandleComplexCTM() const override { return true; }

    SkMatrix fLocalM;

    typedef SkImageFilter_Base INHERITED;
};

#endif
