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
#include "GrGpu.h"
#include "gl/GrGLProgramStage.h"
#include "effects/Gr1DKernelEffect.h"
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
    SkBitmap src = this->getInputResult(proxy, source, ctm, offset);
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
    SkBitmap src = this->getInputResult(proxy, source, ctm, offset);
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

    GrMorphologyEffect(GrTexture*, Direction, int radius, MorphologyType);
    virtual ~GrMorphologyEffect();

    MorphologyType type() const { return fType; }

    static const char* Name() { return "Morphology"; }

    typedef GrGLMorphologyEffect GLProgramStage;

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

protected:

    MorphologyType fType;

private:
    GR_DECLARE_CUSTOM_STAGE_TEST;

    typedef Gr1DKernelEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect  : public GrGLProgramStage {
public:
    GrGLMorphologyEffect (const GrProgramStageFactory& factory,
                          const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE {};
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage& s, const GrGLCaps& caps);

    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                 fRadius;
    GrMorphologyEffect::MorphologyType  fType;
    GrGLUniformManager::UniformHandle   fImageIncrementUni;

    typedef GrGLProgramStage INHERITED;
};

GrGLMorphologyEffect::GrGLMorphologyEffect(const GrProgramStageFactory& factory,
                                           const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fImageIncrementUni(GrGLUniformManager::kInvalidUniformHandle) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(stage);
    fRadius = m.radius();
    fType = m.type();
}

void GrGLMorphologyEffect::setupVariables(GrGLShaderBuilder* builder) {
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "ImageIncrement");
}

void GrGLMorphologyEffect::emitFS(GrGLShaderBuilder* builder,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const TextureSamplerArray& samplers) {
    SkString* code = &builder->fFSCode;

    const char* func;
    switch (fType) {
        case GrMorphologyEffect::kErode_MorphologyType:
            code->appendf("\t\t%s = vec4(1, 1, 1, 1);\n", outputColor);
            func = "min";
            break;
        case GrMorphologyEffect::kDilate_MorphologyType:
            code->appendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);
            func = "max";
            break;
        default:
            GrCrash("Unexpected type");
            func = ""; // suppress warning
            break;
    }
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    code->appendf("\t\tvec2 coord = %s - %d.0 * %s;\n",
                   builder->defaultTexCoordsName(), fRadius, imgInc);
    code->appendf("\t\tfor (int i = 0; i < %d; i++) {\n", this->width());
    code->appendf("\t\t\t%s = %s(%s, ", outputColor, func, outputColor);
    builder->appendTextureLookup(&builder->fFSCode, samplers[0], "coord");
    code->appendf(");\n");
    code->appendf("\t\t\tcoord += %s;\n", imgInc);
    code->appendf("\t\t}\n");
    GrGLSLMulVarBy4f(code, 2, outputColor, inputColor);
}

GrGLProgramStage::StageKey GrGLMorphologyEffect::GenKey(const GrCustomStage& s,
                                                        const GrGLCaps& caps) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(s);
    StageKey key = static_cast<StageKey>(m.radius());
    key |= (m.type() << 8);
    return key;
}

void GrGLMorphologyEffect::setData(const GrGLUniformManager& uman,
                                   const GrCustomStage& data,
                                   const GrRenderTarget*,
                                   int stageNum) {
    const Gr1DKernelEffect& kern =
        static_cast<const Gr1DKernelEffect&>(data);
    GrGLTexture& texture =
        *static_cast<GrGLTexture*>(data.texture(0));
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

const GrProgramStageFactory& GrMorphologyEffect::getFactory() const {
    return GrTProgramStageFactory<GrMorphologyEffect>::getInstance();
}

bool GrMorphologyEffect::isEqual(const GrCustomStage& sBase) const {
    const GrMorphologyEffect& s =
        static_cast<const GrMorphologyEffect&>(sBase);
    return (INHERITED::isEqual(sBase) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->type() == s.type());
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrMorphologyEffect);

GrCustomStage* GrMorphologyEffect::TestCreate(SkRandom* random,
                                              GrContext* context,
                                              GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrCustomStageUnitTest::kSkiaPMTextureIdx :
                                      GrCustomStageUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    static const int kMaxRadius = 10;
    int radius = random->nextRangeU(1, kMaxRadius);
    MorphologyType type = random->nextBool() ? GrMorphologyEffect::kErode_MorphologyType :
                                               GrMorphologyEffect::kDilate_MorphologyType;

    return SkNEW_ARGS(GrMorphologyEffect, (textures[texIdx], dir, radius, type));
}

namespace {

void apply_morphology_pass(GrContext* context,
                           GrTexture* texture,
                           const SkRect& rect,
                           int radius,
                           GrMorphologyEffect::MorphologyType morphType,
                           Gr1DKernelEffect::Direction direction) {
    GrMatrix sampleM;
    sampleM.setIDiv(texture->width(), texture->height());
    GrPaint paint;
    paint.reset();
    paint.textureSampler(0)->reset(sampleM);
    paint.textureSampler(0)->setCustomStage(SkNEW_ARGS(GrMorphologyEffect, (texture, direction, radius, morphType)))->unref();
    context->drawRect(paint, rect);
}

GrTexture* apply_morphology(GrTexture* srcTexture,
                            const GrRect& rect,
                            GrMorphologyEffect::MorphologyType morphType,
                            SkISize radius) {
    GrContext* context = srcTexture->getContext();
    srcTexture->ref();
    GrContext::AutoMatrix avm(context, GrMatrix::I());
    GrContext::AutoClip acs(context, GrRect::MakeWH(SkIntToScalar(srcTexture->width()),
                                                    SkIntToScalar(srcTexture->height())));
    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = SkScalarCeilToInt(rect.width());
    desc.fHeight = SkScalarCeilToInt(rect.height());
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    if (radius.fWidth > 0) {
        GrAutoScratchTexture ast(context, desc);
        GrContext::AutoRenderTarget art(context, ast.texture()->asRenderTarget());
        apply_morphology_pass(context, srcTexture, rect, radius.fWidth,
                              morphType, Gr1DKernelEffect::kX_Direction);
        SkIRect clearRect = SkIRect::MakeXYWH(
                    SkScalarFloorToInt(rect.fLeft),
                    SkScalarFloorToInt(rect.fBottom),
                    SkScalarFloorToInt(rect.width()),
                    radius.fHeight);
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

GrTexture* SkDilateImageFilter::onFilterImageGPU(GrTexture* src, const SkRect& rect) {
    SkAutoTUnref<GrTexture> input(this->getInputResultAsTexture(src, rect));
    return apply_morphology(src, rect, GrMorphologyEffect::kDilate_MorphologyType, radius());
}

GrTexture* SkErodeImageFilter::onFilterImageGPU(GrTexture* src, const SkRect& rect) {
    SkAutoTUnref<GrTexture> input(this->getInputResultAsTexture(src, rect));
    return apply_morphology(src, rect, GrMorphologyEffect::kErode_MorphologyType, radius());
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR(SkDilateImageFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR(SkErodeImageFilter)
