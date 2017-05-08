/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAmbientShadowMaskFilter.h"
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

class SkAmbientShadowMaskFilterImpl : public SkMaskFilter {
public:
    SkAmbientShadowMaskFilterImpl(SkScalar occluderHeight, SkScalar ambientAlpha, uint32_t flags);

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
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAmbientShadowMaskFilterImpl)

private:
    SkScalar fOccluderHeight;
    SkScalar fAmbientAlpha;
    uint32_t fFlags;

    SkAmbientShadowMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkAmbientShadowMaskFilter;

    typedef SkMaskFilter INHERITED;
};

sk_sp<SkMaskFilter> SkAmbientShadowMaskFilter::Make(SkScalar occluderHeight, SkScalar ambientAlpha,
                                                    uint32_t flags) {
    // add some param checks here for early exit

    return sk_sp<SkMaskFilter>(new SkAmbientShadowMaskFilterImpl(occluderHeight, ambientAlpha,
                                                                 flags));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkAmbientShadowMaskFilterImpl::SkAmbientShadowMaskFilterImpl(SkScalar occluderHeight,
                                                             SkScalar ambientAlpha,
                                                             uint32_t flags)
    : fOccluderHeight(occluderHeight)
    , fAmbientAlpha(ambientAlpha)
    , fFlags(flags) {
    SkASSERT(fOccluderHeight > 0);
    SkASSERT(fAmbientAlpha >= 0);
}

SkMask::Format SkAmbientShadowMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkAmbientShadowMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                               const SkMatrix& matrix,
                                               SkIPoint* margin) const {
    // TODO something
    return false;
}

void SkAmbientShadowMaskFilterImpl::computeFastBounds(const SkRect& src, SkRect* dst) const {
    // TODO compute based on ambient data
    dst->set(src.fLeft, src.fTop, src.fRight, src.fBottom);
}

sk_sp<SkFlattenable> SkAmbientShadowMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    const SkScalar occluderHeight = buffer.readScalar();
    const SkScalar ambientAlpha = buffer.readScalar();
    const uint32_t flags = buffer.readUInt();

    return SkAmbientShadowMaskFilter::Make(occluderHeight, ambientAlpha, flags);
}

void SkAmbientShadowMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fOccluderHeight);
    buffer.writeScalar(fAmbientAlpha);
    buffer.writeUInt(fFlags);
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkAmbientShadowMaskFilterImpl::canFilterMaskGPU(const SkRRect& devRRect,
                                                     const SkIRect& clipBounds,
                                                     const SkMatrix& ctm,
                                                     SkRect* maskRect) const {
    // TODO
    *maskRect = devRRect.rect();
    return true;
}

static const float kHeightFactor = 1.0f / 128.0f;
static const float kGeomFactor = 64.0f;

