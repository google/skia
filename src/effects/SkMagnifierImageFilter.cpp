/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkMagnifierImageFilter.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "effects/GrSingleTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"

class GrGLMagnifierEffect;

class GrMagnifierEffect : public GrSingleTextureEffect {

public:
    static GrEffectRef* Create(GrTexture* texture,
                               float xOffset,
                               float yOffset,
                               float xInvZoom,
                               float yInvZoom,
                               float xInvInset,
                               float yInvInset) {
        AutoEffectUnref effect(SkNEW_ARGS(GrMagnifierEffect, (texture,
                                                              xOffset,
                                                              yOffset,
                                                              xInvZoom,
                                                              yInvZoom,
                                                              xInvInset,
                                                              yInvInset)));
        return CreateEffectRef(effect);
    }

    virtual ~GrMagnifierEffect() {};

    static const char* Name() { return "Magnifier"; }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    float x_offset() const { return fXOffset; }
    float y_offset() const { return fYOffset; }
    float x_inv_zoom() const { return fXInvZoom; }
    float y_inv_zoom() const { return fYInvZoom; }
    float x_inv_inset() const { return fXInvInset; }
    float y_inv_inset() const { return fYInvInset; }

    typedef GrGLMagnifierEffect GLEffect;

private:
    GrMagnifierEffect(GrTexture* texture,
                      float xOffset,
                      float yOffset,
                      float xInvZoom,
                      float yInvZoom,
                      float xInvInset,
                      float yInvInset)
        : GrSingleTextureEffect(texture, MakeDivByTextureWHMatrix(texture))
        , fXOffset(xOffset)
        , fYOffset(yOffset)
        , fXInvZoom(xInvZoom)
        , fYInvZoom(yInvZoom)
        , fXInvInset(xInvInset)
        , fYInvInset(yInvInset) {}

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GR_DECLARE_EFFECT_TEST;

    float fXOffset;
    float fYOffset;
    float fXInvZoom;
    float fYInvZoom;
    float fXInvInset;
    float fYInvInset;

    typedef GrSingleTextureEffect INHERITED;
};

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;

class GrGLMagnifierEffect : public GrGLEffect {
public:
    GrGLMagnifierEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    UniformHandle       fOffsetVar;
    UniformHandle       fInvZoomVar;
    UniformHandle       fInvInsetVar;

    typedef GrGLEffect INHERITED;
};

GrGLMagnifierEffect::GrGLMagnifierEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
    : INHERITED(factory) {
}

void GrGLMagnifierEffect::emitCode(GrGLShaderBuilder* builder,
                                   const GrDrawEffect&,
                                   EffectKey key,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const TransformedCoordsArray& coords,
                                   const TextureSamplerArray& samplers) {
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    fOffsetVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility |
        GrGLShaderBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "Offset");
    fInvZoomVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility |
        GrGLShaderBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "InvZoom");
    fInvInsetVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility |
        GrGLShaderBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "InvInset");

    builder->fsCodeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    builder->fsCodeAppendf("\t\tvec2 zoom_coord = %s + %s * %s;\n",
                           builder->getUniformCStr(fOffsetVar),
                           coords2D.c_str(),
                           builder->getUniformCStr(fInvZoomVar));

    builder->fsCodeAppend("\t\tvec2 delta = min(coord, vec2(1.0, 1.0) - coord);\n");

    builder->fsCodeAppendf("\t\tdelta = delta * %s;\n", builder->getUniformCStr(fInvInsetVar));

    builder->fsCodeAppend("\t\tfloat weight = 0.0;\n");
    builder->fsCodeAppend("\t\tif (delta.s < 2.0 && delta.t < 2.0) {\n");
    builder->fsCodeAppend("\t\t\tdelta = vec2(2.0, 2.0) - delta;\n");
    builder->fsCodeAppend("\t\t\tfloat dist = length(delta);\n");
    builder->fsCodeAppend("\t\t\tdist = max(2.0 - dist, 0.0);\n");
    builder->fsCodeAppend("\t\t\tweight = min(dist * dist, 1.0);\n");
    builder->fsCodeAppend("\t\t} else {\n");
    builder->fsCodeAppend("\t\t\tvec2 delta_squared = delta * delta;\n");
    builder->fsCodeAppend("\t\t\tweight = min(min(delta_squared.x, delta_squared.y), 1.0);\n");
    builder->fsCodeAppend("\t\t}\n");

    builder->fsCodeAppend("\t\tvec2 mix_coord = mix(coord, zoom_coord, weight);\n");
    builder->fsCodeAppend("\t\tvec4 output_color = ");
    builder->fsAppendTextureLookup(samplers[0], "mix_coord");
    builder->fsCodeAppend(";\n");

    builder->fsCodeAppendf("\t\t%s = output_color;", outputColor);
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    builder->fsCodeAppend(modulate.c_str());
}

