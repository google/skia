/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixConvolutionImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"
#include "SkUnPreMultiply.h"

#if SK_SUPPORT_GPU
#include "gl/GrGLProgramStage.h"
#endif

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(const SkISize& kernelSize, const SkScalar* kernel, SkScalar gain, SkScalar bias, const SkIPoint& target, TileMode tileMode, bool convolveAlpha, SkImageFilter* input)
  : INHERITED(input),
    fKernelSize(kernelSize),
    fGain(gain),
    fBias(bias),
    fTarget(target),
    fTileMode(tileMode),
    fConvolveAlpha(convolveAlpha) {
    uint32_t size = fKernelSize.fWidth * fKernelSize.fHeight;
    fKernel = SkNEW_ARRAY(SkScalar, size);
    memcpy(fKernel, kernel, size * sizeof(SkScalar));
    SkASSERT(kernelSize.fWidth >= 1 && kernelSize.fHeight >= 1);
    SkASSERT(target.fX >= 0 && target.fX < kernelSize.fWidth);
    SkASSERT(target.fY >= 0 && target.fY < kernelSize.fHeight);
}

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fKernelSize.fWidth = buffer.readInt();
    fKernelSize.fHeight = buffer.readInt();
    uint32_t size = fKernelSize.fWidth * fKernelSize.fHeight;
    fKernel = SkNEW_ARRAY(SkScalar, size);
    uint32_t readSize = buffer.readScalarArray(fKernel);
    SkASSERT(readSize == size);
    fGain = buffer.readScalar();
    fBias = buffer.readScalar();
    fTarget.fX = buffer.readScalar();
    fTarget.fY = buffer.readScalar();
    fTileMode = (TileMode) buffer.readInt();
    fConvolveAlpha = buffer.readBool();
}

void SkMatrixConvolutionImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fKernelSize.fWidth);
    buffer.writeInt(fKernelSize.fHeight);
    buffer.writeScalarArray(fKernel, fKernelSize.fWidth * fKernelSize.fHeight);
    buffer.writeScalar(fGain);
    buffer.writeScalar(fBias);
    buffer.writeScalar(fTarget.fX);
    buffer.writeScalar(fTarget.fY);
    buffer.writeInt((int) fTileMode);
    buffer.writeBool(fConvolveAlpha);
}

SkMatrixConvolutionImageFilter::~SkMatrixConvolutionImageFilter() {
    delete[] fKernel;
}

class UncheckedPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        return *src.getAddr32(x, y);
    }
};

class ClampPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        x = SkClampMax(x, src.width() - 1);
        y = SkClampMax(y, src.height() - 1);
        return *src.getAddr32(x, y);
    }
};

class RepeatPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        x %= src.width();
        y %= src.height();
        if (x < 0) {
            x += src.width();
        }
        if (y < 0) {
            y += src.height();
        }
        return *src.getAddr32(x, y);
    }
};

class ClampToBlackPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        if (x < 0 || x >= src.width() || y < 0 || y >= src.height()) {
            return 0;
        } else {
            return *src.getAddr32(x, y);
        }
    }
};

