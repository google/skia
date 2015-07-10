/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvolutionEffect.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLTexture.h"
#include "gl/builders/GrGLProgramBuilder.h"

// For brevity
typedef GrGLProgramDataManager::UniformHandle UniformHandle;

/**
 * Base class with shared functionality for GrGLBoundedConvolutionEffect and
 * GrGLLerpConvolutionEffect.
 */
class GrGLConvolutionEffect : public GrGLFragmentProcessor {
public:
    GrGLConvolutionEffect(const GrProcessor&);
    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    int radius() const { return fRadius; }
    int width() const { return Gr1DKernelEffect::WidthFromRadius(fRadius); }
    Gr1DKernelEffect::Direction direction() const { return fDirection; }
    void getImageIncrement(const GrConvolutionEffect&, float (*)[2]) const;

private:
    int fRadius;
    Gr1DKernelEffect::Direction fDirection;

    typedef GrGLFragmentProcessor INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(const GrProcessor& processor) {
    const GrConvolutionEffect& c = processor.cast<GrConvolutionEffect>();
    fRadius = c.radius();
    fDirection = c.direction();
}

void GrGLConvolutionEffect::GenKey(const GrProcessor& processor,
                                   const GrGLSLCaps&,
                                   GrProcessorKeyBuilder* b) {
    const GrConvolutionEffect& conv = processor.cast<GrConvolutionEffect>();
    uint32_t key = conv.radius();
    key <<= 2;
    if (conv.useBounds()) {
        key |= 0x2;
        key |= GrConvolutionEffect::kY_Direction == conv.direction() ? 0x1 : 0x0;
    }
    b->add32(key);
}

void GrGLConvolutionEffect::getImageIncrement(const GrConvolutionEffect& conv,
                                              float (*imageIncrement)[2]) const {
    GrTexture& texture = *conv.texture(0);
    (*imageIncrement)[0] = (*imageIncrement)[1] = 0;
    float ySign = texture.origin() != kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    switch (conv.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            (*imageIncrement)[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            (*imageIncrement)[1] = ySign / texture.height();
            break;
        default:
            SkFAIL("Unknown filter direction.");
    }
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Applies a convolution effect which restricts samples to the provided bounds
 * using shader logic.
 */
class GrGLBoundedConvolutionEffect : public GrGLConvolutionEffect {
public:
    GrGLBoundedConvolutionEffect(const GrProcessor& processor) : INHERITED(processor) {}

    virtual void emitCode(GrGLFPBuilder*,
                          const GrFragmentProcessor&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) override;

    void setData(const GrGLProgramDataManager& pdman, const GrProcessor&) override;

private:
    UniformHandle       fKernelUni;
    UniformHandle       fImageIncrementUni;
    UniformHandle       fBoundsUni;

    typedef GrGLConvolutionEffect INHERITED;
};

void GrGLBoundedConvolutionEffect::emitCode(GrGLFPBuilder* builder,
                                            const GrFragmentProcessor& processor,
                                            const char* outputColor,
                                            const char* inputColor,
                                            const TransformedCoordsArray& coords,
                                            const TextureSamplerArray& samplers) {
    fImageIncrementUni =
        builder->addUniform(GrGLProgramBuilder::kFragment_Visibility, kVec2f_GrSLType,
                            kDefault_GrSLPrecision, "ImageIncrement");

    fBoundsUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility, kVec2f_GrSLType,
                                     kDefault_GrSLPrecision, "Bounds");

    fKernelUni = builder->addUniformArray(GrGLProgramBuilder::kFragment_Visibility, kFloat_GrSLType,
                                          kDefault_GrSLPrecision, "Kernel", this->width());

    GrGLFragmentBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);

    fsBuilder->codeAppendf("%s = vec4(0, 0, 0, 0);\n", outputColor);

    int width = this->width();
    const GrGLShaderVar& kernel = builder->getUniformVariable(fKernelUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    fsBuilder->codeAppendf("vec2 coord = %s - %d.0 * %s;\n", coords2D.c_str(), this->radius(),
                           imgInc);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i);
        kernel.appendArrayAccess(index.c_str(), &kernelIndex);
        // We used to compute a bool indicating whether we're in bounds or not, cast it to a
        // float, and then mul weight*texture_sample by the float. However, the Adreno 430 seems
        // to have a bug that caused corruption.
        const char* bounds = builder->getUniformCStr(fBoundsUni);
        const char* component = this->direction() == Gr1DKernelEffect::kY_Direction ? "y" : "x";
        fsBuilder->codeAppendf("if (coord.%s >= %s.x && coord.%s <= %s.y) {",
            component, bounds, component, bounds);
        fsBuilder->codeAppendf("%s += ", outputColor);
        fsBuilder->appendTextureLookup(samplers[0], "coord");
        fsBuilder->codeAppendf(" * %s;\n", kernelIndex.c_str());
        fsBuilder->codeAppend("}");
        fsBuilder->codeAppendf("coord += %s;\n", imgInc);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, outputColor, inputColor);
    fsBuilder->codeAppend(modulate.c_str());
}

