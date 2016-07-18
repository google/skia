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

class GrCircleBlurFragmentProcessor::GLSLProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fDataUniform;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrCircleBlurFragmentProcessor::GLSLProcessor::emitCode(EmitArgs& args) {

    const char *dataName;

    // The data is formatted as:
    // x,y  - the center of the circle
    // z    - the distance at which the intensity starts falling off (e.g., the start of the table)
    // w    - the inverse of the distance over which the texture is stretched.
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

void GrCircleBlurFragmentProcessor::GLSLProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                             const GrProcessor& proc) {
    const GrCircleBlurFragmentProcessor& cbfp = proc.cast<GrCircleBlurFragmentProcessor>();
    const SkRect& circle = cbfp.fCircle;

    // The data is formatted as:
    // x,y  - the center of the circle
    // z    - the distance at which the intensity starts falling off (e.g., the start of the table)
    // w    - the inverse of the distance over which the profile texture is stretched.
    pdman.set4f(fDataUniform, circle.centerX(), circle.centerY(), cbfp.fSolidRadius,
                1.f / cbfp.fTextureRadius);
}

///////////////////////////////////////////////////////////////////////////////

GrCircleBlurFragmentProcessor::GrCircleBlurFragmentProcessor(const SkRect& circle,
                                                             float sigma,
                                                             float solidRadius,
                                                             float textureRadius,
                                                             GrTexture* blurProfile)
    : fCircle(circle)
    , fSigma(sigma)
    , fSolidRadius(solidRadius)
    , fTextureRadius(textureRadius)
    , fBlurProfileAccess(blurProfile, GrTextureParams::kBilerp_FilterMode) {
    this->initClassID<GrCircleBlurFragmentProcessor>();
    this->addTextureAccess(&fBlurProfileAccess);
    this->setWillReadFragmentPosition();
}

GrGLSLFragmentProcessor* GrCircleBlurFragmentProcessor::onCreateGLSLInstance() const {
    return new GLSLProcessor;
}

void GrCircleBlurFragmentProcessor::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                          GrProcessorKeyBuilder* b) const {
    // The code for this processor is always the same so there is nothing to add to the key.
    return;
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

// Applies the 1D half kernel vertically at points along the x axis to a circle centered at the
// origin with radius circleR.
void apply_kernel_in_y(float* results, int numSteps, float firstX, float circleR,
                       int halfKernelSize, const float* summedHalfKernelTable) {
    float x = firstX;
    for (int i = 0; i < numSteps; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            results[i] = 0;
            continue;
        }
        float y = sqrtf(circleR * circleR - x * x);
        // In the column at x we exit the circle at +y and -y
        // The summed table entry j is actually reflects an offset of j + 0.5.
        y -= 0.5f;
        int yInt = SkScalarFloorToInt(y);
        SkASSERT(yInt >= -1);
        if (y < 0) {
            results[i] = (y + 0.5f) * summedHalfKernelTable[0];
        } else if (yInt >= halfKernelSize - 1) {
            results[i] = 0.5f;
        } else {
            float yFrac = y - yInt;
            results[i] = (1.f - yFrac) * summedHalfKernelTable[yInt] +
                         yFrac * summedHalfKernelTable[yInt + 1];
        }
    }
}

// Apply a Gaussian at point (evalX, 0) to a circle centered at the origin with radius circleR.
// This relies on having a half kernel computed for the Gaussian and a table of applications of
// the half kernel in y to columns at (evalX - halfKernel, evalX - halfKernel + 1, ..., evalX +
// halfKernel) passed in as yKernelEvaluations.
static uint8_t eval_at(float evalX, float circleR, const float* halfKernel, int halfKernelSize,
                       const float* yKernelEvaluations) {
    float acc = 0;

    float x = evalX - halfKernelSize;
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i];
        acc += verticalEval * halfKernel[halfKernelSize - i - 1];
    }
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i + halfKernelSize];
        acc += verticalEval * halfKernel[i];
    }
    // Since we applied a half kernel in y we multiply acc by 2 (the circle is symmetric about the
    // x axis).
    return SkUnitScalarClampToByte(2.f * acc);
}

