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

class SK_API SkAlphaThresholdFilterImpl : public SkImageFilter {
public:
    SkAlphaThresholdFilterImpl(const SkRegion& region, SkScalar innerThreshold,
                               SkScalar outerThreshold, SkImageFilter* input);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAlphaThresholdFilterImpl)

protected:
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkAlphaThresholdFilterImpl(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool asFragmentProcessor(GrFragmentProcessor**, GrTexture*, const SkMatrix&,
                                     const SkIRect& bounds) const SK_OVERRIDE;
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
#include "GrProcessor.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "GrTBackendProcessorFactory.h"
#include "GrTextureAccess.h"

#include "SkGr.h"

class GrGLAlphaThresholdEffect;

class AlphaThresholdEffect : public GrFragmentProcessor {

public:
    static GrFragmentProcessor* Create(GrTexture* texture,
                                       GrTexture* maskTexture,
                                       float innerThreshold,
                                       float outerThreshold) {
        return SkNEW_ARGS(AlphaThresholdEffect, (texture,
                                                 maskTexture,
                                                 innerThreshold,
                                                 outerThreshold));
    }

    virtual ~AlphaThresholdEffect() {};

    static const char* Name() { return "Alpha Threshold"; }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    float innerThreshold() const { return fInnerThreshold; }
    float outerThreshold() const { return fOuterThreshold; }

    typedef GrGLAlphaThresholdEffect GLProcessor;

private:
    AlphaThresholdEffect(GrTexture* texture,
                         GrTexture* maskTexture,
                         float innerThreshold,
                         float outerThreshold)
        : fInnerThreshold(innerThreshold)
        , fOuterThreshold(outerThreshold)
        , fImageCoordTransform(kLocal_GrCoordSet,
                               GrCoordTransform::MakeDivByTextureWHMatrix(texture), texture)
        , fImageTextureAccess(texture)
        , fMaskCoordTransform(kLocal_GrCoordSet,
                              GrCoordTransform::MakeDivByTextureWHMatrix(maskTexture), maskTexture)
        , fMaskTextureAccess(maskTexture) {
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureAccess(&fImageTextureAccess);
        this->addCoordTransform(&fMaskCoordTransform);
        this->addTextureAccess(&fMaskTextureAccess);
    }

    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

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
    GrGLAlphaThresholdEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder*,
                          const GrFragmentProcessor&,
                          const GrProcessorKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:

    GrGLProgramDataManager::UniformHandle fInnerThresholdVar;
    GrGLProgramDataManager::UniformHandle fOuterThresholdVar;

    typedef GrGLFragmentProcessor INHERITED;
};

GrGLAlphaThresholdEffect::GrGLAlphaThresholdEffect(const GrBackendProcessorFactory& factory,
                                                   const GrProcessor&)
    : INHERITED(factory) {
}

void GrGLAlphaThresholdEffect::emitCode(GrGLProgramBuilder* builder,
                                        const GrFragmentProcessor&,
                                        const GrProcessorKey& key,
                                        const char* outputColor,
                                        const char* inputColor,
                                        const TransformedCoordsArray& coords,
                                        const TextureSamplerArray& samplers) {
    fInnerThresholdVar = builder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility,
        kFloat_GrSLType, "inner_threshold");
    fOuterThresholdVar = builder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility,
        kFloat_GrSLType, "outer_threshold");

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);
    SkString maskCoords2D = fsBuilder->ensureFSCoords2D(coords, 1);

    fsBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fsBuilder->codeAppendf("\t\tvec2 mask_coord = %s;\n", maskCoords2D.c_str());
    fsBuilder->codeAppend("\t\tvec4 input_color = ");
    fsBuilder->appendTextureLookup(samplers[0], "coord");
    fsBuilder->codeAppend(";\n");
    fsBuilder->codeAppend("\t\tvec4 mask_color = ");
    fsBuilder->appendTextureLookup(samplers[1], "mask_coord");
    fsBuilder->codeAppend(";\n");

    fsBuilder->codeAppendf("\t\tfloat inner_thresh = %s;\n",
                           builder->getUniformCStr(fInnerThresholdVar));
    fsBuilder->codeAppendf("\t\tfloat outer_thresh = %s;\n",
                           builder->getUniformCStr(fOuterThresholdVar));
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

    fsBuilder->codeAppendf("%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr4("color")).c_str());
}

