
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
#include "gl/GrGLShaderBuilder.h"
#include "effects/GrSimpleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#include "SkGrPixelRef.h"
#include "SkDraw.h"
#endif

SkScalar SkBlurMaskFilter::ConvertRadiusToSigma(SkScalar radius) {
    return SkBlurMask::ConvertRadiusToSigma(radius);
}

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle, uint32_t flags);

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
    virtual bool asABlur(BlurRec*) const SK_OVERRIDE;

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

    SkScalar    fSigma;
    SkBlurStyle fBlurStyle;
    uint32_t    fBlurFlags;

    SkBlurQuality getQuality() const {
        return (fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag) ?
                kHigh_SkBlurQuality : kLow_SkBlurQuality;
    }

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

SkMaskFilter* SkBlurMaskFilter::Create(SkBlurStyle style, SkScalar sigma, uint32_t flags) {
    if (!SkScalarIsFinite(sigma) || sigma <= 0) {
        return NULL;
    }
    if ((unsigned)style > (unsigned)kLastEnum_SkBlurStyle) {
        return NULL;
    }
    if (flags > SkBlurMaskFilter::kAll_BlurFlag) {
        return NULL;
    }
    return SkNEW_ARGS(SkBlurMaskFilterImpl, (sigma, style, flags));
}

