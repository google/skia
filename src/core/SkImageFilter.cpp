/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilter.h"

#include "SkCanvas.h"
#include "SkColorSpace_Base.h"
#include "SkFuzzLogging.h"
#include "SkImageFilterCache.h"
#include "SkLocalMatrixImageFilter.h"
#include "SkMatrixImageFilter.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkValidationUtils.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrFixedClip.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#endif

#ifndef SK_IGNORE_TO_STRING
void SkImageFilter::CropRect::toString(SkString* str) const {
    if (!fFlags) {
        return;
    }

    str->appendf("cropRect (");
    if (fFlags & CropRect::kHasLeft_CropEdge) {
        str->appendf("%.2f, ", fRect.fLeft);
    } else {
        str->appendf("X, ");
    }
    if (fFlags & CropRect::kHasTop_CropEdge) {
        str->appendf("%.2f, ", fRect.fTop);
    } else {
        str->appendf("X, ");
    }
    if (fFlags & CropRect::kHasWidth_CropEdge) {
        str->appendf("%.2f, ", fRect.width());
    } else {
        str->appendf("X, ");
    }
    if (fFlags & CropRect::kHasHeight_CropEdge) {
        str->appendf("%.2f", fRect.height());
    } else {
        str->appendf("X");
    }
    str->appendf(") ");
}
#endif

void SkImageFilter::CropRect::applyTo(const SkIRect& imageBounds,
                                      const SkMatrix& ctm,
                                      bool embiggen,
                                      SkIRect* cropped) const {
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
            devICropR.fRight = cropped->fLeft + devICropR.width();
        }
        if (fFlags & kHasTop_CropEdge) {
            if (embiggen || devICropR.fTop > cropped->fTop) {
                cropped->fTop = devICropR.fTop;
            }
        } else {
            devICropR.fBottom = cropped->fTop + devICropR.height();
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

///////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t next_image_filter_unique_id() {
    static int32_t gImageFilterUniqueID;

    // Never return 0.
    int32_t id;
    do {
        id = sk_atomic_inc(&gImageFilterUniqueID) + 1;
    } while (0 == id);
    return id;
}

void SkImageFilter::Common::allocInputs(int count) {
    fInputs.reset(count);
}

bool SkImageFilter::Common::unflatten(SkReadBuffer& buffer, int expectedCount) {
    const int count = buffer.readInt();
    if (!buffer.validate(count >= 0)) {
        return false;
    }
    if (!buffer.validate(expectedCount < 0 || count == expectedCount)) {
        return false;
    }

    SkFUZZF(("allocInputs: %d\n", count));
    this->allocInputs(count);
    for (int i = 0; i < count; i++) {
        if (buffer.readBool()) {
            fInputs[i] = sk_sp<SkImageFilter>(buffer.readImageFilter());
        }
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
    fCropRect = CropRect(rect, flags);
    if (buffer.isVersionLT(SkReadBuffer::kImageFilterNoUniqueID_Version)) {

        (void) buffer.readUInt();
    }
    return buffer.isValid();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkImageFilter::init(sk_sp<SkImageFilter>* inputs,
                         int inputCount,
                         const CropRect* cropRect) {
    fCropRect = cropRect ? *cropRect : CropRect(SkRect(), 0x0);

    fInputs.reset(inputCount);

    for (int i = 0; i < inputCount; ++i) {
        if (!inputs[i] || inputs[i]->usesSrcInput()) {
            fUsesSrcInput = true;
        }
        fInputs[i] = inputs[i];
    }
}

SkImageFilter::SkImageFilter(sk_sp<SkImageFilter>* inputs,
                             int inputCount,
                             const CropRect* cropRect)
    : fUsesSrcInput(false)
    , fUniqueID(next_image_filter_unique_id()) {
    this->init(inputs, inputCount, cropRect);
}

SkImageFilter::~SkImageFilter() {
    SkImageFilterCache::Get()->purgeByKeys(fCacheKeys.begin(), fCacheKeys.count());
}

SkImageFilter::SkImageFilter(int inputCount, SkReadBuffer& buffer)
    : fUsesSrcInput(false)
    , fCropRect(SkRect(), 0x0)
    , fUniqueID(next_image_filter_unique_id()) {
    Common common;
    if (common.unflatten(buffer, inputCount)) {
        this->init(common.inputs(), common.inputCount(), &common.cropRect());
    }
}

void SkImageFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fInputs.count());
    for (int i = 0; i < fInputs.count(); i++) {
        SkImageFilter* input = this->getInput(i);
        buffer.writeBool(input != nullptr);
        if (input != nullptr) {
            buffer.writeFlattenable(input);
        }
    }
    buffer.writeRect(fCropRect.rect());
    buffer.writeUInt(fCropRect.flags());
}

