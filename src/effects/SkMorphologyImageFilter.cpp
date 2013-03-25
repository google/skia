/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphologyImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "effects/Gr1DKernelEffect.h"
#include "SkImageFilterUtils.h"
#endif

SkMorphologyImageFilter::SkMorphologyImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fRadius.fWidth = buffer.readInt();
    fRadius.fHeight = buffer.readInt();
}

SkMorphologyImageFilter::SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input)
    : INHERITED(input), fRadius(SkISize::Make(radiusX, radiusY)) {
}


void SkMorphologyImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fRadius.fWidth);
    buffer.writeInt(fRadius.fHeight);
}

static void erode(const SkPMColor* src, SkPMColor* dst,
                  int radius, int width, int height,
                  int srcStrideX, int srcStrideY,
                  int dstStrideX, int dstStrideY)
{
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            int minB = 255, minG = 255, minR = 255, minA = 255;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p);
                int g = SkGetPackedG32(*p);
                int r = SkGetPackedR32(*p);
                int a = SkGetPackedA32(*p);
                if (b < minB) minB = b;
                if (g < minG) minG = g;
                if (r < minR) minR = r;
                if (a < minA) minA = a;
            }
            *dptr = SkPackARGB32(minA, minR, minG, minB);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

static void erodeX(const SkBitmap& src, SkBitmap* dst, int radiusX)
{
    erode(src.getAddr32(0, 0), dst->getAddr32(0, 0),
          radiusX, src.width(), src.height(),
          1, src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels());
}

static void erodeY(const SkBitmap& src, SkBitmap* dst, int radiusY)
{
    erode(src.getAddr32(0, 0), dst->getAddr32(0, 0),
          radiusY, src.height(), src.width(),
          src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels(), 1);
}

static void dilate(const SkPMColor* src, SkPMColor* dst,
                   int radius, int width, int height,
                   int srcStrideX, int srcStrideY,
                   int dstStrideX, int dstStrideY)
{
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            int maxB = 0, maxG = 0, maxR = 0, maxA = 0;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p);
                int g = SkGetPackedG32(*p);
                int r = SkGetPackedR32(*p);
                int a = SkGetPackedA32(*p);
                if (b > maxB) maxB = b;
                if (g > maxG) maxG = g;
                if (r > maxR) maxR = r;
                if (a > maxA) maxA = a;
            }
            *dptr = SkPackARGB32(maxA, maxR, maxG, maxB);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

static void dilateX(const SkBitmap& src, SkBitmap* dst, int radiusX)
{
    dilate(src.getAddr32(0, 0), dst->getAddr32(0, 0),
           radiusX, src.width(), src.height(),
           1, src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels());
}

static void dilateY(const SkBitmap& src, SkBitmap* dst, int radiusY)
{
    dilate(src.getAddr32(0, 0), dst->getAddr32(0, 0),
           radiusY, src.height(), src.width(),
           src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels(), 1);
}

bool SkErodeImageFilter::onFilterImage(Proxy* proxy,
                                       const SkBitmap& source, const SkMatrix& ctm,
                                       SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = this->getInputResult(0, proxy, source, ctm, offset);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();

    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.copyTo(dst, dst->config());
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (width > 0 && height > 0) {
        erodeX(src, &temp, width);
        erodeY(temp, dst, height);
    } else if (width > 0) {
        erodeX(src, dst, width);
    } else if (height > 0) {
        erodeY(src, dst, height);
    }
    return true;
}

bool SkDilateImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source, const SkMatrix& ctm,
                                        SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = this->getInputResult(0, proxy, source, ctm, offset);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();

    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.copyTo(dst, dst->config());
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (width > 0 && height > 0) {
        dilateX(src, &temp, width);
        dilateY(temp, dst, height);
    } else if (width > 0) {
        dilateX(src, dst, width);
    } else if (height > 0) {
        dilateY(src, dst, height);
    }
    return true;
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect;

/**
 * Morphology effects. Depending upon the type of morphology, either the
 * component-wise min (Erode_Type) or max (Dilate_Type) of all pixels in the
 * kernel is selected as the new color. The new color is modulated by the input
 * color.
 */
class GrMorphologyEffect : public Gr1DKernelEffect {

public:

    enum MorphologyType {
        kErode_MorphologyType,
        kDilate_MorphologyType,
    };

