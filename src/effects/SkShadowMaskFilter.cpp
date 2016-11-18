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
                             GrPaint* grp,
                             const GrClip&,
                             const SkMatrix& viewMatrix,
                             const SkStrokeRec& strokeRec,
                             const SkPath& path) const override;
    bool directFilterRRectMaskGPU(GrContext*,
                                  GrRenderTargetContext* drawContext,
                                  GrPaint* grp,
                                  const GrClip&,
                                  const SkMatrix& viewMatrix,
                                  const SkStrokeRec& strokeRec,
                                  const SkRRect& rrect,
                                  const SkRRect& devRRect) const override;
    bool filterMaskGPU(GrTexture* src,
                       const SkMatrix& ctm,
                       const SkIRect& maskRect,
                       GrTexture** result) const override;
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

class GrShadowEdgeEffect : public GrFragmentProcessor {
public:
    enum Type {
        kGaussian_Type,
        kSmoothStep_Type,
        kGeometric_Type
    };

    static sk_sp<GrFragmentProcessor> Make(Type type);

    ~GrShadowEdgeEffect() override {}
    const char* name() const override { return "GrShadowEdge"; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    GrShadowEdgeEffect(Type type);

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                               GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    Type fType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> GrShadowEdgeEffect::Make(Type type) {
    return sk_sp<GrFragmentProcessor>(new GrShadowEdgeEffect(type));
}

void GrShadowEdgeEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

GrShadowEdgeEffect::GrShadowEdgeEffect(Type type)
    : fType(type) {
    this->initClassID<GrShadowEdgeEffect>();
    // TODO: remove this when we switch to a non-distance based approach
    // enable output of distance information for shape
    fUsesDistanceVectorField = true;
}

bool GrShadowEdgeEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrShadowEdgeEffect& see = other.cast<GrShadowEdgeEffect>();
    return fType == see.fType;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrShadowEdgeEffect);

sk_sp<GrFragmentProcessor> GrShadowEdgeEffect::TestCreate(GrProcessorTestData* d) {
    int t = d->fRandom->nextRangeU(0, 2);
    GrShadowEdgeEffect::Type type = kGaussian_Type;
    if (1 == t) {
        type = kSmoothStep_Type;
    } else if (2 == t) {
        type = kGeometric_Type;
    }
    return GrShadowEdgeEffect::Make(type);
}

//////////////////////////////////////////////////////////////////////////////

class GrGLShadowEdgeEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLShadowEdgeEffect::emitCode(EmitArgs& args) {

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    // TODO: handle smoothstep and geometric cases
    if (!args.fGpImplementsDistanceVector) {
        fragBuilder->codeAppendf("// GP does not implement fsDistanceVector - "
                                 " returning semi-transparent black in GrGLShadowEdgeEffect\n");
        fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);
        fragBuilder->codeAppendf("%s = vec4(0.0, 0.0, 0.0, color.r);", args.fOutputColor);
    } else {
        fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);
        fragBuilder->codeAppend("float radius = color.r*256.0*64.0 + color.g*64.0;");
        fragBuilder->codeAppend("float pad = color.b*64.0;");

        fragBuilder->codeAppendf("float factor = 1.0 - clamp((%s.z - pad)/radius, 0.0, 1.0);",
                                 fragBuilder->distanceVectorName());
        fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
        fragBuilder->codeAppendf("%s = factor*vec4(0.0, 0.0, 0.0, color.a);",
                                 args.fOutputColor);
    }
}

void GrGLShadowEdgeEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                    const GrProcessor& proc) {
}

void GrShadowEdgeEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                              GrProcessorKeyBuilder* b) const {
    GrGLShadowEdgeEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrShadowEdgeEffect::onCreateGLSLInstance() const {
    return new GrGLShadowEdgeEffect;
}

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
                                                 GrPaint* grp,
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
        return this->directFilterRRectMaskGPU(nullptr, drawContext, grp, clip, SkMatrix::I(),
                                              strokeRec, rrect, rrect);
    } else if (path.isRect(nullptr)) {
        SkRRect rrect = SkRRect::MakeRect(path.getBounds());
        return this->directFilterRRectMaskGPU(nullptr, drawContext, grp, clip, SkMatrix::I(),
                                              strokeRec, rrect, rrect);
    }

    // TODO
    return false;
}

#define MAX_BLUR_RADIUS 16383.75f
#define MAX_PAD         64