sk_sp<SkSpecialImage> SkImageFilter::filterImage(SkSpecialImage* src, const Context& context,
                                                 SkIPoint* offset) const {
    SkASSERT(src && offset);

    uint32_t srcGenID = fUsesSrcInput ? src->uniqueID() : 0;
    const SkIRect srcSubset = fUsesSrcInput ? src->subset() : SkIRect::MakeWH(0, 0);
    SkImageFilterCacheKey key(fUniqueID, context.ctm(), context.clipBounds(), srcGenID, srcSubset);
    if (context.cache()) {
        sk_sp<SkSpecialImage> result = context.cache()->get(key, offset);
        if (result) {
            return result;
        }
    }

    sk_sp<SkSpecialImage> result(this->onFilterImage(src, context, offset));

#if SK_SUPPORT_GPU
    if (src->isTextureBacked() && result && !result->isTextureBacked()) {
        // Keep the result on the GPU - this is still required for some
        // image filters that don't support GPU in all cases
        GrContext* context = src->getContext();
        result = result->makeTextureImage(context);
    }
#endif

    if (result && context.cache()) {
        context.cache()->set(key, result.get(), *offset);
        SkAutoMutexAcquire mutex(fMutex);
        fCacheKeys.push_back(key);
    }

    return result;
}

SkIRect SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                 MapDirection direction) const {
    if (kReverse_MapDirection == direction) {
        SkIRect bounds = this->onFilterNodeBounds(src, ctm, direction);
        return this->onFilterBounds(bounds, ctm, direction);
    } else {
        SkIRect bounds = this->onFilterBounds(src, ctm, direction);
        bounds = this->onFilterNodeBounds(bounds, ctm, direction);
        SkIRect dst;
        this->getCropRect().applyTo(bounds, ctm, this->affectsTransparentBlack(), &dst);
        return dst;
    }
}

SkRect SkImageFilter::computeFastBounds(const SkRect& src) const {
    if (0 == this->countInputs()) {
        return src;
    }
    SkRect combinedBounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    for (int i = 1; i < this->countInputs(); i++) {
        SkImageFilter* input = this->getInput(i);
        if (input) {
            combinedBounds.join(input->computeFastBounds(src));
        } else {
            combinedBounds.join(src);
        }
    }
    return combinedBounds;
}

bool SkImageFilter::canComputeFastBounds() const {
    if (this->affectsTransparentBlack()) {
        return false;
    }
    for (int i = 0; i < this->countInputs(); i++) {
        SkImageFilter* input = this->getInput(i);
        if (input && !input->canComputeFastBounds()) {
            return false;
        }
    }
    return true;
}

