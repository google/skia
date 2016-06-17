/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCircleBlurFragmentProcessor.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrInvariantOutput.h"
#include "GrTextureProvider.h"

#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

#include "SkFixed.h"

class GrGLCircleBlurFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fDataUniform;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLCircleBlurFragmentProcessor::emitCode(EmitArgs& args) {

    const char *dataName;

    // The data is formatted as:
    // x,y  - the center of the circle
    // z    - the distance at which the intensity starts falling off (e.g., the start of the table)
    // w    - the inverse of the profile texture size
    fDataUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kVec4f_GrSLType,
                                                    kDefault_GrSLPrecision,
                                                    "data",
                                                    &dataName);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const char *fragmentPos = fragBuilder->fragmentPosition();

    if (args.fInputColor) {
        fragBuilder->codeAppendf("vec4 src=%s;", args.fInputColor);
    } else {
        fragBuilder->codeAppendf("vec4 src=vec4(1);");
    }

    // We just want to compute "length(vec) - %s.z + 0.5) * %s.w" but need to rearrange
    // for precision
    fragBuilder->codeAppendf("vec2 vec = vec2( (%s.x - %s.x) * %s.w , (%s.y - %s.y) * %s.w );",
                             fragmentPos, dataName, dataName,
                             fragmentPos, dataName, dataName);
    fragBuilder->codeAppendf("float dist = length(vec) + ( 0.5 - %s.z ) * %s.w;",
                             dataName, dataName);

    fragBuilder->codeAppendf("float intensity = ");
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], "vec2(dist, 0.5)");
    fragBuilder->codeAppend(".a;");

    fragBuilder->codeAppendf("%s = src * intensity;\n", args.fOutputColor );
}

void GrGLCircleBlurFragmentProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                const GrProcessor& proc) {
    const GrCircleBlurFragmentProcessor& cbfp = proc.cast<GrCircleBlurFragmentProcessor>();
    const SkRect& circle = cbfp.circle();

    // The data is formatted as:
    // x,y  - the center of the circle
    // z    - the distance at which the intensity starts falling off (e.g., the start of the table)
    // w    - the inverse of the profile texture size
    pdman.set4f(fDataUniform, circle.centerX(), circle.centerY(), cbfp.offset(),
                1.0f / cbfp.profileSize());
}

///////////////////////////////////////////////////////////////////////////////

GrCircleBlurFragmentProcessor::GrCircleBlurFragmentProcessor(const SkRect& circle,
                                                             float sigma,
                                                             float offset,
                                                             GrTexture* blurProfile)
    : fCircle(circle)
    , fSigma(sigma)
    , fOffset(offset)
    , fBlurProfileAccess(blurProfile, GrTextureParams::kBilerp_FilterMode) {
    this->initClassID<GrCircleBlurFragmentProcessor>();
    this->addTextureAccess(&fBlurProfileAccess);
    this->setWillReadFragmentPosition();
}

GrGLSLFragmentProcessor* GrCircleBlurFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLCircleBlurFragmentProcessor;
}

void GrCircleBlurFragmentProcessor::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                          GrProcessorKeyBuilder* b) const {
    GrGLCircleBlurFragmentProcessor::GenKey(*this, caps, b);
}

void GrCircleBlurFragmentProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

// Evaluate an AA circle function centered at the origin with 'radius' at (x,y)
static inline float disk(float x, float y, float radius) {
    float distSq = x*x + y*y;
    if (distSq <= (radius - 0.5f) * (radius - 0.5f)) {
        return 1.0f;
    } else if (distSq >= (radius + 0.5f) * (radius + 0.5f)) {
        return 0.0f;
    } else {
        float ramp = radius + 0.5f - sqrtf(distSq);
        SkASSERT(ramp >= 0.0f && ramp <= 1.0f);
        return ramp;
    }
}

// Create the top half of an even-sized Gaussian kernel
static void make_half_kernel(float* kernel, int kernelWH, float sigma) {
    SkASSERT(!(kernelWH & 1));

    // We treat each cell in the half-kernel as a 1x1 window and evaluate it
    // at the center. So the evaluations go from -kernelOff to kernelOff in x
    // and -kernelOff to -.5 in y (since this is a top-half kernel).
    const float kernelOff = (kernelWH - 1) / 2.0f;

    float b = 1.0f / (2.0f * sigma * sigma);
    // omit the scale term since we're just going to renormalize

    float tot = 0.0f;
    for (int y = 0; y < kernelWH / 2; ++y) {
        for (int x = 0; x < kernelWH / 2; ++x) {
            // TODO: use a cheap approximation of the 2D Guassian?
            float x2 = (x - kernelOff) * (x - kernelOff);
            float y2 = (y - kernelOff) * (y - kernelOff);
            // The kernel is symmetric so only compute it once for both sides
            float value = expf(-(x2 + y2) * b);
            kernel[y * kernelWH + x] = value;
            kernel[y * kernelWH + (kernelWH - x - 1)] = value;
            tot += 2.0f * value;
        }
    }
    // Normalize the half kernel to 1.0 (rather than 0.5) so we don't have to scale by 2.0 after
    // convolution.
    for (int y = 0; y < kernelWH / 2; ++y) {
        for (int x = 0; x < kernelWH; ++x) {
            kernel[y * kernelWH + x] /= tot;
        }
    }
}

