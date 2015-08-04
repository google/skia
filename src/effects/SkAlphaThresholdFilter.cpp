/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAlphaThresholdFilter.h"
#include "SkBitmap.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRegion.h"
#if SK_SUPPORT_GPU
#include "GrDrawContext.h"
#endif

class SK_API SkAlphaThresholdFilterImpl : public SkImageFilter {
public:
    SkAlphaThresholdFilterImpl(const SkRegion& region, SkScalar innerThreshold,
                               SkScalar outerThreshold, SkImageFilter* input);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAlphaThresholdFilterImpl)

protected:
    void flatten(SkWriteBuffer&) const override;

    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    bool asFragmentProcessor(GrFragmentProcessor**, GrProcessorDataManager*, GrTexture*,
                             const SkMatrix&, const SkIRect& bounds) const override;
#endif

private:
    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;
    typedef SkImageFilter INHERITED;
};

SkImageFilter* SkAlphaThresholdFilter::Create(const SkRegion& region,
                                              SkScalar innerThreshold,
                                              SkScalar outerThreshold,
                                              SkImageFilter* input) {
    return SkNEW_ARGS(SkAlphaThresholdFilterImpl, (region, innerThreshold, outerThreshold, input));
}

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrTextureAccess.h"
#include "effects/GrPorterDuffXferProcessor.h"

#include "SkGr.h"

#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

class AlphaThresholdEffect : public GrFragmentProcessor {

public:
    static GrFragmentProcessor* Create(GrProcessorDataManager* procDataManager,
                                       GrTexture* texture,
                                       GrTexture* maskTexture,
                                       float innerThreshold,
                                       float outerThreshold) {
        return SkNEW_ARGS(AlphaThresholdEffect, (procDataManager,
                                                 texture,
                                                 maskTexture,
                                                 innerThreshold,
                                                 outerThreshold));
    }

    virtual ~AlphaThresholdEffect() {};

    const char* name() const override { return "Alpha Threshold"; }

    float innerThreshold() const { return fInnerThreshold; }
    float outerThreshold() const { return fOuterThreshold; }

    GrGLFragmentProcessor* createGLInstance() const override;

private:
    AlphaThresholdEffect(GrProcessorDataManager*,
                         GrTexture* texture,
                         GrTexture* maskTexture,
                         float innerThreshold,
                         float outerThreshold)
        : fInnerThreshold(innerThreshold)
        , fOuterThreshold(outerThreshold)
        , fImageCoordTransform(kLocal_GrCoordSet,
                               GrCoordTransform::MakeDivByTextureWHMatrix(texture), texture,
                               GrTextureParams::kNone_FilterMode)
        , fImageTextureAccess(texture)
        , fMaskCoordTransform(kLocal_GrCoordSet,
                              GrCoordTransform::MakeDivByTextureWHMatrix(maskTexture), maskTexture,
                              GrTextureParams::kNone_FilterMode)
        , fMaskTextureAccess(maskTexture) {
        this->initClassID<AlphaThresholdEffect>();
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureAccess(&fImageTextureAccess);
        this->addCoordTransform(&fMaskCoordTransform);
        this->addTextureAccess(&fMaskTextureAccess);
    }

    void onGetGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    float fInnerThreshold;
    float fOuterThreshold;
    GrCoordTransform fImageCoordTransform;
    GrTextureAccess  fImageTextureAccess;
    GrCoordTransform fMaskCoordTransform;
    GrTextureAccess  fMaskTextureAccess;

    typedef GrFragmentProcessor INHERITED;
};

class GrGLAlphaThresholdEffect : public GrGLFragmentProcessor {
public:
    GrGLAlphaThresholdEffect(const GrFragmentProcessor&) {}

    virtual void emitCode(EmitArgs&) override;

    void setData(const GrGLProgramDataManager&, const GrProcessor&) override;

private:

    GrGLProgramDataManager::UniformHandle fInnerThresholdVar;
    GrGLProgramDataManager::UniformHandle fOuterThresholdVar;

    typedef GrGLFragmentProcessor INHERITED;
};