void GrGLAlphaThresholdEffect::setData(const GrGLProgramDataManager& pdman,
                                       const GrProcessor& proc) {
    const AlphaThresholdEffect& alpha_threshold = proc.cast<AlphaThresholdEffect>();
    pdman.set1f(fInnerThresholdVar, alpha_threshold.innerThreshold());
    pdman.set1f(fOuterThresholdVar, alpha_threshold.outerThreshold());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AlphaThresholdEffect);

GrFragmentProcessor* AlphaThresholdEffect::TestCreate(SkRandom* random,
                                           GrContext* context,
                                           const GrDrawTargetCaps&,
                                           GrTexture** textures) {
    GrTexture* bmpTex = textures[GrProcessorUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = textures[GrProcessorUnitTest::kAlphaTextureIdx];
    float inner_thresh = random->nextUScalar1();
    float outer_thresh = random->nextUScalar1();
    return AlphaThresholdEffect::Create(bmpTex, maskTex, inner_thresh, outer_thresh);
}

///////////////////////////////////////////////////////////////////////////////

const GrBackendFragmentProcessorFactory& AlphaThresholdEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<AlphaThresholdEffect>::getInstance();
}

bool AlphaThresholdEffect::onIsEqual(const GrProcessor& sBase) const {
    const AlphaThresholdEffect& s = sBase.cast<AlphaThresholdEffect>();
    return (this->texture(0) == s.texture(0) &&
            this->fInnerThreshold == s.fInnerThreshold &&
            this->fOuterThreshold == s.fOuterThreshold);
}

void AlphaThresholdEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    if ((*validFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
        GrPixelConfigIsOpaque(this->texture(0)->config())) {
        *validFlags = kA_GrColorComponentFlag;
    } else {
        *validFlags = 0;
    }
}

#endif

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkAlphaThresholdFilterImpl::SkAlphaThresholdFilterImpl(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fInnerThreshold = buffer.readScalar();
    fOuterThreshold = buffer.readScalar();
    buffer.readRegion(&fRegion);
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
                                                     GrTexture* texture,
                                                     const SkMatrix& in_matrix,
                                                     const SkIRect&) const {
    if (fp) {
        GrContext* context = texture->getContext();
        GrTextureDesc maskDesc;
        if (context->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
            maskDesc.fConfig = kAlpha_8_GrPixelConfig;
        } else {
            maskDesc.fConfig = kRGBA_8888_GrPixelConfig;
        }
        maskDesc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
        // Add one pixel of border to ensure that clamp mode will be all zeros
        // the outside.
        maskDesc.fWidth = texture->width();
        maskDesc.fHeight = texture->height();
        GrAutoScratchTexture ast(context, maskDesc, GrContext::kApprox_ScratchTexMatch);
        GrTexture*  maskTexture = ast.texture();
        if (NULL == maskTexture) {
            return false;
        }

        {
            GrContext::AutoRenderTarget art(context, ast.texture()->asRenderTarget());
            GrPaint grPaint;
            grPaint.setBlendFunc(kOne_GrBlendCoeff, kZero_GrBlendCoeff);
            SkRegion::Iterator iter(fRegion);
            context->clear(NULL, 0x0, true);

            SkMatrix old_matrix = context->getMatrix();
            context->setMatrix(in_matrix);

            while (!iter.done()) {
                SkRect rect = SkRect::Make(iter.rect());
                context->drawRect(grPaint, rect);
                iter.next();
            }
            context->setMatrix(old_matrix);
        }

        *fp = AlphaThresholdEffect::Create(texture,
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