bool SkShadowMaskFilterImpl::directFilterRRectMaskGPU(GrContext*,
                                                      GrRenderTargetContext* drawContext,
                                                      GrPaint* grp,
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
        // the device-space radius sent to the blur shader must fit in 14.2 fixed point
        if (srcSpaceAmbientRadius*scaleFactor > MAX_BLUR_RADIUS) {
            srcSpaceAmbientRadius = MAX_BLUR_RADIUS / scaleFactor;
        }
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

        // we outset the stroke a little to cover up AA on the interior edge
        float pad = 0.5f;
        // handle scale of radius and pad due to CTM
        pad *= scaleFactor;
        const SkScalar devSpaceAmbientRadius = srcSpaceAmbientRadius * scaleFactor;
        SkASSERT(devSpaceAmbientRadius <= MAX_BLUR_RADIUS);
        SkASSERT(pad < MAX_PAD);
        // convert devSpaceAmbientRadius to 14.2 fixed point and place in the R & G components
        // convert pad to 6.2 fixed point and place in the B component
        // TODO: replace this with separate vertex attributes passed by a new GeoProc.
        // For now we can't easily pass attributes to the fragment shader, so we're overriding
        // the paint color.
        uint16_t iDevSpaceAmbientRadius = (uint16_t)(4.0f * devSpaceAmbientRadius);

        GrPaint newPaint(*grp);
        newPaint.setAntiAlias(true);
        SkStrokeRec ambientStrokeRec(SkStrokeRec::kHairline_InitStyle);
        ambientStrokeRec.setStrokeStyle(srcSpaceAmbientRadius + 2.0f * pad, false);
        newPaint.setColor4f(GrColor4f((iDevSpaceAmbientRadius >> 8)/255.f,
                                      (iDevSpaceAmbientRadius & 0xff)/255.f,
                                      4.0f * pad/255.f,
                                      fAmbientAlpha));

        sk_sp<GrFragmentProcessor> fp(GrShadowEdgeEffect::Make(GrShadowEdgeEffect::kGaussian_Type));
        // TODO: switch to coverage FP
        newPaint.addColorFragmentProcessor(std::move(fp));
        drawContext->drawRRect(clip, newPaint, viewMatrix, ambientRRect,
                               GrStyle(ambientStrokeRec, nullptr));
    }

    if (fSpotAlpha > 0.0f) {
        float zRatio = SkTPin(fOccluderHeight / (fLightPos.fZ - fOccluderHeight), 0.0f, 0.95f);

        SkScalar srcSpaceSpotRadius = 2.0f * fLightRadius * zRatio;
        // the device-space radius sent to the blur shader must fit in 14.2 fixed point
        if (srcSpaceSpotRadius > MAX_BLUR_RADIUS) {
            srcSpaceSpotRadius = MAX_BLUR_RADIUS;
        }

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
        // We also add 1/2 to cover up AA on the interior edge.
        SkScalar scaleOffset = (scale - 1.0f) * SkTMax(SkTMax(SkTAbs(rrect.rect().fLeft),
                                                              SkTAbs(rrect.rect().fRight)),
                                                       SkTMax(SkTAbs(rrect.rect().fTop),
                                                              SkTAbs(rrect.rect().fBottom)));
        SkScalar insetAmount = spotOffset.length() - (0.5f * srcSpaceSpotRadius) +
                               scaleOffset + 0.5f;

        // Compute area
        SkScalar strokeWidth = srcSpaceSpotRadius + insetAmount;
        SkScalar strokedArea = 2.0f*strokeWidth *
                               (spotShadowRRect.width() + spotShadowRRect.height());
        SkScalar filledArea = (spotShadowRRect.height() + srcSpaceSpotRadius) *
                              (spotShadowRRect.width() + srcSpaceSpotRadius);

        GrPaint newPaint(*grp);
        newPaint.setAntiAlias(true);
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
        SkASSERT(devSpaceSpotRadius <= MAX_BLUR_RADIUS);

        const SkScalar devSpaceSpotPad = 0;
        SkASSERT(devSpaceSpotPad < MAX_PAD);

        // convert devSpaceSpotRadius to 14.2 fixed point and place in the R & G
        // components convert devSpaceSpotPad to 6.2 fixed point and place in the B component
        // TODO: replace this with separate vertex attributes passed by a new GeoProc.
        // For now we can't easily pass attributes to the fragment shader, so we're overriding
        // the paint color.
        uint16_t iDevSpaceSpotRadius = (uint16_t)(4.0f * devSpaceSpotRadius);
        newPaint.setColor4f(GrColor4f((iDevSpaceSpotRadius >> 8) / 255.f,
                                      (iDevSpaceSpotRadius & 0xff) / 255.f,
                                      devSpaceSpotPad,
                                      fSpotAlpha));
        spotShadowRRect.offset(spotOffset.fX, spotOffset.fY);

        sk_sp<GrFragmentProcessor> fp(GrShadowEdgeEffect::Make(GrShadowEdgeEffect::kGaussian_Type));
        // TODO: switch to coverage FP
        newPaint.addColorFragmentProcessor(std::move(fp));

        drawContext->drawRRect(clip, newPaint, viewMatrix, spotShadowRRect,
                               GrStyle(spotStrokeRec, nullptr));
    }

    return true;
}

bool SkShadowMaskFilterImpl::filterMaskGPU(GrTexture* src,
                                           const SkMatrix& ctm,
                                           const SkIRect& maskRect,
                                           GrTexture** result) const {
    // TODO
    return false;
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
