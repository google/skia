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

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "effects/GrSingleTextureEffect.h"
#include "gl/GrGLProgramStage.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrProgramStageFactory.h"

class GrGLMagnifierEffect;

class GrMagnifierEffect : public GrSingleTextureEffect {

public:
    GrMagnifierEffect(GrTexture* texture,
                      float xOffset,
                      float yOffset,
                      float xZoom,
                      float yZoom,
                      float xInset,
                      float yInset)
        : GrSingleTextureEffect(texture)
        , fXOffset(xOffset)
        , fYOffset(yOffset)
        , fXZoom(xZoom)
        , fYZoom(yZoom)
        , fXInset(xInset)
        , fYInset(yInset) {}

    virtual ~GrMagnifierEffect() {};

    static const char* Name() { return "Magnifier"; }

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    float x_offset() const { return fXOffset; }
    float y_offset() const { return fYOffset; }
    float x_zoom() const { return fXZoom; }
    float y_zoom() const { return fYZoom; }
    float x_inset() const { return fXInset; }
    float y_inset() const { return fYInset; }

    typedef GrGLMagnifierEffect GLProgramStage;

private:
    GR_DECLARE_CUSTOM_STAGE_TEST;

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

class GrGLMagnifierEffect : public GrGLProgramStage {
public:
    GrGLMagnifierEffect(const GrProgramStageFactory& factory,
                        const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* state) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager& uman,
                         const GrCustomStage& data,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage&, const GrGLCaps&);

private:

    UniformHandle  fOffsetVar;
    UniformHandle  fZoomVar;
    UniformHandle  fInsetVar;

    typedef GrGLProgramStage INHERITED;
};

GrGLMagnifierEffect::GrGLMagnifierEffect(const GrProgramStageFactory& factory,
                                         const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fOffsetVar(GrGLUniformManager::kInvalidUniformHandle)
    , fZoomVar(GrGLUniformManager::kInvalidUniformHandle)
    , fInsetVar(GrGLUniformManager::kInvalidUniformHandle) {
}

void GrGLMagnifierEffect::setupVariables(GrGLShaderBuilder* state) {
    fOffsetVar = state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType |
        GrGLShaderBuilder::kVertex_ShaderType,
        kVec2f_GrSLType, "uOffset");
    fZoomVar = state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType |
        GrGLShaderBuilder::kVertex_ShaderType,
        kVec2f_GrSLType, "uZoom");
    fInsetVar = state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType |
        GrGLShaderBuilder::kVertex_ShaderType,
        kVec2f_GrSLType, "uInset");
}

void GrGLMagnifierEffect::emitVS(GrGLShaderBuilder* state,
                                 const char* vertexCoords) {
}

void GrGLMagnifierEffect::emitFS(GrGLShaderBuilder* state,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const TextureSamplerArray& samplers) {
    SkString* code = &state->fFSCode;

    code->appendf("\t\tvec2 coord = %s;\n", state->defaultTexCoordsName());
    code->appendf("\t\tvec2 zoom_coord = %s + %s / %s;\n",
                  state->getUniformCStr(fOffsetVar),
                  state->defaultTexCoordsName(),
                  state->getUniformCStr(fZoomVar));

    code->appendf("\t\tvec2 delta = min(coord, vec2(1.0, 1.0) - coord);\n");

    code->appendf(
        "\t\tdelta = delta / %s;\n", state->getUniformCStr(fInsetVar));

    code->appendf("\t\tfloat weight = 0.0;\n");
    code->appendf("\t\tif (delta.s < 2.0 && delta.t < 2.0) {\n");
    code->appendf("\t\t\tdelta = vec2(2.0, 2.0) - delta;\n");
    code->appendf("\t\t\tfloat dist = length(delta);\n");
    code->appendf("\t\t\tdist = max(2.0 - dist, 0.0);\n");
    code->appendf("\t\t\tweight = min(dist * dist, 1.0);\n");
    code->appendf("\t\t} else {\n");
    code->appendf("\t\t\tvec2 delta_squared = delta * delta;\n");
    code->appendf(
        "\t\t\tweight = min(min(delta_squared.s, delta_squared.y), 1.0);\n");
    code->appendf("\t\t}\n");

    code->appendf("\t\tvec2 mix_coord = mix(coord, zoom_coord, weight);\n");
    code->appendf("\t\tvec4 output_color = ");
    state->appendTextureLookup(code, samplers[0], "mix_coord");
    code->append(";\n");

    code->appendf("\t\t%s = output_color;", outputColor);
    GrGLSLMulVarBy4f(code, 2, outputColor, inputColor);
}