void GrGLBoundedConvolutionEffect::setData(const GrGLProgramDataManager& pdman,
                                           const GrProcessor& processor) {
    const GrConvolutionEffect& conv = processor.cast<GrConvolutionEffect>();

    // the code we generated was for a specific kernel radius
    SkASSERT(conv.radius() == this->radius());

    // the code we generated was for a specific bounding mode.
    SkASSERT(conv.useBounds());

    GrTexture& texture = *conv.texture(0);
    float imageIncrement[2];
    getImageIncrement(conv, &imageIncrement);
    pdman.set2fv(fImageIncrementUni, 1, imageIncrement);
    const float* bounds = conv.bounds();
    if (Gr1DKernelEffect::kY_Direction == conv.direction() &&
        texture.origin() != kTopLeft_GrSurfaceOrigin) {
        pdman.set2f(fBoundsUni, 1.0f - bounds[1], 1.0f - bounds[0]);
    } else {
        pdman.set2f(fBoundsUni, bounds[0], bounds[1]);
    }
    pdman.set1fv(fKernelUni, this->width(), conv.kernel());
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Applies a convolution effect which applies the convolution using a linear
 * interpolation optimization to use half as many samples.
 */
class GrGLLerpConvolutionEffect : public GrGLConvolutionEffect {
public:
    GrGLLerpConvolutionEffect(const GrProcessor& processor) : INHERITED(processor) {}

    virtual void emitCode(GrGLFPBuilder*,
                          const GrFragmentProcessor&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) override;

    void setData(const GrGLProgramDataManager& pdman, const GrProcessor&) override;

private:
    int bilerpSampleCount() const;

    // Bounded uniforms
    UniformHandle fSampleWeightUni;
    UniformHandle fSampleOffsetUni;

    typedef GrGLConvolutionEffect INHERITED;
};

void GrGLLerpConvolutionEffect::emitCode(GrGLFPBuilder* builder,
                                         const GrFragmentProcessor& processor,
                                         const char* outputColor,
                                         const char* inputColor,
                                         const TransformedCoordsArray& coords,
                                         const TextureSamplerArray& samplers) {
    int sampleCount = bilerpSampleCount();

    // We use 2 * sampleCount uniforms. The maximum allowed by PS2.0 is 32, so
    // ensure we don't exceed this. Note that it is currently impossible to
    // exceed this as bilerpSampleCount = (kernelWidth + 1) / 2, and kernelWidth
    // maxes out at 25, resulting in a max sampleCount of 26.
    SkASSERT(sampleCount < 16);

    fSampleOffsetUni =
        builder->addUniformArray(GrGLProgramBuilder::kFragment_Visibility, kVec2f_GrSLType,
                                 kDefault_GrSLPrecision, "SampleOffset", sampleCount);
    fSampleWeightUni =
        builder->addUniformArray(GrGLProgramBuilder::kFragment_Visibility, kFloat_GrSLType,
                                 kDefault_GrSLPrecision, "SampleWeight", sampleCount);

    GrGLFragmentBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);

    fsBuilder->codeAppendf("%s = vec4(0, 0, 0, 0);\n", outputColor);

    const GrGLShaderVar& kernel = builder->getUniformVariable(fSampleWeightUni);
    const GrGLShaderVar& imgInc = builder->getUniformVariable(fSampleOffsetUni);

    fsBuilder->codeAppendf("vec2 coord; \n");

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < sampleCount; i++) {
        SkString index;
        SkString weightIndex;
        SkString offsetIndex;
        index.appendS32(i);
        kernel.appendArrayAccess(index.c_str(), &weightIndex);
        imgInc.appendArrayAccess(index.c_str(), &offsetIndex);
        fsBuilder->codeAppendf("coord = %s + %s;\n", coords2D.c_str(), offsetIndex.c_str());
        fsBuilder->codeAppendf("%s += ", outputColor);
        fsBuilder->appendTextureLookup(samplers[0], "coord");
        fsBuilder->codeAppendf(" * %s;\n", weightIndex.c_str());
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, outputColor, inputColor);
    fsBuilder->codeAppend(modulate.c_str());
}