void GrGLAlphaThresholdEffect::emitCode(EmitArgs& args) {
    fInnerThresholdVar = args.fBuilder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility,
        kFloat_GrSLType, kDefault_GrSLPrecision,
        "inner_threshold");
    fOuterThresholdVar = args.fBuilder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility,
        kFloat_GrSLType, kDefault_GrSLPrecision,
        "outer_threshold");

    GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(args.fCoords, 0);
    SkString maskCoords2D = fsBuilder->ensureFSCoords2D(args.fCoords, 1);

    fsBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fsBuilder->codeAppendf("\t\tvec2 mask_coord = %s;\n", maskCoords2D.c_str());
    fsBuilder->codeAppend("\t\tvec4 input_color = ");
    fsBuilder->appendTextureLookup(args.fSamplers[0], "coord");
    fsBuilder->codeAppend(";\n");
    fsBuilder->codeAppend("\t\tvec4 mask_color = ");
    fsBuilder->appendTextureLookup(args.fSamplers[1], "mask_coord");
    fsBuilder->codeAppend(";\n");

    fsBuilder->codeAppendf("\t\tfloat inner_thresh = %s;\n",
                           args.fBuilder->getUniformCStr(fInnerThresholdVar));
    fsBuilder->codeAppendf("\t\tfloat outer_thresh = %s;\n",
                           args.fBuilder->getUniformCStr(fOuterThresholdVar));
    fsBuilder->codeAppend("\t\tfloat mask = mask_color.a;\n");

    fsBuilder->codeAppend("vec4 color = input_color;\n");
    fsBuilder->codeAppend("\t\tif (mask < 0.5) {\n"
                          "\t\t\tif (color.a > outer_thresh) {\n"
                          "\t\t\t\tfloat scale = outer_thresh / color.a;\n"
                          "\t\t\t\tcolor.rgb *= scale;\n"
                          "\t\t\t\tcolor.a = outer_thresh;\n"
                          "\t\t\t}\n"
                          "\t\t} else if (color.a < inner_thresh) {\n"
                          "\t\t\tfloat scale = inner_thresh / max(0.001, color.a);\n"
                          "\t\t\tcolor.rgb *= scale;\n"
                          "\t\t\tcolor.a = inner_thresh;\n"
                          "\t\t}\n");

    fsBuilder->codeAppendf("%s = %s;\n", args.fOutputColor,
                           (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr4("color")).c_str());
}

void GrGLAlphaThresholdEffect::setData(const GrGLProgramDataManager& pdman,
                                       const GrProcessor& proc) {
    const AlphaThresholdEffect& alpha_threshold = proc.cast<AlphaThresholdEffect>();
    pdman.set1f(fInnerThresholdVar, alpha_threshold.innerThreshold());
    pdman.set1f(fOuterThresholdVar, alpha_threshold.outerThreshold());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AlphaThresholdEffect);

GrFragmentProcessor* AlphaThresholdEffect::TestCreate(GrProcessorTestData* d) {
    GrTexture* bmpTex = d->fTextures[GrProcessorUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx];
    float innerThresh = d->fRandom->nextUScalar1();
    float outerThresh = d->fRandom->nextUScalar1();
    return AlphaThresholdEffect::Create(d->fProcDataManager, bmpTex, maskTex, innerThresh,
                                        outerThresh);
}

///////////////////////////////////////////////////////////////////////////////

void AlphaThresholdEffect::onGetGLProcessorKey(const GrGLSLCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    GrGLAlphaThresholdEffect::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* AlphaThresholdEffect::createGLInstance() const {
    return SkNEW_ARGS(GrGLAlphaThresholdEffect, (*this));
}

bool AlphaThresholdEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const AlphaThresholdEffect& s = sBase.cast<AlphaThresholdEffect>();
    return (this->fInnerThreshold == s.fInnerThreshold &&
            this->fOuterThreshold == s.fOuterThreshold);
}

void AlphaThresholdEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
        inout->mulByUnknownSingleComponent();
    } else if (GrPixelConfigIsOpaque(this->texture(0)->config()) && fOuterThreshold >= 1.f) {
        inout->mulByUnknownOpaqueFourComponents();
    } else {
        inout->mulByUnknownFourComponents();
    }
}

#endif

SkFlattenable* SkAlphaThresholdFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar inner = buffer.readScalar();
    SkScalar outer = buffer.readScalar();
    SkRegion rgn;
    buffer.readRegion(&rgn);
    return SkAlphaThresholdFilter::Create(rgn, inner, outer, common.getInput(0));
}

SkAlphaThresholdFilterImpl::SkAlphaThresholdFilterImpl(const SkRegion& region,
                                                       SkScalar innerThreshold,
                                                       SkScalar outerThreshold,
                                                       SkImageFilter* input)
    : INHERITED(1, &input)
    , fRegion(region)
    , fInnerThreshold(innerThreshold)
    , fOuterThreshold(outerThreshold) {
}