void GrGLMagnifierEffect::setData(const GrGLUniformManager& uman,
                                  const GrDrawEffect& drawEffect) {
    const GrMagnifierEffect& zoom = drawEffect.castEffect<GrMagnifierEffect>();
    uman.set2f(fOffsetVar, zoom.x_offset(), zoom.y_offset());
    uman.set2f(fInvZoomVar, zoom.x_inv_zoom(), zoom.y_inv_zoom());
    uman.set2f(fInvInsetVar, zoom.x_inv_inset(), zoom.y_inv_inset());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrMagnifierEffect);

GrEffectRef* GrMagnifierEffect::TestCreate(SkRandom* random,
                                           GrContext* context,
                                           const GrDrawTargetCaps&,
                                           GrTexture** textures) {
    GrTexture* texture = textures[0];
    const int kMaxWidth = 200;
    const int kMaxHeight = 200;
    const int kMaxInset = 20;
    uint32_t width = random->nextULessThan(kMaxWidth);
    uint32_t height = random->nextULessThan(kMaxHeight);
    uint32_t x = random->nextULessThan(kMaxWidth - width);
    uint32_t y = random->nextULessThan(kMaxHeight - height);
    uint32_t inset = random->nextULessThan(kMaxInset);

    GrEffectRef* effect = GrMagnifierEffect::Create(
        texture,
        (float) width / texture->width(),
        (float) height / texture->height(),
        texture->width() / (float) x,
        texture->height() / (float) y,
        (float) inset / texture->width(),
        (float) inset / texture->height());
    SkASSERT(NULL != effect);
    return effect;
}

///////////////////////////////////////////////////////////////////////////////

const GrBackendEffectFactory& GrMagnifierEffect::getFactory() const {
    return GrTBackendEffectFactory<GrMagnifierEffect>::getInstance();
}

bool GrMagnifierEffect::onIsEqual(const GrEffect& sBase) const {
    const GrMagnifierEffect& s = CastEffect<GrMagnifierEffect>(sBase);
    return (this->texture(0) == s.texture(0) &&
            this->fXOffset == s.fXOffset &&
            this->fYOffset == s.fYOffset &&
            this->fXInvZoom == s.fXInvZoom &&
            this->fYInvZoom == s.fYInvZoom &&
            this->fXInvInset == s.fXInvInset &&
            this->fYInvInset == s.fYInvInset);
}

void GrMagnifierEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

#endif

////////////////////////////////////////////////////////////////////////////////
SkMagnifierImageFilter::SkMagnifierImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    float x = buffer.readScalar();
    float y = buffer.readScalar();
    float width = buffer.readScalar();
    float height = buffer.readScalar();
    fSrcRect = SkRect::MakeXYWH(x, y, width, height);
    fInset = buffer.readScalar();

    buffer.validate(SkScalarIsFinite(fInset) && SkIsValidRect(fSrcRect) &&
                    // Negative numbers in src rect are not supported
                    (fSrcRect.fLeft >= 0) && (fSrcRect.fTop >= 0));
}

// FIXME:  implement single-input semantics
SkMagnifierImageFilter::SkMagnifierImageFilter(const SkRect& srcRect, SkScalar inset)
    : INHERITED(0), fSrcRect(srcRect), fInset(inset) {
    SkASSERT(srcRect.x() >= 0 && srcRect.y() >= 0 && inset >= 0);
}