// Apply the half-kernel at 't' away from the center of the circle
static uint8_t eval_at(float t, float circleR, float* halfKernel, int kernelWH) {
    SkASSERT(!(kernelWH & 1));

    float acc = 0;

    // We evaluate the kernel application at (x=t, y=0) using halfKernel which represents the top
    // half of a 2D Guassian kernel. The full kernel is symmetric so evaluating just the upper half
    // is sufficient. The half kernel has been normalized to 1 rather than 0.5 so there is no need
    // to double after evaluation.

    // The sample positions relative to (t, 0) match the sampling used to create the half kernel.
    const float kernelOff = (kernelWH - 1) / 2.0f;

    for (int j = 0; j < kernelWH / 2; ++j) {
        float y = (kernelOff - j);
        if (y > circleR + 0.5f) {
            // The entire row is above the circle.
            continue;
        }

        for (int i = 0; i < kernelWH; ++i) {
            float x = t - kernelOff + i;
            if (x > circleR + 0.5f) {
                // Stop evaluation once x crosses outside the circle.
                break;
            }
            float image = disk(x, y, circleR);
            float kernel = halfKernel[j * kernelWH + i];
            acc += kernel * image;
        }
    }

    return SkUnitScalarClampToByte(acc);
}

static inline void compute_profile_offset_and_size(float circleR, float sigma,
                                                   float* offset, int* size) {

    if (3*sigma <= circleR) {
        // The circle is bigger than the Gaussian. In this case we know the interior of the
        // blurred circle is solid.
        *offset = circleR - 3 * sigma; // This location maps to 0.5f in the weights texture.
                                       // It should always be 255.
        *size = SkScalarCeilToInt(6*sigma);
    } else {
        // The Gaussian is bigger than the circle.
        *offset = 0.0f;
        *size = SkScalarCeilToInt(circleR + 3*sigma);
    }
}

static uint8_t* create_profile(float circleR, float sigma) {

    int kernelWH = SkScalarCeilToInt(6.0f*sigma);
    kernelWH = (kernelWH + 1) & ~1; // make it the next even number up

    SkAutoTArray<float> halfKernel(kernelWH * kernelWH / 2);

    make_half_kernel(halfKernel.get(), kernelWH, sigma);

    float offset;
    int numSteps;

    compute_profile_offset_and_size(circleR, sigma, &offset, &numSteps);

    uint8_t* weights = new uint8_t[numSteps];
    for (int i = 0; i < numSteps - 1; ++i) {
        weights[i] = eval_at(offset+i, circleR, halfKernel.get(), kernelWH);
    }
    // Ensure the tail of the Gaussian goes to zero.
    weights[numSteps - 1] = 0;

    return weights;
}

GrTexture* GrCircleBlurFragmentProcessor::CreateCircleBlurProfileTexture(
                                                                GrTextureProvider* textureProvider,
                                                                const SkRect& circle,
                                                                float sigma,
                                                                float* offset) {
    float circleR = circle.width() / 2.0f;

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 2);
    // The profile curve varies with both the sigma of the Gaussian and the size of the
    // disk. Quantizing to 16.16 should be close enough though.
    builder[0] = SkScalarToFixed(sigma);
    builder[1] = SkScalarToFixed(circleR);
    builder.finish();

    GrTexture *blurProfile = textureProvider->findAndRefTextureByUniqueKey(key);

    int profileSize;
    compute_profile_offset_and_size(circleR, sigma, offset, &profileSize);

    if (!blurProfile) {

        GrSurfaceDesc texDesc;
        texDesc.fWidth = profileSize;
        texDesc.fHeight = 1;
        texDesc.fConfig = kAlpha_8_GrPixelConfig;

        SkAutoTDeleteArray<uint8_t> profile(create_profile(circleR, sigma));

        blurProfile = textureProvider->createTexture(texDesc, SkBudgeted::kYes, profile.get(), 0);
        if (blurProfile) {
            textureProvider->assignUniqueKeyToTexture(key, blurProfile);
        }
    }

    return blurProfile;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrCircleBlurFragmentProcessor);

const GrFragmentProcessor* GrCircleBlurFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    SkScalar wh = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar sigma = d->fRandom->nextRangeF(1.f,10.f);
    SkRect circle = SkRect::MakeWH(wh, wh);
    return GrCircleBlurFragmentProcessor::Create(d->fContext->textureProvider(), circle, sigma);
}

#endif