template<class PixelFetcher, bool convolveAlpha>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    for (int y = rect.fTop; y < rect.fBottom; ++y) {
        SkPMColor* dptr = result->getAddr32(rect.fLeft, y);
        for (int x = rect.fLeft; x < rect.fRight; ++x) {
            SkScalar sumA = 0, sumR = 0, sumG = 0, sumB = 0;
            for (int cy = 0; cy < fKernelSize.fHeight; cy++) {
                for (int cx = 0; cx < fKernelSize.fWidth; cx++) {
                    SkPMColor s = PixelFetcher::fetch(src, x + cx - fTarget.fX, y + cy - fTarget.fY);
                    SkScalar k = fKernel[cy * fKernelSize.fWidth + cx];
                    if (convolveAlpha) {
                        sumA += SkScalarMul(SkIntToScalar(SkGetPackedA32(s)), k);
                    }
                    sumR += SkScalarMul(SkIntToScalar(SkGetPackedR32(s)), k);
                    sumG += SkScalarMul(SkIntToScalar(SkGetPackedG32(s)), k);
                    sumB += SkScalarMul(SkIntToScalar(SkGetPackedB32(s)), k);
                }
            }
            int a = convolveAlpha
                  ? SkClampMax(SkScalarFloorToInt(SkScalarMul(sumA, fGain) + fBias), 255)
                  : 255;
            int r = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumR, fGain) + fBias), a);
            int g = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumG, fGain) + fBias), a);
            int b = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumB, fGain) + fBias), a);
            if (!convolveAlpha) {
                a = SkGetPackedA32(PixelFetcher::fetch(src, x, y));
                *dptr++ = SkPreMultiplyARGB(a, r, g, b);
            } else {
                *dptr++ = SkPackARGB32(a, r, g, b);
            }
        }
    }
}

template<class PixelFetcher>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    if (fConvolveAlpha) {
        filterPixels<PixelFetcher, true>(src, result, rect);
    } else {
        filterPixels<PixelFetcher, false>(src, result, rect);
    }
}

void SkMatrixConvolutionImageFilter::filterInteriorPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    filterPixels<UncheckedPixelFetcher>(src, result, rect);
}

void SkMatrixConvolutionImageFilter::filterBorderPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    switch (fTileMode) {
        case kClamp_TileMode:
            filterPixels<ClampPixelFetcher>(src, result, rect);
            break;
        case kRepeat_TileMode:
            filterPixels<RepeatPixelFetcher>(src, result, rect);
            break;
        case kClampToBlack_TileMode:
            filterPixels<ClampToBlackPixelFetcher>(src, result, rect);
            break;
    }
}

// FIXME:  This should be refactored to SkSingleInputImageFilter for
// use by other filters.  For now, we assume the input is always
// premultiplied and unpremultiply it
static SkBitmap unpremultiplyBitmap(const SkBitmap& src)
{
    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return SkBitmap();
    }
    SkBitmap result;
    result.setConfig(src.config(), src.width(), src.height());
    result.allocPixels();
    if (!result.getPixels()) {
        return SkBitmap();
    }
    for (int y = 0; y < src.height(); ++y) {
        const uint32_t* srcRow = src.getAddr32(0, y);
        uint32_t* dstRow = result.getAddr32(0, y);
        for (int x = 0; x < src.width(); ++x) {
            dstRow[x] = SkUnPreMultiply::PMColorToColor(srcRow[x]);
        }
    }
    return result;
}

