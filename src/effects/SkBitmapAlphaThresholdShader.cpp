/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapAlphaThresholdShader.h"

class BATShader : public SkShader {
public:
    SK_DECLARE_INST_COUNT(SkThresholdShader);

    BATShader(const SkBitmap& bitmap, SkRegion region, U8CPU);
    BATShader(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        // We should probably do something here.
    }


    virtual void shadeSpan(int x, int y, SkPMColor[], int count) SK_OVERRIDE {};

#if SK_SUPPORT_GPU
    virtual GrEffectRef* asNewEffect(GrContext* context, const SkPaint& paint) const SK_OVERRIDE;
#endif

    SK_DEVELOPER_TO_STRING();
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(BATShader)

private:
    SkBitmap fBitmap;
    SkRegion fRegion;
    U8CPU    fThreshold;

    typedef SkShader INHERITED;
};

SkShader* SkBitmapAlphaThresholdShader::Create(const SkBitmap& bitmap,
                                               const SkRegion& region,
                                               U8CPU threshold) {
    SkASSERT(threshold < 256);
    return SkNEW_ARGS(BATShader, (bitmap, region, threshold));
}

BATShader::BATShader(const SkBitmap& bitmap, SkRegion region, U8CPU threshold)
: fBitmap(bitmap)
, fRegion(region)
, fThreshold(threshold) {
};


#ifdef SK_DEVELOPER
void BATShader::toString(SkString* str) const {
    str->append("BATShader: (");

    fBitmap.toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "GrTBackendEffectFactory.h"
#include "GrTextureAccess.h"

#include "SkGr.h"

/**
 * Could create specializations for some simple cases:
 *  - The region is empty.
 *  - The region fully contains the bitmap.
 *  - The regions is 1 rect (or maybe a small number of rects).
 */
class ThresholdEffect : public GrEffect {
public:
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<ThresholdEffect>::getInstance();
    }

    static GrEffectRef* Create(GrTexture* bmpTexture, const SkMatrix& bmpMatrix,
                               GrTexture* maskTexture, const SkMatrix& maskMatrix,
                               U8CPU threshold) {
        SkScalar thresh = SkIntToScalar(threshold) / 255;

        AutoEffectUnref effect(SkNEW_ARGS(ThresholdEffect, (bmpTexture, bmpMatrix,
                                                            maskTexture, maskMatrix,
                                                            thresh)));
        return CreateEffectRef(effect);
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        if ((kA_GrColorComponentFlag & *validFlags) && 0 == GrColorUnpackA(*color)) {
            return;
        }
        *validFlags = 0;
        return;
    }

    static const char* Name() { return "Bitmap Alpha Threshold"; }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory,
                    const GrDrawEffect& e)
        : GrGLEffect(factory)
        , fBmpMatrix(GrEffect::kLocal_CoordsType)
        , fMaskMatrix(GrEffect::kLocal_CoordsType)
        , fPrevThreshold(-SK_Scalar1) {
        }

        virtual void emitCode(GrGLShaderBuilder* builder,
                                const GrDrawEffect& drawEffect,
                                EffectKey key,
                                const char* outputColor,
                                const char* inputColor,
                                const TextureSamplerArray& samplers) SK_OVERRIDE {
            SkString bmpCoord;
            SkString maskCoord;

            GrSLType bmpCoordType = fBmpMatrix.emitCode(builder, key, &bmpCoord, NULL, "Bmp");
            EffectKey maskMatrixKey = key >> GrGLEffectMatrix::kKeyBits;
            GrSLType maskCoordType = fMaskMatrix.emitCode(builder,
                                                          maskMatrixKey,
                                                          &maskCoord,
                                                          NULL,
                                                          "Mask");

            // put bitmap color in "color"
            builder->fsCodeAppend("\t\tvec4 color = ");
            builder->fsAppendTextureLookup(samplers[0], bmpCoord.c_str(), bmpCoordType);
            builder->fsCodeAppend(";\n");

            // put alpha from mask texture in "mask"
            builder->fsCodeAppend("\t\tfloat mask = ");
            builder->fsAppendTextureLookup(samplers[1], maskCoord.c_str(), maskCoordType);
            builder->fsCodeAppend(".a;\n");

            const char* threshold;

            fThresholdUniHandle = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                      kFloat_GrSLType,
                                                      "threshold",
                                                      &threshold);
            builder->fsCodeAppendf("\t\tfloat thresh = %s;\n", threshold);

            builder->fsCodeAppend("\t\tif (mask < 0.5) {\n"
                                    "\t\t\tif (color.a > thresh) {\n"
                                    "\t\t\t\tfloat scale = thresh / color.a;\n"
                                    "\t\t\t\tcolor.rgb *= scale;\n"
                                    "\t\t\t\tcolor.a = thresh;\n"
                                    "\t\t\t}\n"
                                    "\t\t} else if (color.a < thresh) {\n"
                                    "\t\t\tfloat scale = thresh / color.a;\n"
                                    "\t\t\tcolor.rgb *= scale;\n"
                                    "\t\t\tcolor.a = thresh;\n"
                                    "\t\t}\n");

            builder->fsCodeAppend("color = ");
            SkString outStr;
            outStr.appendf("\t\t%s = ", outputColor);
            GrGLSLModulatef<4>(&outStr, inputColor, "color");
            outStr.append(";\n");
            builder->fsCodeAppend(outStr.c_str());
        }

        virtual void setData(const GrGLUniformManager& uman, const GrDrawEffect& e) SK_OVERRIDE {
            const ThresholdEffect& effect = e.castEffect<ThresholdEffect>();
            fBmpMatrix.setData(uman, effect.fBmpMatrix, e, effect.fBmpAccess.getTexture());
            fMaskMatrix.setData(uman, effect.fMaskMatrix, e, effect.fMaskAccess.getTexture());
            if (fPrevThreshold != effect.fThreshold) {
                uman.set1f(fThresholdUniHandle, effect.fThreshold);
            }
        }

        static inline EffectKey GenKey(const GrDrawEffect& e, const GrGLCaps&) {
            const ThresholdEffect& effect = e.castEffect<ThresholdEffect>();

            EffectKey bmpMKey = GrGLEffectMatrix::GenKey(effect.fBmpMatrix,
                                                         e,
                                                         GrEffect::kLocal_CoordsType,
                                                         effect.fBmpAccess.getTexture());
            EffectKey maskMKey = GrGLEffectMatrix::GenKey(effect.fMaskMatrix,
                                                          e,
                                                          GrEffect::kLocal_CoordsType,
                                                          effect.fMaskAccess.getTexture());
            return bmpMKey | (maskMKey << GrGLEffectMatrix::kKeyBits);
        }

    private:
        GrGLEffectMatrix fBmpMatrix;
        GrGLEffectMatrix fMaskMatrix;

        GrGLUniformManager::UniformHandle fThresholdUniHandle;
        SkScalar                          fPrevThreshold;
    };

    GR_DECLARE_EFFECT_TEST;

