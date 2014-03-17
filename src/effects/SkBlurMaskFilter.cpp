
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkGpuBlurUtils.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMaskFilter.h"
#include "SkRRect.h"
#include "SkRTConf.h"
#include "SkStringUtils.h"
#include "SkStrokeRec.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "GrEffect.h"
#include "gl/GrGLEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#include "SkGrPixelRef.h"
#endif

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurMaskFilter::BlurStyle, uint32_t flags);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat() const SK_OVERRIDE;
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual bool canFilterMaskGPU(const SkRect& devBounds,
                                  const SkIRect& clipBounds,
                                  const SkMatrix& ctm,
                                  SkRect* maskRect) const SK_OVERRIDE;
    virtual bool directFilterMaskGPU(GrContext* context,
                                     GrPaint* grp,
                                     const SkStrokeRec& strokeRec,
                                     const SkPath& path) const SK_OVERRIDE;
    virtual bool directFilterRRectMaskGPU(GrContext* context,
                                          GrPaint* grp,
                                          const SkStrokeRec& strokeRec,
                                          const SkRRect& rrect) const SK_OVERRIDE;

    virtual bool filterMaskGPU(GrTexture* src,
                               const SkMatrix& ctm,
                               const SkRect& maskRect,
                               GrTexture** result,
                               bool canOverwriteSrc) const SK_OVERRIDE;
#endif

    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurMaskFilterImpl)

protected:
    virtual FilterReturn filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                           const SkIRect& clipBounds,
                                           NinePatch*) const SK_OVERRIDE;

    virtual FilterReturn filterRRectToNine(const SkRRect&, const SkMatrix&,
                                           const SkIRect& clipBounds,
                                           NinePatch*) const SK_OVERRIDE;

    bool filterRectMask(SkMask* dstM, const SkRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMask::CreateMode createMode) const;
    bool filterRRectMask(SkMask* dstM, const SkRRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMask::CreateMode createMode) const;

private:
    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar kMAX_BLUR_SIGMA;

    SkScalar                    fSigma;
    SkBlurMaskFilter::BlurStyle fBlurStyle;
    uint32_t                    fBlurFlags;

    SkBlurMaskFilterImpl(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    SkScalar computeXformedSigma(const SkMatrix& ctm) const {
        bool ignoreTransform = SkToBool(fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag);

        SkScalar xformedSigma = ignoreTransform ? fSigma : ctm.mapRadius(fSigma);
        return SkMinScalar(xformedSigma, kMAX_BLUR_SIGMA);
    }

    typedef SkMaskFilter INHERITED;
};

const SkScalar SkBlurMaskFilterImpl::kMAX_BLUR_SIGMA = SkIntToScalar(128);

SkMaskFilter* SkBlurMaskFilter::Create(SkScalar radius,
                                       SkBlurMaskFilter::BlurStyle style,
                                       uint32_t flags) {
    // use !(radius > 0) instead of radius <= 0 to reject NaN values
    if (!(radius > 0) || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount
        || flags > SkBlurMaskFilter::kAll_BlurFlag) {
        return NULL;
    }

    SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(radius);

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (sigma, style, flags));
}

SkMaskFilter* SkBlurMaskFilter::Create(SkBlurMaskFilter::BlurStyle style,
                                       SkScalar sigma,
                                       uint32_t flags) {
    // use !(sigma > 0) instead of sigma <= 0 to reject NaN values
    if (!(sigma > 0) || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount
        || flags > SkBlurMaskFilter::kAll_BlurFlag) {
        return NULL;
    }

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (sigma, style, flags));
}

