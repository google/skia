/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixConvolutionImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRect.h"
#include "SkUnPreMultiply.h"

#if SK_SUPPORT_GPU
#include "gl/GrGLEffect.h"
#include "effects/GrSingleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"
#include "SkMatrix.h"
#endif

static bool tile_mode_is_valid(SkMatrixConvolutionImageFilter::TileMode tileMode) {
    switch (tileMode) {
    case SkMatrixConvolutionImageFilter::kClamp_TileMode:
    case SkMatrixConvolutionImageFilter::kRepeat_TileMode:
    case SkMatrixConvolutionImageFilter::kClampToBlack_TileMode:
        return true;
    default:
        break;
    }
    return false;
}

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(
    const SkISize& kernelSize,
    const SkScalar* kernel,
    SkScalar gain,
    SkScalar bias,
    const SkIPoint& kernelOffset,
    TileMode tileMode,
    bool convolveAlpha,
    SkImageFilter* input,
    const CropRect* cropRect)
  : INHERITED(input, cropRect),
    fKernelSize(kernelSize),
    fGain(gain),
    fBias(bias),
    fKernelOffset(kernelOffset),
    fTileMode(tileMode),
    fConvolveAlpha(convolveAlpha) {
    uint32_t size = fKernelSize.fWidth * fKernelSize.fHeight;
    fKernel = SkNEW_ARRAY(SkScalar, size);
    memcpy(fKernel, kernel, size * sizeof(SkScalar));
    SkASSERT(kernelSize.fWidth >= 1 && kernelSize.fHeight >= 1);
    SkASSERT(kernelOffset.fX >= 0 && kernelOffset.fX < kernelSize.fWidth);
    SkASSERT(kernelOffset.fY >= 0 && kernelOffset.fY < kernelSize.fHeight);
}

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(SkReadBuffer& buffer)
    : INHERITED(1, buffer) {
    // We need to be able to read at most SK_MaxS32 bytes, so divide that
    // by the size of a scalar to know how many scalars we can read.
    static const int32_t kMaxSize = SK_MaxS32 / sizeof(SkScalar);
    fKernelSize.fWidth = buffer.readInt();
    fKernelSize.fHeight = buffer.readInt();
    if ((fKernelSize.fWidth >= 1) && (fKernelSize.fHeight >= 1) &&
        // Make sure size won't be larger than a signed int,
        // which would still be extremely large for a kernel,
        // but we don't impose a hard limit for kernel size
        (kMaxSize / fKernelSize.fWidth >= fKernelSize.fHeight)) {
        size_t size = fKernelSize.fWidth * fKernelSize.fHeight;
        fKernel = SkNEW_ARRAY(SkScalar, size);
        SkDEBUGCODE(bool success =) buffer.readScalarArray(fKernel, size);
        SkASSERT(success);
    } else {
        fKernel = 0;
    }
    fGain = buffer.readScalar();
    fBias = buffer.readScalar();
    fKernelOffset.fX = buffer.readInt();
    fKernelOffset.fY = buffer.readInt();
    fTileMode = (TileMode) buffer.readInt();
    fConvolveAlpha = buffer.readBool();
    buffer.validate((fKernel != 0) &&
                    SkScalarIsFinite(fGain) &&
                    SkScalarIsFinite(fBias) &&
                    tile_mode_is_valid(fTileMode));
}

void SkMatrixConvolutionImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fKernelSize.fWidth);
    buffer.writeInt(fKernelSize.fHeight);
    buffer.writeScalarArray(fKernel, fKernelSize.fWidth * fKernelSize.fHeight);
    buffer.writeScalar(fGain);
    buffer.writeScalar(fBias);
    buffer.writeInt(fKernelOffset.fX);
    buffer.writeInt(fKernelOffset.fY);
    buffer.writeInt((int) fTileMode);
    buffer.writeBool(fConvolveAlpha);
}

SkMatrixConvolutionImageFilter::~SkMatrixConvolutionImageFilter() {
    delete[] fKernel;
}

class UncheckedPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        return *src.getAddr32(x, y);
    }
};

class ClampPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        x = SkPin32(x, bounds.fLeft, bounds.fRight - 1);
        y = SkPin32(y, bounds.fTop, bounds.fBottom - 1);
        return *src.getAddr32(x, y);
    }
};

class RepeatPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        x = (x - bounds.left()) % bounds.width() + bounds.left();
        y = (y - bounds.top()) % bounds.height() + bounds.top();
        if (x < bounds.left()) {
            x += bounds.width();
        }
        if (y < bounds.top()) {
            y += bounds.height();
        }
        return *src.getAddr32(x, y);
    }
};

class ClampToBlackPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        if (x < bounds.fLeft || x >= bounds.fRight || y < bounds.fTop || y >= bounds.fBottom) {
            return 0;
        } else {
            return *src.getAddr32(x, y);
        }
    }
};

template<class PixelFetcher, bool convolveAlpha>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src,
                                                  SkBitmap* result,
                                                  const SkIRect& rect,
                                                  const SkIRect& bounds) const {
    for (int y = rect.fTop; y < rect.fBottom; ++y) {
        SkPMColor* dptr = result->getAddr32(rect.fLeft - bounds.fLeft, y - bounds.fTop);
        for (int x = rect.fLeft; x < rect.fRight; ++x) {
            SkScalar sumA = 0, sumR = 0, sumG = 0, sumB = 0;
            for (int cy = 0; cy < fKernelSize.fHeight; cy++) {
                for (int cx = 0; cx < fKernelSize.fWidth; cx++) {
                    SkPMColor s = PixelFetcher::fetch(src,
                                                      x + cx - fKernelOffset.fX,
                                                      y + cy - fKernelOffset.fY,
                                                      bounds);
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
                a = SkGetPackedA32(PixelFetcher::fetch(src, x, y, bounds));
                *dptr++ = SkPreMultiplyARGB(a, r, g, b);
            } else {
                *dptr++ = SkPackARGB32(a, r, g, b);
            }
        }
    }
}

template<class PixelFetcher>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src,
                                                  SkBitmap* result,
                                                  const SkIRect& rect,
                                                  const SkIRect& bounds) const {
    if (fConvolveAlpha) {
        filterPixels<PixelFetcher, true>(src, result, rect, bounds);
    } else {
        filterPixels<PixelFetcher, false>(src, result, rect, bounds);
    }
}

void SkMatrixConvolutionImageFilter::filterInteriorPixels(const SkBitmap& src,
                                                          SkBitmap* result,
                                                          const SkIRect& rect,
                                                          const SkIRect& bounds) const {
    filterPixels<UncheckedPixelFetcher>(src, result, rect, bounds);
}

