/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrTexture.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLSampler.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

class SkShadowMaskFilterImpl : public SkMaskFilter {
public:
    SkShadowMaskFilterImpl(SkScalar occluderHeight, const SkPoint3& lightPos, SkScalar lightRadius,
                           SkScalar ambientAlpha, SkScalar spotAlpha);

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
                             GrDrawContext* drawContext,
                             GrPaint* grp,
                             const GrClip&,
                             const SkMatrix& viewMatrix,
                             const SkStrokeRec& strokeRec,
                             const SkPath& path) const override;
    bool directFilterRRectMaskGPU(GrContext*,
                                  GrDrawContext* drawContext,
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
    bool asABlur(BlurRec*) const override;

    SK_TO_STRING_OVERRIDE()
        SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkShadowMaskFilterImpl)

protected:
    FilterReturn filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   NinePatch*) const override;

    FilterReturn filterRRectToNine(const SkRRect&, const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   NinePatch*) const override;

private:
    SkScalar fOccluderHeight;
    SkPoint3 fLightPos;
    SkScalar fLightRadius;
    SkScalar fAmbientAlpha;
    SkScalar fSpotAlpha;

    SkShadowMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkShadowMaskFilter;

    typedef SkMaskFilter INHERITED;
};

sk_sp<SkMaskFilter> SkShadowMaskFilter::Make(SkScalar occluderHeight, const SkPoint3& lightPos,
                                             SkScalar lightRadius, SkScalar ambientAlpha,
                                             SkScalar spotAlpha) {
    // add some param checks here for early exit

    return sk_sp<SkMaskFilter>(new SkShadowMaskFilterImpl(occluderHeight, lightPos, lightRadius,
                                                          ambientAlpha, spotAlpha));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkShadowMaskFilterImpl::SkShadowMaskFilterImpl(SkScalar occluderHeight, const SkPoint3& lightPos,
                                               SkScalar lightRadius, SkScalar ambientAlpha,
                                               SkScalar spotAlpha)
    : fOccluderHeight(occluderHeight)
    , fLightPos(lightPos)
    , fLightRadius(lightRadius)
    , fAmbientAlpha(ambientAlpha)
    , fSpotAlpha(spotAlpha) {
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

void SkShadowMaskFilterImpl::computeFastBounds(const SkRect& src,
                                               SkRect* dst) const {
    // TODO compute based on ambient + spot data
    dst->set(src.fLeft, src.fTop,
             src.fRight, src.fBottom);
}

bool SkShadowMaskFilterImpl::asABlur(BlurRec*) const {
    // can't be represented as a simple blur
    return false;
}

static bool rect_exceeds(const SkRect& r, SkScalar v) {
    return r.fLeft < -v || r.fTop < -v || r.fRight > v || r.fBottom > v ||
        r.width() > v || r.height() > v;
}

SkMaskFilter::FilterReturn
SkShadowMaskFilterImpl::filterRRectToNine(const SkRRect& rrect, const SkMatrix& matrix,
                                          const SkIRect& clipBounds,
                                          NinePatch* patch) const {
    SkASSERT(patch != nullptr);
    switch (rrect.getType()) {
        case SkRRect::kEmpty_Type:
            // Nothing to draw.
            return kFalse_FilterReturn;

        case SkRRect::kRect_Type:
            // We should have caught this earlier.
            SkASSERT(false);
            // Fall through.
        case SkRRect::kOval_Type:
            // The nine patch special case does not handle ovals, and we
            // already have code for rectangles.
            return kUnimplemented_FilterReturn;

            // These three can take advantage of this fast path.
        case SkRRect::kSimple_Type:
        case SkRRect::kNinePatch_Type:
        case SkRRect::kComplex_Type:
            break;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rrect.rect(), SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    // for now
    return kUnimplemented_FilterReturn;
}

SkMaskFilter::FilterReturn
SkShadowMaskFilterImpl::filterRectsToNine(const SkRect rects[], int count,
                                          const SkMatrix& matrix,
                                          const SkIRect& clipBounds,
                                          NinePatch* patch) const {
    if (count < 1 || count > 2) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rects[0], SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    // for now
    return kUnimplemented_FilterReturn;
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

    return SkShadowMaskFilter::Make(occluderHeight, lightPos, lightRadius,
                                    ambientAlpha, spotAlpha);
}

void SkShadowMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fOccluderHeight);
    buffer.writeScalar(fLightPos.fX);
    buffer.writeScalar(fLightPos.fY);
    buffer.writeScalar(fLightPos.fZ);
    buffer.writeScalar(fLightRadius);
    buffer.writeScalar(fAmbientAlpha);
    buffer.writeScalar(fSpotAlpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

bool SkShadowMaskFilterImpl::canFilterMaskGPU(const SkRRect& devRRect,
                                              const SkIRect& clipBounds,
                                              const SkMatrix& ctm,
                                              SkRect* maskRect) const {
    // TODO
    return false;
}

bool SkShadowMaskFilterImpl::directFilterMaskGPU(GrTextureProvider* texProvider,
                                                 GrDrawContext* drawContext,
                                                 GrPaint* grp,
                                                 const GrClip& clip,
                                                 const SkMatrix& viewMatrix,
                                                 const SkStrokeRec& strokeRec,
                                                 const SkPath& path) const {
    SkASSERT(drawContext);

    // TODO
    return false;
}


bool SkShadowMaskFilterImpl::directFilterRRectMaskGPU(GrContext*,
                                                      GrDrawContext* drawContext,
                                                      GrPaint* grp,
                                                      const GrClip&,
                                                      const SkMatrix& viewMatrix,
                                                      const SkStrokeRec& strokeRec,
                                                      const SkRRect& rrect,
                                                      const SkRRect& devRRect) const {
    // TODO
    return false;
}

bool SkShadowMaskFilterImpl::filterMaskGPU(GrTexture* src,
                                           const SkMatrix& ctm,
                                           const SkIRect& maskRect,
                                           GrTexture** result) const {
    // TODO
    return false;
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkShadowMaskFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkShadowMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
