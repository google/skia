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
    static sk_sp<SkImageFilter> Make(const SkMatrix& localMatrix, sk_sp<SkImageFilter> input);

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLocalMatrixImageFilter)

    SkLocalMatrixImageFilter(const SkMatrix& localMatrix,
                             const SkMatrix& invLocalMatrix,
                             sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fLocalMatrix{localMatrix}
            , fInvLocalMatrix{invLocalMatrix} {}

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context& ctx) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping&,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping&,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    skif::Mapping localMapping(const skif::Mapping&) const;

    // NOTE: This is not a ParameterSpace<SkMatrix> like that of SkMatrixTransformImageFilter.
    // It's a bit pedantic, but does impact the math. A parameter-space transform has to be modified
    // to represent a layer-space transform: (L*P*L^-1); while this local matrix changes L directly
    // to L*P for its child filter.
    SkMatrix fLocalMatrix;
    SkMatrix fInvLocalMatrix;
};

#endif