///////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar sigma,
                                           SkBlurMaskFilter::BlurStyle style,
                                           uint32_t flags)
    : fSigma(sigma), fBlurStyle(style), fBlurFlags(flags) {
#if 0
    fGamma = NULL;
    if (gammaScale) {
        fGamma = new U8[256];
        if (gammaScale > 0)
            SkBlurMask::BuildSqrGamma(fGamma, gammaScale);
        else
            SkBlurMask::BuildSqrtGamma(fGamma, -gammaScale);
    }
#endif
    SkASSERT(fSigma >= 0);
    SkASSERT((unsigned)style < SkBlurMaskFilter::kBlurStyleCount);
    SkASSERT(flags <= SkBlurMaskFilter::kAll_BlurFlag);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const{
    SkScalar sigma = this->computeXformedSigma(matrix);

    SkBlurMask::Quality blurQuality =
        (fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag) ?
            SkBlurMask::kHigh_Quality : SkBlurMask::kLow_Quality;

    return SkBlurMask::BoxBlur(dst, src, sigma, (SkBlurMask::Style)fBlurStyle,
                               blurQuality, margin);
}

bool SkBlurMaskFilterImpl::filterRectMask(SkMask* dst, const SkRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const{
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRect(sigma, dst, r, (SkBlurMask::Style)fBlurStyle,
                                margin, createMode);
}

bool SkBlurMaskFilterImpl::filterRRectMask(SkMask* dst, const SkRRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const{
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRRect(sigma, dst, r, (SkBlurMask::Style)fBlurStyle,
                                margin, createMode);
}

#include "SkCanvas.h"

static bool prepare_to_draw_into_mask(const SkRect& bounds, SkMask* mask) {
    SkASSERT(mask != NULL);

    bounds.roundOut(&mask->fBounds);
    mask->fRowBytes = SkAlign4(mask->fBounds.width());
    mask->fFormat = SkMask::kA8_Format;
    const size_t size = mask->computeImageSize();
    mask->fImage = SkMask::AllocImage(size);
    if (NULL == mask->fImage) {
        return false;
    }

    // FIXME: use sk_calloc in AllocImage?
    sk_bzero(mask->fImage, size);
    return true;
}

static bool draw_rrect_into_mask(const SkRRect rrect, SkMask* mask) {
    if (!prepare_to_draw_into_mask(rrect.rect(), mask)) {
        return false;
    }

    // FIXME: This code duplicates code in draw_rects_into_mask, below. Is there a
    // clean way to share more code?
    SkBitmap bitmap;
    bitmap.installMaskPixels(*mask);

    SkCanvas canvas(bitmap);
    canvas.translate(-SkIntToScalar(mask->fBounds.left()),
                     -SkIntToScalar(mask->fBounds.top()));

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawRRect(rrect, paint);
    return true;
}

static bool draw_rects_into_mask(const SkRect rects[], int count, SkMask* mask) {
    if (!prepare_to_draw_into_mask(rects[0], mask)) {
        return false;
    }

    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::Make(mask->fBounds.width(),
                                           mask->fBounds.height(),
                                           kAlpha_8_SkColorType,
                                           kPremul_SkAlphaType),
                         mask->fImage, mask->fRowBytes, NULL, NULL);

    SkCanvas canvas(bitmap);
    canvas.translate(-SkIntToScalar(mask->fBounds.left()),
                     -SkIntToScalar(mask->fBounds.top()));

    SkPaint paint;
    paint.setAntiAlias(true);

    if (1 == count) {
        canvas.drawRect(rects[0], paint);
    } else {
        // todo: do I need a fast way to do this?
        SkPath path;
        path.addRect(rects[0]);
        path.addRect(rects[1]);
        path.setFillType(SkPath::kEvenOdd_FillType);
        canvas.drawPath(path, paint);
    }
    return true;
}

static bool rect_exceeds(const SkRect& r, SkScalar v) {
    return r.fLeft < -v || r.fTop < -v || r.fRight > v || r.fBottom > v ||
           r.width() > v || r.height() > v;
}

#ifdef SK_IGNORE_FAST_RRECT_BLUR
SK_CONF_DECLARE( bool, c_analyticBlurRRect, "mask.filter.blur.analyticRRect", false, "Use the faster analytic blur approach for ninepatch rects" );
#else
SK_CONF_DECLARE( bool, c_analyticBlurRRect, "mask.filter.blur.analyticRRect", true, "Use the faster analytic blur approach for ninepatch round rects" );
#endif

SkMaskFilter::FilterReturn
SkBlurMaskFilterImpl::filterRRectToNine(const SkRRect& rrect, const SkMatrix& matrix,
                                        const SkIRect& clipBounds,
                                        NinePatch* patch) const {
    SkASSERT(patch != NULL);
    switch (rrect.getType()) {
        case SkRRect::kUnknown_Type:
            // Unknown should never be returned.
            SkASSERT(false);
            // Fall through.
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

        case SkRRect::kSimple_Type:
            // Fall through.
        case SkRRect::kComplex_Type:
            // These can take advantage of this fast path.
            break;
    }

    // TODO: report correct metrics for innerstyle, where we do not grow the
    // total bounds, but we do need an inset the size of our blur-radius
    if (SkBlurMaskFilter::kInner_BlurStyle == fBlurStyle) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rrect.rect(), SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    SkIPoint margin;
    SkMask  srcM, dstM;
    rrect.rect().roundOut(&srcM.fBounds);
    srcM.fImage = NULL;
    srcM.fFormat = SkMask::kA8_Format;
    srcM.fRowBytes = 0;

    bool filterResult = false;
    if (c_analyticBlurRRect) {
        // special case for fast round rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRRectMask(&dstM, rrect, matrix, &margin,
                                            SkMask::kJustComputeBounds_CreateMode);
    }

    if (!filterResult) {
        filterResult = this->filterMask(&dstM, srcM, matrix, &margin);
    }

    if (!filterResult) {
        return kFalse_FilterReturn;
    }

    // Now figure out the appropriate width and height of the smaller round rectangle
    // to stretch. It will take into account the larger radius per side as well as double
    // the margin, to account for inner and outer blur.
    const SkVector& UL = rrect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& UR = rrect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& LR = rrect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& LL = rrect.radii(SkRRect::kLowerLeft_Corner);

    const SkScalar leftUnstretched = SkTMax(UL.fX, LL.fX) + SkIntToScalar(2 * margin.fX);
    const SkScalar rightUnstretched = SkTMax(UR.fX, LR.fX) + SkIntToScalar(2 * margin.fX);

    // Extra space in the middle to ensure an unchanging piece for stretching. Use 3 to cover
    // any fractional space on either side plus 1 for the part to stretch.
    const SkScalar stretchSize = SkIntToScalar(3);

    const SkScalar totalSmallWidth = leftUnstretched + rightUnstretched + stretchSize;
    if (totalSmallWidth >= rrect.rect().width()) {
        // There is no valid piece to stretch.
        return kUnimplemented_FilterReturn;
    }

    const SkScalar topUnstretched = SkTMax(UL.fY, UR.fY) + SkIntToScalar(2 * margin.fY);
    const SkScalar bottomUnstretched = SkTMax(LL.fY, LR.fY) + SkIntToScalar(2 * margin.fY);

    const SkScalar totalSmallHeight = topUnstretched + bottomUnstretched + stretchSize;
    if (totalSmallHeight >= rrect.rect().height()) {
        // There is no valid piece to stretch.
        return kUnimplemented_FilterReturn;
    }

    SkRect smallR = SkRect::MakeWH(totalSmallWidth, totalSmallHeight);

    SkRRect smallRR;
    SkVector radii[4];
    radii[SkRRect::kUpperLeft_Corner] = UL;
    radii[SkRRect::kUpperRight_Corner] = UR;
    radii[SkRRect::kLowerRight_Corner] = LR;
    radii[SkRRect::kLowerLeft_Corner] = LL;
    smallRR.setRectRadii(smallR, radii);

    bool analyticBlurWorked = false;
    if (c_analyticBlurRRect) {
        analyticBlurWorked =
            this->filterRRectMask(&patch->fMask, smallRR, matrix, &margin,
                                  SkMask::kComputeBoundsAndRenderImage_CreateMode);
    }

    if (!analyticBlurWorked) {
        if (!draw_rrect_into_mask(smallRR, &srcM)) {
            return kFalse_FilterReturn;
        }

        SkAutoMaskFreeImage amf(srcM.fImage);

        if (!this->filterMask(&patch->fMask, srcM, matrix, &margin)) {
            return kFalse_FilterReturn;
        }
    }

    patch->fMask.fBounds.offsetTo(0, 0);
    patch->fOuterRect = dstM.fBounds;
    patch->fCenter.fX = SkScalarCeilToInt(leftUnstretched) + 1;
    patch->fCenter.fY = SkScalarCeilToInt(topUnstretched) + 1;
    return kTrue_FilterReturn;
}

SK_CONF_DECLARE( bool, c_analyticBlurNinepatch, "mask.filter.analyticNinePatch", true, "Use the faster analytic blur approach for ninepatch rects" );

SkMaskFilter::FilterReturn
SkBlurMaskFilterImpl::filterRectsToNine(const SkRect rects[], int count,
                                        const SkMatrix& matrix,
                                        const SkIRect& clipBounds,
                                        NinePatch* patch) const {
    if (count < 1 || count > 2) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: report correct metrics for innerstyle, where we do not grow the
    // total bounds, but we do need an inset the size of our blur-radius
    if (SkBlurMaskFilter::kInner_BlurStyle == fBlurStyle ||
        SkBlurMaskFilter::kOuter_BlurStyle == fBlurStyle) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rects[0], SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    SkIPoint margin;
    SkMask  srcM, dstM;
    rects[0].roundOut(&srcM.fBounds);
    srcM.fImage = NULL;
    srcM.fFormat = SkMask::kA8_Format;
    srcM.fRowBytes = 0;

    bool filterResult = false;
    if (count == 1 && c_analyticBlurNinepatch) {
        // special case for fast rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRectMask(&dstM, rects[0], matrix, &margin,
                                            SkMask::kJustComputeBounds_CreateMode);
    } else {
        filterResult = this->filterMask(&dstM, srcM, matrix, &margin);
    }

    if (!filterResult) {
        return kFalse_FilterReturn;
    }

    /*
     *  smallR is the smallest version of 'rect' that will still guarantee that
     *  we get the same blur results on all edges, plus 1 center row/col that is
     *  representative of the extendible/stretchable edges of the ninepatch.
     *  Since our actual edge may be fractional we inset 1 more to be sure we
     *  don't miss any interior blur.
     *  x is an added pixel of blur, and { and } are the (fractional) edge
     *  pixels from the original rect.
     *
     *   x x { x x .... x x } x x
     *
     *  Thus, in this case, we inset by a total of 5 (on each side) beginning
     *  with our outer-rect (dstM.fBounds)
     */
    SkRect smallR[2];
    SkIPoint center;

    // +2 is from +1 for each edge (to account for possible fractional edges
    int smallW = dstM.fBounds.width() - srcM.fBounds.width() + 2;
    int smallH = dstM.fBounds.height() - srcM.fBounds.height() + 2;
    SkIRect innerIR;

    if (1 == count) {
        innerIR = srcM.fBounds;
        center.set(smallW, smallH);
    } else {
        SkASSERT(2 == count);
        rects[1].roundIn(&innerIR);
        center.set(smallW + (innerIR.left() - srcM.fBounds.left()),
                   smallH + (innerIR.top() - srcM.fBounds.top()));
    }

    // +1 so we get a clean, stretchable, center row/col
    smallW += 1;
    smallH += 1;

    // we want the inset amounts to be integral, so we don't change any
    // fractional phase on the fRight or fBottom of our smallR.
    const SkScalar dx = SkIntToScalar(innerIR.width() - smallW);
    const SkScalar dy = SkIntToScalar(innerIR.height() - smallH);
    if (dx < 0 || dy < 0) {
        // we're too small, relative to our blur, to break into nine-patch,
        // so we ask to have our normal filterMask() be called.
        return kUnimplemented_FilterReturn;
    }

    smallR[0].set(rects[0].left(), rects[0].top(), rects[0].right() - dx, rects[0].bottom() - dy);
    if (smallR[0].width() < 2 || smallR[0].height() < 2) {
        return kUnimplemented_FilterReturn;
    }
    if (2 == count) {
        smallR[1].set(rects[1].left(), rects[1].top(),
                      rects[1].right() - dx, rects[1].bottom() - dy);
        SkASSERT(!smallR[1].isEmpty());
    }

    if (count > 1 || !c_analyticBlurNinepatch) {
        if (!draw_rects_into_mask(smallR, count, &srcM)) {
            return kFalse_FilterReturn;
        }

        SkAutoMaskFreeImage amf(srcM.fImage);

        if (!this->filterMask(&patch->fMask, srcM, matrix, &margin)) {
            return kFalse_FilterReturn;
        }
    } else {
        if (!this->filterRectMask(&patch->fMask, smallR[0], matrix, &margin,
                                  SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
            return kFalse_FilterReturn;
        }
    }
    patch->fMask.fBounds.offsetTo(0, 0);
    patch->fOuterRect = dstM.fBounds;
    patch->fCenter = center;
    return kTrue_FilterReturn;
}

void SkBlurMaskFilterImpl::computeFastBounds(const SkRect& src,
                                             SkRect* dst) const {
    SkScalar pad = 3.0f * fSigma;

    dst->set(src.fLeft  - pad, src.fTop    - pad,
             src.fRight + pad, src.fBottom + pad);
}

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkReadBuffer& buffer)
        : SkMaskFilter(buffer) {
    fSigma = buffer.readScalar();
    fBlurStyle = (SkBlurMaskFilter::BlurStyle)buffer.readInt();
    fBlurFlags = buffer.readUInt() & SkBlurMaskFilter::kAll_BlurFlag;
    SkASSERT(fSigma >= 0);
    SkASSERT((unsigned)fBlurStyle < SkBlurMaskFilter::kBlurStyleCount);
}

void SkBlurMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma);
    buffer.writeInt(fBlurStyle);
    buffer.writeUInt(fBlurFlags);
}

#if SK_SUPPORT_GPU

class GrGLRectBlurEffect;

class GrRectBlurEffect : public GrEffect {
public:
    virtual ~GrRectBlurEffect();

    static const char* Name() { return "RectBlur"; }

    typedef GrGLRectBlurEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    /**
     * Create a simple filter effect with custom bicubic coefficients.
     */
    static GrEffectRef* Create(GrContext *context, const SkRect& rect,
                               float sigma) {
        GrTexture *horizontalScanline = NULL, *verticalScanline = NULL;
        bool createdScanlines = CreateScanlineTextures(context, sigma,
                                                       SkScalarCeilToInt(rect.width()),
                                                       SkScalarCeilToInt(rect.height()),
                                                       &horizontalScanline, &verticalScanline);
         SkAutoTUnref<GrTexture> hunref(horizontalScanline), vunref(verticalScanline);
         if (!createdScanlines) {
            return NULL;
        }
        AutoEffectUnref effect(SkNEW_ARGS(GrRectBlurEffect, (rect, sigma,
                                                             horizontalScanline, verticalScanline)));
        return CreateEffectRef(effect);
    }

    unsigned int getWidth() const { return fWidth; }
    unsigned int getHeight() const { return fHeight; }
    float getSigma() const { return fSigma; }
    const GrCoordTransform& getTransform() const { return fTransform; }

private:
    GrRectBlurEffect(const SkRect& rect, float sigma,
                     GrTexture *horizontal_scanline, GrTexture *vertical_scanline);
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    static bool CreateScanlineTextures(GrContext *context, float sigma,
                                       unsigned int width, unsigned int height,
                                       GrTexture **horizontalScanline,
                                       GrTexture **verticalScanline);