#if SK_SUPPORT_GPU
bool SkMagnifierImageFilter::asNewEffect(GrEffectRef** effect, GrTexture* texture, const SkMatrix&, const SkIRect&) const {
    if (effect) {
        SkScalar yOffset = (texture->origin() == kTopLeft_GrSurfaceOrigin) ? fSrcRect.y() :
                           (texture->height() - (fSrcRect.y() + fSrcRect.height()));
        SkScalar invInset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;
        *effect = GrMagnifierEffect::Create(texture,
                                            fSrcRect.x() / texture->width(),
                                            yOffset / texture->height(),
                                            fSrcRect.width() / texture->width(),
                                            fSrcRect.height() / texture->height(),
                                            texture->width() * invInset,
                                            texture->height() * invInset);
    }
    return true;
}
#endif

void SkMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSrcRect.x());
    buffer.writeScalar(fSrcRect.y());
    buffer.writeScalar(fSrcRect.width());
    buffer.writeScalar(fSrcRect.height());
    buffer.writeScalar(fInset);
}

bool SkMagnifierImageFilter::onFilterImage(Proxy*, const SkBitmap& src,
                                           const Context&, SkBitmap* dst,
                                           SkIPoint* offset) const {
    SkASSERT(src.colorType() == kPMColor_SkColorType);
    SkASSERT(fSrcRect.width() < src.width());
    SkASSERT(fSrcRect.height() < src.height());

    if ((src.colorType() != kPMColor_SkColorType) ||
        (fSrcRect.width() >= src.width()) ||
        (fSrcRect.height() >= src.height())) {
      return false;
    }

    SkAutoLockPixels alp(src);
    SkASSERT(src.getPixels());
    if (!src.getPixels() || src.width() <= 0 || src.height() <= 0) {
      return false;
    }

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();
    if (!dst->getPixels()) {
        return false;
    }

    SkScalar inv_inset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;

    SkScalar inv_x_zoom = fSrcRect.width() / src.width();
    SkScalar inv_y_zoom = fSrcRect.height() / src.height();

    SkColor* sptr = src.getAddr32(0, 0);
    SkColor* dptr = dst->getAddr32(0, 0);
    int width = src.width(), height = src.height();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            SkScalar x_dist = SkMin32(x, width - x - 1) * inv_inset;
            SkScalar y_dist = SkMin32(y, height - y - 1) * inv_inset;
            SkScalar weight = 0;

            static const SkScalar kScalar2 = SkScalar(2);

            // To create a smooth curve at the corners, we need to work on
            // a square twice the size of the inset.
            if (x_dist < kScalar2 && y_dist < kScalar2) {
                x_dist = kScalar2 - x_dist;
                y_dist = kScalar2 - y_dist;

                SkScalar dist = SkScalarSqrt(SkScalarSquare(x_dist) +
                                             SkScalarSquare(y_dist));
                dist = SkMaxScalar(kScalar2 - dist, 0);
                weight = SkMinScalar(SkScalarSquare(dist), SK_Scalar1);
            } else {
                SkScalar sqDist = SkMinScalar(SkScalarSquare(x_dist),
                                              SkScalarSquare(y_dist));
                weight = SkMinScalar(sqDist, SK_Scalar1);
            }

            SkScalar x_interp = SkScalarMul(weight, (fSrcRect.x() + x * inv_x_zoom)) +
                           (SK_Scalar1 - weight) * x;
            SkScalar y_interp = SkScalarMul(weight, (fSrcRect.y() + y * inv_y_zoom)) +
                           (SK_Scalar1 - weight) * y;

            int x_val = SkPin32(SkScalarFloorToInt(x_interp), 0, width - 1);
            int y_val = SkPin32(SkScalarFloorToInt(y_interp), 0, height - 1);

            *dptr = sptr[y_val * width + x_val];
            dptr++;
        }
    }
    return true;
}
