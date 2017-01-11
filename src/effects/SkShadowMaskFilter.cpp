/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShadowMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkStringUtils.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrStyle.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkStrokeRec.h"
#endif

class SkShadowMaskFilterImpl : public SkMaskFilter {
public:
    SkShadowMaskFilterImpl(SkScalar occluderHeight, const SkPoint3& lightPos, SkScalar lightRadius,
                           SkScalar ambientAlpha, SkScalar spotAlpha, uint32_t flags);

    // overrides from SkMaskFilter
    SkMask::Format getFormat() const override;
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

#if SK_SUPPORT_GPU
    bool canFilterMaskGPU(const SkRRect& devRRect,
                          const SkIRect& clipBounds,
                          const SkMatrix& ctm,
                          SkRect* maskRect) const override;
    bool directFilterMaskGPU(GrTextureProvider* texProvider,
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
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkShadowMaskFilterImpl)

private:
    SkScalar fOccluderHeight;
    SkPoint3 fLightPos;
    SkScalar fLightRadius;
    SkScalar fAmbientAlpha;
    SkScalar fSpotAlpha;
    uint32_t fFlags;

    SkShadowMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkShadowMaskFilter;

    typedef SkMaskFilter INHERITED;
};

sk_sp<SkMaskFilter> SkShadowMaskFilter::Make(SkScalar occluderHeight, const SkPoint3& lightPos,
                                             SkScalar lightRadius, SkScalar ambientAlpha,
                                             SkScalar spotAlpha, uint32_t flags) {
    // add some param checks here for early exit

    return sk_sp<SkMaskFilter>(new SkShadowMaskFilterImpl(occluderHeight, lightPos, lightRadius,
                                                          ambientAlpha, spotAlpha, flags));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkShadowMaskFilterImpl::SkShadowMaskFilterImpl(SkScalar occluderHeight, const SkPoint3& lightPos,
                                               SkScalar lightRadius, SkScalar ambientAlpha,
                                               SkScalar spotAlpha, uint32_t flags)
    : fOccluderHeight(occluderHeight)
    , fLightPos(lightPos)
    , fLightRadius(lightRadius)
    , fAmbientAlpha(ambientAlpha)
    , fSpotAlpha(spotAlpha)
    , fFlags(flags) {
    SkASSERT(fOccluderHeight > 0);
    SkASSERT(fLightPos.z() > 0 && fLightPos.z() > fOccluderHeight);
    SkASSERT(fLightRadius > 0);
    SkASSERT(fAmbientAlpha >= 0);
    SkASSERT(fSpotAlpha >= 0);
}

SkMask::Format SkShadowMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkShadowMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                        const SkMatrix& matrix,
                                        SkIPoint* margin) const {
    // TODO something
    return false;
}

void SkShadowMaskFilterImpl::computeFastBounds(const SkRect& src, SkRect* dst) const {
    // TODO compute based on ambient + spot data
    dst->set(src.fLeft, src.fTop, src.fRight, src.fBottom);
}

sk_sp<SkFlattenable> SkShadowMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    const SkScalar occluderHeight = buffer.readScalar();
    const SkScalar lightX = buffer.readScalar();
    const SkScalar lightY = buffer.readScalar();
    const SkScalar lightZ = buffer.readScalar();
    const SkPoint3 lightPos = SkPoint3::Make(lightX, lightY, lightZ);
    const SkScalar lightRadius = buffer.readScalar();
    const SkScalar ambientAlpha = buffer.readScalar();
    const SkScalar spotAlpha = buffer.readScalar();
    const uint32_t flags = buffer.readUInt();

    return SkShadowMaskFilter::Make(occluderHeight, lightPos, lightRadius,
                                    ambientAlpha, spotAlpha, flags);
}

void SkShadowMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fOccluderHeight);
    buffer.writeScalar(fLightPos.fX);
    buffer.writeScalar(fLightPos.fY);
    buffer.writeScalar(fLightPos.fZ);
    buffer.writeScalar(fLightRadius);
    buffer.writeScalar(fAmbientAlpha);
    buffer.writeScalar(fSpotAlpha);
    buffer.writeUInt(fFlags);
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkShadowMaskFilterImpl::canFilterMaskGPU(const SkRRect& devRRect,
                                              const SkIRect& clipBounds,
                                              const SkMatrix& ctm,
                                              SkRect* maskRect) const {
    // TODO
    *maskRect = devRRect.rect();
    return true;
}