void GrGLLerpConvolutionEffect::setData(const GrGLProgramDataManager& pdman,
                                        const GrProcessor& processor) {
    const GrConvolutionEffect& conv = processor.cast<GrConvolutionEffect>();

    // the code we generated was for a specific kernel radius
    SkASSERT(conv.radius() == this->radius());

    // the code we generated was for a specific bounding mode.
    SkASSERT(!conv.useBounds());

    int sampleCount = bilerpSampleCount();
    SkAutoTArray<float> imageIncrements(sampleCount * 2);  // X and Y floats per sample.
    SkAutoTArray<float> kernel(sampleCount);

    float baseImageIncrement[2];
    getImageIncrement(conv, &baseImageIncrement);

    for (int i = 0; i < sampleCount; i++) {
        int sampleIndex1 = i * 2;
        int sampleIndex2 = sampleIndex1 + 1;

        // If we have an odd number of samples in our filter, the last sample won't use
        // the linear interpolation optimization (it will be pixel aligned).
        if (sampleIndex2 >= this->width()) {
            sampleIndex2 = sampleIndex1;
        }

        float kernelWeight1 = conv.kernel()[sampleIndex1];
        float kernelWeight2 = conv.kernel()[sampleIndex2];

        float totalKernelWeight =
            (sampleIndex1 == sampleIndex2) ? kernelWeight1 : (kernelWeight1 + kernelWeight2);

        float sampleRatio =
            (sampleIndex1 == sampleIndex2) ? 0 : kernelWeight2 / (kernelWeight1 + kernelWeight2);

        imageIncrements[i * 2] = (-this->radius() + i * 2 + sampleRatio) * baseImageIncrement[0];
        imageIncrements[i * 2 + 1] =
            (-this->radius() + i * 2 + sampleRatio) * baseImageIncrement[1];

        kernel[i] = totalKernelWeight;
    }
    pdman.set2fv(fSampleOffsetUni, sampleCount, imageIncrements.get());
    pdman.set1fv(fSampleWeightUni, sampleCount, kernel.get());
}

int GrGLLerpConvolutionEffect::bilerpSampleCount() const {
    // We use a linear interpolation optimization to only sample once for each
    // two pixel aligned samples in the kernel. If we have an odd number of
    // samples, we will have to skip this optimization for the last sample.
    // Because of this we always round up our sample count (by adding 1 before
    // dividing).
    return (this->width() + 1) / 2;
}

///////////////////////////////////////////////////////////////////////////////