void SkMatrixConvolutionImageFilter::filterBorderPixels(const SkBitmap& src,
                                                        SkBitmap* result,
                                                        const SkIRect& rect,
                                                        const SkIRect& bounds) const {
    switch (fTileMode) {
        case kClamp_TileMode:
            filterPixels<ClampPixelFetcher>(src, result, rect, bounds);
            break;
        case kRepeat_TileMode:
            filterPixels<RepeatPixelFetcher>(src, result, rect, bounds);
            break;
        case kClampToBlack_TileMode:
            filterPixels<ClampToBlackPixelFetcher>(src, result, rect, bounds);
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
                                                   const Context& ctx,
                                                   SkBitmap* result,
                                                   SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kPMColor_SkColorType) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, src, &srcOffset, &bounds, &src)) {
        return false;
    }

    if (!fConvolveAlpha && !src.isOpaque()) {
        src = unpremultiplyBitmap(src);
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    result->setConfig(src.config(), bounds.width(), bounds.height());
    result->allocPixels();
    if (!result->getPixels()) {
        return false;
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    bounds.offset(-srcOffset);
    SkIRect interior = SkIRect::MakeXYWH(bounds.left() + fKernelOffset.fX,
                                         bounds.top() + fKernelOffset.fY,
                                         bounds.width() - fKernelSize.fWidth + 1,
                                         bounds.height() - fKernelSize.fHeight + 1);
    SkIRect top = SkIRect::MakeLTRB(bounds.left(), bounds.top(), bounds.right(), interior.top());
    SkIRect bottom = SkIRect::MakeLTRB(bounds.left(), interior.bottom(),
                                       bounds.right(), bounds.bottom());
    SkIRect left = SkIRect::MakeLTRB(bounds.left(), interior.top(),
                                     interior.left(), interior.bottom());
    SkIRect right = SkIRect::MakeLTRB(interior.right(), interior.top(),
                                      bounds.right(), interior.bottom());
    filterBorderPixels(src, result, top, bounds);
    filterBorderPixels(src, result, left, bounds);
    filterInteriorPixels(src, result, interior, bounds);
    filterBorderPixels(src, result, right, bounds);
    filterBorderPixels(src, result, bottom, bounds);
    return true;
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

class GrGLMatrixConvolutionEffect;

class GrMatrixConvolutionEffect : public GrSingleTextureEffect {
public:
    typedef SkMatrixConvolutionImageFilter::TileMode TileMode;
    static GrEffectRef* Create(GrTexture* texture,
                               const SkIRect& bounds,
                               const SkISize& kernelSize,
                               const SkScalar* kernel,
                               SkScalar gain,
                               SkScalar bias,
                               const SkIPoint& kernelOffset,
                               TileMode tileMode,
                               bool convolveAlpha) {
        AutoEffectUnref effect(SkNEW_ARGS(GrMatrixConvolutionEffect, (texture,
                                                                      bounds,
                                                                      kernelSize,
                                                                      kernel,
                                                                      gain,
                                                                      bias,
                                                                      kernelOffset,
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
    const SkIRect& bounds() const { return fBounds; }
    const SkISize& kernelSize() const { return fKernelSize; }
    const float* kernelOffset() const { return fKernelOffset; }
    const float* kernel() const { return fKernel; }
    float gain() const { return fGain; }
    float bias() const { return fBias; }
    TileMode tileMode() const { return fTileMode; }
    bool convolveAlpha() const { return fConvolveAlpha; }

    typedef GrGLMatrixConvolutionEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrMatrixConvolutionEffect(GrTexture*,
                              const SkIRect& bounds,
                              const SkISize& kernelSize,
                              const SkScalar* kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& kernelOffset,
                              TileMode tileMode,
                              bool convolveAlpha);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    SkIRect  fBounds;
    SkISize  fKernelSize;
    float   *fKernel;
    float    fGain;
    float    fBias;
    float    fKernelOffset[2];
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

    UniformHandle       fBoundsUni;
    UniformHandle       fKernelUni;
    UniformHandle       fImageIncrementUni;
    UniformHandle       fKernelOffsetUni;
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
                                const char* bounds,
                                SkMatrixConvolutionImageFilter::TileMode tileMode) {
    SkString clampedCoord;
    switch (tileMode) {
        case SkMatrixConvolutionImageFilter::kClamp_TileMode:
            clampedCoord.printf("clamp(%s, %s.xy, %s.zw)", coord, bounds, bounds);
            coord = clampedCoord.c_str();
            break;
        case SkMatrixConvolutionImageFilter::kRepeat_TileMode:
            clampedCoord.printf("mod(%s - %s.xy, %s.zw - %s.xy) + %s.xy", coord, bounds, bounds, bounds, bounds);
            coord = clampedCoord.c_str();
            break;
        case SkMatrixConvolutionImageFilter::kClampToBlack_TileMode:
            builder->fsCodeAppendf("clamp(%s, %s.xy, %s.zw) != %s ? vec4(0, 0, 0, 0) : ", coord, bounds, bounds, coord);
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
    sk_ignore_unused_variable(inputColor);
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    fBoundsUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                     kVec4f_GrSLType, "Bounds");
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "ImageIncrement");
    fKernelUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_Visibility,
                                             kFloat_GrSLType,
                                             "Kernel",
                                             fKernelSize.width() * fKernelSize.height());
    fKernelOffsetUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "KernelOffset");
    fGainUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Gain");
    fBiasUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Bias");

    const char* bounds = builder->getUniformCStr(fBoundsUni);
    const char* kernelOffset = builder->getUniformCStr(fKernelOffsetUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    const char* kernel = builder->getUniformCStr(fKernelUni);
    const char* gain = builder->getUniformCStr(fGainUni);
    const char* bias = builder->getUniformCStr(fBiasUni);
    int kWidth = fKernelSize.width();
    int kHeight = fKernelSize.height();

    builder->fsCodeAppend("\t\tvec4 sum = vec4(0, 0, 0, 0);\n");
    builder->fsCodeAppendf("\t\tvec2 coord = %s - %s * %s;\n", coords2D.c_str(), kernelOffset, imgInc);
    builder->fsCodeAppendf("\t\tfor (int y = 0; y < %d; y++) {\n", kHeight);
    builder->fsCodeAppendf("\t\t\tfor (int x = 0; x < %d; x++) {\n", kWidth);
    builder->fsCodeAppendf("\t\t\t\tfloat k = %s[y * %d + x];\n", kernel, kWidth);
    builder->fsCodeAppendf("\t\t\t\tvec2 coord2 = coord + vec2(x, y) * %s;\n", imgInc);
    builder->fsCodeAppend("\t\t\t\tvec4 c = ");
    appendTextureLookup(builder, samplers[0], "coord2", bounds, fTileMode);
    builder->fsCodeAppend(";\n");
    if (!fConvolveAlpha) {
        builder->fsCodeAppend("\t\t\t\tc.rgb /= c.a;\n");
    }
    builder->fsCodeAppend("\t\t\t\tsum += c * k;\n");
    builder->fsCodeAppend("\t\t\t}\n");
    builder->fsCodeAppend("\t\t}\n");
    if (fConvolveAlpha) {
        builder->fsCodeAppendf("\t\t%s = sum * %s + %s;\n", outputColor, gain, bias);
        builder->fsCodeAppendf("\t\t%s.rgb = clamp(%s.rgb, 0.0, %s.a);\n",
            outputColor, outputColor, outputColor);
    } else {
        builder->fsCodeAppend("\t\tvec4 c = ");
        appendTextureLookup(builder, samplers[0], coords2D.c_str(), bounds, fTileMode);
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
    uman.set2fv(fImageIncrementUni, 1, imageIncrement);
    uman.set2fv(fKernelOffsetUni, 1, conv.kernelOffset());
    uman.set1fv(fKernelUni, fKernelSize.width() * fKernelSize.height(), conv.kernel());
    uman.set1f(fGainUni, conv.gain());
    uman.set1f(fBiasUni, conv.bias());
    const SkIRect& bounds = conv.bounds();
    float left = (float) bounds.left() / texture.width();
    float top = (float) bounds.top() / texture.height();
    float right = (float) bounds.right() / texture.width();
    float bottom = (float) bounds.bottom() / texture.height();
    if (texture.origin() == kBottomLeft_GrSurfaceOrigin) {
        uman.set4f(fBoundsUni, left, 1.0f - bottom, right, 1.0f - top);
    } else {
        uman.set4f(fBoundsUni, left, top, right, bottom);
    }
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(GrTexture* texture,
                                                     const SkIRect& bounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     TileMode tileMode,
                                                     bool convolveAlpha)
  : INHERITED(texture, MakeDivByTextureWHMatrix(texture)),
    fBounds(bounds),
    fKernelSize(kernelSize),
    fGain(SkScalarToFloat(gain)),
    fBias(SkScalarToFloat(bias) / 255.0f),
    fTileMode(tileMode),
    fConvolveAlpha(convolveAlpha) {
    fKernel = new float[kernelSize.width() * kernelSize.height()];
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fKernelOffset[0] = static_cast<float>(kernelOffset.x());
    fKernelOffset[1] = static_cast<float>(kernelOffset.y());
    this->setWillNotUseInputColor();
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
           !memcmp(fKernel, s.kernel(),
                   fKernelSize.width() * fKernelSize.height() * sizeof(float)) &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fKernelOffset == s.kernelOffset() &&
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
    SkIPoint kernelOffset = SkIPoint::Make(random->nextRangeU(0, kernelSize.width()),
                                           random->nextRangeU(0, kernelSize.height()));
    SkIRect bounds = SkIRect::MakeXYWH(random->nextRangeU(0, textures[texIdx]->width()),
                                       random->nextRangeU(0, textures[texIdx]->height()),
                                       random->nextRangeU(0, textures[texIdx]->width()),
                                       random->nextRangeU(0, textures[texIdx]->height()));
    TileMode tileMode = static_cast<TileMode>(random->nextRangeU(0, 2));
    bool convolveAlpha = random->nextBool();
    return GrMatrixConvolutionEffect::Create(textures[texIdx],
                                             bounds,
                                             kernelSize,
                                             kernel.get(),
                                             gain,
                                             bias,
                                             kernelOffset,
                                             tileMode,
                                             convolveAlpha);
}

bool SkMatrixConvolutionImageFilter::asNewEffect(GrEffectRef** effect,
                                                 GrTexture* texture,
                                                 const SkMatrix&,
                                                 const SkIRect& bounds
                                                 ) const {
    if (!effect) {
        return fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE;
    }
    SkASSERT(fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE);
    *effect = GrMatrixConvolutionEffect::Create(texture,
                                                bounds,
                                                fKernelSize,
                                                fKernel,
                                                fGain,
                                                fBias,
                                                fKernelOffset,
                                                fTileMode,
                                                fConvolveAlpha);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#endif