bool SkAmbientShadowMaskFilterImpl::directFilterMaskGPU(GrContext* context,
                                                        GrRenderTargetContext* rtContext,
                                                        GrPaint&& paint,
                                                        const GrClip& clip,
                                                        const SkMatrix& viewMatrix,
                                                        const SkStrokeRec& strokeRec,
                                                        const SkPath& path) const {
    SkASSERT(rtContext);
    // TODO: this will not handle local coordinates properly

    if (fAmbientAlpha <= 0.0f) {
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
    // TODO: switch to SkScalarNearlyEqual when either oval renderer is updated or we
    // have our own GeometryProc.
    if (path.isOval(nullptr) && path.getBounds().width() == path.getBounds().height()) {
        SkRRect rrect = SkRRect::MakeOval(path.getBounds());
        return this->directFilterRRectMaskGPU(context, rtContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    } else if (path.isRect(nullptr)) {
        SkRRect rrect = SkRRect::MakeRect(path.getBounds());
        return this->directFilterRRectMaskGPU(context, rtContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    }

    // TODO
    return false;
}

bool SkAmbientShadowMaskFilterImpl::directFilterRRectMaskGPU(GrContext*,
                                                             GrRenderTargetContext* rtContext,
                                                             GrPaint&& paint,
                                                             const GrClip& clip,
                                                             const SkMatrix& viewMatrix,
                                                             const SkStrokeRec& strokeRec,
                                                             const SkRRect& rrect,
                                                             const SkRRect& devRRect) const {
    // It's likely the caller has already done these checks, but we have to be sure.
    // TODO: support analytic blurring of general rrect

    // Fast path only supports filled rrects for now.
    // TODO: fill and stroke as well.
    if (SkStrokeRec::kFill_Style != strokeRec.getStyle()) {
        return false;
    }
    // Fast path only supports simple rrects with circular corners.
    SkASSERT(devRRect.allCornersCircular());
    if (!rrect.isRect() && !rrect.isOval() && !rrect.isSimple()) {
        return false;
    }
    // Fast path only supports uniform scale.
    SkScalar scaleFactors[2];
    if (!viewMatrix.getMinMaxScales(scaleFactors)) {
        // matrix is degenerate
        return false;
    }
    if (scaleFactors[0] != scaleFactors[1]) {
        return false;
    }
    SkScalar scaleFactor = scaleFactors[0];

    // For all of these, we need to ensure we have a rrect with radius >= 0.5f in device space
    const SkScalar minRadius = 0.5f / scaleFactor;
    bool isRect = rrect.getSimpleRadii().fX <= minRadius;

    // TODO: take flags into account when generating shadow data

    if (fAmbientAlpha > 0.0f) {
        SkScalar srcSpaceAmbientRadius = fOccluderHeight * kHeightFactor * kGeomFactor;
        const float umbraAlpha = (1.0f + SkTMax(fOccluderHeight * kHeightFactor, 0.0f));
        const SkScalar strokeWidth = srcSpaceAmbientRadius * umbraAlpha;

        // For the ambient rrect, we outset the offset rect by srcSpaceAmbientRadius
        // minus half the strokeWidth to get our stroke shape.
        SkScalar ambientPathOutset = SkTMax(srcSpaceAmbientRadius - strokeWidth * 0.5f,
                                            minRadius);

        SkRRect ambientRRect;
        if (isRect) {
            const SkRect temp = rrect.rect().makeOutset(ambientPathOutset, ambientPathOutset);
            ambientRRect = SkRRect::MakeRectXY(temp, ambientPathOutset, ambientPathOutset);
        } else {
             rrect.outset(ambientPathOutset, ambientPathOutset, &ambientRRect);
        }

        const SkScalar devSpaceAmbientRadius = strokeWidth * scaleFactor;

        GrPaint newPaint(paint);
        GrColor4f color = newPaint.getColor4f();
        color.fRGBA[3] *= fAmbientAlpha;
        newPaint.setColor4f(color);
        SkStrokeRec ambientStrokeRec(SkStrokeRec::kHairline_InitStyle);
        ambientStrokeRec.setStrokeStyle(strokeWidth, false);

        rtContext->drawShadowRRect(clip, std::move(newPaint), viewMatrix, ambientRRect,
                                   devSpaceAmbientRadius,
                                   GrStyle(ambientStrokeRec, nullptr));
    }

    return true;
}

sk_sp<GrTextureProxy> SkAmbientShadowMaskFilterImpl::filterMaskGPU(GrContext*,
                                                                   sk_sp<GrTextureProxy> srcProxy,
                                                                   const SkMatrix& ctm,
                                                                   const SkIRect& maskRect) const {
    // This filter is generative and doesn't operate on pre-existing masks
    return nullptr;
}

#endif // SK_SUPPORT_GPU

#ifndef SK_IGNORE_TO_STRING
void SkAmbientShadowMaskFilterImpl::toString(SkString* str) const {
    str->append("SkAmbientShadowMaskFilterImpl: (");

    str->append("occluderHeight: ");
    str->appendScalar(fOccluderHeight);
    str->append(" ");

    str->append("ambientAlpha: ");
    str->appendScalar(fAmbientAlpha);
    str->append(" ");

    str->append("flags: (");
    if (fFlags) {
        bool needSeparator = false;
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag),
                          "TransparentOccluder", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowFlags::kGaussianEdge_ShadowFlag),
                          "GaussianEdge", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowFlags::kLargerUmbra_ShadowFlag),
                          "LargerUmbra", &needSeparator);
    } else {
        str->append("None");
    }
    str->append("))");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkAmbientShadowMaskFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkAmbientShadowMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
