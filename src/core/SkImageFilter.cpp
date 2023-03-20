/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkSafe32.h"
#include "src/core/SkFuzzLogging.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkLocalMatrixImageFilter.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#if defined(SK_GANESH)
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#endif
#include <atomic>

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
        if (as_IFB(this)->cropRectIsSet()) {
           skif::LayerSpace<SkIRect> outputCrop = mapping.paramToLayer(
                    skif::ParameterSpace<SkRect>(as_IFB(this)->getCropRect().rect())).roundOut();
            // Just intersect directly; unlike the forward-mapping case, since we start with the
            // external target output, there's no need to embiggen due to affecting trans. black
            if (!targetOutput.intersect(outputCrop)) {
                // Nothing would be output by the filter, so return empty rect
                return SkIRect::MakeEmpty();
            }
        }
        skif::LayerSpace<SkIRect> content(inputRect ? *inputRect : src);
        return SkIRect(as_IFB(this)->onGetInputLayerBounds(mapping, targetOutput, content));
    } else {
        SkASSERT(!inputRect);
        skif::LayerSpace<SkIRect> content(src);
        skif::LayerSpace<SkIRect> output = as_IFB(this)->onGetOutputLayerBounds(mapping, content);
        // Manually apply the crop rect for now, until cropping is performed by a dedicated SkIF.
        SkIRect dst;
        as_IFB(this)->getCropRect().applyTo(
                SkIRect(output), ctm, as_IFB(this)->onAffectsTransparentBlack(), &dst);
        return dst;
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
                                       int inputCount, const SkRect* cropRect)
        : fUsesSrcInput(false)
        , fCropRect(cropRect)
        , fUniqueID(next_image_filter_unique_id()) {
    fInputs.reset(inputCount);

    for (int i = 0; i < inputCount; ++i) {
        if (!inputs[i] || as_IFB(inputs[i])->fUsesSrcInput) {
            fUsesSrcInput = true;
        }
        fInputs[i] = inputs[i];
    }
}

SkImageFilter_Base::~SkImageFilter_Base() {
    SkImageFilterCache::Get()->purgeByImageFilter(this);
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
    SkRect rect;
    buffer.readRect(&rect);
    if (!buffer.isValid() || !buffer.validate(SkIsValidRect(rect))) {
        return false;
    }

    uint32_t flags = buffer.readUInt();
    if (!buffer.isValid() ||
        !buffer.validate(flags == 0x0 || flags == CropRect::kHasAll_CropEdge)) {
        return false;
    }
    fCropRect = CropRect(flags ? &rect : nullptr);
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
    buffer.writeRect(fCropRect.rect());
    buffer.writeUInt(fCropRect.flags());
}