// This function creates a profile of a blurred circle. It does this by computing a kernel for
// half the Gaussian and a matching summed area table. The summed area table is used to compute
// an array of vertical applications of the half kernel to the circle along the x axis. The table
// of y evaluations has 2 * k + n entries where k is the size of the half kernel and n is the size
// of the profile being computed. Then for each of the n profile entries we walk out k steps in each
// horizontal direction multiplying the corresponding y evaluation by the half kernel entry and
// sum these values to compute the profile entry.
static uint8_t* create_profile(float sigma, float circleR, float offset, int profileTextureWidth) {
    const int numSteps = profileTextureWidth;
    uint8_t* weights = new uint8_t[numSteps];

    // The full kernel is 6 sigmas wide.
    int halfKernelSize = SkScalarCeilToInt(6.0f*sigma);
    // round up to next multiple of 2 and then divide by 2
    halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;

    // Number of x steps at which to apply kernel in y to cover all the profile samples in x.
    int numYSteps = numSteps + 2 * halfKernelSize;

    SkAutoTArray<float> bulkAlloc(halfKernelSize + halfKernelSize + numYSteps);
    float* halfKernel = bulkAlloc.get();
    float* summedKernel = bulkAlloc.get() + halfKernelSize;
    float* yEvals = bulkAlloc.get() + 2 * halfKernelSize;
    make_half_kernel_and_summed_table(halfKernel, summedKernel, halfKernelSize, sigma);

    float firstX = offset - halfKernelSize + 0.5f;
    apply_kernel_in_y(yEvals, numYSteps, firstX, circleR, halfKernelSize, summedKernel);

    for (int i = 0; i < numSteps - 1; ++i) {
        float evalX = offset + i + 0.5f;
        weights[i] = eval_at(evalX, circleR, halfKernel, halfKernelSize, yEvals + i);
    }
    // Ensure the tail of the Gaussian goes to zero.
    weights[numSteps - 1] = 0;
    return weights;
}

static int next_pow2_16bits(int x) {
    SkASSERT(x > 0);
    SkASSERT(x <= SK_MaxS16);
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x + 1;
}

GrTexture* GrCircleBlurFragmentProcessor::CreateCircleBlurProfileTexture(
                                                                GrTextureProvider* textureProvider,
                                                                const SkRect& circle,
                                                                float sigma,
                                                                float* solidRadius,
                                                                float* textureRadius) {
    float circleR = circle.width() / 2.0f;
    // Profile textures are cached by the ratio of sigma to circle radius and by the size of the
    // profile texture (binned by powers of 2).
    SkScalar sigmaToCircleRRatio = sigma / circleR;
    // When sigma is really small this becomes a equivalent to convolving a Gaussian with a half-
    // plane. We could do that simpler computation. However, right now we're just using a lower
    // bound off the ratio. Similarly, in the extreme high ratio cases circle becomes a point WRT to
    // the Guassian and the profile texture is a just a Gaussian evaluation.
    sigmaToCircleRRatio = SkTPin(sigmaToCircleRRatio, 0.05f, 8.f);
    // Convert to fixed point for the key.
    SkFixed sigmaToCircleRRatioFixed = SkScalarToFixed(sigmaToCircleRRatio);
    // We shave off some bits to reduce the number of unique entries. We could probably shave off
    // more than we do.
    sigmaToCircleRRatioFixed &= ~0xff;
    // From the circle center to solidRadius is all 1s and represented by the leftmost pixel (with
    // value 255) in the profile texture. If it is zero then there is no solid center to the
    // blurred circle.
    if (3*sigma <= circleR) {
        // The circle is bigger than the Gaussian. In this case we know the interior of the
        // blurred circle is solid.
        *solidRadius = circleR - 3 * sigma; // This location maps to 0.5f in the weights texture.
                                            // It should always be 255.
        *textureRadius = SkScalarCeilToScalar(6 * sigma);
    } else {
        // The Gaussian is bigger than the circle.
        *solidRadius = 0.0f;
        *textureRadius = SkScalarCeilToScalar(circleR + 3 * sigma);
    }
    int profileTextureWidth = SkScalarCeilToInt(*textureRadius);
    profileTextureWidth = (profileTextureWidth >= 1024) ? 1024 :
                          next_pow2_16bits(profileTextureWidth);

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 2);
    builder[0] = sigmaToCircleRRatioFixed;
    builder[1] = profileTextureWidth;
    builder.finish();

    GrTexture *blurProfile = textureProvider->findAndRefTextureByUniqueKey(key);

    if (!blurProfile) {
        GrSurfaceDesc texDesc;
        texDesc.fWidth = profileTextureWidth;
        texDesc.fHeight = 1;
        texDesc.fConfig = kAlpha_8_GrPixelConfig;

        // Rescale params to the size of the texture we're creating.
        SkScalar scale = profileTextureWidth / *textureRadius;
        SkAutoTDeleteArray<uint8_t> profile(create_profile(sigma * scale, circleR * scale,
                                                           *solidRadius * scale,
                                                           profileTextureWidth));

        blurProfile = textureProvider->createTexture(texDesc, SkBudgeted::kYes, profile.get(), 0);
        if (blurProfile) {
            textureProvider->assignUniqueKeyToTexture(key, blurProfile);
        }
    }

    return blurProfile;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrCircleBlurFragmentProcessor);

sk_sp<GrFragmentProcessor> GrCircleBlurFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    SkScalar wh = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar sigma = d->fRandom->nextRangeF(1.f,10.f);
    SkRect circle = SkRect::MakeWH(wh, wh);
    return GrCircleBlurFragmentProcessor::Make(d->fContext->textureProvider(), circle, sigma);
}

#endif
