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
    SkAlphaThresholdFilterImpl(const SkRegion& region, SkScalar innerThreshold, SkScalar outerThreshold);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAlphaThresholdFilterImpl)

protected:
    explicit SkAlphaThresholdFilterImpl(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool asNewEffect(GrEffectRef** effect, GrTexture* texture,
                             const SkMatrix& matrix, const SkIRect& bounds) const SK_OVERRIDE;
#endif

private:
    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;
    typedef SkImageFilter INHERITED;
};

SkImageFilter* SkAlphaThresholdFilter::Create(const SkRegion& region,
                                              SkScalar innerThreshold,
                                              SkScalar outerThreshold) {
    return SkNEW_ARGS(SkAlphaThresholdFilterImpl, (region, innerThreshold, outerThreshold));
}

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrEffect.h"
#include "gl/GrGLEffect.h"
#include "GrTBackendEffectFactory.h"
#include "GrTextureAccess.h"

#include "SkGr.h"

class GrGLAlphaThresholdEffect;

class AlphaThresholdEffect : public GrEffect {

public:
    static GrEffectRef* Create(GrTexture* texture,
                               GrTexture* maskTexture,
                               float innerThreshold,
                               float outerThreshold) {
        AutoEffectUnref effect(SkNEW_ARGS(AlphaThresholdEffect, (texture,
                                                                 maskTexture,
                                                                 innerThreshold,
                                                                 outerThreshold)));
        return CreateEffectRef(effect);
    }

    virtual ~AlphaThresholdEffect() {};

    static const char* Name() { return "Alpha Threshold"; }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    float innerThreshold() const { return fInnerThreshold; }
    float outerThreshold() const { return fOuterThreshold; }

    typedef GrGLAlphaThresholdEffect GLEffect;

private:
    AlphaThresholdEffect(GrTexture* texture,
                         GrTexture* maskTexture,
                         float innerThreshold,
                         float outerThreshold)
        : fInnerThreshold(innerThreshold)
        , fOuterThreshold(outerThreshold)
        , fImageCoordTransform(kLocal_GrCoordSet, MakeDivByTextureWHMatrix(texture), texture)
        , fImageTextureAccess(texture)
        , fMaskCoordTransform(kLocal_GrCoordSet, MakeDivByTextureWHMatrix(maskTexture), maskTexture)
        , fMaskTextureAccess(maskTexture) {
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureAccess(&fImageTextureAccess);
        this->addCoordTransform(&fMaskCoordTransform);
        this->addTextureAccess(&fMaskTextureAccess);
    }

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GR_DECLARE_EFFECT_TEST;

    float fInnerThreshold;
    float fOuterThreshold;
    GrCoordTransform fImageCoordTransform;
    GrTextureAccess  fImageTextureAccess;
    GrCoordTransform fMaskCoordTransform;
    GrTextureAccess  fMaskTextureAccess;

    typedef GrEffect INHERITED;
};

class GrGLAlphaThresholdEffect : public GrGLEffect {
public:
    GrGLAlphaThresholdEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:

    GrGLUniformManager::UniformHandle       fInnerThresholdVar;
    GrGLUniformManager::UniformHandle       fOuterThresholdVar;

    typedef GrGLEffect INHERITED;
};

GrGLAlphaThresholdEffect::GrGLAlphaThresholdEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
    : INHERITED(factory) {
}

void GrGLAlphaThresholdEffect::emitCode(GrGLShaderBuilder* builder,
                                        const GrDrawEffect&,
                                        EffectKey key,
                                        const char* outputColor,
                                        const char* inputColor,
                                        const TransformedCoordsArray& coords,
                                        const TextureSamplerArray& samplers) {
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    SkString maskCoords2D = builder->ensureFSCoords2D(coords, 1);
    fInnerThresholdVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility,
        kFloat_GrSLType, "inner_threshold");
    fOuterThresholdVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility,
        kFloat_GrSLType, "outer_threshold");

    builder->fsCodeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    builder->fsCodeAppendf("\t\tvec2 mask_coord = %s;\n", maskCoords2D.c_str());
    builder->fsCodeAppend("\t\tvec4 input_color = ");
    builder->fsAppendTextureLookup(samplers[0], "coord");
    builder->fsCodeAppend(";\n");
    builder->fsCodeAppend("\t\tvec4 mask_color = ");
    builder->fsAppendTextureLookup(samplers[1], "mask_coord");
    builder->fsCodeAppend(";\n");

    builder->fsCodeAppendf("\t\tfloat inner_thresh = %s;\n",
                           builder->getUniformCStr(fInnerThresholdVar));
    builder->fsCodeAppendf("\t\tfloat outer_thresh = %s;\n",
                           builder->getUniformCStr(fOuterThresholdVar));
    builder->fsCodeAppend("\t\tfloat mask = mask_color.a;\n");

    builder->fsCodeAppend("vec4 color = input_color;\n");
    builder->fsCodeAppend("\t\tif (mask < 0.5) {\n"
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

    builder->fsCodeAppendf("%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr4("color")).c_str());
}

void GrGLAlphaThresholdEffect::setData(const GrGLUniformManager& uman,
                                  const GrDrawEffect& drawEffect) {
    const AlphaThresholdEffect& alpha_threshold =
        drawEffect.castEffect<AlphaThresholdEffect>();
    uman.set1f(fInnerThresholdVar, alpha_threshold.innerThreshold());
    uman.set1f(fOuterThresholdVar, alpha_threshold.outerThreshold());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(AlphaThresholdEffect);

GrEffectRef* AlphaThresholdEffect::TestCreate(SkRandom* random,
                                              GrContext* context,
                                              const GrDrawTargetCaps&,
                                              GrTexture** textures) {
    GrTexture* bmpTex = textures[GrEffectUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = textures[GrEffectUnitTest::kAlphaTextureIdx];
    float inner_thresh = random->nextUScalar1();
    float outer_thresh = random->nextUScalar1();
    return AlphaThresholdEffect::Create(bmpTex, maskTex, inner_thresh, outer_thresh);
}

///////////////////////////////////////////////////////////////////////////////

const GrBackendEffectFactory& AlphaThresholdEffect::getFactory() const {
    return GrTBackendEffectFactory<AlphaThresholdEffect>::getInstance();
}

bool AlphaThresholdEffect::onIsEqual(const GrEffect& sBase) const {
    const AlphaThresholdEffect& s = CastEffect<AlphaThresholdEffect>(sBase);
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

SkAlphaThresholdFilterImpl::SkAlphaThresholdFilterImpl(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fInnerThreshold = buffer.readScalar();
    fOuterThreshold = buffer.readScalar();
    buffer.readRegion(&fRegion);
}

SkAlphaThresholdFilterImpl::SkAlphaThresholdFilterImpl(const SkRegion& region,
                                                       SkScalar innerThreshold,
                                                       SkScalar outerThreshold)
    : INHERITED(0)
    , fRegion(region)
    , fInnerThreshold(innerThreshold)
    , fOuterThreshold(outerThreshold) {
}

#if SK_SUPPORT_GPU
bool SkAlphaThresholdFilterImpl::asNewEffect(GrEffectRef** effect, GrTexture* texture,
                                             const SkMatrix& in_matrix, const SkIRect&) const {
    if (effect) {
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

        *effect = AlphaThresholdEffect::Create(texture,
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
    SkASSERT(src.colorType() == kPMColor_SkColorType);

    if (src.colorType() != kPMColor_SkColorType) {
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

    dst->setConfig(src.config(), src.width(), src.height());
    if (!dst->allocPixels()) {
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