    unsigned int fWidth, fHeight;
    float fSigma;
    GrTextureAccess  fHorizontalScanlineAccess;
    GrTextureAccess  fVerticalScanlineAccess;
    GrCoordTransform fTransform;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

class GrGLRectBlurEffect : public GrGLEffect {
public:
    GrGLRectBlurEffect(const GrBackendEffectFactory& factory,
                      const GrDrawEffect&);
    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLUniformManager::UniformHandle        UniformHandle;

    UniformHandle       fWidthUni;
    UniformHandle       fHeightUni;

    typedef GrGLEffect INHERITED;
};

GrGLRectBlurEffect::GrGLRectBlurEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
    : INHERITED(factory) {
}

void GrGLRectBlurEffect::emitCode(GrGLShaderBuilder* builder,
                                 const GrDrawEffect&,
                                 EffectKey key,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const TransformedCoordsArray& coords,
                                 const TextureSamplerArray& samplers) {

    SkString texture_coords = builder->ensureFSCoords2D(coords, 0);

    if (inputColor) {
        builder->fsCodeAppendf("\tvec4 src=%s;\n", inputColor);
    } else {
        builder->fsCodeAppendf("\tvec4 src=vec4(1)\n;");
    }

    builder->fsCodeAppendf("\tvec4 horiz = ");
    builder->fsAppendTextureLookup( samplers[0], texture_coords.c_str() );
    builder->fsCodeAppendf(";\n");
    builder->fsCodeAppendf("\tvec4 vert = ");
    builder->fsAppendTextureLookup( samplers[1], texture_coords.c_str() );
    builder->fsCodeAppendf(";\n");

    builder->fsCodeAppendf("\tfloat final = (horiz*vert).r;\n");
    builder->fsCodeAppendf("\t%s = final*src;\n", outputColor);
}

void GrGLRectBlurEffect::setData(const GrGLUniformManager& uman,
                                const GrDrawEffect& drawEffect) {
}

bool GrRectBlurEffect::CreateScanlineTextures(GrContext *context, float sigma,
                                              unsigned int width, unsigned int height,
                                              GrTexture **horizontalScanline,
                                              GrTexture **verticalScanline) {
    GrTextureParams params;
    GrTextureDesc texDesc;

    unsigned int profile_size = SkScalarFloorToInt(6*sigma);

    texDesc.fWidth = width;
    texDesc.fHeight = 1;
    texDesc.fConfig = kAlpha_8_GrPixelConfig;

    static const GrCacheID::Domain gBlurProfileDomain = GrCacheID::GenerateDomain();
    GrCacheID::Key key;
    memset(&key, 0, sizeof(key));
    key.fData32[0] = profile_size;
    key.fData32[1] = width;
    key.fData32[2] = 1;
    GrCacheID horizontalCacheID(gBlurProfileDomain, key);

    uint8_t *profile = NULL;
    SkAutoTDeleteArray<uint8_t> ada(NULL);

    *horizontalScanline = context->findAndRefTexture(texDesc, horizontalCacheID, &params);

    if (NULL == *horizontalScanline) {

        SkBlurMask::ComputeBlurProfile(sigma, &profile);
        ada.reset(profile);

        SkAutoTMalloc<uint8_t> horizontalPixels(width);
        SkBlurMask::ComputeBlurredScanline(horizontalPixels, profile, width, sigma);

        *horizontalScanline = context->createTexture(&params, texDesc, horizontalCacheID,
                                                     horizontalPixels, 0);

        if (NULL == *horizontalScanline) {
            return false;
        }
    }

    texDesc.fWidth = 1;
    texDesc.fHeight = height;
    key.fData32[1] = 1;
    key.fData32[2] = height;
    GrCacheID verticalCacheID(gBlurProfileDomain, key);

    *verticalScanline = context->findAndRefTexture(texDesc, verticalCacheID, &params);
    if (NULL == *verticalScanline) {
        if (NULL == profile) {
            SkBlurMask::ComputeBlurProfile(sigma, &profile);
            ada.reset(profile);
        }

        SkAutoTMalloc<uint8_t> verticalPixels(height);
        SkBlurMask::ComputeBlurredScanline(verticalPixels, profile, height, sigma);

        *verticalScanline = context->createTexture(&params, texDesc, verticalCacheID,
                                                   verticalPixels, 0);

        if (NULL == *verticalScanline) {
            SkSafeSetNull(*horizontalScanline);
            return false;
        }

    }
    return true;
}

GrRectBlurEffect::GrRectBlurEffect(const SkRect& rect, float sigma,
                                   GrTexture *horizontal_scanline, GrTexture *vertical_scanline)
  : INHERITED(),
    fWidth(horizontal_scanline->width()),
    fHeight(vertical_scanline->width()),
    fSigma(sigma),
    fHorizontalScanlineAccess(horizontal_scanline),
    fVerticalScanlineAccess(vertical_scanline) {
    SkMatrix mat;
    mat.setRectToRect(rect, SkRect::MakeWH(1,1), SkMatrix::kFill_ScaleToFit);
    fTransform.reset(kLocal_GrCoordSet, mat);
    this->addTextureAccess(&fHorizontalScanlineAccess);
    this->addTextureAccess(&fVerticalScanlineAccess);
    this->addCoordTransform(&fTransform);
}

GrRectBlurEffect::~GrRectBlurEffect() {
}

const GrBackendEffectFactory& GrRectBlurEffect::getFactory() const {
    return GrTBackendEffectFactory<GrRectBlurEffect>::getInstance();
}

bool GrRectBlurEffect::onIsEqual(const GrEffect& sBase) const {
    const GrRectBlurEffect& s = CastEffect<GrRectBlurEffect>(sBase);
    return this->getWidth() == s.getWidth() &&
           this->getHeight() == s.getHeight() &&
           this->getSigma() == s.getSigma() &&
           this->getTransform() == s.getTransform();
}

void GrRectBlurEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
    return;
}