bool SkShadowMaskFilterImpl::directFilterMaskGPU(GrTextureProvider* texProvider,
                                                 GrRenderTargetContext* drawContext,
                                                 GrPaint&& paint,
                                                 const GrClip& clip,
                                                 const SkMatrix& viewMatrix,
                                                 const SkStrokeRec& strokeRec,
                                                 const SkPath& path) const {
    SkASSERT(drawContext);
    // TODO: this will not handle local coordinates properly

    // if circle
    // TODO: switch to SkScalarNearlyEqual when either oval renderer is updated or we
    // have our own GeometryProc.
    if (path.isOval(nullptr) && path.getBounds().width() == path.getBounds().height()) {
        SkRRect rrect = SkRRect::MakeOval(path.getBounds());
        return this->directFilterRRectMaskGPU(nullptr, drawContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    } else if (path.isRect(nullptr)) {
        SkRRect rrect = SkRRect::MakeRect(path.getBounds());
        return this->directFilterRRectMaskGPU(nullptr, drawContext, std::move(paint), clip,
                                              SkMatrix::I(), strokeRec, rrect, rrect);
    }

    // TODO
    return false;
}

bool SkShadowMaskFilterImpl::directFilterRRectMaskGPU(GrContext*,
                                                      GrRenderTargetContext* renderTargetContext,
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
        static const float kHeightFactor = 1.0f / 128.0f;
        static const float kGeomFactor = 64.0f;

        SkScalar srcSpaceAmbientRadius = fOccluderHeight * kHeightFactor * kGeomFactor;
        const float umbraAlpha = 1.0f / (1.0f + SkTMax(fOccluderHeight * kHeightFactor, 0.0f));
        const SkScalar ambientOffset = srcSpaceAmbientRadius * umbraAlpha;

        // For the ambient rrect, we inset the offset rect by half the srcSpaceAmbientRadius
        // to get our stroke shape.
        SkScalar ambientPathOutset = SkTMax(ambientOffset - srcSpaceAmbientRadius * 0.5f,
                                              minRadius);

        SkRRect ambientRRect;
        if (isRect) {
            const SkRect temp = rrect.rect().makeOutset(ambientPathOutset, ambientPathOutset);
            ambientRRect = SkRRect::MakeRectXY(temp, ambientPathOutset, ambientPathOutset);
        } else {
             rrect.outset(ambientPathOutset, ambientPathOutset, &ambientRRect);
        }

        const SkScalar devSpaceAmbientRadius = srcSpaceAmbientRadius * scaleFactor;

        GrPaint newPaint(paint);
        GrColor4f color = newPaint.getColor4f();
        color.fRGBA[3] *= fAmbientAlpha;
        newPaint.setColor4f(color);
        SkStrokeRec ambientStrokeRec(SkStrokeRec::kHairline_InitStyle);
        ambientStrokeRec.setStrokeStyle(srcSpaceAmbientRadius, false);

        renderTargetContext->drawShadowRRect(clip, std::move(newPaint), viewMatrix, ambientRRect,
                                             devSpaceAmbientRadius,
                                             GrStyle(ambientStrokeRec, nullptr));
    }

    if (fSpotAlpha > 0.0f) {
        float zRatio = SkTPin(fOccluderHeight / (fLightPos.fZ - fOccluderHeight), 0.0f, 0.95f);

        SkScalar srcSpaceSpotRadius = 2.0f * fLightRadius * zRatio;

        SkRRect spotRRect;
        if (isRect) {
            spotRRect = SkRRect::MakeRectXY(rrect.rect(), minRadius, minRadius);
        } else {
            spotRRect = rrect;
        }

        SkRRect spotShadowRRect;
        // Compute the scale and translation for the spot shadow.
        const SkScalar scale = fLightPos.fZ / (fLightPos.fZ - fOccluderHeight);
        spotRRect.transform(SkMatrix::MakeScale(scale, scale), &spotShadowRRect);

        SkPoint center = SkPoint::Make(spotShadowRRect.rect().centerX(),
                                       spotShadowRRect.rect().centerY());
        SkMatrix ctmInverse;
        if (!viewMatrix.invert(&ctmInverse)) {
            SkDebugf("Matrix is degenerate. Will not render spot shadow!\n");
            //**** TODO: this is not good
            return true;
        }
        SkPoint lightPos2D = SkPoint::Make(fLightPos.fX, fLightPos.fY);
        ctmInverse.mapPoints(&lightPos2D, 1);
        const SkPoint spotOffset = SkPoint::Make(zRatio*(center.fX - lightPos2D.fX),
                                                 zRatio*(center.fY - lightPos2D.fY));

        // We want to extend the stroked area in so that it meets up with the caster
        // geometry. The stroked geometry will, by definition already be inset half the
        // stroke width but we also have to account for the scaling.
        SkScalar scaleOffset = (scale - 1.0f) * SkTMax(SkTMax(SkTAbs(rrect.rect().fLeft),
                                                              SkTAbs(rrect.rect().fRight)),
                                                       SkTMax(SkTAbs(rrect.rect().fTop),
                                                              SkTAbs(rrect.rect().fBottom)));
        SkScalar insetAmount = spotOffset.length() - (0.5f * srcSpaceSpotRadius) + scaleOffset;

        // Compute area
        SkScalar strokeWidth = srcSpaceSpotRadius + insetAmount;
        SkScalar strokedArea = 2.0f*strokeWidth *
                               (spotShadowRRect.width() + spotShadowRRect.height());
        SkScalar filledArea = (spotShadowRRect.height() + srcSpaceSpotRadius) *
                              (spotShadowRRect.width() + srcSpaceSpotRadius);

        GrColor4f color = paint.getColor4f();
        color.fRGBA[3] *= fSpotAlpha;
        paint.setColor4f(color);

        SkStrokeRec spotStrokeRec(SkStrokeRec::kFill_InitStyle);
        // If the area of the stroked geometry is larger than the fill geometry,
        // or if the caster is transparent, just fill it.
        if (strokedArea > filledArea ||
            fFlags & SkShadowMaskFilter::kTransparentOccluder_ShadowFlag) {
            spotStrokeRec.setStrokeStyle(srcSpaceSpotRadius, true);
        } else {
            // Since we can't have unequal strokes, inset the shadow rect so the inner
            // and outer edges of the stroke will land where we want.
            SkRect insetRect = spotShadowRRect.rect().makeInset(insetAmount / 2.0f,
                                                                insetAmount / 2.0f);
            SkScalar insetRad = SkTMax(spotShadowRRect.getSimpleRadii().fX - insetAmount / 2.0f,
                                       minRadius);
            spotShadowRRect = SkRRect::MakeRectXY(insetRect, insetRad, insetRad);
            spotStrokeRec.setStrokeStyle(strokeWidth, false);
        }

        // handle scale of radius and pad due to CTM
        const SkScalar devSpaceSpotRadius = srcSpaceSpotRadius * scaleFactor;

        spotShadowRRect.offset(spotOffset.fX, spotOffset.fY);

        renderTargetContext->drawShadowRRect(clip, std::move(paint), viewMatrix, spotShadowRRect,
                                             devSpaceSpotRadius, GrStyle(spotStrokeRec, nullptr));
    }

    return true;
}

sk_sp<GrTextureProxy> SkShadowMaskFilterImpl::filterMaskGPU(GrContext*,
                                                            sk_sp<GrTextureProxy> srcProxy,
                                                            const SkMatrix& ctm,
                                                            const SkIRect& maskRect) const {
    // This filter is generative and doesn't operate on pre-existing masks
    return nullptr;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkShadowMaskFilterImpl::toString(SkString* str) const {
    str->append("SkShadowMaskFilterImpl: (");

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

    str->append("ambientAlpha: ");
    str->appendScalar(fAmbientAlpha);
    str->append(" ");

    str->append("spotAlpha: ");
    str->appendScalar(fSpotAlpha);
    str->append(" ");

    str->append("flags: (");
    if (fFlags) {
        bool needSeparator = false;
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowMaskFilter::kTransparentOccluder_ShadowFlag),
                          "TransparentOccluder", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowMaskFilter::kGaussianEdge_ShadowFlag),
                          "GaussianEdge", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fFlags & SkShadowMaskFilter::kLargerUmbra_ShadowFlag),
                          "LargerUmbra", &needSeparator);
    } else {
        str->append("None");
    }
    str->append("))");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkShadowMaskFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkShadowMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
