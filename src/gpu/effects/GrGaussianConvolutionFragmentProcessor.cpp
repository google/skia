/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGaussianConvolutionFragmentProcessor.h"

#include "GrProxyMove.h"
#include "GrTextureProxy.h"
#include "../private/GrGLSL.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

class GrGLConvolutionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    UniformHandle fKernelUni;
    UniformHandle fImageIncrementUni;
    UniformHandle fBoundsUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLConvolutionEffect::emitCode(EmitArgs& args) {
    const GrGaussianConvolutionFragmentProcessor& ce =
            args.fFp.cast<GrGaussianConvolutionFragmentProcessor>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fImageIncrementUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kVec2f_GrSLType,
                                                    kDefault_GrSLPrecision, "ImageIncrement");
    if (ce.useBounds()) {
        fBoundsUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kVec2f_GrSLType,
                                                kDefault_GrSLPrecision, "Bounds");
    }

    int width = Gr1DKernelEffect::WidthFromRadius(ce.radius());

    int arrayCount = (width + 3) / 4;
    SkASSERT(4 * arrayCount >= width);

    fKernelUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag, kVec4f_GrSLType,
                                                 kDefault_GrSLPrecision, "Kernel", arrayCount);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);

    fragBuilder->codeAppendf("%s = vec4(0, 0, 0, 0);", args.fOutputColor);

    const GrShaderVar& kernel = uniformHandler->getUniformVariable(fKernelUni);
    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);

    fragBuilder->codeAppendf("vec2 coord = %s - %d.0 * %s;", coords2D.c_str(), ce.radius(), imgInc);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    const char* kVecSuffix[4] = {".x", ".y", ".z", ".w"};
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i / 4);
        kernel.appendArrayAccess(index.c_str(), &kernelIndex);
        kernelIndex.append(kVecSuffix[i & 0x3]);

        if (ce.useBounds()) {
            // We used to compute a bool indicating whether we're in bounds or not, cast it to a
            // float, and then mul weight*texture_sample by the float. However, the Adreno 430 seems
            // to have a bug that caused corruption.
            const char* bounds = uniformHandler->getUniformCStr(fBoundsUni);
            const char* component = ce.direction() == Gr1DKernelEffect::kY_Direction ? "y" : "x";
            fragBuilder->codeAppendf("if (coord.%s >= %s.x && coord.%s <= %s.y) {", component,
                                     bounds, component, bounds);
        }
        fragBuilder->codeAppendf("%s += ", args.fOutputColor);
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "coord");
        fragBuilder->codeAppendf(" * %s;\n", kernelIndex.c_str());
        if (ce.useBounds()) {
            fragBuilder->codeAppend("}");
        }
        fragBuilder->codeAppendf("coord += %s;\n", imgInc);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLConvolutionEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                      const GrFragmentProcessor& processor) {
    const GrGaussianConvolutionFragmentProcessor& conv =
            processor.cast<GrGaussianConvolutionFragmentProcessor>();
    GrTexture& texture = *conv.textureSampler(0).texture();

    float imageIncrement[2] = {0};
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
        const int* bounds = conv.bounds();
        if (Gr1DKernelEffect::kX_Direction == conv.direction()) {
            SkScalar inv = SkScalarInvert(SkIntToScalar(texture.width()));
            pdman.set2f(fBoundsUni, inv * bounds[0], inv * bounds[1]);
        } else {
            SkScalar inv = SkScalarInvert(SkIntToScalar(texture.height()));
            if (texture.origin() != kTopLeft_GrSurfaceOrigin) {
                pdman.set2f(fBoundsUni, 1.0f - (inv * bounds[1]), 1.0f - (inv * bounds[0]));
            } else {
                pdman.set2f(fBoundsUni, inv * bounds[1], inv * bounds[0]);
            }
        }
    }
    int width = Gr1DKernelEffect::WidthFromRadius(conv.radius());

    int arrayCount = (width + 3) / 4;
    SkASSERT(4 * arrayCount >= width);
    pdman.set4fv(fKernelUni, arrayCount, conv.kernel());
}

