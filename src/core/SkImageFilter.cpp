/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageFilter.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkLocalMatrixImageFilter.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <optional>
#include <utility>

///////////////////////////////////////////////////////////////////////////////////////////////////
// SkImageFilter - A number of the public APIs on SkImageFilter downcast to SkImageFilter_Base
// in order to perform their actual work.
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  Returns the number of inputs this filter will accept (some inputs can
 *  be NULL).
 */
int SkImageFilter::countInputs() const { return as_IFB(this)->fInputs.count(); }

/**
 *  Returns the input filter at a given index, or NULL if no input is
 *  connected.  The indices used are filter-specific.
 */
const SkImageFilter* SkImageFilter::getInput(int i) const {
    SkASSERT(i < this->countInputs());
    return as_IFB(this)->fInputs[i].get();
}

bool SkImageFilter::isColorFilterNode(SkColorFilter** filterPtr) const {
    return as_IFB(this)->onIsColorFilterNode(filterPtr);
}

SkIRect SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                    MapDirection direction, const SkIRect* inputRect) const {
    // The old filterBounds() function uses SkIRects that are defined in layer space so, while
    // we still are supporting it, bypass SkIF_B's new public filter bounds functions and go right
    // to the internal layer-space calculations.
    skif::Mapping mapping{ctm};
    if (kReverse_MapDirection == direction) {
        skif::LayerSpace<SkIRect> targetOutput(src);
        skif::LayerSpace<SkIRect> content(inputRect ? *inputRect : src);
        return SkIRect(as_IFB(this)->onGetInputLayerBounds(mapping, targetOutput, content));
    } else {
        SkASSERT(!inputRect);
        skif::LayerSpace<SkIRect> content(src);
        return SkIRect(as_IFB(this)->onGetOutputLayerBounds(mapping, content));
    }
}

SkRect SkImageFilter::computeFastBounds(const SkRect& src) const {
    if (0 == this->countInputs()) {
        return src;
    }
    SkRect combinedBounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    for (int i = 1; i < this->countInputs(); i++) {
        const SkImageFilter* input = this->getInput(i);
        if (input) {
            combinedBounds.join(input->computeFastBounds(src));
        } else {
            combinedBounds.join(src);
        }
    }
    return combinedBounds;
}

bool SkImageFilter::canComputeFastBounds() const {
    return !as_IFB(this)->affectsTransparentBlack();
}

bool SkImageFilter_Base::affectsTransparentBlack() const {
    if (this->onAffectsTransparentBlack()) {
        return true;
    } else if (this->ignoreInputsAffectsTransparentBlack()) {
        // TODO(skbug.com/14611): Automatically infer this from output bounds being finite
        return false;
    }
    for (int i = 0; i < this->countInputs(); i++) {
        const SkImageFilter* input = this->getInput(i);
        if (input && as_IFB(input)->affectsTransparentBlack()) {
            return true;
        }
    }
    return false;
}

bool SkImageFilter::asAColorFilter(SkColorFilter** filterPtr) const {
    SkASSERT(nullptr != filterPtr);
    if (!this->isColorFilterNode(filterPtr)) {
        return false;
    }
    if (nullptr != this->getInput(0) || as_CFB(*filterPtr)->affectsTransparentBlack()) {
        (*filterPtr)->unref();
        return false;
    }
    return true;
}

sk_sp<SkImageFilter> SkImageFilter::makeWithLocalMatrix(const SkMatrix& matrix) const {
    return SkLocalMatrixImageFilter::Make(matrix, this->refMe());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SkImageFilter_Base
///////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t next_image_filter_unique_id() {
    static std::atomic<int32_t> nextID{1};

    int32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == 0);
    return id;
}

SkImageFilter_Base::SkImageFilter_Base(sk_sp<SkImageFilter> const* inputs,
                                       int inputCount,
                                       std::optional<bool> usesSrc)
        : fUsesSrcInput(usesSrc.has_value() ? *usesSrc : false)
        , fUniqueID(next_image_filter_unique_id()) {
    fInputs.reset(inputCount);

    for (int i = 0; i < inputCount; ++i) {
        if (!usesSrc.has_value() && (!inputs[i] || as_IFB(inputs[i])->usesSource())) {
            fUsesSrcInput = true;
        }
        fInputs[i] = inputs[i];
    }
}

SkImageFilter_Base::~SkImageFilter_Base() {
    SkImageFilterCache::Get()->purgeByImageFilter(this);
}

std::pair<sk_sp<SkImageFilter>, std::optional<SkRect>>
SkImageFilter_Base::Unflatten(SkReadBuffer& buffer) {
    Common common;
    if (!common.unflatten(buffer, 1)) {
        return {nullptr, std::nullopt};
    } else {
        return {common.getInput(0), common.cropRect()};
    }
}

