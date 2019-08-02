/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/effects/SkComposeImageFilter.h"
#include "include/private/SkSafe32.h"
#include "src/core/SkFuzzLogging.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkLocalMatrixImageFilter.h"
#include "src/core/SkMatrixImageFilter.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
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
    if (kReverse_MapDirection == direction) {
        SkIRect bounds = as_IFB(this)->onFilterNodeBounds(src, ctm, direction, inputRect);
        return as_IFB(this)->onFilterBounds(bounds, ctm, direction, &bounds);
    } else {
        SkASSERT(!inputRect);
        SkIRect bounds = as_IFB(this)->onFilterBounds(src, ctm, direction, nullptr);
        bounds = as_IFB(this)->onFilterNodeBounds(bounds, ctm, direction, nullptr);
        SkIRect dst;
        as_IFB(this)->getCropRect().applyTo(
                bounds, ctm, as_IFB(this)->affectsTransparentBlack(), &dst);
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
    if (as_IFB(this)->affectsTransparentBlack()) {
        return false;
    }
    for (int i = 0; i < this->countInputs(); i++) {
        const SkImageFilter* input = this->getInput(i);
        if (input && !input->canComputeFastBounds()) {
            return false;
        }
    }
    return true;
}

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

sk_sp<SkImageFilter> SkImageFilter::MakeMatrixFilter(const SkMatrix& matrix,
                                                     SkFilterQuality filterQuality,
                                                     sk_sp<SkImageFilter> input) {
    return SkMatrixImageFilter::Make(matrix, filterQuality, std::move(input));
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
        id = nextID++;
    } while (id == 0);
    return id;
}

SkImageFilter_Base::SkImageFilter_Base(sk_sp<SkImageFilter> const* inputs,
                                       int inputCount, const CropRect* cropRect)
        : fUsesSrcInput(false)
        , fUniqueID(next_image_filter_unique_id()) {
    fCropRect = cropRect ? *cropRect : CropRect(SkRect(), 0x0);

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
    fCropRect = CropRect(rect, flags);
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

sk_sp<SkSpecialImage> SkImageFilter_Base::filterImage(SkSpecialImage* src, const Context& context,
                                                      SkIPoint* offset) const {
    SkASSERT(src && offset);
    if (!context.isValid()) {
        return nullptr;
    }

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
        auto context = src->getContext();
        result = result->makeTextureImage(context);
    }
#endif

    if (result && context.cache()) {
        context.cache()->set(key, result.get(), *offset, this);
    }

    return result;
}

bool SkImageFilter_Base::canHandleComplexCTM() const {
    // CropRects need to apply in the source coordinate system, but are not aware of complex CTMs
    // when performing clipping. For a simple fix, any filter with a crop rect set cannot support
    // complex CTMs until that's updated.
    if (this->cropRectIsSet() || !this->onCanHandleComplexCTM()) {
        return false;
    }
    const int count = this->countInputs();
    for (int i = 0; i < count; ++i) {
        const SkImageFilter_Base* input = as_IFB(this->getInput(i));
        if (input && !input->canHandleComplexCTM()) {
            return false;
        }
    }
    return true;
}

void SkImageFilter::CropRect::applyTo(const SkIRect& imageBounds, const SkMatrix& ctm,
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
    fCropRect.applyTo(tmpDst, ctx.ctm(), this->affectsTransparentBlack(), dstBounds);
    // Intersect against the clip bounds, in case the crop rect has
    // grown the bounds beyond the original clip. This can happen for
    // example in tiling, where the clip is much smaller than the filtered
    // primitive. If we didn't do this, we would be processing the filter
    // at the full crop rect size in every tile.
    return dstBounds->intersect(ctx.clipBounds());
}

// Return a larger (newWidth x newHeight) copy of 'src' with black padding
// around it.
static sk_sp<SkSpecialImage> pad_image(SkSpecialImage* src,
                                       const SkImageFilter_Base::OutputProperties& outProps,
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
        sk_sp<SkSpecialImage> img(pad_image(src, ctx.outputProperties(),
                                            bounds->width(), bounds->height(),
                                            Sk32_sat_sub(srcOffset->x(), bounds->x()),
                                            Sk32_sat_sub(srcOffset->y(), bounds->y())));
        *srcOffset = SkIPoint::Make(bounds->x(), bounds->y());
        return img;
    }
}

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

sk_sp<SkSpecialImage> SkImageFilter_Base::filterInput(int index,
                                                      SkSpecialImage* src,
                                                      const Context& ctx,
                                                      SkIPoint* offset) const {
    const SkImageFilter* input = this->getInput(index);
    if (!input) {
        return sk_sp<SkSpecialImage>(SkRef(src));
    }

    sk_sp<SkSpecialImage> result(as_IFB(input)->filterImage(src, this->mapContext(ctx), offset));

    SkASSERT(!result || src->isTextureBacked() == result->isTextureBacked());

    return result;
}

SkImageFilter_Base::Context SkImageFilter_Base::mapContext(const Context& ctx) const {
    SkIRect clipBounds = this->onFilterNodeBounds(ctx.clipBounds(), ctx.ctm(),
                                                  MapDirection::kReverse_MapDirection,
                                                  &ctx.clipBounds());
    return Context(ctx.ctm(), clipBounds, ctx.cache(), ctx.outputProperties());
}

