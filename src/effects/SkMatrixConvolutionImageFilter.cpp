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
#include "gl/GrGLEffect.h"
#include "effects/GrSingleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"
#include "SkMatrix.h"

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
    SkDEBUGCODE(uint32_t readSize = )buffer.readScalarArray(fKernel);
    SkASSERT(readSize == size);
    fGain = buffer.readScalar();
    fBias = buffer.readScalar();
    fTarget.fX = buffer.readInt();
    fTarget.fY = buffer.readInt();
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
    buffer.writeInt(fTarget.fX);
    buffer.writeInt(fTarget.fY);
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

// FIXME:  This should be refactored to SkImageFilterUtils for
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
    SkBitmap src = source;
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, matrix, &src, loc)) {
        return false;
    }

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
    static GrEffectRef* Create(GrTexture* texture,
                               const SkISize& kernelSize,
                               const SkScalar* kernel,
                               SkScalar gain,
                               SkScalar bias,
                               const SkIPoint& target,
                               TileMode tileMode,
                               bool convolveAlpha) {
        AutoEffectUnref effect(SkNEW_ARGS(GrMatrixConvolutionEffect, (texture,
                                                                      kernelSize,
                                                                      kernel,
                                                                      gain,
                                                                      bias,
                                                                      target,
                                                                      tileMode,
                                                                      convolveAlpha)));
        return CreateEffectRef(effect);
    }
    virtual ~GrMatrixConvolutionEffect();

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // TODO: Try to do better?
        *validFlags = 0;
    }

    static const char* Name() { return "MatrixConvolution"; }
    const SkISize& kernelSize() const { return fKernelSize; }
    const float* target() const { return fTarget; }
    const float* kernel() const { return fKernel; }
    float gain() const { return fGain; }
    float bias() const { return fBias; }
    TileMode tileMode() const { return fTileMode; }
    bool convolveAlpha() const { return fConvolveAlpha; }

    typedef GrGLMatrixConvolutionEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrMatrixConvolutionEffect(GrTexture*,
                              const SkISize& kernelSize,
                              const SkScalar* kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& target,
                              TileMode tileMode,
                              bool convolveAlpha);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    SkISize  fKernelSize;
    float   *fKernel;
    float    fGain;
    float    fBias;
    float    fTarget[2];
    TileMode fTileMode;
    bool     fConvolveAlpha;

    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

class GrGLMatrixConvolutionEffect : public GrGLEffect {
public:
    GrGLMatrixConvolutionEffect(const GrBackendEffectFactory& factory,
                                const GrDrawEffect& effect);
    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLUniformManager::UniformHandle        UniformHandle;
    typedef SkMatrixConvolutionImageFilter::TileMode TileMode;
    SkISize             fKernelSize;
    TileMode            fTileMode;
    bool                fConvolveAlpha;

    UniformHandle       fKernelUni;
    UniformHandle       fImageIncrementUni;
    UniformHandle       fTargetUni;
    UniformHandle       fGainUni;
    UniformHandle       fBiasUni;

    typedef GrGLEffect INHERITED;
};

GrGLMatrixConvolutionEffect::GrGLMatrixConvolutionEffect(const GrBackendEffectFactory& factory,
                                                         const GrDrawEffect& drawEffect)
    : INHERITED(factory) {
    const GrMatrixConvolutionEffect& m = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    fKernelSize = m.kernelSize();
    fTileMode = m.tileMode();
    fConvolveAlpha = m.convolveAlpha();
}

static void appendTextureLookup(GrGLShaderBuilder* builder,
                                const GrGLShaderBuilder::TextureSampler& sampler,
                                const char* coord,
                                SkMatrixConvolutionImageFilter::TileMode tileMode) {
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
            builder->fsCodeAppendf("clamp(%s, 0.0, 1.0) != %s ? vec4(0, 0, 0, 0) : ", coord, coord);
            break;
    }
    builder->fsAppendTextureLookup(sampler, coord);
}