bool SkImageFilter_Base::Common::unflatten(SkReadBuffer& buffer, int expectedCount) {
    const int count = buffer.readInt();
    if (!buffer.validate(count >= 0)) {
        return false;
    }
    if (!buffer.validate(expectedCount < 0 || count == expectedCount)) {
        return false;
    }

#if defined(SK_BUILD_FOR_FUZZER)
    if (count > 4) {
        return false;
    }
#endif

    SkASSERT(fInputs.empty());
    for (int i = 0; i < count; i++) {
        fInputs.push_back(buffer.readBool() ? buffer.readImageFilter() : nullptr);
        if (!buffer.isValid()) {
            return false;
        }
    }

    if (buffer.isVersionLT(SkPicturePriv::kRemoveDeprecatedCropRect)) {
        static constexpr uint32_t kHasAll_CropEdge = 0x0F;
        SkRect rect;
        buffer.readRect(&rect);
        if (!buffer.isValid() || !buffer.validate(SkIsValidRect(rect))) {
            return false;
        }

        uint32_t flags = buffer.readUInt();
        if (!buffer.isValid() ||
            !buffer.validate(flags == 0x0 || flags == kHasAll_CropEdge)) {
            return false;
        }
        if (flags == kHasAll_CropEdge) {
            fCropRect = rect;
        }
    }
    return buffer.isValid();
}

void SkImageFilter_Base::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fInputs.count());
    for (int i = 0; i < fInputs.count(); i++) {
        const SkImageFilter* input = this->getInput(i);
        buffer.writeBool(input != nullptr);
        if (input != nullptr) {
            buffer.writeFlattenable(input);
        }
    }
}

skif::FilterResult SkImageFilter_Base::filterImage(const skif::Context& context) const {
    // TODO: Once all image filters operate on FilterResult, we should allow null source images.
    // Some filters that use a source input will produce non-transparent black values even if the
    // input is fully transparent (null). For now, at least allow filters that do not use the source
    // at all to still produce an output image.
    skif::FilterResult result;
    if (context.desiredOutput().isEmpty() ||
        (fUsesSrcInput && !context.source()) ||
        !context.mapping().layerMatrix().isFinite()) {
        return result;
    }

    uint32_t srcGenID = fUsesSrcInput ? context.source().image()->uniqueID() : 0;
    const SkIRect srcSubset = fUsesSrcInput ? context.source().image()->subset()
                                            : SkIRect::MakeWH(0, 0);

    SkImageFilterCacheKey key(fUniqueID,
                              context.mapping().layerMatrix(),
                              SkIRect(context.desiredOutput()),
                              srcGenID, srcSubset);
    if (context.cache() && context.cache()->get(key, &result)) {
        return result;
    }

    result = this->onFilterImage(context);

    if (context.gpuBacked()) {
        SkASSERT(!result.image() ||
                 result.image()->isGaneshBacked() ||
                 result.image()->isGraphiteBacked());
    }

    if (context.cache()) {
        context.cache()->set(key, this, result);
    }

    return result;
}

