/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvolutionEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

class GrGLConvolutionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor&) override;

private:
    UniformHandle       fKernelUni;
    UniformHandle       fImageIncrementUni;
    UniformHandle       fBoundsUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLConvolutionEffect::emitCode(EmitArgs& args) {
    const GrConvolutionEffect& ce = args.fFp.cast<GrConvolutionEffect>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fImageIncrementUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                    "ImageIncrement");
    if (ce.useBounds()) {
        fBoundsUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                "Bounds");
    }

    int width = Gr1DKernelEffect::WidthFromRadius(ce.radius());

    int arrayCount = (width + 3) / 4;
    SkASSERT(4 * arrayCount >= width);

    fKernelUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                 kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                 "Kernel", arrayCount);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);

    fragBuilder->codeAppendf("%s = vec4(0, 0, 0, 0);", args.fOutputColor);

    const GrGLSLShaderVar& kernel = uniformHandler->getUniformVariable(fKernelUni);
    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);

    fragBuilder->codeAppendf("vec2 coord = %s - %d.0 * %s;", coords2D.c_str(), ce.radius(), imgInc);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    const char* kVecSuffix[4] = { ".x", ".y", ".z", ".w" };
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i/4);
        kernel.appendArrayAccess(index.c_str(), &kernelIndex);
        kernelIndex.append(kVecSuffix[i & 0x3]);

        if (ce.useBounds()) {
            // We used to compute a bool indicating whether we're in bounds or not, cast it to a
            // float, and then mul weight*texture_sample by the float. However, the Adreno 430 seems
            // to have a bug that caused corruption.
            const char* bounds = uniformHandler->getUniformCStr(fBoundsUni);
            const char* component = ce.direction() == Gr1DKernelEffect::kY_Direction ? "y" : "x";
            fragBuilder->codeAppendf("if (coord.%s >= %s.x && coord.%s <= %s.y) {",
                                     component, bounds, component, bounds);
        }
        fragBuilder->codeAppendf("\t\t%s += ", args.fOutputColor);
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "coord");
        fragBuilder->codeAppendf(" * %s;\n", kernelIndex.c_str());
        if (ce.useBounds()) {
            fragBuilder->codeAppend("}");
        }
        fragBuilder->codeAppendf("\t\tcoord += %s;\n", imgInc);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLConvolutionEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                      const GrProcessor& processor) {
    const GrConvolutionEffect& conv = processor.cast<GrConvolutionEffect>();
    GrTexture& texture = *conv.texture(0);

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
    int width = Gr1DKernelEffect::WidthFromRadius(conv.radius());

    int arrayCount = (width + 3) / 4;
    SkASSERT(4 * arrayCount >= width);
    pdman.set4fv(fKernelUni, arrayCount, conv.kernel());
}

void GrGLConvolutionEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
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
    : INHERITED(texture, direction, radius), fUseBounds(useBounds) {
    this->initClassID<GrConvolutionEffect>();
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
    : INHERITED(texture, direction, radius), fUseBounds(useBounds) {
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

void GrConvolutionEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                GrProcessorKeyBuilder* b) const {
    GrGLConvolutionEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrConvolutionEffect::onCreateGLSLInstance() const  {
    return new GrGLConvolutionEffect;
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

sk_sp<GrFragmentProcessor> GrConvolutionEffect::TestCreate(GrProcessorTestData* d) {
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
    return GrConvolutionEffect::Make(d->fTextures[texIdx],
                                     dir,
                                     radius,
                                     kernel,
                                     useBounds,
                                     bounds);
}
