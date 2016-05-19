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

// Create a Gaussian half-kernel and a summed area table given a sigma and number of discrete
// steps. The half kernel is normalized to sum to 0.5.
static void make_half_kernel_and_summed_table(float* halfKernel, float* summedHalfKernel,
                                              int halfKernelSize, float sigma) {
    const float invSigma = 1.f / sigma;
    const float b = -0.5f * invSigma * invSigma;
    float tot = 0.0f;
    // Compute half kernel values at half pixel steps out from the center.
    float t = 0.5f;
    for (int i = 0; i < halfKernelSize; ++i) {
        float value = expf(t * t * b);
        tot += value;
        halfKernel[i] = value;
        t += 1.f;
    }
    float sum = 0.f;
    // The half kernel should sum to 0.5 not 1.0.
    tot *= 2.f;
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[i] /= tot;
        sum += halfKernel[i];
        summedHalfKernel[i] = sum;
    }
}

// Applies the 1D half kernel vertically at a point (x, 0) to a circle centered at the origin with
// radius circleR.
static float eval_vertically(float x, float circleR, const float* summedHalfKernelTable,
                             int halfKernelSize) {
    // Given x find the positive y that is on the edge of the circle.
    float y = sqrtf(fabs(circleR * circleR - x * x));
    // In the column at x we exit the circle at +y and -y
    // table entry j is actually the kernel evaluated at j + 0.5.
    y -= 0.5f;
    int yInt = SkScalarFloorToInt(y);
    SkASSERT(yInt >= -1);
    if (y < 0) {
        return (y + 0.5f) * summedHalfKernelTable[0];
    } else if (yInt >= halfKernelSize - 1) {
        return 0.5f;
    } else {
        float yFrac = y - yInt;
        return (1.f - yFrac) * summedHalfKernelTable[yInt] +
                       yFrac * summedHalfKernelTable[yInt + 1];
    }
}

// Apply the kernel at point (t, 0) to a circle centered at the origin with radius circleR.
static uint8_t eval_at(float t, float circleR, const float* halfKernel,
                       const float* summedHalfKernelTable, int halfKernelSize) {
    float acc = 0;

    for (int i = 0; i < halfKernelSize; ++i) {
        float x = t - i - 0.5f;
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = eval_vertically(x, circleR, summedHalfKernelTable, halfKernelSize);
        acc += verticalEval * halfKernel[i];
    }
    for (int i = 0; i < halfKernelSize; ++i) {
        float x = t + i + 0.5f;
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = eval_vertically(x, circleR, summedHalfKernelTable, halfKernelSize);
        acc += verticalEval * halfKernel[i];
    }
    // Since we applied a half kernel in y we multiply acc by 2 (the circle is symmetric about the
    // x axis).
    return SkUnitScalarClampToByte(2.f * acc);
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

// This function creates a profile of a blurred circle. It does this by computing a kernel for
// half the Gaussian and a matching summed area table. To compute a profile value at x = r it steps
// outward in x from (r, 0) in both directions. There is a step for each direction for each entry
// in the half kernel. The y contribution at each step is computed from the summed area table using
// the height of the circle above the step point. Each y contribution is multiplied by the half
// kernel value corresponding to the step in x.
static uint8_t* create_profile(float circleR, float sigma) {
    float offset;
    int numSteps;
    compute_profile_offset_and_size(circleR, sigma, &offset, &numSteps);

    uint8_t* weights = new uint8_t[numSteps];

    // The full kernel is 6 sigmas wide.
    int halfKernelSize = SkScalarCeilToInt(6.0f*sigma);
    // round up to next multiple of 2 and then divide by 2
    halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;
    SkAutoTArray<float> halfKernel(halfKernelSize);
    SkAutoTArray<float> summedKernel(halfKernelSize);
    make_half_kernel_and_summed_table(halfKernel.get(), summedKernel.get(), halfKernelSize,
                                      sigma);
    for (int i = 0; i < numSteps - 1; ++i) {
        weights[i] = eval_at(offset+i, circleR, halfKernel.get(), summedKernel.get(),
                             halfKernelSize);
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