GrConvolutionEffect::GrConvolutionEffect(GrProcessorDataManager* procDataManager,
                                         GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         const float* kernel,
                                         bool useBounds,
                                         float bounds[2])
    : INHERITED(procDataManager,
                texture,
                direction,
                radius,
                useBounds ? GrTextureParams::FilterMode::kNone_FilterMode
                          : GrTextureParams::FilterMode::kBilerp_FilterMode)
    , fUseBounds(useBounds) {
    this->initClassID<GrConvolutionEffect>();
    SkASSERT(radius <= kMaxKernelRadius);
    SkASSERT(kernel);
    int width = this->width();
    for (int i = 0; i < width; i++) {
        fKernel[i] = kernel[i];
    }
    memcpy(fBounds, bounds, sizeof(fBounds));
}

GrConvolutionEffect::GrConvolutionEffect(GrProcessorDataManager* procDataManager,
                                         GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         float gaussianSigma,
                                         bool useBounds,
                                         float bounds[2])
    : INHERITED(procDataManager,
                texture,
                direction,
                radius,
                useBounds ? GrTextureParams::FilterMode::kNone_FilterMode
                          : GrTextureParams::FilterMode::kBilerp_FilterMode)
    , fUseBounds(useBounds) {
    this->initClassID<GrConvolutionEffect>();
    SkASSERT(radius <= kMaxKernelRadius);
    int width = this->width();

    float sum = 0.0f;
    float denom = 1.0f / (2.0f * gaussianSigma * gaussianSigma);
    for (int i = 0; i < width; ++i) {
        float x = static_cast<float>(i - this->radius());
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
        // is dropped here, since we renormalize the kernel below.
        fKernel[i] = sk_float_exp(- x * x * denom);
        sum += fKernel[i];
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < width; ++i) {
        fKernel[i] *= scale;
    }
    memcpy(fBounds, bounds, sizeof(fBounds));
}

GrConvolutionEffect::~GrConvolutionEffect() {
}

void GrConvolutionEffect::getGLProcessorKey(const GrGLSLCaps& caps,
                                        GrProcessorKeyBuilder* b) const {
    GrGLConvolutionEffect::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* GrConvolutionEffect::createGLInstance() const  {
    // We support a linear interpolation optimization which (when feasible) uses
    // half the number of samples to apply the kernel. This is not always
    // applicable, as the linear interpolation optimization does not support
    // bounded sampling.
    if (this->useBounds()) {
        return SkNEW_ARGS(GrGLBoundedConvolutionEffect, (*this));
    } else {
        return SkNEW_ARGS(GrGLLerpConvolutionEffect, (*this));
    }
}

bool GrConvolutionEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrConvolutionEffect& s = sBase.cast<GrConvolutionEffect>();
    return (this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->useBounds() == s.useBounds() &&
            0 == memcmp(fBounds, s.fBounds, sizeof(fBounds)) &&
            0 == memcmp(fKernel, s.fKernel, this->width() * sizeof(float)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConvolutionEffect);

GrFragmentProcessor* GrConvolutionEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    Direction dir = d->fRandom->nextBool() ? kX_Direction : kY_Direction;
    int radius = d->fRandom->nextRangeU(1, kMaxKernelRadius);
    float kernel[kMaxKernelWidth];
    for (size_t i = 0; i < SK_ARRAY_COUNT(kernel); ++i) {
        kernel[i] = d->fRandom->nextSScalar1();
    }
    float bounds[2];
    for (size_t i = 0; i < SK_ARRAY_COUNT(bounds); ++i) {
        bounds[i] = d->fRandom->nextF();
    }

    bool useBounds = d->fRandom->nextBool();
    return GrConvolutionEffect::Create(d->fProcDataManager,
                                       d->fTextures[texIdx],
                                       dir,
                                       radius,
                                       kernel,
                                       useBounds,
                                       bounds);
}
