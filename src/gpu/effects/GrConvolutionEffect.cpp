/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrConvolutionEffect.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendProcessorFactory.h"

// For brevity
typedef GrGLProgramDataManager::UniformHandle UniformHandle;

class GrGLConvolutionEffect : public GrGLFragmentProcessor {
public:
    GrGLConvolutionEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder*,
                          const GrFragmentProcessor&,
                          const GrProcessorKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLProgramDataManager& pdman, const GrProcessor&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

private:
    int width() const { return Gr1DKernelEffect::WidthFromRadius(fRadius); }
    bool useBounds() const { return fUseBounds; }
    Gr1DKernelEffect::Direction direction() const { return fDirection; }

    int                 fRadius;
    bool                fUseBounds;
    Gr1DKernelEffect::Direction    fDirection;
    UniformHandle       fKernelUni;
    UniformHandle       fImageIncrementUni;
    UniformHandle       fBoundsUni;

    typedef GrGLFragmentProcessor INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(const GrBackendProcessorFactory& factory,
                                             const GrProcessor& processor)
    : INHERITED(factory) {
    const GrConvolutionEffect& c = processor.cast<GrConvolutionEffect>();
    fRadius = c.radius();
    fUseBounds = c.useBounds();
    fDirection = c.direction();
}

void GrGLConvolutionEffect::emitCode(GrGLProgramBuilder* builder,
                                     const GrFragmentProcessor&,
                                     const GrProcessorKey& key,
                                     const char* outputColor,
                                     const char* inputColor,
                                     const TransformedCoordsArray& coords,
                                     const TextureSamplerArray& samplers) {
    fImageIncrementUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "ImageIncrement");
    if (this->useBounds()) {
        fBoundsUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                         kVec2f_GrSLType, "Bounds");
    }
    fKernelUni = builder->addUniformArray(GrGLProgramBuilder::kFragment_Visibility,
                                          kFloat_GrSLType, "Kernel", this->width());

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);

    fsBuilder->codeAppendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);

    int width = this->width();
    const GrGLShaderVar& kernel = builder->getUniformVariable(fKernelUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    fsBuilder->codeAppendf("\t\tvec2 coord = %s - %d.0 * %s;\n", coords2D.c_str(), fRadius, imgInc);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i);
        kernel.appendArrayAccess(index.c_str(), &kernelIndex);
        fsBuilder->codeAppendf("\t\t%s += ", outputColor);
        fsBuilder->appendTextureLookup(samplers[0], "coord");
        if (this->useBounds()) {
            const char* bounds = builder->getUniformCStr(fBoundsUni);
            const char* component = this->direction() == Gr1DKernelEffect::kY_Direction ? "y" : "x";
            fsBuilder->codeAppendf(" * float(coord.%s >= %s.x && coord.%s <= %s.y)",
                component, bounds, component, bounds);
        }
        fsBuilder->codeAppendf(" * %s;\n", kernelIndex.c_str());
        fsBuilder->codeAppendf("\t\tcoord += %s;\n", imgInc);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    fsBuilder->codeAppend(modulate.c_str());
}

void GrGLConvolutionEffect::setData(const GrGLProgramDataManager& pdman,
                                    const GrProcessor& processor) {
    const GrConvolutionEffect& conv = processor.cast<GrConvolutionEffect>();
    GrTexture& texture = *conv.texture(0);
    // the code we generated was for a specific kernel radius
    SkASSERT(conv.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    float ySign = texture.origin() != kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    switch (conv.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            imageIncrement[1] = ySign / texture.height();
            break;
        default:
            SkFAIL("Unknown filter direction.");
    }
    pdman.set2fv(fImageIncrementUni, 1, imageIncrement);
    if (conv.useBounds()) {
        const float* bounds = conv.bounds();
        if (Gr1DKernelEffect::kY_Direction == conv.direction() &&
            texture.origin() != kTopLeft_GrSurfaceOrigin) {
            pdman.set2f(fBoundsUni, 1.0f - bounds[1], 1.0f - bounds[0]);
        } else {
            pdman.set2f(fBoundsUni, bounds[0], bounds[1]);
        }
    }
    pdman.set1fv(fKernelUni, this->width(), conv.kernel());
}

void GrGLConvolutionEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
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

///////////////////////////////////////////////////////////////////////////////

GrConvolutionEffect::GrConvolutionEffect(GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         const float* kernel,
                                         bool useBounds,
                                         float bounds[2])
    : Gr1DKernelEffect(texture, direction, radius), fUseBounds(useBounds) {
    SkASSERT(radius <= kMaxKernelRadius);
    SkASSERT(kernel);
    int width = this->width();
    for (int i = 0; i < width; i++) {
        fKernel[i] = kernel[i];
    }
    memcpy(fBounds, bounds, sizeof(fBounds));
}

GrConvolutionEffect::GrConvolutionEffect(GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         float gaussianSigma,
                                         bool useBounds,
                                         float bounds[2])
    : Gr1DKernelEffect(texture, direction, radius), fUseBounds(useBounds) {
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

const GrBackendFragmentProcessorFactory& GrConvolutionEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrConvolutionEffect>::getInstance();
}

bool GrConvolutionEffect::onIsEqual(const GrProcessor& sBase) const {
    const GrConvolutionEffect& s = sBase.cast<GrConvolutionEffect>();
    return (this->texture(0) == s.texture(0) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->useBounds() == s.useBounds() &&
            0 == memcmp(fBounds, s.fBounds, sizeof(fBounds)) &&
            0 == memcmp(fKernel, s.fKernel, this->width() * sizeof(float)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConvolutionEffect);

GrFragmentProcessor* GrConvolutionEffect::TestCreate(SkRandom* random,
                                                     GrContext*,
                                                     const GrDrawTargetCaps&,
                                                     GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    int radius = random->nextRangeU(1, kMaxKernelRadius);
    float kernel[kMaxKernelWidth];
    for (size_t i = 0; i < SK_ARRAY_COUNT(kernel); ++i) {
        kernel[i] = random->nextSScalar1();
    }
    float bounds[2];
    for (size_t i = 0; i < SK_ARRAY_COUNT(bounds); ++i) {
        bounds[i] = random->nextF();
    }

    bool useBounds = random->nextBool();
    return GrConvolutionEffect::Create(textures[texIdx],
                                       dir,
                                       radius,
                                       kernel,
                                       useBounds,
                                       bounds);
}