skif::FilterResult SkImageFilter_Base::filterImage(const skif::Context& context) const {
    // TODO (michaelludwig) - Old filters have an implicit assumption that the source image
    // (originally passed separately) has an origin of (0, 0). SkComposeImageFilter makes an effort
    // to ensure that remains the case. Once everyone uses the new type systems for bounds, non
    // (0, 0) source origins will be easy to support.
    SkASSERT(context.source().layerBounds().left() == 0 &&
             context.source().layerBounds().top() == 0 &&
             context.source().layerBounds().right() == context.source().image()->width() &&
             context.source().layerBounds().bottom() == context.source().image()->height());

    skif::FilterResult result;
    if (context.desiredOutput().isEmpty() || !context.isValid()) {
        return result;
    }

    uint32_t srcGenID = fUsesSrcInput ? context.sourceImage()->uniqueID() : 0;
    const SkIRect srcSubset = fUsesSrcInput ? context.sourceImage()->subset()
                                            : SkIRect::MakeWH(0, 0);

    SkImageFilterCacheKey key(fUniqueID, context.mapping().layerMatrix(), context.clipBounds(),
                              srcGenID, srcSubset);
    if (context.cache() && context.cache()->get(key, &result)) {
        return result;
    }

    result = this->onFilterImage(context);

    if (context.gpuBacked()) {
        SkASSERT(!result.image() || result.image()->isTextureBacked());
    }

    if (context.cache()) {
        context.cache()->set(key, this, result);
    }

    return result;
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::getInputBounds(
        const skif::Mapping& mapping, const skif::DeviceSpace<SkIRect>& desiredOutput,
        const skif::ParameterSpace<SkRect>* knownContentBounds) const {
    // Map both the device-space desired coverage area and the known content bounds to layer space
    skif::LayerSpace<SkIRect> desiredBounds = mapping.deviceToLayer(desiredOutput);

    // TODO (michaelludwig) - To be removed once cropping is its own filter, since then an output
    // crop would automatically adjust the required input of its child filter in this same way.
    if (this->cropRectIsSet()) {
        skif::LayerSpace<SkIRect> outputCrop =
                mapping.paramToLayer(skif::ParameterSpace<SkRect>(fCropRect.rect())).roundOut();
        if (!desiredBounds.intersect(outputCrop)) {
            // Nothing would be output by the filter, so return empty rect
            return skif::LayerSpace<SkIRect>(SkIRect::MakeEmpty());
        }
    }

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
    // TODO (michaelludwig) - This logic could be moved into visitInputLayerBounds() when an input
    // filter is null. Additionally, once all filters are robust to FilterResults with tile modes,
    // we can always restrict the required input by content bounds since any additional transparent
    // black is handled when producing the output result and sampling outside the input image with
    // a decal tile mode.
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
    // FIXME (michaelludwig) - To be removed once cropping is isolated, but remain consistent with
    // old filterBounds(kForward) behavior.
    SkIRect dst;
    as_IFB(this)->getCropRect().applyTo(
            SkIRect(filterOutput), mapping.layerMatrix(),
            as_IFB(this)->onAffectsTransparentBlack(), &dst);

    // Map all the way to device space
    return mapping.layerToDevice(skif::LayerSpace<SkIRect>(dst));
}

// TODO (michaelludwig) - Default to using the old onFilterImage, as filters are updated one by one.
// Once the old function is gone, this onFilterImage() will be made a pure virtual.
skif::FilterResult SkImageFilter_Base::onFilterImage(const skif::Context& context) const {
    SkIPoint origin = {0, 0};
    auto image = this->onFilterImage(context, &origin);
    return skif::FilterResult(std::move(image), skif::LayerSpace<SkIPoint>(origin));
}

SkImageFilter_Base::MatrixCapability SkImageFilter_Base::getCTMCapability() const {
    MatrixCapability result = this->onGetCTMCapability();
    // CropRects need to apply in the source coordinate system, but are not aware of complex CTMs
    // when performing clipping. For a simple fix, any filter with a crop rect set cannot support
    // more than scale+translate CTMs until that's updated.
    if (this->cropRectIsSet()) {
        result = std::min(result, MatrixCapability::kScaleTranslate);
    }
    const int count = this->countInputs();
    for (int i = 0; i < count; ++i) {
        if (const SkImageFilter_Base* input = as_IFB(this->getInput(i))) {
            result = std::min(result, input->getCTMCapability());
        }
    }
    return result;
}

void SkImageFilter_Base::CropRect::applyTo(const SkIRect& imageBounds, const SkMatrix& ctm,
                                           bool embiggen, SkIRect* cropped) const {
    *cropped = imageBounds;
    if (fFlags) {
        SkRect devCropR;
        ctm.mapRect(&devCropR, fRect);
        SkIRect devICropR = devCropR.roundOut();

        // Compute the left/top first, in case we need to modify the right/bottom for a missing edge
        if (fFlags & kHasLeft_CropEdge) {
            if (embiggen || devICropR.fLeft > cropped->fLeft) {
                cropped->fLeft = devICropR.fLeft;
            }
        } else {
            devICropR.fRight = Sk32_sat_add(cropped->fLeft, devICropR.width());
        }
        if (fFlags & kHasTop_CropEdge) {
            if (embiggen || devICropR.fTop > cropped->fTop) {
                cropped->fTop = devICropR.fTop;
            }
        } else {
            devICropR.fBottom = Sk32_sat_add(cropped->fTop, devICropR.height());
        }
        if (fFlags & kHasWidth_CropEdge) {
            if (embiggen || devICropR.fRight < cropped->fRight) {
                cropped->fRight = devICropR.fRight;
            }
        }
        if (fFlags & kHasHeight_CropEdge) {
            if (embiggen || devICropR.fBottom < cropped->fBottom) {
                cropped->fBottom = devICropR.fBottom;
            }
        }
    }
}

bool SkImageFilter_Base::applyCropRect(const Context& ctx, const SkIRect& srcBounds,
                                       SkIRect* dstBounds) const {
    SkIRect tmpDst = this->onFilterNodeBounds(srcBounds, ctx.ctm(), kForward_MapDirection, nullptr);
    fCropRect.applyTo(tmpDst, ctx.ctm(), this->onAffectsTransparentBlack(), dstBounds);
    // Intersect against the clip bounds, in case the crop rect has
    // grown the bounds beyond the original clip. This can happen for
    // example in tiling, where the clip is much smaller than the filtered
    // primitive. If we didn't do this, we would be processing the filter
    // at the full crop rect size in every tile.
    return dstBounds->intersect(ctx.clipBounds());
}

// Return a larger (newWidth x newHeight) copy of 'src' with black padding
// around it.
static sk_sp<SkSpecialImage> pad_image(SkSpecialImage* src, const SkImageFilter_Base::Context& ctx,
                                       int newWidth, int newHeight, int offX, int offY) {
    // We would like to operate in the source's color space (so that we return an "identical"
    // image, other than the padding. To achieve that, we'd create a new context using
    // src->getColorSpace() to replace ctx.colorSpace().

    // That fails in at least two ways. For formats that are texturable but not renderable (like
    // F16 on some ES implementations), we can't create a surface to do the work. For sRGB, images
    // may be tagged with an sRGB color space (which leads to an sRGB config in makeSurface). But
    // the actual config of that sRGB image on a device with no sRGB support is non-sRGB.
    //
    // Rather than try to special case these situations, we execute the image padding in the
    // destination color space. This should not affect the output of the DAG in (almost) any case,
    // because the result of this call is going to be used as an input, where it would have been
    // switched to the destination space anyway. The one exception would be a filter that expected
    // to consume unclamped F16 data, but the padded version of the image is pre-clamped to 8888.
    // We can revisit this logic if that ever becomes an actual problem.
    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(SkISize::Make(newWidth, newHeight)));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    src->draw(canvas, offX, offY);

    return surf->makeImageSnapshot();
}

