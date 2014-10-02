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
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendProcessorFactory.h"

class GrGLMagnifierEffect;

class GrMagnifierEffect : public GrSingleTextureEffect {

public:
    static GrFragmentProcessor* Create(GrTexture* texture,
                                       float xOffset,
                                       float yOffset,
                                       float xInvZoom,
                                       float yInvZoom,
                                       float xInvInset,
                                       float yInvInset) {
        return SkNEW_ARGS(GrMagnifierEffect, (texture,
                                              xOffset,
                                              yOffset,
                                              xInvZoom,
                                              yInvZoom,
                                              xInvInset,
                                              yInvInset));
    }

    virtual ~GrMagnifierEffect() {};

    static const char* Name() { return "Magnifier"; }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    float x_offset() const { return fXOffset; }
    float y_offset() const { return fYOffset; }
    float x_inv_zoom() const { return fXInvZoom; }
    float y_inv_zoom() const { return fYInvZoom; }
    float x_inv_inset() const { return fXInvInset; }
    float y_inv_inset() const { return fYInvInset; }

    typedef GrGLMagnifierEffect GLProcessor;

private:
    GrMagnifierEffect(GrTexture* texture,
                      float xOffset,
                      float yOffset,
                      float xInvZoom,
                      float yInvZoom,
                      float xInvInset,
                      float yInvInset)
        : GrSingleTextureEffect(texture, GrCoordTransform::MakeDivByTextureWHMatrix(texture))
        , fXOffset(xOffset)
        , fYOffset(yOffset)
        , fXInvZoom(xInvZoom)
        , fYInvZoom(yInvZoom)
        , fXInvInset(xInvInset)
        , fYInvInset(yInvInset) {}

    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    float fXOffset;
    float fYOffset;
    float fXInvZoom;
    float fYInvZoom;
    float fXInvInset;
    float fYInvInset;

    typedef GrSingleTextureEffect INHERITED;
};

// For brevity
typedef GrGLProgramDataManager::UniformHandle UniformHandle;

class GrGLMagnifierEffect : public GrGLFragmentProcessor {
public:
    GrGLMagnifierEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder*,
                          const GrFragmentProcessor&,
                          const GrProcessorKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:
    UniformHandle       fOffsetVar;
    UniformHandle       fInvZoomVar;
    UniformHandle       fInvInsetVar;

    typedef GrGLFragmentProcessor INHERITED;
};

GrGLMagnifierEffect::GrGLMagnifierEffect(const GrBackendProcessorFactory& factory,
                                         const GrProcessor&)
    : INHERITED(factory) {
}

void GrGLMagnifierEffect::emitCode(GrGLProgramBuilder* builder,
                                   const GrFragmentProcessor&,
                                   const GrProcessorKey& key,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const TransformedCoordsArray& coords,
                                   const TextureSamplerArray& samplers) {
    fOffsetVar = builder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility |
        GrGLProgramBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "Offset");
    fInvZoomVar = builder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility |
        GrGLProgramBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "InvZoom");
    fInvInsetVar = builder->addUniform(
        GrGLProgramBuilder::kFragment_Visibility |
        GrGLProgramBuilder::kVertex_Visibility,
        kVec2f_GrSLType, "InvInset");

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);
    fsBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fsBuilder->codeAppendf("\t\tvec2 zoom_coord = %s + %s * %s;\n",
                           builder->getUniformCStr(fOffsetVar),
                           coords2D.c_str(),
                           builder->getUniformCStr(fInvZoomVar));

    fsBuilder->codeAppend("\t\tvec2 delta = min(coord, vec2(1.0, 1.0) - coord);\n");

    fsBuilder->codeAppendf("\t\tdelta = delta * %s;\n", builder->getUniformCStr(fInvInsetVar));

    fsBuilder->codeAppend("\t\tfloat weight = 0.0;\n");
    fsBuilder->codeAppend("\t\tif (delta.s < 2.0 && delta.t < 2.0) {\n");
    fsBuilder->codeAppend("\t\t\tdelta = vec2(2.0, 2.0) - delta;\n");
    fsBuilder->codeAppend("\t\t\tfloat dist = length(delta);\n");
    fsBuilder->codeAppend("\t\t\tdist = max(2.0 - dist, 0.0);\n");
    fsBuilder->codeAppend("\t\t\tweight = min(dist * dist, 1.0);\n");
    fsBuilder->codeAppend("\t\t} else {\n");
    fsBuilder->codeAppend("\t\t\tvec2 delta_squared = delta * delta;\n");
    fsBuilder->codeAppend("\t\t\tweight = min(min(delta_squared.x, delta_squared.y), 1.0);\n");
    fsBuilder->codeAppend("\t\t}\n");

    fsBuilder->codeAppend("\t\tvec2 mix_coord = mix(coord, zoom_coord, weight);\n");
    fsBuilder->codeAppend("\t\tvec4 output_color = ");
    fsBuilder->appendTextureLookup(samplers[0], "mix_coord");
    fsBuilder->codeAppend(";\n");

    fsBuilder->codeAppendf("\t\t%s = output_color;", outputColor);
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    fsBuilder->codeAppend(modulate.c_str());
}