bool SkMatrixConvolutionImageFilter::onFilterImage(Proxy* proxy,
                                                   const SkBitmap& source,
                                                   const SkMatrix& matrix,
                                                   SkBitmap* result,
                                                   SkIPoint* loc) {
    SkBitmap src = this->getInputResult(proxy, source, matrix, loc);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    if (!fConvolveAlpha && !src.isOpaque()) {
        src = unpremultiplyBitmap(src);
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    result->setConfig(src.config(), src.width(), src.height());
    result->allocPixels();

    SkIRect interior = SkIRect::MakeXYWH(fTarget.fX, fTarget.fY,
                                         src.width() - fKernelSize.fWidth + 1,
                                         src.height() - fKernelSize.fHeight + 1);
    SkIRect top = SkIRect::MakeWH(src.width(), fTarget.fY);
    SkIRect bottom = SkIRect::MakeLTRB(0, interior.bottom(),
                                       src.width(), src.height());
    SkIRect left = SkIRect::MakeXYWH(0, interior.top(),
                                     fTarget.fX, interior.height());
    SkIRect right = SkIRect::MakeLTRB(interior.right(), interior.top(),
                                      src.width(), interior.bottom());
    filterBorderPixels(src, result, top);
    filterBorderPixels(src, result, left);
    filterInteriorPixels(src, result, interior);
    filterBorderPixels(src, result, right);
    filterBorderPixels(src, result, bottom);
    return true;
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

class GrGLMatrixConvolutionEffect;

class GrMatrixConvolutionEffect : public GrSingleTextureEffect {
public:
    typedef SkMatrixConvolutionImageFilter::TileMode TileMode;
    GrMatrixConvolutionEffect(GrTexture*,
                              const SkISize& kernelSize,
                              const SkScalar* kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& target,
                              TileMode tileMode,
                              bool convolveAlpha);
    virtual ~GrMatrixConvolutionEffect();

    static const char* Name() { return "MatrixConvolution"; }
    const SkISize& kernelSize() const { return fKernelSize; }
    const float* target() const { return fTarget; }
    const float* kernel() const { return fKernel; }
    float gain() const { return fGain; }
    float bias() const { return fBias; }
    TileMode tileMode() const { return fTileMode; }
    bool convolveAlpha() const { return fConvolveAlpha; }

    typedef GrGLMatrixConvolutionEffect GLProgramStage;

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

private:
    SkISize  fKernelSize;
    float   *fKernel;
    float    fGain;
    float    fBias;
    float    fTarget[2];
    TileMode fTileMode;
    bool     fConvolveAlpha;

    GR_DECLARE_CUSTOM_STAGE_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

class GrGLMatrixConvolutionEffect : public GrGLProgramStage {
public:
    GrGLMatrixConvolutionEffect(const GrProgramStageFactory& factory,
                                const GrCustomStage& stage);
    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE {}
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
    typedef GrGLUniformManager::UniformHandle        UniformHandle;
    typedef SkMatrixConvolutionImageFilter::TileMode TileMode;
    SkISize        fKernelSize;
    TileMode       fTileMode;
    bool           fConvolveAlpha;

    UniformHandle  fKernelUni;
    UniformHandle  fImageIncrementUni;
    UniformHandle  fTargetUni;
    UniformHandle  fGainUni;
    UniformHandle  fBiasUni;
};

GrGLMatrixConvolutionEffect::GrGLMatrixConvolutionEffect(const GrProgramStageFactory& factory,
                                           const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fKernelUni(GrGLUniformManager::kInvalidUniformHandle)
    , fImageIncrementUni(GrGLUniformManager::kInvalidUniformHandle)
    , fTargetUni(GrGLUniformManager::kInvalidUniformHandle)
    , fGainUni(GrGLUniformManager::kInvalidUniformHandle)
    , fBiasUni(GrGLUniformManager::kInvalidUniformHandle) {
    const GrMatrixConvolutionEffect& m = static_cast<const GrMatrixConvolutionEffect&>(stage);
    fKernelSize = m.kernelSize();
    fTileMode = m.tileMode();
    fConvolveAlpha = m.convolveAlpha();
}

void GrGLMatrixConvolutionEffect::setupVariables(GrGLShaderBuilder* builder) {
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "ImageIncrement");
    fKernelUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_ShaderType,
                                             kFloat_GrSLType, "Kernel", fKernelSize.width() * fKernelSize.height());
    fTargetUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "Target");
    fGainUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                   kFloat_GrSLType, "Gain");
    fBiasUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                   kFloat_GrSLType, "Bias");
}

static void appendTextureLookup(GrGLShaderBuilder* builder,
                                const GrGLShaderBuilder::TextureSampler& sampler,
                                const char* coord,
                                SkMatrixConvolutionImageFilter::TileMode tileMode) {
    SkString* code = &builder->fFSCode;
    SkString clampedCoord;
    switch (tileMode) {
        case SkMatrixConvolutionImageFilter::kClamp_TileMode:
            clampedCoord.printf("clamp(%s, 0.0, 1.0)", coord);
            coord = clampedCoord.c_str();
            break;
        case SkMatrixConvolutionImageFilter::kRepeat_TileMode:
            clampedCoord.printf("fract(%s)", coord);
            coord = clampedCoord.c_str();
            break;
        case SkMatrixConvolutionImageFilter::kClampToBlack_TileMode:
            code->appendf("clamp(%s, 0.0, 1.0) != %s ? vec4(0, 0, 0, 0) : ", coord, coord);
            break;
    }
    builder->appendTextureLookup(code, sampler, coord);
}