sk_sp<SkSpecialImage> SkImageFilter_Base::applyCropRectAndPad(const Context& ctx,
                                                              SkSpecialImage* src,
                                                              SkIPoint* srcOffset,
                                                              SkIRect* bounds) const {
    const SkIRect srcBounds = SkIRect::MakeXYWH(srcOffset->x(), srcOffset->y(),
                                                src->width(), src->height());

    if (!this->applyCropRect(ctx, srcBounds, bounds)) {
        return nullptr;
    }

    if (srcBounds.contains(*bounds)) {
        return sk_sp<SkSpecialImage>(SkRef(src));
    } else {
        sk_sp<SkSpecialImage> img(pad_image(src, ctx, bounds->width(), bounds->height(),
                                            Sk32_sat_sub(srcOffset->x(), bounds->x()),
                                            Sk32_sat_sub(srcOffset->y(), bounds->y())));
        *srcOffset = SkIPoint::Make(bounds->x(), bounds->y());
        return img;
    }
}

// NOTE: The new onGetOutputLayerBounds() and onGetInputLayerBounds() default to calling into the
// deprecated onFilterBounds and onFilterNodeBounds. While these functions are not tagged, they do
// match the documented default behavior for the new bounds functions.
SkIRect SkImageFilter_Base::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                           MapDirection dir, const SkIRect* inputRect) const {
    if (this->countInputs() < 1) {
        return src;
    }

    SkIRect totalBounds;
    for (int i = 0; i < this->countInputs(); ++i) {
        const SkImageFilter* filter = this->getInput(i);
        SkIRect rect = filter ? filter->filterBounds(src, ctm, dir, inputRect) : src;
        if (0 == i) {
            totalBounds = rect;
        } else {
            totalBounds.join(rect);
        }
    }

    return totalBounds;
}

