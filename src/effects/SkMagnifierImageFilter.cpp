/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkMagnifierImageFilter.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"

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
                               float xZoom,
                               float yZoom,
                               float xInset,
                               float yInset) {
        AutoEffectUnref effect(SkNEW_ARGS(GrMagnifierEffect, (texture,
                                                              xOffset,
                                                              yOffset,
                                                              xZoom,
                                                              yZoom,
                                                              xInset,
                                                              yInset)));
        return CreateEffectRef(effect);
    }

    virtual ~GrMagnifierEffect() {};

    static const char* Name() { return "Magnifier"; }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    float x_offset() const { return fXOffset; }
    float y_offset() const { return fYOffset; }
    float x_zoom() const { return fXZoom; }
    float y_zoom() const { return fYZoom; }
    float x_inset() const { return fXInset; }
    float y_inset() const { return fYInset; }

    typedef GrGLMagnifierEffect GLEffect;

private:
    GrMagnifierEffect(GrTexture* texture,
                      float xOffset,
                      float yOffset,
                      float xZoom,
                      float yZoom,
                      float xInset,
                      float yInset)
        : GrSingleTextureEffect(texture, MakeDivByTextureWHMatrix(texture))
        , fXOffset(xOffset)
        , fYOffset(yOffset)
        , fXZoom(xZoom)
        , fYZoom(yZoom)
        , fXInset(xInset)
        , fYInset(yInset) {}

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GR_DECLARE_EFFECT_TEST;

    float fXOffset;
    float fYOffset;
    float fXZoom;
    float fYZoom;
    float fXInset;
    float fYInset;

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
    UniformHandle       fZoomVar;
    UniformHandle       fInsetVar;

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
        kVec2f_GrSLType, "uOffset");
    fZoomVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility |
        GrGLShaderBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "uZoom");
    fInsetVar = builder->addUniform(
        GrGLShaderBuilder::kFragment_Visibility |
        GrGLShaderBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "uInset");

    builder->fsCodeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    builder->fsCodeAppendf("\t\tvec2 zoom_coord = %s + %s / %s;\n",
                           builder->getUniformCStr(fOffsetVar),
                           coords2D.c_str(),
                           builder->getUniformCStr(fZoomVar));

    builder->fsCodeAppend("\t\tvec2 delta = min(coord, vec2(1.0, 1.0) - coord);\n");

    builder->fsCodeAppendf("\t\tdelta = delta / %s;\n", builder->getUniformCStr(fInsetVar));

    builder->fsCodeAppend("\t\tfloat weight = 0.0;\n");
    builder->fsCodeAppend("\t\tif (delta.s < 2.0 && delta.t < 2.0) {\n");
    builder->fsCodeAppend("\t\t\tdelta = vec2(2.0, 2.0) - delta;\n");
    builder->fsCodeAppend("\t\t\tfloat dist = length(delta);\n");
    builder->fsCodeAppend("\t\t\tdist = max(2.0 - dist, 0.0);\n");
    builder->fsCodeAppend("\t\t\tweight = min(dist * dist, 1.0);\n");
    builder->fsCodeAppend("\t\t} else {\n");
    builder->fsCodeAppend("\t\t\tvec2 delta_squared = delta * delta;\n");
    builder->fsCodeAppend("\t\t\tweight = min(min(delta_squared.s, delta_squared.y), 1.0);\n");
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
    uman.set2f(fZoomVar, zoom.x_zoom(), zoom.y_zoom());
    uman.set2f(fInsetVar, zoom.x_inset(), zoom.y_inset());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrMagnifierEffect);

GrEffectRef* GrMagnifierEffect::TestCreate(SkRandom* random,
                                           GrContext* context,
                                           const GrDrawTargetCaps&,
                                           GrTexture** textures) {
    const int kMaxWidth = 200;
    const int kMaxHeight = 200;
    const int kMaxInset = 20;
    uint32_t width = random->nextULessThan(kMaxWidth);
    uint32_t height = random->nextULessThan(kMaxHeight);
    uint32_t x = random->nextULessThan(kMaxWidth - width);
    uint32_t y = random->nextULessThan(kMaxHeight - height);
    SkScalar inset = SkIntToScalar(random->nextULessThan(kMaxInset));

    SkAutoTUnref<SkMagnifierImageFilter> filter(
            new SkMagnifierImageFilter(
                SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                 SkIntToScalar(width), SkIntToScalar(height)),
                inset));
    GrEffectRef* effect;
    filter->asNewEffect(&effect, textures[0], SkMatrix::I());
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
            this->fXZoom == s.fXZoom &&
            this->fYZoom == s.fYZoom &&
            this->fXInset == s.fXInset &&
            this->fYInset == s.fYInset);
}

void GrMagnifierEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

#endif

////////////////////////////////////////////////////////////////////////////////
SkMagnifierImageFilter::SkMagnifierImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    float x = buffer.readScalar();
    float y = buffer.readScalar();
    float width = buffer.readScalar();
    float height = buffer.readScalar();
    fSrcRect = SkRect::MakeXYWH(x, y, width, height);
    fInset = buffer.readScalar();
}

// FIXME:  implement single-input semantics
SkMagnifierImageFilter::SkMagnifierImageFilter(SkRect srcRect, SkScalar inset)
    : INHERITED(0), fSrcRect(srcRect), fInset(inset) {
    SkASSERT(srcRect.x() >= 0 && srcRect.y() >= 0 && inset >= 0);
}

#if SK_SUPPORT_GPU
bool SkMagnifierImageFilter::asNewEffect(GrEffectRef** effect, GrTexture* texture, const SkMatrix&) const {
    if (effect) {
        *effect = GrMagnifierEffect::Create(texture,
                                            fSrcRect.x() / texture->width(),
                                            fSrcRect.y() / texture->height(),
                                            texture->width() / fSrcRect.width(),
                                            texture->height() / fSrcRect.height(),
                                            fInset / texture->width(),
                                            fInset / texture->height());
    }
    return true;
}
#endif

void SkMagnifierImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSrcRect.x());
    buffer.writeScalar(fSrcRect.y());
    buffer.writeScalar(fSrcRect.width());
    buffer.writeScalar(fSrcRect.height());
    buffer.writeScalar(fInset);
}

bool SkMagnifierImageFilter::onFilterImage(Proxy*, const SkBitmap& src,
                                           const SkMatrix&, SkBitmap* dst,
                                           SkIPoint* offset) {
    SkASSERT(src.config() == SkBitmap::kARGB_8888_Config);
    SkASSERT(fSrcRect.width() < src.width());
    SkASSERT(fSrcRect.height() < src.height());

    if (src.config() != SkBitmap::kARGB_8888_Config) {
      return false;
    }

    SkAutoLockPixels alp(src);
    SkASSERT(src.getPixels());
    if (!src.getPixels() || src.width() <= 0 || src.height() <= 0) {
      return false;
    }

    SkScalar inv_inset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;

    SkScalar inv_x_zoom = fSrcRect.width() / src.width();
    SkScalar inv_y_zoom = fSrcRect.height() / src.height();

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();
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

            int x_val = SkMin32(SkScalarFloorToInt(x_interp), width - 1);
            int y_val = SkMin32(SkScalarFloorToInt(y_interp), height - 1);

            *dptr = sptr[y_val * width + x_val];
            dptr++;
        }
    }
    return true;
}