///////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle style, uint32_t flags)
    : fSigma(sigma)
    , fBlurStyle(style)
    , fBlurFlags(flags) {
    SkASSERT(fSigma > 0);
    SkASSERT((unsigned)style <= kLastEnum_SkBlurStyle);
    SkASSERT(flags <= SkBlurMaskFilter::kAll_BlurFlag);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::asABlur(BlurRec* rec) const {
    if (fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag) {
        return false;
    }

    if (rec) {
        rec->fSigma = fSigma;
        rec->fStyle = fBlurStyle;
        rec->fQuality = this->getQuality();
    }
    return true;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const{
    SkScalar sigma = this->computeXformedSigma(matrix);
    return SkBlurMask::BoxBlur(dst, src, sigma, fBlurStyle, this->getQuality(), margin);
}

bool SkBlurMaskFilterImpl::filterRectMask(SkMask* dst, const SkRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const{
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRect(sigma, dst, r, fBlurStyle,
                                margin, createMode);
}

bool SkBlurMaskFilterImpl::filterRRectMask(SkMask* dst, const SkRRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const{
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRRect(sigma, dst, r, fBlurStyle,
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
                         mask->fImage, mask->fRowBytes);

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
SK_CONF_DECLARE( bool, c_analyticBlurRRect, "mask.filter.blur.analyticblurrrect", false, "Use the faster analytic blur approach for ninepatch rects" );
#else
SK_CONF_DECLARE( bool, c_analyticBlurRRect, "mask.filter.blur.analyticblurrrect", true, "Use the faster analytic blur approach for ninepatch round rects" );
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

        // These three can take advantage of this fast path.
        case SkRRect::kSimple_Type:
        case SkRRect::kNinePatch_Type:
        case SkRRect::kComplex_Type:
            break;
    }

    // TODO: report correct metrics for innerstyle, where we do not grow the
    // total bounds, but we do need an inset the size of our blur-radius
    if (kInner_SkBlurStyle == fBlurStyle) {
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
    if (kInner_SkBlurStyle == fBlurStyle || kOuter_SkBlurStyle == fBlurStyle) {
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
    fBlurStyle = (SkBlurStyle)buffer.readInt();
    fBlurFlags = buffer.readUInt() & SkBlurMaskFilter::kAll_BlurFlag;
    SkASSERT(fSigma > 0);
    SkASSERT((unsigned)fBlurStyle <= kLastEnum_SkBlurStyle);
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
    static GrEffect* Create(GrContext *context, const SkRect& rect, float sigma) {
        GrTexture *blurProfileTexture = NULL;
        int doubleProfileSize = SkScalarCeilToInt(12*sigma);

        if (doubleProfileSize >= rect.width() || doubleProfileSize >= rect.height()) {
            // if the blur sigma is too large so the gaussian overlaps the whole
            // rect in either direction, fall back to CPU path for now.

            return NULL;
        }

        bool createdBlurProfileTexture = CreateBlurProfileTexture(context, sigma, &blurProfileTexture);
        SkAutoTUnref<GrTexture> hunref(blurProfileTexture);
        if (!createdBlurProfileTexture) {
           return NULL;
        }
        return SkNEW_ARGS(GrRectBlurEffect, (rect, sigma, blurProfileTexture));
    }

    const SkRect& getRect() const { return fRect; }
    float getSigma() const { return fSigma; }

private:
    GrRectBlurEffect(const SkRect& rect, float sigma, GrTexture *blur_profile);
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    static bool CreateBlurProfileTexture(GrContext *context, float sigma,
                                       GrTexture **blurProfileTexture);

    SkRect          fRect;
    float           fSigma;
    GrTextureAccess fBlurProfileAccess;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

class GrGLRectBlurEffect : public GrGLEffect {
public:
    GrGLRectBlurEffect(const GrBackendEffectFactory& factory,
                      const GrDrawEffect&);
    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLUniformManager::UniformHandle        UniformHandle;

    UniformHandle       fProxyRectUniform;
    UniformHandle       fProfileSizeUniform;

    typedef GrGLEffect INHERITED;
};



GrGLRectBlurEffect::GrGLRectBlurEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
    : INHERITED(factory) {
}

void OutputRectBlurProfileLookup(GrGLShaderBuilder* builder,
                                 const GrGLShaderBuilder::TextureSampler& sampler,
                                 const char *output,
                                 const char *profileSize, const char *loc,
                                 const char *blurred_width,
                                 const char *sharp_width) {
    builder->fsCodeAppendf("\tfloat %s;\n", output);
    builder->fsCodeAppendf("\t\t{\n");
    builder->fsCodeAppendf("\t\t\tfloat coord = (0.5 * (abs(2.0*%s - %s) - %s))/%s;\n",
                           loc, blurred_width, sharp_width, profileSize);
    builder->fsCodeAppendf("\t\t\t%s = ", output);
    builder->fsAppendTextureLookup(sampler, "vec2(coord,0.5)");
    builder->fsCodeAppend(".a;\n");
    builder->fsCodeAppendf("\t\t}\n");
}

void GrGLRectBlurEffect::emitCode(GrGLShaderBuilder* builder,
                                 const GrDrawEffect&,
                                 const GrEffectKey& key,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const TransformedCoordsArray& coords,
                                 const TextureSamplerArray& samplers) {

    const char *rectName;
    const char *profileSizeName;

    fProxyRectUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec4f_GrSLType,
                                            "proxyRect",
                                            &rectName);
    fProfileSizeUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kFloat_GrSLType,
                                            "profileSize",
                                            &profileSizeName);

    const char *fragmentPos = builder->fragmentPosition();

    if (inputColor) {
        builder->fsCodeAppendf("\tvec4 src=%s;\n", inputColor);
    } else {
        builder->fsCodeAppendf("\tvec4 src=vec4(1)\n;");
    }

    builder->fsCodeAppendf("\tvec2 translatedPos = %s.xy - %s.xy;\n", fragmentPos, rectName );
    builder->fsCodeAppendf("\tfloat width = %s.z - %s.x;\n", rectName, rectName);
    builder->fsCodeAppendf("\tfloat height = %s.w - %s.y;\n", rectName, rectName);

    builder->fsCodeAppendf("\tvec2 smallDims = vec2(width - %s, height-%s);\n", profileSizeName, profileSizeName);
    builder->fsCodeAppendf("\tfloat center = 2.0 * floor(%s/2.0 + .25) - 1.0;\n", profileSizeName);
    builder->fsCodeAppendf("\tvec2 wh = smallDims - vec2(center,center);\n");

    OutputRectBlurProfileLookup(builder, samplers[0], "horiz_lookup", profileSizeName, "translatedPos.x", "width", "wh.x");
    OutputRectBlurProfileLookup(builder, samplers[0], "vert_lookup", profileSizeName, "translatedPos.y", "height", "wh.y");

    builder->fsCodeAppendf("\tfloat final = horiz_lookup * vert_lookup;\n");
    builder->fsCodeAppendf("\t%s = src * vec4(final);\n", outputColor );
}

void GrGLRectBlurEffect::setData(const GrGLUniformManager& uman,
                                 const GrDrawEffect& drawEffect) {
    const GrRectBlurEffect& rbe = drawEffect.castEffect<GrRectBlurEffect>();
    SkRect rect = rbe.getRect();

    uman.set4f(fProxyRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    uman.set1f(fProfileSizeUniform, SkScalarCeilToScalar(6*rbe.getSigma()));
}

bool GrRectBlurEffect::CreateBlurProfileTexture(GrContext *context, float sigma,
                                              GrTexture **blurProfileTexture) {
    GrTextureParams params;
    GrTextureDesc texDesc;

    unsigned int profile_size = SkScalarCeilToInt(6*sigma);

    texDesc.fWidth = profile_size;
    texDesc.fHeight = 1;
    texDesc.fConfig = kAlpha_8_GrPixelConfig;

    static const GrCacheID::Domain gBlurProfileDomain = GrCacheID::GenerateDomain();
    GrCacheID::Key key;
    memset(&key, 0, sizeof(key));
    key.fData32[0] = profile_size;
    key.fData32[1] = 1;
    GrCacheID blurProfileKey(gBlurProfileDomain, key);

    uint8_t *profile = NULL;
    SkAutoTDeleteArray<uint8_t> ada(NULL);

    *blurProfileTexture = context->findAndRefTexture(texDesc, blurProfileKey, &params);

    if (NULL == *blurProfileTexture) {

        SkBlurMask::ComputeBlurProfile(sigma, &profile);
        ada.reset(profile);

        *blurProfileTexture = context->createTexture(&params, texDesc, blurProfileKey,
                                                     profile, 0);

        if (NULL == *blurProfileTexture) {
            return false;
        }
    }

    return true;
}

GrRectBlurEffect::GrRectBlurEffect(const SkRect& rect, float sigma,
                                   GrTexture *blur_profile)
  : INHERITED(),
    fRect(rect),
    fSigma(sigma),
    fBlurProfileAccess(blur_profile) {
    this->addTextureAccess(&fBlurProfileAccess);
    this->setWillReadFragmentPosition();
}

GrRectBlurEffect::~GrRectBlurEffect() {
}

const GrBackendEffectFactory& GrRectBlurEffect::getFactory() const {
    return GrTBackendEffectFactory<GrRectBlurEffect>::getInstance();
}

bool GrRectBlurEffect::onIsEqual(const GrEffect& sBase) const {
    const GrRectBlurEffect& s = CastEffect<GrRectBlurEffect>(sBase);
    return this->getSigma() == s.getSigma() && this->getRect() == s.getRect();
}

void GrRectBlurEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
    return;
}

GR_DEFINE_EFFECT_TEST(GrRectBlurEffect);

GrEffect* GrRectBlurEffect::TestCreate(SkRandom* random,
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
    if (fBlurStyle != kNormal_SkBlurStyle) {
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

    int pad=SkScalarCeilToInt(6*xformedSigma)/2;
    rect.outset(SkIntToScalar(pad), SkIntToScalar(pad));

    SkAutoTUnref<GrEffect> effect(GrRectBlurEffect::Create(context, rect, xformedSigma));
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

class GrGLRRectBlurEffect;

class GrRRectBlurEffect : public GrEffect {
public:

    static GrEffect* Create(GrContext* context, float sigma, const SkRRect&);

    virtual ~GrRRectBlurEffect() {};
    static const char* Name() { return "GrRRectBlur"; }

    const SkRRect& getRRect() const { return fRRect; }
    float getSigma() const { return fSigma; }

    typedef GrGLRRectBlurEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrRRectBlurEffect(float sigma, const SkRRect&, GrTexture* profileTexture);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    SkRRect             fRRect;
    float               fSigma;
    GrTextureAccess     fNinePatchAccess;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};


GrEffect* GrRRectBlurEffect::Create(GrContext* context, float sigma, const SkRRect& rrect) {
    if (!rrect.isSimpleCircular()) {
        return NULL;
    }

    // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be
    // sufficiently small relative to both the size of the corner radius and the
    // width (and height) of the rrect.

    unsigned int blurRadius = 3*SkScalarCeilToInt(sigma-1/6.0f);
    unsigned int cornerRadius = SkScalarCeilToInt(rrect.getSimpleRadii().x());
    if (cornerRadius + blurRadius > rrect.width()/2 ||
        cornerRadius + blurRadius > rrect.height()/2) {
        return NULL;
    }

    static const GrCacheID::Domain gRRectBlurDomain = GrCacheID::GenerateDomain();
    GrCacheID::Key key;
    memset(&key, 0, sizeof(key));
    key.fData32[0] = blurRadius;
    key.fData32[1] = cornerRadius;
    GrCacheID blurRRectNinePatchID(gRRectBlurDomain, key);

    GrTextureParams params;
    params.setFilterMode(GrTextureParams::kBilerp_FilterMode);

    unsigned int smallRectSide = 2*(blurRadius + cornerRadius) + 1;
    unsigned int texSide = smallRectSide + 2*blurRadius;
    GrTextureDesc texDesc;
    texDesc.fWidth = texSide;
    texDesc.fHeight = texSide;
    texDesc.fConfig = kAlpha_8_GrPixelConfig;

    GrTexture *blurNinePatchTexture = context->findAndRefTexture(texDesc, blurRRectNinePatchID, &params);

    if (NULL == blurNinePatchTexture) {
        SkMask mask;

        mask.fBounds = SkIRect::MakeWH(smallRectSide, smallRectSide);
        mask.fFormat = SkMask::kA8_Format;
        mask.fRowBytes = mask.fBounds.width();
        mask.fImage = SkMask::AllocImage(mask.computeTotalImageSize());
        SkAutoMaskFreeImage amfi(mask.fImage);

        memset(mask.fImage, 0, mask.computeTotalImageSize());

        SkRect smallRect;
        smallRect.setWH(SkIntToScalar(smallRectSide), SkIntToScalar(smallRectSide));

        SkRRect smallRRect;
        smallRRect.setRectXY(smallRect, SkIntToScalar(cornerRadius), SkIntToScalar(cornerRadius));

        SkPath path;
        path.addRRect( smallRRect );

        SkDraw::DrawToMask(path, &mask.fBounds, NULL, NULL, &mask, SkMask::kJustRenderImage_CreateMode, SkPaint::kFill_Style);

        SkMask blurred_mask;
        SkBlurMask::BoxBlur(&blurred_mask, mask, sigma, kNormal_SkBlurStyle, kHigh_SkBlurQuality, NULL, true );

        blurNinePatchTexture = context->createTexture(&params, texDesc, blurRRectNinePatchID, blurred_mask.fImage, 0);
        SkMask::FreeImage(blurred_mask.fImage);
    }

    SkAutoTUnref<GrTexture> blurunref(blurNinePatchTexture);
    if (NULL == blurNinePatchTexture) {
        return NULL;
    }

    return SkNEW_ARGS(GrRRectBlurEffect, (sigma, rrect, blurNinePatchTexture));
}

void GrRRectBlurEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& GrRRectBlurEffect::getFactory() const {
    return GrTBackendEffectFactory<GrRRectBlurEffect>::getInstance();
}

GrRRectBlurEffect::GrRRectBlurEffect(float sigma, const SkRRect& rrect, GrTexture *ninePatchTexture)
    : fRRect(rrect),
      fSigma(sigma),
      fNinePatchAccess(ninePatchTexture) {
    this->addTextureAccess(&fNinePatchAccess);
    this->setWillReadFragmentPosition();
}

bool GrRRectBlurEffect::onIsEqual(const GrEffect& other) const {
    const GrRRectBlurEffect& rrbe = CastEffect<GrRRectBlurEffect>(other);
    return fRRect.getSimpleRadii().fX == rrbe.fRRect.getSimpleRadii().fX && fSigma == rrbe.fSigma;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrRRectBlurEffect);

GrEffect* GrRRectBlurEffect::TestCreate(SkRandom* random,
                                        GrContext* context,
                                        const GrDrawTargetCaps& caps,
                                        GrTexture*[]) {
    SkScalar w = random->nextRangeScalar(100.f, 1000.f);
    SkScalar h = random->nextRangeScalar(100.f, 1000.f);
    SkScalar r = random->nextRangeF(1.f, 9.f);
    SkScalar sigma = random->nextRangeF(1.f,10.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    return GrRRectBlurEffect::Create(context, sigma, rrect);
}

//////////////////////////////////////////////////////////////////////////////

class GrGLRRectBlurEffect : public GrGLEffect {
public:
    GrGLRRectBlurEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    GrGLUniformManager::UniformHandle   fProxyRectUniform;
    GrGLUniformManager::UniformHandle   fCornerRadiusUniform;
    GrGLUniformManager::UniformHandle   fBlurRadiusUniform;
    typedef GrGLEffect INHERITED;
};

GrGLRRectBlurEffect::GrGLRRectBlurEffect(const GrBackendEffectFactory& factory,
                             const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
}

void GrGLRRectBlurEffect::emitCode(GrGLShaderBuilder* builder,
                             const GrDrawEffect& drawEffect,
                             const GrEffectKey& key,
                             const char* outputColor,
                             const char* inputColor,
                             const TransformedCoordsArray&,
                             const TextureSamplerArray& samplers) {
    const char *rectName;
    const char *cornerRadiusName;
    const char *blurRadiusName;

    // The proxy rect has left, top, right, and bottom edges correspond to
    // components x, y, z, and w, respectively.

    fProxyRectUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec4f_GrSLType,
                                            "proxyRect",
                                            &rectName);
    fCornerRadiusUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                 kFloat_GrSLType,
                                                 "cornerRadius",
                                                 &cornerRadiusName);
    fBlurRadiusUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                 kFloat_GrSLType,
                                                 "blurRadius",
                                                 &blurRadiusName);
    const char* fragmentPos = builder->fragmentPosition();

    // warp the fragment position to the appropriate part of the 9patch blur texture

    builder->fsCodeAppendf("\t\tvec2 rectCenter = (%s.xy + %s.zw)/2.0;\n", rectName, rectName);
    builder->fsCodeAppendf("\t\tvec2 translatedFragPos = %s.xy - %s.xy;\n", fragmentPos, rectName);
    builder->fsCodeAppendf("\t\tfloat threshold = %s + 2.0*%s;\n", cornerRadiusName, blurRadiusName );
    builder->fsCodeAppendf("\t\tvec2 middle = %s.zw - %s.xy - 2.0*threshold;\n", rectName, rectName );

    builder->fsCodeAppendf("\t\tif (translatedFragPos.x >= threshold && translatedFragPos.x < (middle.x+threshold)) {\n" );
    builder->fsCodeAppendf("\t\t\ttranslatedFragPos.x = threshold;\n");
    builder->fsCodeAppendf("\t\t} else if (translatedFragPos.x >= (middle.x + threshold)) {\n");
    builder->fsCodeAppendf("\t\t\ttranslatedFragPos.x -= middle.x - 1.0;\n");
    builder->fsCodeAppendf("\t\t}\n");

    builder->fsCodeAppendf("\t\tif (translatedFragPos.y > threshold && translatedFragPos.y < (middle.y+threshold)) {\n" );
    builder->fsCodeAppendf("\t\t\ttranslatedFragPos.y = threshold;\n");
    builder->fsCodeAppendf("\t\t} else if (translatedFragPos.y >= (middle.y + threshold)) {\n");
    builder->fsCodeAppendf("\t\t\ttranslatedFragPos.y -= middle.y - 1.0;\n");
    builder->fsCodeAppendf("\t\t}\n");

    builder->fsCodeAppendf("\t\tvec2 proxyDims = vec2(2.0*threshold+1.0);\n");
    builder->fsCodeAppendf("\t\tvec2 texCoord = translatedFragPos / proxyDims;\n");

    builder->fsCodeAppendf("\t%s = ", outputColor);
    builder->fsAppendTextureLookupAndModulate(inputColor, samplers[0], "texCoord");
    builder->fsCodeAppend(";\n");
}

void GrGLRRectBlurEffect::setData(const GrGLUniformManager& uman,
                                    const GrDrawEffect& drawEffect) {
    const GrRRectBlurEffect& brre = drawEffect.castEffect<GrRRectBlurEffect>();
    SkRRect rrect = brre.getRRect();

    float blurRadius = 3.f*SkScalarCeilToScalar(brre.getSigma()-1/6.0f);
    uman.set1f(fBlurRadiusUniform, blurRadius);

    SkRect rect = rrect.getBounds();
    rect.outset(blurRadius, blurRadius);
    uman.set4f(fProxyRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);

    SkScalar radius = 0;
    SkASSERT(rrect.isSimpleCircular() || rrect.isRect());
    radius = rrect.getSimpleRadii().fX;
    uman.set1f(fCornerRadiusUniform, radius);
}


bool SkBlurMaskFilterImpl::directFilterRRectMaskGPU(GrContext* context,
                                                    GrPaint* grp,
                                                    const SkStrokeRec& strokeRec,
                                                    const SkRRect& rrect) const {
    if (fBlurStyle != kNormal_SkBlurStyle) {
        return false;
    }

    if (!strokeRec.isFillStyle()) {
        return false;
    }

    SkRect proxy_rect = rrect.rect();
    SkMatrix ctm = context->getMatrix();
    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    float extra=3.f*SkScalarCeilToScalar(xformedSigma-1/6.0f);
    proxy_rect.outset(extra, extra);

    SkAutoTUnref<GrEffect> effect(GrRRectBlurEffect::Create(
            context, xformedSigma, rrect));
    if (!effect) {
        return false;
    }

    GrContext::AutoMatrix am;
    if (!am.setIdentity(context, grp)) {
       return false;
    }

    grp->addCoverageEffect(effect);

    context->drawRect(*grp, proxy_rect);
    return true;
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
    bool isNormalBlur = (kNormal_SkBlurStyle == fBlurStyle);
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
        if (kInner_SkBlurStyle == fBlurStyle) {
            // inner:  dst = dst * src
            paint.setBlendFunc(kDC_GrBlendCoeff, kZero_GrBlendCoeff);
        } else if (kSolid_SkBlurStyle == fBlurStyle) {
            // solid:  dst = src + dst - src * dst
            //             = (1 - dst) * src + 1 * dst
            paint.setBlendFunc(kIDC_GrBlendCoeff, kOne_GrBlendCoeff);
        } else if (kOuter_SkBlurStyle == fBlurStyle) {
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

    static const char* gStyleName[kLastEnum_SkBlurStyle + 1] = {
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