GR_DEFINE_EFFECT_TEST(GrRectBlurEffect);

GrEffectRef* GrRectBlurEffect::TestCreate(SkRandom* random,
                                         GrContext* context,
                                         const GrDrawTargetCaps&,
                                         GrTexture**) {
    float sigma = random->nextRangeF(3,8);
    float width = random->nextRangeF(200,300);
    float height = random->nextRangeF(200,300);
    return GrRectBlurEffect::Create(context, SkRect::MakeWH(width, height), sigma);
}


bool SkBlurMaskFilterImpl::directFilterMaskGPU(GrContext* context,
                                               GrPaint* grp,
                                               const SkStrokeRec& strokeRec,
                                               const SkPath& path) const {
    if (fBlurStyle != SkBlurMaskFilter::kNormal_BlurStyle) {
        return false;
    }

    SkRect rect;
    if (!path.isRect(&rect)) {
        return false;
    }

    if (!strokeRec.isFillStyle()) {
        return false;
    }

    SkMatrix ctm = context->getMatrix();
    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    rect.outset(3*xformedSigma, 3*xformedSigma);

    SkAutoTUnref<GrEffectRef> effect(GrRectBlurEffect::Create(
            context, rect, xformedSigma));
    if (!effect) {
        return false;
    }

    GrContext::AutoMatrix am;
    if (!am.setIdentity(context, grp)) {
       return false;
    }


    grp->addCoverageEffect(effect);

    context->drawRect(*grp, rect);
    return true;
}

