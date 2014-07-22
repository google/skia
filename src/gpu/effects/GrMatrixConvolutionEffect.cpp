/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrMatrixConvolutionEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"

class GrGLMatrixConvolutionEffect : public GrGLEffect {
public:
    GrGLMatrixConvolutionEffect(const GrBackendEffectFactory& factory,
                                const GrDrawEffect& effect);
    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLUniformManager::UniformHandle        UniformHandle;
    typedef GrMatrixConvolutionEffect::TileMode TileMode;
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
                                GrMatrixConvolutionEffect::TileMode tileMode) {
    SkString clampedCoord;
    switch (tileMode) {
        case GrMatrixConvolutionEffect::kClamp_TileMode:
            clampedCoord.printf("clamp(%s, %s.xy, %s.zw)", coord, bounds, bounds);
            coord = clampedCoord.c_str();
            break;
        case GrMatrixConvolutionEffect::kRepeat_TileMode:
            clampedCoord.printf("mod(%s - %s.xy, %s.zw - %s.xy) + %s.xy", coord, bounds, bounds, bounds, bounds);
            coord = clampedCoord.c_str();
            break;
        case GrMatrixConvolutionEffect::kClampToBlack_TileMode:
            builder->fsCodeAppendf("clamp(%s, %s.xy, %s.zw) != %s ? vec4(0, 0, 0, 0) : ", coord, bounds, bounds, coord);
            break;
    }
    builder->fsAppendTextureLookup(sampler, coord);
}

void GrGLMatrixConvolutionEffect::emitCode(GrGLShaderBuilder* builder,
                                           const GrDrawEffect&,
                                           const GrEffectKey& key,
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

void GrGLMatrixConvolutionEffect::GenKey(const GrDrawEffect& drawEffect,
                                         const GrGLCaps&, GrEffectKeyBuilder* b) {
    const GrMatrixConvolutionEffect& m = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    uint32_t key = encodeXY(m.kernelSize().width(), m.kernelSize().height());
    key |= m.tileMode() << 7;
    key |= m.convolveAlpha() ? 1 << 9 : 0;
    b->add32(key);
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

GrEffect* GrMatrixConvolutionEffect::TestCreate(SkRandom* random,
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