void GrGLMatrixConvolutionEffect::emitFS(GrGLShaderBuilder* builder,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const TextureSamplerArray& samplers) {
    SkString* code = &builder->fFSCode;

    const char* target = builder->getUniformCStr(fTargetUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    const char* kernel = builder->getUniformCStr(fKernelUni);
    const char* gain = builder->getUniformCStr(fGainUni);
    const char* bias = builder->getUniformCStr(fBiasUni);
    int kWidth = fKernelSize.width();
    int kHeight = fKernelSize.height();

    code->appendf("\t\tvec4 sum = vec4(0, 0, 0, 0);\n");
    code->appendf("\t\tvec2 coord = %s - %s * %s;\n",
                  builder->defaultTexCoordsName(), target, imgInc);
    code->appendf("\t\tfor (int y = 0; y < %d; y++) {\n", kHeight);
    code->appendf("\t\t\tfor (int x = 0; x < %d; x++) {\n", kWidth);
    code->appendf("\t\t\t\tfloat k = %s[y * %d + x];\n", kernel, kWidth);
    code->appendf("\t\t\t\tvec2 coord2 = coord + vec2(x, y) * %s;\n", imgInc);
    code->appendf("\t\t\t\tvec4 c = ");
    appendTextureLookup(builder, samplers[0], "coord2", fTileMode);
    code->appendf(";\n");
    if (!fConvolveAlpha) {
        code->appendf("\t\t\t\tc.rgb /= c.a;\n");
    }
    code->appendf("\t\t\t\tsum += c * k;\n");
    code->appendf("\t\t\t}\n");
    code->appendf("\t\t}\n");
    if (fConvolveAlpha) {
        code->appendf("\t\t%s = sum * %s + %s;\n", outputColor, gain, bias);
        code->appendf("\t\t%s.rgb = clamp(%s.rgb, 0.0, %s.a);\n", outputColor, outputColor, outputColor);
    } else {
        code->appendf("\t\tvec4 c = ");
        appendTextureLookup(builder, samplers[0], builder->defaultTexCoordsName(), fTileMode);
        code->appendf(";\n");
        code->appendf("\t\t%s.a = c.a;\n", outputColor);
        code->appendf("\t\t%s.rgb = sum.rgb * %s + %s;\n", outputColor, gain, bias);
        code->appendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
    }
}

namespace {

int encodeXY(int x, int y) {
    SkASSERT(x >= 1 && y >= 1 && x * y <= 32);
    if (y < x)
        return 0x40 | encodeXY(y, x);
    else
        return (0x40 >> x) | (y - x);
}

};

GrGLProgramStage::StageKey GrGLMatrixConvolutionEffect::GenKey(const GrCustomStage& s,
                                                        const GrGLCaps& caps) {
    const GrMatrixConvolutionEffect& m = static_cast<const GrMatrixConvolutionEffect&>(s);
    StageKey key = encodeXY(m.kernelSize().width(), m.kernelSize().height());
    key |= m.tileMode() << 7;
    key |= m.convolveAlpha() ? 1 << 9 : 0;
    return key;
}

void GrGLMatrixConvolutionEffect::setData(const GrGLUniformManager& uman,
                                   const GrCustomStage& data,
                                   const GrRenderTarget*,
                                   int stageNum) {
    const GrMatrixConvolutionEffect& effect =
        static_cast<const GrMatrixConvolutionEffect&>(data);
    GrGLTexture& texture =
        *static_cast<GrGLTexture*>(data.texture(0));
    // the code we generated was for a specific kernel size
    GrAssert(effect.kernelSize() == fKernelSize);
    GrAssert(effect.tileMode() == fTileMode);
    float imageIncrement[2];
    imageIncrement[0] = 1.0f / texture.width();
    imageIncrement[1] = 1.0f / texture.height();
    uman.set2fv(fImageIncrementUni, 0, 1, imageIncrement);
    uman.set2fv(fTargetUni, 0, 1, effect.target());
    uman.set1fv(fKernelUni, 0, fKernelSize.width() * fKernelSize.height(), effect.kernel());
    uman.set1f(fGainUni, effect.gain());
    uman.set1f(fBiasUni, effect.bias());
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(GrTexture* texture,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& target,
                                                     TileMode tileMode,
                                                     bool convolveAlpha)
  : INHERITED(texture),
    fKernelSize(kernelSize),
    fGain(SkScalarToFloat(gain)),
    fBias(SkScalarToFloat(bias) / 255.0f),
    fTileMode(tileMode),
    fConvolveAlpha(convolveAlpha) {
    fKernel = new float[kernelSize.width() * kernelSize.height()];
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fTarget[0] = target.x();
    fTarget[1] = target.y();
}

GrMatrixConvolutionEffect::~GrMatrixConvolutionEffect() {
    delete[] fKernel;
}

const GrProgramStageFactory& GrMatrixConvolutionEffect::getFactory() const {
    return GrTProgramStageFactory<GrMatrixConvolutionEffect>::getInstance();
}

bool GrMatrixConvolutionEffect::isEqual(const GrCustomStage& sBase) const {
    const GrMatrixConvolutionEffect& s =
        static_cast<const GrMatrixConvolutionEffect&>(sBase);
    return INHERITED::isEqual(sBase) &&
           fKernelSize == s.kernelSize() &&
           !memcmp(fKernel, s.kernel(), fKernelSize.width() * fKernelSize.height() * sizeof(float)) &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fTarget == s.target() &&
           fTileMode == s.tileMode() &&
           fConvolveAlpha == s.convolveAlpha();
}

GR_DEFINE_CUSTOM_STAGE_TEST(GrMatrixConvolutionEffect);

// A little bit less than the minimum # uniforms required by DX9SM2 (32).
// Allows for a 5x5 kernel (or 25x1, for that matter).
#define MAX_KERNEL_SIZE 25

GrCustomStage* GrMatrixConvolutionEffect::TestCreate(SkRandom* random,
                                                     GrContext* context,
                                                     GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrCustomStageUnitTest::kSkiaPMTextureIdx :
                                      GrCustomStageUnitTest::kAlphaTextureIdx;
    int width = random->nextRangeU(1, MAX_KERNEL_SIZE);
    int height = random->nextRangeU(1, MAX_KERNEL_SIZE / width);
    SkISize kernelSize = SkISize::Make(width, height);
    SkScalar* kernel = new SkScalar[width * height];
    for (int i = 0; i < width * height; i++) {
        kernel[i] = random->nextSScalar1();
    }
    SkScalar gain = random->nextSScalar1();
    SkScalar bias = random->nextSScalar1();
    SkIPoint target = SkIPoint::Make(random->nextRangeU(0, kernelSize.width()),
                                     random->nextRangeU(0, kernelSize.height()));
    TileMode tileMode = static_cast<TileMode>(random->nextRangeU(0, 2));
    bool convolveAlpha = random->nextBool();
    return SkNEW_ARGS(GrMatrixConvolutionEffect, (textures[texIdx],
                                                  kernelSize,
                                                  kernel,
                                                  gain,
                                                  bias,
                                                  target,
                                                  tileMode,
                                                  convolveAlpha));

}

bool SkMatrixConvolutionImageFilter::asNewCustomStage(GrCustomStage** stage,
                                                      GrTexture* texture) const {
    bool ok = fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE;
    if (ok && stage) {
        *stage = SkNEW_ARGS(GrMatrixConvolutionEffect, (texture,
                                                        fKernelSize,
                                                        fKernel,
                                                        fGain,
                                                        fBias,
                                                        fTarget,
                                                        fTileMode,
                                                        fConvolveAlpha));
    }
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

#endif