#if SK_SUPPORT_GPU
sk_sp<SkSpecialImage> SkImageFilter_Base::DrawWithFP(GrRecordingContext* context,
                                                     std::unique_ptr<GrFragmentProcessor> fp,
                                                     const SkIRect& bounds,
                                                     const OutputProperties& outputProperties,
                                                     GrProtected isProtected) {
    GrPaint paint;
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    sk_sp<SkColorSpace> colorSpace = sk_ref_sp(outputProperties.colorSpace());
    GrColorType colorType = SkColorTypeToGrColorType(outputProperties.colorType());
    sk_sp<GrRenderTargetContext> renderTargetContext(
            context->priv().makeDeferredRenderTargetContext(
                    SkBackingFit::kApprox,
                    bounds.width(),
                    bounds.height(),
                    colorType,
                    std::move(colorSpace),
                    1,
                    GrMipMapped::kNo,
                    kBottomLeft_GrSurfaceOrigin,
                    nullptr,
                    SkBudgeted::kYes,
                    isProtected));
    if (!renderTargetContext) {
        return nullptr;
    }

    SkIRect dstIRect = SkIRect::MakeWH(bounds.width(), bounds.height());
    SkRect srcRect = SkRect::Make(bounds);
    SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
    GrFixedClip clip(dstIRect);
    renderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), dstRect,
                                        srcRect);

    return SkSpecialImage::MakeDeferredFromGpu(
            context, dstIRect, kNeedNewImageUniqueID_SpecialImage,
            renderTargetContext->asTextureProxyRef(),
            renderTargetContext->colorSpaceInfo().refColorSpace());
}

sk_sp<SkSpecialImage> SkImageFilter_Base::ImageToColorSpace(SkSpecialImage* src,
                                                            const OutputProperties& outProps) {
    // There are several conditions that determine if we actually need to convert the source to the
    // destination's color space. Rather than duplicate that logic here, just try to make an xform
    // object. If that produces something, then both are tagged, and the source is in a different
    // gamut than the dest. There is some overhead to making the xform, but those are cached, and
    // if we get one back, that means we're about to use it during the conversion anyway.
    auto colorSpaceXform = GrColorSpaceXform::Make(src->getColorSpace(),  src->alphaType(),
                                                   outProps.colorSpace(), kPremul_SkAlphaType);

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

static sk_sp<SkImageFilter> apply_ctm_to_filter(sk_sp<SkImageFilter> input, const SkMatrix& ctm,
                                                SkMatrix* remainder, bool asBackdrop) {
    if (ctm.isScaleTranslate() || as_IFB(input)->canHandleComplexCTM()) {
        // The filter supports the CTM, so leave it as-is and 'remainder' stores the whole CTM
        *remainder = ctm;
        return input;
    }

    // We have a complex CTM and a filter that can't support them, so it needs to use the matrix
    // transform filter that resamples the image contents. Decompose the simple portion of the ctm
    // into 'remainder'
    SkMatrix ctmToEmbed;
    SkSize scale;
    if (ctm.decomposeScale(&scale, &ctmToEmbed)) {
        // decomposeScale splits ctm into scale * ctmToEmbed, so bake ctmToEmbed into DAG
        // with a matrix filter and return scale as the remaining matrix for the real CTM.
        remainder->setScale(scale.fWidth, scale.fHeight);

        // ctmToEmbed is passed to SkMatrixImageFilter, which performs its transforms as if it were
        // a pre-transformation before applying the image-filter context's CTM. In this case, we
        // need ctmToEmbed to be a post-transformation (i.e. after the scale matrix since
        // decomposeScale produces ctm = ctmToEmbed * scale). Giving scale^-1 * ctmToEmbed * scale
        // to the matrix filter achieves this effect.
        // TODO (michaelludwig) - When the original root node of a filter can be drawn directly to a
        // device using ctmToEmbed, this abuse of SkMatrixImageFilter can go away.
        ctmToEmbed.preScale(scale.fWidth, scale.fHeight);
        ctmToEmbed.postScale(1.f / scale.fWidth, 1.f / scale.fHeight);
    } else {
        // Unable to decompose
        // FIXME Ideally we'd embed the entire CTM as part of the matrix image filter, but
        // the device <-> src bounds calculations for filters are very brittle under perspective,
        // and can easily run into precision issues (wrong bounds that clip), or performance issues
        // (producing large source-space images where 80% of the image is compressed into a few
        // device pixels). A longer term solution for perspective-space image filtering is needed
        // see skbug.com/9074
        if (ctm.hasPerspective()) {
                *remainder = ctm;
            return input;
        }

        ctmToEmbed = ctm;
        remainder->setIdentity();
    }

    if (asBackdrop) {
        // In the backdrop case we also have to transform the existing device-space buffer content
        // into the source coordinate space prior to the filtering. Non-backdrop filter inputs are
        // already in the source space because of how the layer is drawn by SkCanvas.
        SkMatrix invEmbed;
        if (ctmToEmbed.invert(&invEmbed)) {
            input = SkComposeImageFilter::Make(std::move(input),
                            SkMatrixImageFilter::Make(invEmbed, kLow_SkFilterQuality, nullptr));
        }
    }
    return SkMatrixImageFilter::Make(ctmToEmbed, kLow_SkFilterQuality, input);
}

sk_sp<SkImageFilter> SkImageFilter_Base::applyCTM(const SkMatrix& ctm, SkMatrix* remainder) const {
    return apply_ctm_to_filter(this->refMe(), ctm, remainder, false);
}

sk_sp<SkImageFilter> SkImageFilter_Base::applyCTMForBackdrop(const SkMatrix& ctm,
                                                             SkMatrix* remainder) const {
    return apply_ctm_to_filter(this->refMe(), ctm, remainder, true);
}