sk_sp<SkImage> SkImageFilter_Base::makeImageWithFilter(const skif::Functors& functors,
                                                       sk_sp<SkImage> src,
                                                       const SkIRect& subset,
                                                       const SkIRect& clipBounds,
                                                       SkIRect* outSubset,
                                                       SkIPoint* offset) const {
    if (!outSubset || !offset || !src->bounds().contains(subset)) {
        return nullptr;
    }

    sk_sp<SkImageFilterCache> cache(
            SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize));

    static const SkSurfaceProps kDefaultSurfaceProps;

    auto srcSpecialImage = functors.fMakeImageFunctor(subset, src, kDefaultSurfaceProps);
    if (!srcSpecialImage) {
        return nullptr;
    }

    // The filters operate in the local space of the src image, where (0,0) corresponds to the
    // subset's top left corner. But the clip bounds and any crop rects on the filters are in the
    // original coordinate system, so configure the CTM to correct crop rects and explicitly adjust
    // the clip bounds (since it is assumed to already be in image space).
    // TODO: Once all image filters support it, we can just use the subset's top left corner as
    // the source FilterResult's origin.
    skif::ContextInfo ctxInfo = {
            skif::Mapping(SkMatrix::Translate(-subset.x(), -subset.y())),
            skif::LayerSpace<SkIRect>(clipBounds.makeOffset(-subset.topLeft())),
            // TODO: Pass subset.topLeft() as the origin of the source FilterResult
            /* fSource= */skif::FilterResult{std::move(srcSpecialImage)},
            src->imageInfo().colorType(),
            src->imageInfo().colorSpace(),
            kDefaultSurfaceProps,
            cache.get()};
    const skif::Context context(ctxInfo, functors);

    sk_sp<SkSpecialImage> result = this->filterImage(context).imageAndOffset(context, offset);
    if (!result) {
        return nullptr;
    }

    // The output image and offset are relative to the subset rectangle, so the offset needs to
    // be shifted to put it in the correct spot with respect to the original coordinate system
    offset->fX += subset.x();
    offset->fY += subset.y();

    // Final clip against the exact clipBounds (the clip provided in the context gets adjusted
    // to account for pixel-moving filters so doesn't always exactly match when finished). The
    // clipBounds are translated into the clippedDstRect coordinate space, including the
    // result->subset() ensures that the result's image pixel origin does not affect results.
    SkIRect dstRect = result->subset();
    SkIRect clippedDstRect = dstRect;
    if (!clippedDstRect.intersect(clipBounds.makeOffset(result->subset().topLeft() - *offset))) {
        return nullptr;
    }

    // Adjust the geometric offset if the top-left corner moved as well
    offset->fX += (clippedDstRect.x() - dstRect.x());
    offset->fY += (clippedDstRect.y() - dstRect.y());
    *outSubset = clippedDstRect;
    return result->asImage();
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::getInputBounds(
        const skif::Mapping& mapping, const skif::DeviceSpace<SkIRect>& desiredOutput,
        const skif::ParameterSpace<SkRect>* knownContentBounds) const {
    // Map both the device-space desired coverage area and the known content bounds to layer space
    skif::LayerSpace<SkIRect> desiredBounds = mapping.deviceToLayer(desiredOutput);

    // If we have no known content bounds use the desired coverage area, because that is the most
    // conservative possibility.
    skif::LayerSpace<SkIRect> contentBounds =
            knownContentBounds ? mapping.paramToLayer(*knownContentBounds).roundOut()
                               : desiredBounds;

    // Process the layer-space desired output with the filter DAG to determine required input
    skif::LayerSpace<SkIRect> requiredInput = this->onGetInputLayerBounds(
            mapping, desiredBounds, contentBounds);
    // If we know what's actually going to be drawn into the layer, and we don't change transparent
    // black, then we can further restrict the layer to what the known content is
    // TODO (michaelludwig) - Once all filters are robust to tiling and transparency-affecting
    // FilterResults, there's no reason this can't always be applied, or be an expectation from the
    // leaf filters.
    if (knownContentBounds && !this->affectsTransparentBlack()) {
        if (!requiredInput.intersect(contentBounds)) {
            // Nothing would be output by the filter, so return empty rect
            return skif::LayerSpace<SkIRect>(SkIRect::MakeEmpty());
        }
    }
    return requiredInput;
}

skif::DeviceSpace<SkIRect> SkImageFilter_Base::getOutputBounds(
        const skif::Mapping& mapping, const skif::ParameterSpace<SkRect>& contentBounds) const {
    // Map the input content into the layer space where filtering will occur
    skif::LayerSpace<SkRect> layerContent = mapping.paramToLayer(contentBounds);
    // Determine the filter DAGs output bounds in layer space
    skif::LayerSpace<SkIRect> filterOutput = this->onGetOutputLayerBounds(
            mapping, layerContent.roundOut());
    // Map all the way to device space
    return mapping.layerToDevice(filterOutput);
}

SkImageFilter_Base::MatrixCapability SkImageFilter_Base::getCTMCapability() const {
    MatrixCapability result = this->onGetCTMCapability();
    const int count = this->countInputs();
    for (int i = 0; i < count; ++i) {
        if (const SkImageFilter_Base* input = as_IFB(this->getInput(i))) {
            result = std::min(result, input->getCTMCapability());
        }
    }
    return result;
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::getChildInputLayerBounds(
        int index,
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The required input for childFilter filter, or 'contentBounds' intersected with 'desiredOutput'
    // if the filter is null and the source image is used (i.e. the identity filter).
    const SkImageFilter* childFilter = this->getInput(index);
    if (childFilter) {
        return as_IFB(childFilter)->onGetInputLayerBounds(mapping, desiredOutput, contentBounds);
    } else {
        // TODO: The leaf bounds should be the contentBounds intersected with the desired output,
        // but currently legacy filters often discard or replace the contentBounds with the
        // desiredOutput. We also need to be robust to unbounded content (i.e. when it's unknown).
        // See skbug.com/10984
        return desiredOutput;
    }
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::getChildOutputLayerBounds(
        int index,
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The output for just childFilter filter, or 'contentBounds' if the filter is null and
    // the source image is used (i.e. the identity filter applied to the source).
    const SkImageFilter* childFilter = this->getInput(index);
    return childFilter ? as_IFB(childFilter)->onGetOutputLayerBounds(mapping, contentBounds)
                       : contentBounds;
}

skif::FilterResult SkImageFilter_Base::getChildOutput(int index, const skif::Context& ctx) const {
    const SkImageFilter* input = this->getInput(index);
    return input ? as_IFB(input)->filterImage(ctx) : ctx.source();
}

void SkImageFilter_Base::PurgeCache() {
    SkImageFilterCache::Get()->purge();
}
