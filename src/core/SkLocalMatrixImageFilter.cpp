/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkLocalMatrixImageFilter.h"

#include "include/core/SkImageFilter.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

sk_sp<SkImageFilter> SkLocalMatrixImageFilter::Make(const SkMatrix& localMatrix,
                                                    sk_sp<SkImageFilter> input) {
    if (!input) {
        return nullptr;
    }
    if (localMatrix.isIdentity()) {
        return input;
    }

    MatrixCapability inputCapability = as_IFB(input)->getCTMCapability();
    if ((inputCapability == MatrixCapability::kTranslate && !localMatrix.isTranslate()) ||
        (inputCapability == MatrixCapability::kScaleTranslate && !localMatrix.isScaleTranslate())) {
        // Nothing we can do at this point
        return nullptr;
    }

    SkMatrix invLocal;
    if (!localMatrix.invert(&invLocal)) {
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkLocalMatrixImageFilter(localMatrix, invLocal, input));
}

sk_sp<SkFlattenable> SkLocalMatrixImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix lm;
    buffer.readMatrix(&lm);
    return SkLocalMatrixImageFilter::Make(lm, common.getInput(0));
}

void SkLocalMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeMatrix(fLocalMatrix);
    // fInvLocalMatrix will be reconstructed
}

///////////////////////////////////////////////////////////////////////////////

skif::Mapping SkLocalMatrixImageFilter::localMapping(const skif::Mapping& mapping) const {
    skif::Mapping localMapping = mapping;
    localMapping.concatLocal(fLocalMatrix);
    return localMapping;
}

skif::FilterResult SkLocalMatrixImageFilter::onFilterImage(const skif::Context& ctx) const {
    skif::Mapping localMapping = this->localMapping(ctx.mapping());
    return this->getChildOutput(0, ctx.withNewMapping(localMapping));
}

skif::LayerSpace<SkIRect> SkLocalMatrixImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // The local matrix changes 'mapping' by adjusting the parameter space of the image filter, but
    // both 'desiredOutput' and 'contentBounds' have already been transformed to the consistent
    // layer space. They remain unchanged with the new mapping.
    return this->getChildInputLayerBounds(0, this->localMapping(mapping),
                                          desiredOutput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkLocalMatrixImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    return this->getChildOutputLayerBounds(0, this->localMapping(mapping), contentBounds);
}

SkRect SkLocalMatrixImageFilter::computeFastBounds(const SkRect& bounds) const {
    // In onGet[Input|Output]LayerBounds, there is a Mapping that can be adjusted by the
    // local matrix, so their layer-space parameters do not need to be modified. Since
    // computeFastBounds() takes no matrix, it always operates as if it has the identity mapping.
    //
    // In order to match the behavior of onGetInputLayerBounds, we map 'bounds' by the inverse of
    // the local matrix, pass that to the child, and then map the result by the local matrix.
    // TODO: Implementing computeFastBounds in terms of onGetOutputLayerBounds() trivially removes
    // this complexity.
    SkRect localBounds = fInvLocalMatrix.mapRect(bounds);
    return fLocalMatrix.mapRect(this->getInput(0)->computeFastBounds(localBounds));
}