bool SkBlurMaskFilterImpl::directFilterRRectMaskGPU(GrContext* context,
                                                    GrPaint* grp,
                                                    const SkStrokeRec& strokeRec,
                                                    const SkRRect& rrect) const {
    return false;
}

bool SkBlurMaskFilterImpl::canFilterMaskGPU(const SkRect& srcBounds,
                                            const SkIRect& clipBounds,
                                            const SkMatrix& ctm,
                                            SkRect* maskRect) const {
    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    if (xformedSigma <= 0) {
        return false;
    }

    static const SkScalar kMIN_GPU_BLUR_SIZE  = SkIntToScalar(64);
    static const SkScalar kMIN_GPU_BLUR_SIGMA = SkIntToScalar(32);

    if (srcBounds.width() <= kMIN_GPU_BLUR_SIZE &&
        srcBounds.height() <= kMIN_GPU_BLUR_SIZE &&
        xformedSigma <= kMIN_GPU_BLUR_SIGMA) {
        // We prefer to blur small rect with small radius via CPU.
        return false;
    }

    if (NULL == maskRect) {
        // don't need to compute maskRect
        return true;
    }

    float sigma3 = 3 * SkScalarToFloat(xformedSigma);

    SkRect clipRect = SkRect::Make(clipBounds);
    SkRect srcRect(srcBounds);

    // Outset srcRect and clipRect by 3 * sigma, to compute affected blur area.
    srcRect.outset(sigma3, sigma3);
    clipRect.outset(sigma3, sigma3);
    srcRect.intersect(clipRect);
    *maskRect = srcRect;
    return true;
}

