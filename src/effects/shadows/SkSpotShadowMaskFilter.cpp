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

    SkRect rect;
    SkRRect rrect;
    if (path.isOval(&rect) && SkScalarNearlyEqual(rect.width(),
                                                  rect.height())) {
        rrect = SkRRect::MakeOval(rect);
        return this->directFilterRRectMaskGPU(context, rtContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    } else if (path.isRect(&rect)) {
        rrect = SkRRect::MakeRect(rect);
        return this->directFilterRRectMaskGPU(context, rtContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    } else if (path.isRRect(&rrect) && rrect.isSimpleCircular()) {
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
    // Fast path only supports filled rrects for now.
    // TODO: fill and stroke as well.
    if (SkStrokeRec::kFill_Style != strokeRec.getStyle()) {
        return false;
    }
    // These should have been checked by the caller.
    // Fast path only supports simple rrects with circular corners.
    SkASSERT(devRRect.isRect() || devRRect.isCircle() ||
        (devRRect.isSimple() && devRRect.allCornersCircular()));
    // Fast path only supports uniform scale.
    SkASSERT(viewMatrix.isSimilarity());
    // Assume we have positive alpha
    SkASSERT(fSpotAlpha > 0);

    // 1/scale
    SkScalar devToSrcScale = viewMatrix.isScaleTranslate() ?
        SkScalarInvert(viewMatrix[SkMatrix::kMScaleX]) :
        sk_float_rsqrt(viewMatrix[SkMatrix::kMScaleX] * viewMatrix[SkMatrix::kMScaleX] +
                       viewMatrix[SkMatrix::kMSkewX] * viewMatrix[SkMatrix::kMSkewX]);

    float zRatio = SkTPin(fOccluderHeight / (fLightPos.fZ - fOccluderHeight), 0.0f, 0.95f);

    SkScalar devSpaceSpotBlur = 2.0f * fLightRadius * zRatio;
    // handle scale of radius and pad due to CTM
    const SkScalar srcSpaceSpotBlur = devSpaceSpotBlur * devToSrcScale;

    // Compute the scale and translation for the spot shadow.
    const SkScalar spotScale = fLightPos.fZ / (fLightPos.fZ - fOccluderHeight);
    SkPoint spotOffset = SkPoint::Make(zRatio*(-fLightPos.fX), zRatio*(-fLightPos.fY));
    // Adjust translate for the effect of the scale.
    spotOffset.fX += spotScale*viewMatrix[SkMatrix::kMTransX];
    spotOffset.fY += spotScale*viewMatrix[SkMatrix::kMTransY];
    // This offset is in dev space, need to transform it into source space.
    SkMatrix ctmInverse;
    if (!viewMatrix.invert(&ctmInverse)) {
        // Since the matrix is a similarity, this should never happen, but just in case...
        SkDebugf("Matrix is degenerate. Will not render spot shadow!\n");
        return true;
    }
    ctmInverse.mapPoints(&spotOffset, 1);

    // Compute the transformed shadow rrect
    SkRRect spotShadowRRect;
    SkMatrix shadowTransform;
    shadowTransform.setScaleTranslate(spotScale, spotScale, spotOffset.fX, spotOffset.fY);
    rrect.transform(shadowTransform, &spotShadowRRect);
    SkScalar spotRadius = spotShadowRRect.getSimpleRadii().fX;

    // Compute the insetWidth
    SkScalar blurOutset = 0.5f*srcSpaceSpotBlur;
    SkScalar insetWidth = blurOutset;
    if (fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag) {
        // If transparent, just do a fill
        insetWidth += spotShadowRRect.width();
    } else {
        // For shadows, instead of using a stroke we specify an inset from the penumbra
        // border. We want to extend this inset area so that it meets up with the caster
        // geometry. The inset geometry will by default already be inset by the blur width.
        //
        // We compare the min and max corners inset by the radius between the original
        // rrect and the shadow rrect. The distance between the two plus the difference
        // between the scaled radius and the original radius gives the distance from the
        // transformed shadow shape to the original shape in that corner. The max
        // of these gives the maximum distance we need to cover.
        //
        // Since we are outsetting by 1/2 the blur distance, we just add the maxOffset to
        // that to get the full insetWidth.
        SkScalar maxOffset;
        if (rrect.isRect()) {
            // Manhattan distance works better for rects
            maxOffset = SkTMax(SkTMax(SkTAbs(spotShadowRRect.rect().fLeft -
                                             rrect.rect().fLeft),
                                      SkTAbs(spotShadowRRect.rect().fTop -
                                             rrect.rect().fTop)),
                               SkTMax(SkTAbs(spotShadowRRect.rect().fRight -
                                             rrect.rect().fRight),
                                      SkTAbs(spotShadowRRect.rect().fBottom -
                                             rrect.rect().fBottom)));
        } else {
            SkScalar dr = spotRadius - rrect.getSimpleRadii().fX;
            SkPoint upperLeftOffset = SkPoint::Make(spotShadowRRect.rect().fLeft -
                                                    rrect.rect().fLeft + dr,
                                                    spotShadowRRect.rect().fTop -
                                                    rrect.rect().fTop + dr);
            SkPoint lowerRightOffset = SkPoint::Make(spotShadowRRect.rect().fRight -
                                                     rrect.rect().fRight - dr,
                                                     spotShadowRRect.rect().fBottom -
                                                     rrect.rect().fBottom - dr);
            maxOffset = SkScalarSqrt(SkTMax(upperLeftOffset.lengthSqd(),
                                            lowerRightOffset.lengthSqd())) + dr;
        }
        insetWidth += maxOffset;
    }

    // Outset the shadow rrect to the border of the penumbra
    SkRect outsetRect = spotShadowRRect.rect().makeOutset(blurOutset, blurOutset);
    if (spotShadowRRect.isOval()) {
        spotShadowRRect = SkRRect::MakeOval(outsetRect);
    } else {
        SkScalar outsetRad = spotRadius + blurOutset;
        spotShadowRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
    }

    GrColor4f color = paint.getColor4f();
    color.fRGBA[3] *= fSpotAlpha;
    paint.setColor4f(color);
    rtContext->drawShadowRRect(clip, std::move(paint), viewMatrix, spotShadowRRect,
                               devSpaceSpotBlur, insetWidth);

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