void GrGLMagnifierEffect::setData(const GrGLUniformManager& uman,
                                  const GrCustomStage& data,
                                  const GrRenderTarget*,
                                  int stageNum) {
    const GrMagnifierEffect& zoom =
        static_cast<const GrMagnifierEffect&>(data);

    uman.set2f(fOffsetVar, zoom.x_offset(), zoom.y_offset());
    uman.set2f(fZoomVar, zoom.x_zoom(), zoom.y_zoom());
    uman.set2f(fInsetVar, zoom.x_inset(), zoom.y_inset());
}

GrGLProgramStage::StageKey GrGLMagnifierEffect::GenKey(const GrCustomStage& s,
                                                       const GrGLCaps& caps) {
    return 0;
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrMagnifierEffect);

GrCustomStage* GrMagnifierEffect::TestCreate(SkRandom* random,
                                             GrContext* context,
                                             GrTexture** textures) {
    const int kMaxWidth = 200;
    const int kMaxHeight = 200;
    const int kMaxInset = 20;
    uint32_t width = random->nextULessThan(kMaxWidth);
    uint32_t height = random->nextULessThan(kMaxHeight);
    uint32_t x = random->nextULessThan(kMaxWidth - width);
    uint32_t y = random->nextULessThan(kMaxHeight - height);
    SkScalar inset = SkIntToScalar(random->nextULessThan(kMaxInset));

    SkAutoTUnref<SkImageFilter> filter(
            new SkMagnifierImageFilter(
                SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                 SkIntToScalar(width), SkIntToScalar(height)),
                inset));
    GrSamplerState sampler;
    GrCustomStage* stage;
    filter->asNewCustomStage(&stage, textures[0]);
    GrAssert(NULL != stage);
    return stage;
}

///////////////////////////////////////////////////////////////////////////////

const GrProgramStageFactory& GrMagnifierEffect::getFactory() const {
    return GrTProgramStageFactory<GrMagnifierEffect>::getInstance();
}

bool GrMagnifierEffect::isEqual(const GrCustomStage& sBase) const {
     const GrMagnifierEffect& s =
        static_cast<const GrMagnifierEffect&>(sBase);
    return (this->fXOffset == s.fXOffset &&
            this->fYOffset == s.fYOffset &&
            this->fXZoom == s.fXZoom &&
            this->fYZoom == s.fYZoom &&
            this->fXInset == s.fXInset &&
            this->fYInset == s.fYInset);
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

SkMagnifierImageFilter::SkMagnifierImageFilter(SkRect srcRect, SkScalar inset)
    : fSrcRect(srcRect), fInset(inset) {
    SkASSERT(srcRect.x() >= 0 && srcRect.y() >= 0 && inset >= 0);
}

bool SkMagnifierImageFilter::asNewCustomStage(GrCustomStage** stage,
                                              GrTexture* texture) const {
#if SK_SUPPORT_GPU
    if (stage) {
      *stage =
          SkNEW_ARGS(GrMagnifierEffect, (texture,
                                         fSrcRect.x() / texture->width(),
                                         fSrcRect.y() / texture->height(),
                                         texture->width() / fSrcRect.width(),
                                         texture->height() / fSrcRect.height(),
                                         fInset / texture->width(),
                                         fInset / texture->height()));
    }
    return true;
#else
    return false;
#endif
}

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

SK_DEFINE_FLATTENABLE_REGISTRAR(SkMagnifierImageFilter)