void GrGLMagnifierEffect::setData(const GrGLProgramDataManager& pdman,
                                  const GrProcessor& effect) {
    const GrMagnifierEffect& zoom = effect.cast<GrMagnifierEffect>();
    pdman.set2f(fOffsetVar, zoom.x_offset(), zoom.y_offset());
    pdman.set2f(fInvZoomVar, zoom.x_inv_zoom(), zoom.y_inv_zoom());
    pdman.set2f(fInvInsetVar, zoom.x_inv_inset(), zoom.y_inv_inset());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMagnifierEffect);

GrFragmentProcessor* GrMagnifierEffect::TestCreate(SkRandom* random,
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

    GrFragmentProcessor* effect = GrMagnifierEffect::Create(
        texture,
        (float) width / texture->width(),
        (float) height / texture->height(),
        texture->width() / (float) x,
        texture->height() / (float) y,
        (float) inset / texture->width(),
        (float) inset / texture->height());
    SkASSERT(effect);
    return effect;
}

///////////////////////////////////////////////////////////////////////////////

const GrBackendFragmentProcessorFactory& GrMagnifierEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrMagnifierEffect>::getInstance();
}

bool GrMagnifierEffect::onIsEqual(const GrProcessor& sBase) const {
    const GrMagnifierEffect& s = sBase.cast<GrMagnifierEffect>();
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

SkImageFilter* SkMagnifierImageFilter::Create(const SkRect& srcRect, SkScalar inset,
                                              SkImageFilter* input) {
    
    if (!SkScalarIsFinite(inset) || !SkIsValidRect(srcRect)) {
        return NULL;
    }
    // Negative numbers in src rect are not supported
    if (srcRect.fLeft < 0 || srcRect.fTop < 0) {
        return NULL;
    }
    return SkNEW_ARGS(SkMagnifierImageFilter, (srcRect, inset, input));
}


#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
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
#endif

SkMagnifierImageFilter::SkMagnifierImageFilter(const SkRect& srcRect, SkScalar inset,
                                               SkImageFilter* input)
    : INHERITED(1, &input), fSrcRect(srcRect), fInset(inset) {
    SkASSERT(srcRect.x() >= 0 && srcRect.y() >= 0 && inset >= 0);
}

#if SK_SUPPORT_GPU
bool SkMagnifierImageFilter::asFragmentProcessor(GrFragmentProcessor** fp, GrTexture* texture,
                                                 const SkMatrix&, const SkIRect&) const {
    if (fp) {
        SkScalar yOffset = (texture->origin() == kTopLeft_GrSurfaceOrigin) ? fSrcRect.y() :
                           (texture->height() - (fSrcRect.y() + fSrcRect.height()));
        SkScalar invInset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;
        *fp = GrMagnifierEffect::Create(texture,
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

SkFlattenable* SkMagnifierImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src;
    buffer.readRect(&src);
    return Create(src, buffer.readScalar(), common.getInput(0));
}

void SkMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeScalar(fInset);
}

bool SkMagnifierImageFilter::onFilterImage(Proxy*, const SkBitmap& src,
                                           const Context&, SkBitmap* dst,
                                           SkIPoint* offset) const {
    SkASSERT(src.colorType() == kN32_SkColorType);
    SkASSERT(fSrcRect.width() < src.width());
    SkASSERT(fSrcRect.height() < src.height());

    if ((src.colorType() != kN32_SkColorType) ||
        (fSrcRect.width() >= src.width()) ||
        (fSrcRect.height() >= src.height())) {
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