bool SkBlurMaskFilterImpl::filterMaskGPU(GrTexture* src,
                                         const SkMatrix& ctm,
                                         const SkRect& maskRect,
                                         GrTexture** result,
                                         bool canOverwriteSrc) const {
    SkRect clipRect = SkRect::MakeWH(maskRect.width(), maskRect.height());

    GrContext* context = src->getContext();

    GrContext::AutoWideOpenIdentityDraw awo(context, NULL);

    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    SkASSERT(xformedSigma > 0);

    // If we're doing a normal blur, we can clobber the pathTexture in the
    // gaussianBlur.  Otherwise, we need to save it for later compositing.
    bool isNormalBlur = (SkBlurMaskFilter::kNormal_BlurStyle == fBlurStyle);
    *result = SkGpuBlurUtils::GaussianBlur(context, src, isNormalBlur && canOverwriteSrc,
                                           clipRect, false, xformedSigma, xformedSigma);
    if (NULL == *result) {
        return false;
    }

    if (!isNormalBlur) {
        context->setIdentityMatrix();
        GrPaint paint;
        SkMatrix matrix;
        matrix.setIDiv(src->width(), src->height());
        // Blend pathTexture over blurTexture.
        GrContext::AutoRenderTarget art(context, (*result)->asRenderTarget());
        paint.addColorEffect(GrSimpleTextureEffect::Create(src, matrix))->unref();
        if (SkBlurMaskFilter::kInner_BlurStyle == fBlurStyle) {
            // inner:  dst = dst * src
            paint.setBlendFunc(kDC_GrBlendCoeff, kZero_GrBlendCoeff);
        } else if (SkBlurMaskFilter::kSolid_BlurStyle == fBlurStyle) {
            // solid:  dst = src + dst - src * dst
            //             = (1 - dst) * src + 1 * dst
            paint.setBlendFunc(kIDC_GrBlendCoeff, kOne_GrBlendCoeff);
        } else if (SkBlurMaskFilter::kOuter_BlurStyle == fBlurStyle) {
            // outer:  dst = dst * (1 - src)
            //             = 0 * src + (1 - src) * dst
            paint.setBlendFunc(kZero_GrBlendCoeff, kISC_GrBlendCoeff);
        }
        context->drawRect(paint, clipRect);
    }

    return true;
}

#endif // SK_SUPPORT_GPU


#ifndef SK_IGNORE_TO_STRING
void SkBlurMaskFilterImpl::toString(SkString* str) const {
    str->append("SkBlurMaskFilterImpl: (");

    str->append("sigma: ");
    str->appendScalar(fSigma);
    str->append(" ");

    static const char* gStyleName[SkBlurMaskFilter::kBlurStyleCount] = {
        "normal", "solid", "outer", "inner"
    };

    str->appendf("style: %s ", gStyleName[fBlurStyle]);
    str->append("flags: (");
    if (fBlurFlags) {
        bool needSeparator = false;
        SkAddFlagToString(str,
                          SkToBool(fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag),
                          "IgnoreXform", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag),
                          "HighQuality", &needSeparator);
    } else {
        str->append("None");
    }
    str->append("))");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkBlurMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