#if SK_SUPPORT_GPU
sk_sp<SkSpecialImage> SkImageFilter::DrawWithFP(GrContext* context,
                                                sk_sp<GrFragmentProcessor> fp,
                                                const SkIRect& bounds,
                                                const OutputProperties& outputProperties) {
    GrPaint paint;
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    sk_sp<SkColorSpace> colorSpace = sk_ref_sp(outputProperties.colorSpace());
    GrPixelConfig config = GrRenderableConfigForColorSpace(colorSpace.get());
    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeDeferredRenderTargetContext(
        SkBackingFit::kApprox, bounds.width(), bounds.height(), config, std::move(colorSpace)));
    if (!renderTargetContext) {
        return nullptr;
    }
    paint.setGammaCorrect(renderTargetContext->isGammaCorrect());

    SkIRect dstIRect = SkIRect::MakeWH(bounds.width(), bounds.height());
    SkRect srcRect = SkRect::Make(bounds);
    SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
    GrFixedClip clip(dstIRect);
    renderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), dstRect,
                                        srcRect);

    return SkSpecialImage::MakeDeferredFromGpu(context, dstIRect,
                                               kNeedNewImageUniqueID_SpecialImage,
                                               renderTargetContext->asTextureProxyRef(),
                                               renderTargetContext->refColorSpace());
}
#endif

bool SkImageFilter::asAColorFilter(SkColorFilter** filterPtr) const {
    SkASSERT(nullptr != filterPtr);
    if (!this->isColorFilterNode(filterPtr)) {
        return false;
    }
    if (nullptr != this->getInput(0) || (*filterPtr)->affectsTransparentBlack()) {
        (*filterPtr)->unref();
        return false;
    }
    return true;
}

bool SkImageFilter::canHandleComplexCTM() const {
    if (!this->onCanHandleComplexCTM()) {
        return false;
    }
    const int count = this->countInputs();
    for (int i = 0; i < count; ++i) {
        SkImageFilter* input = this->getInput(i);
        if (input && !input->canHandleComplexCTM()) {
            return false;
        }
    }
    return true;
}

bool SkImageFilter::applyCropRect(const Context& ctx, const SkIRect& srcBounds,
                                  SkIRect* dstBounds) const {
    SkIRect temp = this->onFilterNodeBounds(srcBounds, ctx.ctm(), kForward_MapDirection);
    fCropRect.applyTo(temp, ctx.ctm(), this->affectsTransparentBlack(), dstBounds);
    // Intersect against the clip bounds, in case the crop rect has
    // grown the bounds beyond the original clip. This can happen for
    // example in tiling, where the clip is much smaller than the filtered
    // primitive. If we didn't do this, we would be processing the filter
    // at the full crop rect size in every tile.
    return dstBounds->intersect(ctx.clipBounds());
}

#if SK_SUPPORT_GPU
sk_sp<SkSpecialImage> SkImageFilter::ImageToColorSpace(SkSpecialImage* src,
                                                       const OutputProperties& outProps) {
    // There are several conditions that determine if we actually need to convert the source to the
    // destination's color space. Rather than duplicate that logic here, just try to make an xform
    // object. If that produces something, then both are tagged, and the source is in a different
    // gamut than the dest. There is some overhead to making the xform, but those are cached, and
    // if we get one back, that means we're about to use it during the conversion anyway.
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(src->getColorSpace(),
                                                                       outProps.colorSpace());

    if (!colorSpaceXform) {
        // No xform needed, just return the original image
        return sk_ref_sp(src);
    }

    sk_sp<SkSpecialSurface> surf(src->makeSurface(outProps,
                                                  SkISize::Make(src->width(), src->height())));
    if (!surf) {
        return sk_ref_sp(src);
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);
    src->draw(canvas, 0, 0, &p);
    return surf->makeImageSnapshot();
}
#endif

// Return a larger (newWidth x newHeight) copy of 'src' with black padding
// around it.
static sk_sp<SkSpecialImage> pad_image(SkSpecialImage* src,
                                       const SkImageFilter::OutputProperties& outProps,
                                       int newWidth, int newHeight, int offX, int offY) {
    // We would like to operate in the source's color space (so that we return an "identical"
    // image, other than the padding. To achieve that, we'd create new output properties:
    //
    // SkImageFilter::OutputProperties outProps(src->getColorSpace());
    //
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
    sk_sp<SkSpecialSurface> surf(src->makeSurface(outProps, SkISize::Make(newWidth, newHeight)));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    src->draw(canvas, offX, offY, nullptr);

    return surf->makeImageSnapshot();
}