#if SK_SUPPORT_GPU
bool SkAlphaThresholdFilterImpl::asFragmentProcessor(GrFragmentProcessor** fp,
                                                     GrProcessorDataManager* procDataManager,
                                                     GrTexture* texture,
                                                     const SkMatrix& in_matrix,
                                                     const SkIRect&) const {
    if (fp) {
        GrContext* context = texture->getContext();
        GrSurfaceDesc maskDesc;
        if (context->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
            maskDesc.fConfig = kAlpha_8_GrPixelConfig;
        } else {
            maskDesc.fConfig = kRGBA_8888_GrPixelConfig;
        }
        maskDesc.fFlags = kRenderTarget_GrSurfaceFlag;
        // Add one pixel of border to ensure that clamp mode will be all zeros
        // the outside.
        maskDesc.fWidth = texture->width();
        maskDesc.fHeight = texture->height();
        SkAutoTUnref<GrTexture> maskTexture(
            context->textureProvider()->createApproxTexture(maskDesc));
        if (!maskTexture) {
            return false;
        }

        GrDrawContext* drawContext = context->drawContext();
        if (drawContext) {
            GrPaint grPaint;
            grPaint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
            SkRegion::Iterator iter(fRegion);
            drawContext->clear(maskTexture->asRenderTarget(), NULL, 0x0, true);

            while (!iter.done()) {
                SkRect rect = SkRect::Make(iter.rect());
                drawContext->drawRect(maskTexture->asRenderTarget(), GrClip::WideOpen(), grPaint,
                                      in_matrix, rect);
                iter.next();
            }
        }

        *fp = AlphaThresholdEffect::Create(procDataManager,
                                           texture,
                                           maskTexture,
                                           fInnerThreshold,
                                           fOuterThreshold);
    }
    return true;
}
#endif

void SkAlphaThresholdFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fInnerThreshold);
    buffer.writeScalar(fOuterThreshold);
    buffer.writeRegion(fRegion);
}

bool SkAlphaThresholdFilterImpl::onFilterImage(Proxy*, const SkBitmap& src,
                                               const Context& ctx, SkBitmap* dst,
                                               SkIPoint* offset) const {
    SkASSERT(src.colorType() == kN32_SkColorType);

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkMatrix localInverse;
    if (!ctx.ctm().invert(&localInverse)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    SkASSERT(src.getPixels());
    if (!src.getPixels() || src.width() <= 0 || src.height() <= 0) {
        return false;
    }

    if (!dst->tryAllocPixels(src.info())) {
        return false;
    }

    U8CPU innerThreshold = (U8CPU)(fInnerThreshold * 0xFF);
    U8CPU outerThreshold = (U8CPU)(fOuterThreshold * 0xFF);
    SkColor* sptr = src.getAddr32(0, 0);
    SkColor* dptr = dst->getAddr32(0, 0);
    int width = src.width(), height = src.height();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const SkColor& source = sptr[y * width + x];
            SkColor output_color(source);
            SkPoint position;
            localInverse.mapXY((SkScalar)x, (SkScalar)y, &position);
            if (fRegion.contains((int32_t)position.x(), (int32_t)position.y())) {
                if (SkColorGetA(source) < innerThreshold) {
                    U8CPU alpha = SkColorGetA(source);
                    if (alpha == 0)
                        alpha = 1;
                    float scale = (float)innerThreshold / alpha;
                    output_color = SkColorSetARGB(innerThreshold,
                                                  (U8CPU)(SkColorGetR(source) * scale),
                                                  (U8CPU)(SkColorGetG(source) * scale),
                                                  (U8CPU)(SkColorGetB(source) * scale));
                }
            } else {
                if (SkColorGetA(source) > outerThreshold) {
                    float scale = (float)outerThreshold / SkColorGetA(source);
                    output_color = SkColorSetARGB(outerThreshold,
                                                  (U8CPU)(SkColorGetR(source) * scale),
                                                  (U8CPU)(SkColorGetG(source) * scale),
                                                  (U8CPU)(SkColorGetB(source) * scale));
                }
            }
            dptr[y * dst->width() + x] = output_color;
        }
    }

    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkAlphaThresholdFilterImpl::toString(SkString* str) const {
    str->appendf("SkAlphaThresholdImageFilter: (");
    str->appendf("inner: %f outer: %f", fInnerThreshold, fOuterThreshold);
    str->append(")");
}
#endif