    static GrEffectRef* Create(GrTexture* tex, Direction dir, int radius, MorphologyType type) {
        AutoEffectUnref effect(SkNEW_ARGS(GrMorphologyEffect, (tex, dir, radius, type)));
        return CreateEffectRef(effect);
    }

    virtual ~GrMorphologyEffect();

    MorphologyType type() const { return fType; }

    static const char* Name() { return "Morphology"; }

    typedef GrGLMorphologyEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

protected:

    MorphologyType fType;

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrMorphologyEffect(GrTexture*, Direction, int radius, MorphologyType);

    GR_DECLARE_EFFECT_TEST;

    typedef Gr1DKernelEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect : public GrGLEffect {
public:
    GrGLMorphologyEffect (const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                 fRadius;
    GrMorphologyEffect::MorphologyType  fType;
    GrGLUniformManager::UniformHandle   fImageIncrementUni;
    GrGLEffectMatrix                    fEffectMatrix;

    typedef GrGLEffect INHERITED;
};

GrGLMorphologyEffect::GrGLMorphologyEffect(const GrBackendEffectFactory& factory,
                                           const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fImageIncrementUni(GrGLUniformManager::kInvalidUniformHandle)
    , fEffectMatrix(drawEffect.castEffect<GrMorphologyEffect>().coordsType()) {
    const GrMorphologyEffect& m = drawEffect.castEffect<GrMorphologyEffect>();
    fRadius = m.radius();
    fType = m.type();
}

void GrGLMorphologyEffect::emitCode(GrGLShaderBuilder* builder,
                                    const GrDrawEffect&,
                                    EffectKey key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TextureSamplerArray& samplers) {
    const char* coords;
    fEffectMatrix.emitCodeMakeFSCoords2D(builder, key, &coords);
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "ImageIncrement");

    const char* func;
    switch (fType) {
        case GrMorphologyEffect::kErode_MorphologyType:
            builder->fsCodeAppendf("\t\t%s = vec4(1, 1, 1, 1);\n", outputColor);
            func = "min";
            break;
        case GrMorphologyEffect::kDilate_MorphologyType:
            builder->fsCodeAppendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);
            func = "max";
            break;
        default:
            GrCrash("Unexpected type");
            func = ""; // suppress warning
            break;
    }
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    builder->fsCodeAppendf("\t\tvec2 coord = %s - %d.0 * %s;\n", coords, fRadius, imgInc);
    builder->fsCodeAppendf("\t\tfor (int i = 0; i < %d; i++) {\n", this->width());
    builder->fsCodeAppendf("\t\t\t%s = %s(%s, ", outputColor, func, outputColor);
    builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType, samplers[0], "coord");
    builder->fsCodeAppend(");\n");
    builder->fsCodeAppendf("\t\t\tcoord += %s;\n", imgInc);
    builder->fsCodeAppend("\t\t}\n");
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    builder->fsCodeAppend(modulate.c_str());
}

GrGLEffect::EffectKey GrGLMorphologyEffect::GenKey(const GrDrawEffect& drawEffect,
                                                   const GrGLCaps&) {
    const GrMorphologyEffect& m = drawEffect.castEffect<GrMorphologyEffect>();
    EffectKey key = static_cast<EffectKey>(m.radius());
    key |= (m.type() << 8);
    key <<= GrGLEffectMatrix::kKeyBits;
    EffectKey matrixKey = GrGLEffectMatrix::GenKey(m.getMatrix(),
                                                   drawEffect,
                                                   m.coordsType(),
                                                   m.texture(0));
    return key | matrixKey;
}

void GrGLMorphologyEffect::setData(const GrGLUniformManager& uman,
                                   const GrDrawEffect& drawEffect) {
    const Gr1DKernelEffect& kern = drawEffect.castEffect<Gr1DKernelEffect>();
    GrTexture& texture = *kern.texture(0);
    // the code we generated was for a specific kernel radius
    GrAssert(kern.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    switch (kern.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            imageIncrement[1] = 1.0f / texture.height();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    uman.set2fv(fImageIncrementUni, 0, 1, imageIncrement);
    fEffectMatrix.setData(uman, kern.getMatrix(), drawEffect, kern.texture(0));
}

///////////////////////////////////////////////////////////////////////////////

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type)
    : Gr1DKernelEffect(texture, direction, radius)
    , fType(type) {
}

GrMorphologyEffect::~GrMorphologyEffect() {
}

const GrBackendEffectFactory& GrMorphologyEffect::getFactory() const {
    return GrTBackendEffectFactory<GrMorphologyEffect>::getInstance();
}

bool GrMorphologyEffect::onIsEqual(const GrEffect& sBase) const {
    const GrMorphologyEffect& s = CastEffect<GrMorphologyEffect>(sBase);
    return (this->texture(0) == s.texture(0) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->type() == s.type());
}

void GrMorphologyEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    // This is valid because the color components of the result of the kernel all come
    // exactly from existing values in the source texture.
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrMorphologyEffect);