private:
    ThresholdEffect(GrTexture* bmpTexture, const SkMatrix& bmpMatrix,
                    GrTexture* maskTexture, const SkMatrix& maskMatrix,
                    SkScalar threshold)
    : fBmpAccess(bmpTexture, GrTextureParams())
    , fMaskAccess(maskTexture, GrTextureParams())
    , fBmpMatrix(bmpMatrix)
    , fMaskMatrix(maskMatrix)
    , fThreshold(threshold) {
        this->addTextureAccess(&fBmpAccess);
        this->addTextureAccess(&fMaskAccess);
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const ThresholdEffect& e = CastEffect<ThresholdEffect>(other);
        return e.fBmpAccess.getTexture() == fBmpAccess.getTexture() &&
               e.fMaskAccess.getTexture() == fMaskAccess.getTexture() &&
               e.fBmpMatrix == fBmpMatrix &&
               e.fMaskMatrix == fMaskMatrix &&
               e.fThreshold == fThreshold;
    }

    GrTextureAccess fBmpAccess;
    GrTextureAccess fMaskAccess;

    SkMatrix fBmpMatrix;
    SkMatrix fMaskMatrix;

    SkScalar fThreshold;
};

GR_DEFINE_EFFECT_TEST(ThresholdEffect);

GrEffectRef* ThresholdEffect::TestCreate(SkMWCRandom* rand,
                                         GrContext*,
                                         const GrDrawTargetCaps&,
                                         GrTexture* textures[]) {
    GrTexture* bmpTex = textures[GrEffectUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = textures[GrEffectUnitTest::kAlphaTextureIdx];
    U8CPU thresh = rand->nextU() % 0xff;
    return ThresholdEffect::Create(bmpTex, SkMatrix::I(), maskTex, SkMatrix::I(), thresh);
}

GrEffectRef* BATShader::asNewEffect(GrContext* context, const SkPaint& paint) const {
    SkMatrix localInverse;
    if (!this->getLocalMatrix().invert(&localInverse)) {
        return NULL;
    }

    GrTextureDesc maskDesc;
    if (context->isConfigRenderable(kAlpha_8_GrPixelConfig)) {
        maskDesc.fConfig = kAlpha_8_GrPixelConfig;
    } else {
        maskDesc.fConfig = kRGBA_8888_GrPixelConfig;
    }
    maskDesc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    const SkIRect& bounds = fRegion.getBounds();
    // Add one pixel of border to ensure that clamp mode will be all zeros
    // the outside.
    maskDesc.fWidth = bounds.width() + 2;
    maskDesc.fHeight = bounds.height() + 2;
    GrAutoScratchTexture ast(context, maskDesc, GrContext::kApprox_ScratchTexMatch);
    GrTexture*  maskTexture = ast.texture();
    if (NULL == maskTexture) {
        return NULL;
    }

    GrPaint grPaint;
    grPaint.setBlendFunc(kOne_GrBlendCoeff, kZero_GrBlendCoeff);
    SkRegion::Iterator iter(fRegion);
    context->setRenderTarget(maskTexture->asRenderTarget());
    context->clear(NULL, 0x0);

    // offset to ensure border is zero on top/left
    SkMatrix matrix;
    matrix.setTranslate(SK_Scalar1, SK_Scalar1);
    context->setMatrix(matrix);

    while (!iter.done()) {
        SkRect rect = SkRect::MakeFromIRect(iter.rect());
        context->drawRect(grPaint, rect);
        iter.next();
    }

    GrTexture* bmpTexture = GrLockAndRefCachedBitmapTexture(context, fBitmap, NULL);
    if (NULL == bmpTexture) {
        return NULL;
    }

    SkMatrix bmpMatrix = localInverse;
    bmpMatrix.postIDiv(bmpTexture->width(), bmpTexture->height());

    SkMatrix maskMatrix = localInverse;
    // compensate for the border
    maskMatrix.postTranslate(SK_Scalar1, SK_Scalar1);
    maskMatrix.postIDiv(maskTexture->width(), maskTexture->height());

    GrEffectRef* effect = ThresholdEffect::Create(bmpTexture, bmpMatrix,
                                                  maskTexture, maskMatrix,
                                                  fThreshold);

    GrUnlockAndUnrefCachedBitmapTexture(bmpTexture);

    return effect;
}

#endif
