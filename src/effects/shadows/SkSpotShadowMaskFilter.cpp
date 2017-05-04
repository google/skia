/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSpotShadowMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkStringUtils.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrFragmentProcessor.h"
#include "GrStyle.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "SkStrokeRec.h"
#endif

class SkSpotShadowMaskFilterImpl : public SkMaskFilter {
public:
    SkSpotShadowMaskFilterImpl(SkScalar occluderHeight, const SkPoint3& lightPos,
                               SkScalar lightRadius, SkScalar spotAlpha, uint32_t flags);

    // overrides from SkMaskFilter
    SkMask::Format getFormat() const override;
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

#if SK_SUPPORT_GPU
    bool canFilterMaskGPU(const SkRRect& devRRect,
                          const SkIRect& clipBounds,
                          const SkMatrix& ctm,
                          SkRect* maskRect) const override;
    bool directFilterMaskGPU(GrContext*,
                             GrRenderTargetContext* drawContext,
                             GrPaint&&,
                             const GrClip&,
                             const SkMatrix& viewMatrix,
                             const SkStrokeRec& strokeRec,
                             const SkPath& path) const override;
    bool directFilterRRectMaskGPU(GrContext*,
                                  GrRenderTargetContext* drawContext,
                                  GrPaint&&,
                                  const GrClip&,
                                  const SkMatrix& viewMatrix,
                                  const SkStrokeRec& strokeRec,
                                  const SkRRect& rrect,
                                  const SkRRect& devRRect) const override;
    sk_sp<GrTextureProxy> filterMaskGPU(GrContext*,
                                        sk_sp<GrTextureProxy> srcProxy,
                                        const SkMatrix& ctm,
                                        const SkIRect& maskRect) const override;
#endif

    void computeFastBounds(const SkRect&, SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSpotShadowMaskFilterImpl)

private:
    SkScalar fOccluderHeight;
    SkPoint3 fLightPos;
    SkScalar fLightRadius;
    SkScalar fSpotAlpha;
    uint32_t fFlags;

    SkSpotShadowMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkSpotShadowMaskFilter;

    typedef SkMaskFilter INHERITED;
};

sk_sp<SkMaskFilter> SkSpotShadowMaskFilter::Make(SkScalar occluderHeight, const SkPoint3& lightPos,
                                                 SkScalar lightRadius, SkScalar spotAlpha,
                                                 uint32_t flags) {
    // add some param checks here for early exit

    return sk_sp<SkMaskFilter>(new SkSpotShadowMaskFilterImpl(occluderHeight, lightPos,
                                                              lightRadius, spotAlpha, flags));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkSpotShadowMaskFilterImpl::SkSpotShadowMaskFilterImpl(SkScalar occluderHeight,
                                                       const SkPoint3& lightPos,
                                                       SkScalar lightRadius,
                                                       SkScalar spotAlpha,
                                                       uint32_t flags)
    : fOccluderHeight(occluderHeight)
    , fLightPos(lightPos)
    , fLightRadius(lightRadius)
    , fSpotAlpha(spotAlpha)
    , fFlags(flags) {
    SkASSERT(fOccluderHeight > 0);
    SkASSERT(fLightPos.z() > 0 && fLightPos.z() > fOccluderHeight);
    SkASSERT(fLightRadius > 0);
    SkASSERT(fSpotAlpha >= 0);
}

SkMask::Format SkSpotShadowMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkSpotShadowMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                            const SkMatrix& matrix,
                                            SkIPoint* margin) const {
    // TODO something
    return false;
}

void SkSpotShadowMaskFilterImpl::computeFastBounds(const SkRect& src, SkRect* dst) const {
    // TODO compute based on ambient + spot data
    dst->set(src.fLeft, src.fTop, src.fRight, src.fBottom);
}

sk_sp<SkFlattenable> SkSpotShadowMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    const SkScalar occluderHeight = buffer.readScalar();
    const SkScalar lightX = buffer.readScalar();
    const SkScalar lightY = buffer.readScalar();
    const SkScalar lightZ = buffer.readScalar();
    const SkPoint3 lightPos = SkPoint3::Make(lightX, lightY, lightZ);
    const SkScalar lightRadius = buffer.readScalar();
    const SkScalar spotAlpha = buffer.readScalar();
    const uint32_t flags = buffer.readUInt();

    return SkSpotShadowMaskFilter::Make(occluderHeight, lightPos, lightRadius,
                                        spotAlpha, flags);
}

void SkSpotShadowMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fOccluderHeight);
    buffer.writeScalar(fLightPos.fX);
    buffer.writeScalar(fLightPos.fY);
    buffer.writeScalar(fLightPos.fZ);
    buffer.writeScalar(fLightRadius);
    buffer.writeScalar(fSpotAlpha);
    buffer.writeUInt(fFlags);
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkSpotShadowMaskFilterImpl::canFilterMaskGPU(const SkRRect& devRRect,
                                                  const SkIRect& clipBounds,
                                                  const SkMatrix& ctm,
                                                  SkRect* maskRect) const {
    // TODO
    *maskRect = devRRect.rect();
    return true;
}

bool SkSpotShadowMaskFilterImpl::directFilterMaskGPU(GrContext* context,
                                                     GrRenderTargetContext* rtContext,
                                                     GrPaint&& paint,
                                                     const GrClip& clip,
                                                     const SkMatrix& viewMatrix,
                                                     const SkStrokeRec& strokeRec,
                                                     const SkPath& path) const {
    SkASSERT(rtContext);
    // TODO: this will not handle local coordinates properly

    if (fSpotAlpha <= 0.0f) {
        return true;
    }

    // only convex paths for now
    if (!path.isConvex()) {
        return false;
    }

    if (strokeRec.getStyle() != SkStrokeRec::kFill_Style) {
        return false;
    }

    // if circle
    if (path.isOval(nullptr) && SkScalarNearlyEqual(path.getBounds().width(),
                                                    path.getBounds().height())) {
        SkRRect rrect = SkRRect::MakeOval(path.getBounds());
        return this->directFilterRRectMaskGPU(context, rtContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    } else if (path.isRect(nullptr)) {
        SkRRect rrect = SkRRect::MakeRect(path.getBounds());
        return this->directFilterRRectMaskGPU(context, rtContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    }

    return false;
}

bool SkSpotShadowMaskFilterImpl::directFilterRRectMaskGPU(GrContext*,
                                                          GrRenderTargetContext* rtContext,
                                                          GrPaint&& paint,
                                                          const GrClip& clip,
                                                          const SkMatrix& viewMatrix,
                                                          const SkStrokeRec& strokeRec,
                                                          const SkRRect& rrect,
                                                          const SkRRect& devRRect) const {
    // It's likely the caller has already done these checks, but we have to be sure.

    // Fast path only supports filled rrects for now.
    // TODO: fill and stroke as well.
    if (SkStrokeRec::kFill_Style != strokeRec.getStyle()) {
        return false;
    }
    // Fast path only supports simple rrects with circular corners.
    if (!devRRect.isRect() && !devRRect.isCircle() &&
        (!devRRect.isSimple() || !devRRect.allCornersCircular())) {
        return false;
    }
    // Fast path only supports uniform scale.
    if (!viewMatrix.isSimilarity()) {
        return false;
    }
    // 1/scale
    SkScalar scaleFactor = viewMatrix.isScaleTranslate() ?
        SkScalarInvert(viewMatrix[SkMatrix::kMScaleX]) :
        sk_float_rsqrt(viewMatrix[SkMatrix::kMScaleX] * viewMatrix[SkMatrix::kMScaleX] +
                       viewMatrix[SkMatrix::kMSkewX] * viewMatrix[SkMatrix::kMSkewX]);

    if (fSpotAlpha > 0.0f) {
        float zRatio = SkTPin(fOccluderHeight / (fLightPos.fZ - fOccluderHeight), 0.0f, 0.95f);

        SkScalar devSpaceSpotRadius = 2.0f * fLightRadius * zRatio;
        // handle scale of radius and pad due to CTM
        const SkScalar srcSpaceSpotRadius = devSpaceSpotRadius * scaleFactor;

        SkRRect spotShadowRRect;
        // Compute the scale and translation for the spot shadow.
        const SkScalar scale = fLightPos.fZ / (fLightPos.fZ - fOccluderHeight);
        rrect.transform(SkMatrix::MakeScale(scale, scale), &spotShadowRRect);

        SkPoint spotOffset = SkPoint::Make(zRatio*(-fLightPos.fX), zRatio*(-fLightPos.fY));
        // Adjust for the effect of the scale.
        spotOffset.fX += scale*viewMatrix[SkMatrix::kMTransX];
        spotOffset.fY += scale*viewMatrix[SkMatrix::kMTransY];
        // This offset is in dev space, need to transform it into source space.
        SkMatrix ctmInverse;
        if (!viewMatrix.invert(&ctmInverse)) {
            // Since the matrix is a similarity, this should never happen, but just in case...
            SkDebugf("Matrix is degenerate. Will not render spot shadow!\n");
            return true;
        }
        ctmInverse.mapPoints(&spotOffset, 1);

        // We want to extend the stroked area in so that it meets up with the caster
        // geometry. The stroked geometry will, by definition already be inset half the
        // stroke width but we also have to account for the scaling and translation when
        // computing a new stroke shape.
        //
        // We begin by transforming the min and max corners -- inset by the radius -- by the scale
        // and translation. The distance from that to the original inset point plus the difference
        // between the scaled radius and the original radius gives the distance from the 
        // transformed shadow shape to the original shape. If the max of the two distances is
        // greater than the strokeWidth then we need to inset and increase the strokeWidth to cover
        // the hole. If less, then we can outset and decrease the strokeWidth to reduce our
        // coverage.

        // This factor handles both the transform scale and subtracting the original value.
        SkScalar comboScale = scale - 1;
        SkScalar r = rrect.getSimpleRadii().fX;
        SkPoint upperLeftOffset = SkPoint::Make(comboScale*(rrect.rect().fLeft + r),
                                                comboScale*(rrect.rect().fTop + r));
        upperLeftOffset += spotOffset;
        SkPoint lowerRightOffset = SkPoint::Make(comboScale*(rrect.rect().fRight - r),
                                                 comboScale*(rrect.rect().fBottom - r));
        lowerRightOffset += spotOffset;

        SkScalar maxOffset = SkTMax(upperLeftOffset.length(), lowerRightOffset.length());
        maxOffset += comboScale*r;
        SkScalar insetAmount = maxOffset - (0.5f * srcSpaceSpotRadius);
        SkScalar strokeWidth = srcSpaceSpotRadius + insetAmount;

        SkScalar strokedDiff = SkTMin(spotShadowRRect.width(), spotShadowRRect.height())
                             - (insetAmount + strokeWidth);

        SkStrokeRec spotStrokeRec(SkStrokeRec::kFill_InitStyle);
        // If the caster has too large a stroke or is transparent, just fill it.
        if (strokedDiff < 0 || fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag) {
            spotStrokeRec.setStrokeStyle(srcSpaceSpotRadius, true);
        } else {
            // Since we can't have unequal strokes, inset the shadow rect so the inner
            // and outer edges of the stroke will land where we want.
            insetAmount *= 0.5f;
            SkRect insetRect = spotShadowRRect.rect().makeInset(insetAmount, insetAmount);
            // If the shadowRRect was an oval then its inset will also be one.
            // We set it explicitly to avoid errors.
            if (spotShadowRRect.isOval()) {
                spotShadowRRect = SkRRect::MakeOval(insetRect);
            } else {
                SkScalar insetRad = spotShadowRRect.getSimpleRadii().fX - insetAmount;
                spotShadowRRect = SkRRect::MakeRectXY(insetRect, insetRad, insetRad);
            }
            spotStrokeRec.setStrokeStyle(strokeWidth, false);
        }

        spotShadowRRect.offset(spotOffset.fX, spotOffset.fY);

        GrColor4f color = paint.getColor4f();
        color.fRGBA[3] *= fSpotAlpha;
        paint.setColor4f(color);
        rtContext->drawShadowRRect(clip, std::move(paint), viewMatrix, spotShadowRRect,
                                   devSpaceSpotRadius, GrStyle(spotStrokeRec, nullptr));
    }

    return true;
}

sk_sp<GrTextureProxy> SkSpotShadowMaskFilterImpl::filterMaskGPU(GrContext*,
                                                                sk_sp<GrTextureProxy> srcProxy,
                                                                const SkMatrix& ctm,
                                                                const SkIRect& maskRect) const {
    // This filter is generative and doesn't operate on pre-existing masks
    return nullptr;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkSpotShadowMaskFilterImpl::toString(SkString* str) const {
    str->append("SkSpotShadowMaskFilterImpl: (");

    str->append("occluderHeight: ");
    str->appendScalar(fOccluderHeight);
    str->append(" ");

    str->append("lightPos: (");
    str->appendScalar(fLightPos.fX);
    str->append(", ");
    str->appendScalar(fLightPos.fY);
    str->append(", ");
    str->appendScalar(fLightPos.fZ);
    str->append(") ");

    str->append("lightRadius: ");
    str->appendScalar(fLightRadius);
    str->append(" ");

    str->append("spotAlpha: ");
    str->appendScalar(fSpotAlpha);
    str->append(" ");

    str->append("flags: (");
    if (fFlags) {
        bool needSeparator = false;
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag),
                          "TransparentOccluder", &needSeparator);
    } else {
        str->append("None");
    }
    str->append("))");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkSpotShadowMaskFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpotShadowMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