GrEffectRef* GrMorphologyEffect::TestCreate(SkMWCRandom* random,
                                            GrContext*,
                                            const GrDrawTargetCaps&,
                                            GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    static const int kMaxRadius = 10;
    int radius = random->nextRangeU(1, kMaxRadius);
    MorphologyType type = random->nextBool() ? GrMorphologyEffect::kErode_MorphologyType :
                                               GrMorphologyEffect::kDilate_MorphologyType;

    return GrMorphologyEffect::Create(textures[texIdx], dir, radius, type);
}

namespace {

void apply_morphology_pass(GrContext* context,
                           GrTexture* texture,
                           const SkIRect& rect,
                           int radius,
                           GrMorphologyEffect::MorphologyType morphType,
                           Gr1DKernelEffect::Direction direction) {
    GrPaint paint;
    paint.colorStage(0)->setEffect(GrMorphologyEffect::Create(texture,
                                                              direction,
                                                              radius,
                                                              morphType))->unref();
    context->drawRect(paint, SkRect::MakeFromIRect(rect));
}

GrTexture* apply_morphology(GrTexture* srcTexture,
                            const SkIRect& rect,
                            GrMorphologyEffect::MorphologyType morphType,
                            SkISize radius) {
    GrContext* context = srcTexture->getContext();
    srcTexture->ref();

    GrContext::AutoMatrix am;
    am.setIdentity(context);

    GrContext::AutoClip acs(context, GrRect::MakeWH(SkIntToScalar(srcTexture->width()),
                                                    SkIntToScalar(srcTexture->height())));

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = rect.width();
    desc.fHeight = rect.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    if (radius.fWidth > 0) {
        GrAutoScratchTexture ast(context, desc);
        GrContext::AutoRenderTarget art(context, ast.texture()->asRenderTarget());
        apply_morphology_pass(context, srcTexture, rect, radius.fWidth,
                              morphType, Gr1DKernelEffect::kX_Direction);
        SkIRect clearRect = SkIRect::MakeXYWH(rect.fLeft, rect.fBottom,
                                              rect.width(), radius.fHeight);
        context->clear(&clearRect, 0x0);
        srcTexture->unref();
        srcTexture = ast.detach();
    }
    if (radius.fHeight > 0) {
        GrAutoScratchTexture ast(context, desc);
        GrContext::AutoRenderTarget art(context, ast.texture()->asRenderTarget());
        apply_morphology_pass(context, srcTexture, rect, radius.fHeight,
                              morphType, Gr1DKernelEffect::kY_Direction);
        srcTexture->unref();
        srcTexture = ast.detach();
    }
    return srcTexture;
}

};

bool SkDilateImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) {
    SkBitmap inputBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, &inputBM)) {
        return false;
    }
    GrTexture* input = (GrTexture*) inputBM.getTexture();
    SkIRect bounds;
    src.getBounds(&bounds);
    SkAutoTUnref<GrTexture> resultTex(apply_morphology(input, bounds,
        GrMorphologyEffect::kDilate_MorphologyType, radius()));
    return SkImageFilterUtils::WrapTexture(resultTex, src.width(), src.height(), result);
}

bool SkErodeImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) {
    SkBitmap inputBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, &inputBM)) {
        return false;
    }
    GrTexture* input = (GrTexture*) inputBM.getTexture();
    SkIRect bounds;
    src.getBounds(&bounds);
    SkAutoTUnref<GrTexture> resultTex(apply_morphology(input, bounds,
        GrMorphologyEffect::kErode_MorphologyType, radius()));
    return SkImageFilterUtils::WrapTexture(resultTex, src.width(), src.height(), result);
}

#endif