SkIRect SkImageFilter_Base::onFilterNodeBounds(const SkIRect& src, const SkMatrix&,
                                               MapDirection, const SkIRect*) const {
    return src;
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::visitInputLayerBounds(
        const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    if (this->countInputs() < 1) {
        // TODO (michaelludwig) - if a filter doesn't have any inputs, it doesn't need any
        // implicit source image, so arguably we could return an empty rect here. 'desiredOutput' is
        // consistent with original behavior, so empty bounds may have unintended side effects
        // but should be explored later. Of note is that right now an empty layer bounds assumes
        // that there's no need to filter on restore, which is not the case for these filters.
        return desiredOutput;
    }

    skif::LayerSpace<SkIRect> netInput;
    for (int i = 0; i < this->countInputs(); ++i) {
        const SkImageFilter* filter = this->getInput(i);
        // The required input for this input filter, or 'targetOutput' if the filter is null and
        // the source image is used (so must be sized to cover 'targetOutput').
        // TODO (michaelludwig) - Right now contentBounds is applied conditionally at the end of
        // the root getInputLayerBounds() based on affecting transparent black. Once that bit only
        // changes output behavior, we can have the required bounds for a null input filter be the
        // intersection of the desired output and the content bounds.
        skif::LayerSpace<SkIRect> requiredInput =
                filter ? as_IFB(filter)->onGetInputLayerBounds(mapping, desiredOutput,
                                                               contentBounds)
                       : desiredOutput;
        // Accumulate with all other filters
        if (i == 0) {
            netInput = requiredInput;
        } else {
            netInput.join(requiredInput);
        }
    }
    return netInput;
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::visitOutputLayerBounds(
        const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& contentBounds) const {
    if (this->countInputs() < 1) {
        // TODO (michaelludwig) - if a filter doesn't have any inputs, it presumably is determining
        // its output size from something other than the implicit source contentBounds, in which
        // case it shouldn't be calling this helper function, so explore adding an unreachable test
        return contentBounds;
    }

    skif::LayerSpace<SkIRect> netOutput;
    for (int i = 0; i < this->countInputs(); ++i) {
        const SkImageFilter* filter = this->getInput(i);
        // The output for just this input filter, or 'contentBounds' if the filter is null and
        // the source image is used (i.e. the identity filter applied to the source).
        skif::LayerSpace<SkIRect> output =
                filter ? as_IFB(filter)->onGetOutputLayerBounds(mapping, contentBounds)
                       : contentBounds;
        // Accumulate with all other filters
        if (i == 0) {
            netOutput = output;
        } else {
            netOutput.join(output);
        }
    }
    return netOutput;
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::onGetInputLayerBounds(
        const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds, VisitChildren recurse) const {
    // Call old functions for now since they may have been overridden by a subclass that's not been
    // updated yet; eventually this will be a pure virtual and impls control visiting children
    SkIRect content = SkIRect(contentBounds);
    SkIRect input = this->onFilterNodeBounds(SkIRect(desiredOutput), mapping.layerMatrix(),
                                             kReverse_MapDirection, &content);
    if (recurse == VisitChildren::kYes) {
        SkIRect aggregate = this->onFilterBounds(input, mapping.layerMatrix(),
                                                 kReverse_MapDirection, &input);
        return skif::LayerSpace<SkIRect>(aggregate);
    } else {
        return skif::LayerSpace<SkIRect>(input);
    }
}

skif::LayerSpace<SkIRect> SkImageFilter_Base::onGetOutputLayerBounds(
        const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Call old functions for now; eventually this will be a pure virtual
    SkIRect aggregate = this->onFilterBounds(SkIRect(contentBounds), mapping.layerMatrix(),
                                             kForward_MapDirection, nullptr);
    SkIRect output = this->onFilterNodeBounds(aggregate, mapping.layerMatrix(),
                                              kForward_MapDirection, nullptr);
    return skif::LayerSpace<SkIRect>(output);
}

skif::FilterResult SkImageFilter_Base::filterInput(int index, const skif::Context& ctx) const {
    const SkImageFilter* input = this->getInput(index);
    if (!input) {
        // Null image filters late bind to the source image
        return ctx.source();
    }

    skif::FilterResult result = as_IFB(input)->filterImage(this->mapContext(ctx));
    SkASSERT(!result.image() || ctx.gpuBacked() == result.image()->isTextureBacked());

    return result;
}

SkImageFilter_Base::Context SkImageFilter_Base::mapContext(const Context& ctx) const {
    // We don't recurse through the child input filters because that happens automatically
    // as part of the filterImage() evaluation. In this case, we want the bounds for the
    // edge from this node to its children, without the effects of the child filters.
    skif::LayerSpace<SkIRect> childOutput = this->onGetInputLayerBounds(
            ctx.mapping(), ctx.desiredOutput(), ctx.desiredOutput(), VisitChildren::kNo);
    return ctx.withNewDesiredOutput(childOutput);
}

#if defined(SK_GANESH)
sk_sp<SkSpecialImage> SkImageFilter_Base::DrawWithFP(GrRecordingContext* rContext,
                                                     std::unique_ptr<GrFragmentProcessor> fp,
                                                     const SkIRect& bounds,
                                                     SkColorType colorType,
                                                     const SkColorSpace* colorSpace,
                                                     const SkSurfaceProps& surfaceProps,
                                                     GrSurfaceOrigin surfaceOrigin,
                                                     GrProtected isProtected) {
    GrImageInfo info(SkColorTypeToGrColorType(colorType),
                     kPremul_SkAlphaType,
                     sk_ref_sp(colorSpace),
                     bounds.size());

    auto sfc = rContext->priv().makeSFC(info,
                                        "ImageFilterBase_DrawWithFP",
                                        SkBackingFit::kApprox,
                                        1,
                                        GrMipmapped::kNo,
                                        isProtected,
                                        surfaceOrigin);
    if (!sfc) {
        return nullptr;
    }

    SkIRect dstIRect = SkIRect::MakeWH(bounds.width(), bounds.height());
    SkRect srcRect = SkRect::Make(bounds);
    sfc->fillRectToRectWithFP(srcRect, dstIRect, std::move(fp));

    return SkSpecialImage::MakeDeferredFromGpu(rContext,
                                               dstIRect,
                                               kNeedNewImageUniqueID_SpecialImage,
                                               sfc->readSurfaceView(),
                                               sfc->colorInfo(),
                                               surfaceProps);
}

sk_sp<SkSpecialImage> SkImageFilter_Base::ImageToColorSpace(SkSpecialImage* src,
                                                            SkColorType colorType,
                                                            SkColorSpace* colorSpace,
                                                            const SkSurfaceProps& surfaceProps) {
    // There are several conditions that determine if we actually need to convert the source to the
    // destination's color space. Rather than duplicate that logic here, just try to make an xform
    // object. If that produces something, then both are tagged, and the source is in a different
    // gamut than the dest. There is some overhead to making the xform, but those are cached, and
    // if we get one back, that means we're about to use it during the conversion anyway.
    auto colorSpaceXform = GrColorSpaceXform::Make(src->getColorSpace(),  src->alphaType(),
                                                   colorSpace, kPremul_SkAlphaType);

    if (!colorSpaceXform) {
        // No xform needed, just return the original image
        return sk_ref_sp(src);
    }

    sk_sp<SkSpecialSurface> surf(src->makeSurface(colorType, colorSpace,
                                                  SkISize::Make(src->width(), src->height()),
                                                  kPremul_SkAlphaType, surfaceProps));
    if (!surf) {
        return sk_ref_sp(src);
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);
    src->draw(canvas, 0, 0, SkSamplingOptions(), &p);
    return surf->makeImageSnapshot();
}
#endif

// In repeat mode, when we are going to sample off one edge of the srcBounds we require the
// opposite side be preserved.
SkIRect SkImageFilter_Base::DetermineRepeatedSrcBound(const SkIRect& srcBounds,
                                                      const SkIVector& filterOffset,
                                                      const SkISize& filterSize,
                                                      const SkIRect& originalSrcBounds) {
    SkIRect tmp = srcBounds;
    tmp.adjust(-filterOffset.fX, -filterOffset.fY,
               filterSize.fWidth - filterOffset.fX, filterSize.fHeight - filterOffset.fY);

    if (tmp.fLeft < originalSrcBounds.fLeft || tmp.fRight > originalSrcBounds.fRight) {
        tmp.fLeft = originalSrcBounds.fLeft;
        tmp.fRight = originalSrcBounds.fRight;
    }
    if (tmp.fTop < originalSrcBounds.fTop || tmp.fBottom > originalSrcBounds.fBottom) {
        tmp.fTop = originalSrcBounds.fTop;
        tmp.fBottom = originalSrcBounds.fBottom;
    }

    return tmp;
}

void SkImageFilter_Base::PurgeCache() {
    SkImageFilterCache::Get()->purge();
}