void GrGLMatrixConvolutionEffect::emitCode(GrGLShaderBuilder* builder,
                                           const GrDrawEffect&,
                                           EffectKey key,
                                           const char* outputColor,
                                           const char* inputColor,
                                           const TransformedCoordsArray& coords,
                                           const TextureSamplerArray& samplers) {
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "ImageIncrement");
    fKernelUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_Visibility,
                                             kFloat_GrSLType, "Kernel", fKernelSize.width() * fKernelSize.height());
    fTargetUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "Target");
    fGainUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Gain");
    fBiasUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Bias");

    const char* target = builder->getUniformCStr(fTargetUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    const char* kernel = builder->getUniformCStr(fKernelUni);
    const char* gain = builder->getUniformCStr(fGainUni);
    const char* bias = builder->getUniformCStr(fBiasUni);
    int kWidth = fKernelSize.width();
    int kHeight = fKernelSize.height();

    builder->fsCodeAppend("\t\tvec4 sum = vec4(0, 0, 0, 0);\n");
    builder->fsCodeAppendf("\t\tvec2 coord = %s - %s * %s;\n", coords2D.c_str(), target, imgInc);
    builder->fsCodeAppendf("\t\tfor (int y = 0; y < %d; y++) {\n", kHeight);
    builder->fsCodeAppendf("\t\t\tfor (int x = 0; x < %d; x++) {\n", kWidth);
    builder->fsCodeAppendf("\t\t\t\tfloat k = %s[y * %d + x];\n", kernel, kWidth);
    builder->fsCodeAppendf("\t\t\t\tvec2 coord2 = coord + vec2(x, y) * %s;\n", imgInc);
    builder->fsCodeAppend("\t\t\t\tvec4 c = ");
    appendTextureLookup(builder, samplers[0], "coord2", fTileMode);
    builder->fsCodeAppend(";\n");
    if (!fConvolveAlpha) {
        builder->fsCodeAppend("\t\t\t\tc.rgb /= c.a;\n");
    }
    builder->fsCodeAppend("\t\t\t\tsum += c * k;\n");
    builder->fsCodeAppend("\t\t\t}\n");
    builder->fsCodeAppend("\t\t}\n");
    if (fConvolveAlpha) {
        builder->fsCodeAppendf("\t\t%s = sum * %s + %s;\n", outputColor, gain, bias);
        builder->fsCodeAppendf("\t\t%s.rgb = clamp(%s.rgb, 0.0, %s.a);\n", outputColor, outputColor, outputColor);
    } else {
        builder->fsCodeAppend("\t\tvec4 c = ");
        appendTextureLookup(builder, samplers[0], coords2D.c_str(), fTileMode);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppendf("\t\t%s.a = c.a;\n", outputColor);
        builder->fsCodeAppendf("\t\t%s.rgb = sum.rgb * %s + %s;\n", outputColor, gain, bias);
        builder->fsCodeAppendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
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

GrGLEffect::EffectKey GrGLMatrixConvolutionEffect::GenKey(const GrDrawEffect& drawEffect,
                                                          const GrGLCaps&) {
    const GrMatrixConvolutionEffect& m = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    EffectKey key = encodeXY(m.kernelSize().width(), m.kernelSize().height());
    key |= m.tileMode() << 7;
    key |= m.convolveAlpha() ? 1 << 9 : 0;
    return key;
}

void GrGLMatrixConvolutionEffect::setData(const GrGLUniformManager& uman,
                                          const GrDrawEffect& drawEffect) {
    const GrMatrixConvolutionEffect& conv = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    GrTexture& texture = *conv.texture(0);
    // the code we generated was for a specific kernel size
    SkASSERT(conv.kernelSize() == fKernelSize);
    SkASSERT(conv.tileMode() == fTileMode);
    float imageIncrement[2];
    float ySign = texture.origin() == kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    imageIncrement[0] = 1.0f / texture.width();
    imageIncrement[1] = ySign / texture.height();
    uman.set2fv(fImageIncrementUni, 0, 1, imageIncrement);
    uman.set2fv(fTargetUni, 0, 1, conv.target());
    uman.set1fv(fKernelUni, 0, fKernelSize.width() * fKernelSize.height(), conv.kernel());
    uman.set1f(fGainUni, conv.gain());
    uman.set1f(fBiasUni, conv.bias());
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(GrTexture* texture,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& target,
                                                     TileMode tileMode,
                                                     bool convolveAlpha)
  : INHERITED(texture, MakeDivByTextureWHMatrix(texture)),
    fKernelSize(kernelSize),
    fGain(SkScalarToFloat(gain)),
    fBias(SkScalarToFloat(bias) / 255.0f),
    fTileMode(tileMode),
    fConvolveAlpha(convolveAlpha) {
    fKernel = new float[kernelSize.width() * kernelSize.height()];
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fTarget[0] = static_cast<float>(target.x());
    fTarget[1] = static_cast<float>(target.y());
}

GrMatrixConvolutionEffect::~GrMatrixConvolutionEffect() {
    delete[] fKernel;
}

const GrBackendEffectFactory& GrMatrixConvolutionEffect::getFactory() const {
    return GrTBackendEffectFactory<GrMatrixConvolutionEffect>::getInstance();
}

bool GrMatrixConvolutionEffect::onIsEqual(const GrEffect& sBase) const {
    const GrMatrixConvolutionEffect& s = CastEffect<GrMatrixConvolutionEffect>(sBase);
    return this->texture(0) == s.texture(0) &&
           fKernelSize == s.kernelSize() &&
           !memcmp(fKernel, s.kernel(), fKernelSize.width() * fKernelSize.height() * sizeof(float)) &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fTarget == s.target() &&
           fTileMode == s.tileMode() &&
           fConvolveAlpha == s.convolveAlpha();
}

GR_DEFINE_EFFECT_TEST(GrMatrixConvolutionEffect);

// A little bit less than the minimum # uniforms required by DX9SM2 (32).
// Allows for a 5x5 kernel (or 25x1, for that matter).
#define MAX_KERNEL_SIZE 25

GrEffectRef* GrMatrixConvolutionEffect::TestCreate(SkRandom* random,
                                                   GrContext* context,
                                                   const GrDrawTargetCaps&,
                                                   GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    int width = random->nextRangeU(1, MAX_KERNEL_SIZE);
    int height = random->nextRangeU(1, MAX_KERNEL_SIZE / width);
    SkISize kernelSize = SkISize::Make(width, height);
    SkAutoTDeleteArray<SkScalar> kernel(new SkScalar[width * height]);
    for (int i = 0; i < width * height; i++) {
        kernel.get()[i] = random->nextSScalar1();
    }
    SkScalar gain = random->nextSScalar1();
    SkScalar bias = random->nextSScalar1();
    SkIPoint target = SkIPoint::Make(random->nextRangeU(0, kernelSize.width()),
                                     random->nextRangeU(0, kernelSize.height()));
    TileMode tileMode = static_cast<TileMode>(random->nextRangeU(0, 2));
    bool convolveAlpha = random->nextBool();
    return GrMatrixConvolutionEffect::Create(textures[texIdx],
                                             kernelSize,
                                             kernel.get(),
                                             gain,
                                             bias,
                                             target,
                                             tileMode,
                                             convolveAlpha);
}

bool SkMatrixConvolutionImageFilter::asNewEffect(GrEffectRef** effect,
                                                 GrTexture* texture,
                                                 const SkMatrix&) const {
    if (!effect) {
        return fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE;
    }
    SkASSERT(fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE);
    *effect = GrMatrixConvolutionEffect::Create(texture,
                                                fKernelSize,
                                                fKernel,
                                                fGain,
                                                fBias,
                                                fTarget,
                                                fTileMode,
                                                fConvolveAlpha);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#endif