sk_sp<SkSpecialImage> SkImageFilter::applyCropRect(const Context& ctx,
                                                   SkSpecialImage* src,
                                                   SkIPoint* srcOffset,
                                                   SkIRect* bounds) const {
    const SkIRect srcBounds = SkIRect::MakeXYWH(srcOffset->x(), srcOffset->y(),
                                                src->width(), src->height());

    SkIRect dstBounds = this->onFilterNodeBounds(srcBounds, ctx.ctm(), kForward_MapDirection);
    fCropRect.applyTo(dstBounds, ctx.ctm(), this->affectsTransparentBlack(), bounds);
    if (!bounds->intersect(ctx.clipBounds())) {
        return nullptr;
    }

    if (srcBounds.contains(*bounds)) {
        return sk_sp<SkSpecialImage>(SkRef(src));
    } else {
        sk_sp<SkSpecialImage> img(pad_image(src, ctx.outputProperties(),
                                            bounds->width(), bounds->height(),
                                            srcOffset->x() - bounds->x(),
                                            srcOffset->y() - bounds->y()));
        *srcOffset = SkIPoint::Make(bounds->x(), bounds->y());
        return img;
    }
}

SkIRect SkImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                      MapDirection direction) const {
    if (this->countInputs() < 1) {
        return src;
    }

    SkIRect totalBounds;
    for (int i = 0; i < this->countInputs(); ++i) {
        SkImageFilter* filter = this->getInput(i);
        SkIRect rect = filter ? filter->filterBounds(src, ctm, direction) : src;
        if (0 == i) {
            totalBounds = rect;
        } else {
            totalBounds.join(rect);
        }
    }

    return totalBounds;
}

SkIRect SkImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const {
    return src;
}


SkImageFilter::Context SkImageFilter::mapContext(const Context& ctx) const {
    SkIRect clipBounds = this->onFilterNodeBounds(ctx.clipBounds(), ctx.ctm(),
                                                  MapDirection::kReverse_MapDirection);
    return Context(ctx.ctm(), clipBounds, ctx.cache(), ctx.outputProperties());
}

sk_sp<SkImageFilter> SkImageFilter::MakeMatrixFilter(const SkMatrix& matrix,
                                                     SkFilterQuality filterQuality,
                                                     sk_sp<SkImageFilter> input) {
    return SkMatrixImageFilter::Make(matrix, filterQuality, std::move(input));
}

sk_sp<SkImageFilter> SkImageFilter::makeWithLocalMatrix(const SkMatrix& matrix) const {
    // SkLocalMatrixImageFilter takes SkImage* in its factory, but logically that parameter
    // is *always* treated as a const ptr. Hence the const-cast here.
    //
    SkImageFilter* nonConstThis = const_cast<SkImageFilter*>(this);
    return SkLocalMatrixImageFilter::Make(matrix, sk_ref_sp<SkImageFilter>(nonConstThis));
}

sk_sp<SkSpecialImage> SkImageFilter::filterInput(int index,
                                                 SkSpecialImage* src,
                                                 const Context& ctx,
                                                 SkIPoint* offset) const {
    SkImageFilter* input = this->getInput(index);
    if (!input) {
        return sk_sp<SkSpecialImage>(SkRef(src));
    }

    sk_sp<SkSpecialImage> result(input->filterImage(src, this->mapContext(ctx), offset));

    SkASSERT(!result || src->isTextureBacked() == result->isTextureBacked());

    return result;
}

void SkImageFilter::PurgeCache() {
    SkImageFilterCache::Get()->purge();
}