void GrGLConvolutionEffect::GenKey(const GrProcessor& processor, const GrShaderCaps&,
                                   GrProcessorKeyBuilder* b) {
    const GrGaussianConvolutionFragmentProcessor& conv =
            processor.cast<GrGaussianConvolutionFragmentProcessor>();
    uint32_t key = conv.radius();
    key <<= 2;
    if (conv.useBounds()) {
        key |= 0x2;
        key |= GrGaussianConvolutionFragmentProcessor::kY_Direction == conv.direction() ? 0x1 : 0x0;
    }
    b->add32(key);
}

///////////////////////////////////////////////////////////////////////////////
static void fill_in_1D_guassian_kernel(float* kernel, int width, float gaussianSigma, int radius) {
    const float denom = 1.0f / (2.0f * gaussianSigma * gaussianSigma);

    float sum = 0.0f;
    for (int i = 0; i < width; ++i) {
        float x = static_cast<float>(i - radius);
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
        // is dropped here, since we renormalize the kernel below.
        kernel[i] = sk_float_exp(-x * x * denom);
        sum += kernel[i];
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < width; ++i) {
        kernel[i] *= scale;
    }
}

GrGaussianConvolutionFragmentProcessor::GrGaussianConvolutionFragmentProcessor(
                                                            GrResourceProvider* resourceProvider,
                                                            sk_sp<GrTextureProxy> proxy,
                                                            Direction direction,
                                                            int radius,
                                                            float gaussianSigma,
                                                            bool useBounds,
                                                            int bounds[2])
        : INHERITED{resourceProvider,
                    ModulationFlags(proxy->config()),
                    GR_PROXY_MOVE(proxy),
                    direction,
                    radius}
        , fUseBounds(useBounds) {
    this->initClassID<GrGaussianConvolutionFragmentProcessor>();
    SkASSERT(radius <= kMaxKernelRadius);

    fill_in_1D_guassian_kernel(fKernel, this->width(), gaussianSigma, this->radius());

    memcpy(fBounds, bounds, sizeof(fBounds));
}

GrGaussianConvolutionFragmentProcessor::~GrGaussianConvolutionFragmentProcessor() {}

void GrGaussianConvolutionFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                                   GrProcessorKeyBuilder* b) const {
    GrGLConvolutionEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrGaussianConvolutionFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLConvolutionEffect;
}

bool GrGaussianConvolutionFragmentProcessor::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrGaussianConvolutionFragmentProcessor& s =
            sBase.cast<GrGaussianConvolutionFragmentProcessor>();
    return (this->radius() == s.radius() && this->direction() == s.direction() &&
            this->useBounds() == s.useBounds() &&
            0 == memcmp(fBounds, s.fBounds, sizeof(fBounds)) &&
            0 == memcmp(fKernel, s.fKernel, this->width() * sizeof(float)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrGaussianConvolutionFragmentProcessor);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrGaussianConvolutionFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

    bool useBounds = d->fRandom->nextBool();
    int bounds[2];

    Direction dir;
    if (d->fRandom->nextBool()) {
        dir = kX_Direction;
        bounds[0] = d->fRandom->nextRangeU(0, proxy->width()-1);
        bounds[1] = d->fRandom->nextRangeU(bounds[0], proxy->width()-1);
    } else {
        dir = kY_Direction;
        bounds[0] = d->fRandom->nextRangeU(0, proxy->height()-1);
        bounds[1] = d->fRandom->nextRangeU(bounds[0], proxy->height()-1);
    }

    int radius = d->fRandom->nextRangeU(1, kMaxKernelRadius);
    float sigma = radius / 3.f;

    return GrGaussianConvolutionFragmentProcessor::Make(
            d->resourceProvider(), d->textureProxy(texIdx),
            dir, radius, sigma, useBounds, bounds);
}
#endif
