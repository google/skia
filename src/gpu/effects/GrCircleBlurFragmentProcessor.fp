/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in half4 circleRect;
in half textureRadius;
in half solidRadius;
in uniform sampler2D blurProfileSampler;

// The data is formatted as:
// x, y - the center of the circle
// z    - inner radius that should map to 0th entry in the texture.
// w    - the inverse of the distance over which the texture is stretched.
uniform half4 circleData;

@optimizationFlags {
    kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext*,
                                                     const SkRect& circle, float sigma);
}

@setData(data) {
    data.set4f(circleData, circleRect.centerX(), circleRect.centerY(), solidRadius,
               1.f / textureRadius);
}

@cpp {
    #include "include/gpu/GrContext.h"
    #include "include/private/GrRecordingContext.h"
    #include "src/gpu/GrBitmapTextureMaker.h"
    #include "src/gpu/GrProxyProvider.h"
    #include "src/gpu/GrRecordingContextPriv.h"

    // Computes an unnormalized half kernel (right side). Returns the summation of all the half
    // kernel values.
    static float make_unnormalized_half_kernel(float* halfKernel, int halfKernelSize, float sigma) {
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
        return tot;
    }

    // Create a Gaussian half-kernel (right side) and a summed area table given a sigma and number
    // of discrete steps. The half kernel is normalized to sum to 0.5.
    static void make_half_kernel_and_summed_table(float* halfKernel, float* summedHalfKernel,
                                                  int halfKernelSize, float sigma) {
        // The half kernel should sum to 0.5 not 1.0.
        const float tot = 2.f * make_unnormalized_half_kernel(halfKernel, halfKernelSize, sigma);
        float sum = 0.f;
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
        // Since we applied a half kernel in y we multiply acc by 2 (the circle is symmetric about
        // the x axis).
        return SkUnitScalarClampToByte(2.f * acc);
    }

    // This function creates a profile of a blurred circle. It does this by computing a kernel for
    // half the Gaussian and a matching summed area table. The summed area table is used to compute
    // an array of vertical applications of the half kernel to the circle along the x axis. The
    // table of y evaluations has 2 * k + n entries where k is the size of the half kernel and n is
    // the size of the profile being computed. Then for each of the n profile entries we walk out k
    // steps in each horizontal direction multiplying the corresponding y evaluation by the half
    // kernel entry and sum these values to compute the profile entry.
    static void create_circle_profile(uint8_t* weights, float sigma, float circleR,
                                      int profileTextureWidth) {
        const int numSteps = profileTextureWidth;

        // The full kernel is 6 sigmas wide.
        int halfKernelSize = SkScalarCeilToInt(6.0f * sigma);
        // round up to next multiple of 2 and then divide by 2
        halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;

        // Number of x steps at which to apply kernel in y to cover all the profile samples in x.
        int numYSteps = numSteps + 2 * halfKernelSize;

        SkAutoTArray<float> bulkAlloc(halfKernelSize + halfKernelSize + numYSteps);
        float* halfKernel = bulkAlloc.get();
        float* summedKernel = bulkAlloc.get() + halfKernelSize;
        float* yEvals = bulkAlloc.get() + 2 * halfKernelSize;
        make_half_kernel_and_summed_table(halfKernel, summedKernel, halfKernelSize, sigma);

        float firstX = -halfKernelSize + 0.5f;
        apply_kernel_in_y(yEvals, numYSteps, firstX, circleR, halfKernelSize, summedKernel);

        for (int i = 0; i < numSteps - 1; ++i) {
            float evalX = i + 0.5f;
            weights[i] = eval_at(evalX, circleR, halfKernel, halfKernelSize, yEvals + i);
        }
        // Ensure the tail of the Gaussian goes to zero.
        weights[numSteps - 1] = 0;
    }

    static void create_half_plane_profile(uint8_t* profile, int profileWidth) {
        SkASSERT(!(profileWidth & 0x1));
        // The full kernel is 6 sigmas wide.
        float sigma = profileWidth / 6.f;
        int halfKernelSize = profileWidth / 2;

        SkAutoTArray<float> halfKernel(halfKernelSize);

        // The half kernel should sum to 0.5.
        const float tot = 2.f * make_unnormalized_half_kernel(halfKernel.get(), halfKernelSize,
                                                              sigma);
        float sum = 0.f;
        // Populate the profile from the right edge to the middle.
        for (int i = 0; i < halfKernelSize; ++i) {
            halfKernel[halfKernelSize - i - 1] /= tot;
            sum += halfKernel[halfKernelSize - i - 1];
            profile[profileWidth - i - 1] = SkUnitScalarClampToByte(sum);
        }
        // Populate the profile from the middle to the left edge (by flipping the half kernel and
        // continuing the summation).
        for (int i = 0; i < halfKernelSize; ++i) {
            sum += halfKernel[i];
            profile[halfKernelSize - i - 1] = SkUnitScalarClampToByte(sum);
        }
        // Ensure tail goes to 0.
        profile[profileWidth - 1] = 0;
    }

    static GrSurfaceProxyView create_profile_texture(GrRecordingContext* context,
                                                     const SkRect& circle,
                                                     float sigma,
                                                     float* solidRadius, float* textureRadius) {
        float circleR = circle.width() / 2.0f;
        if (circleR < SK_ScalarNearlyZero) {
            return {};
        }
        // Profile textures are cached by the ratio of sigma to circle radius and by the size of the
        // profile texture (binned by powers of 2).
        SkScalar sigmaToCircleRRatio = sigma / circleR;
        // When sigma is really small this becomes a equivalent to convolving a Gaussian with a
        // half-plane. Similarly, in the extreme high ratio cases circle becomes a point WRT to the
        // Guassian and the profile texture is a just a Gaussian evaluation. However, we haven't yet
        // implemented this latter optimization.
        sigmaToCircleRRatio = std::min(sigmaToCircleRRatio, 8.f);
        SkFixed sigmaToCircleRRatioFixed;
        static const SkScalar kHalfPlaneThreshold = 0.1f;
        bool useHalfPlaneApprox = false;
        if (sigmaToCircleRRatio <= kHalfPlaneThreshold) {
            useHalfPlaneApprox = true;
            sigmaToCircleRRatioFixed = 0;
            *solidRadius = circleR - 3 * sigma;
            *textureRadius = 6 * sigma;
        } else {
            // Convert to fixed point for the key.
            sigmaToCircleRRatioFixed = SkScalarToFixed(sigmaToCircleRRatio);
            // We shave off some bits to reduce the number of unique entries. We could probably
            // shave off more than we do.
            sigmaToCircleRRatioFixed &= ~0xff;
            sigmaToCircleRRatio = SkFixedToScalar(sigmaToCircleRRatioFixed);
            sigma = circleR * sigmaToCircleRRatio;
            *solidRadius = 0;
            *textureRadius = circleR + 3 * sigma;
        }

        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 1, "1-D Circular Blur");
        builder[0] = sigmaToCircleRRatioFixed;
        builder.finish();

        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        if (sk_sp<GrTextureProxy> blurProfile = proxyProvider->findOrCreateProxyByUniqueKey(key)) {
            GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(blurProfile->backendFormat(),
                                                                       GrColorType::kAlpha_8);
            return {std::move(blurProfile), kTopLeft_GrSurfaceOrigin, swizzle};
        }

        static constexpr int kProfileTextureWidth = 512;

        SkBitmap bm;
        if (!bm.tryAllocPixels(SkImageInfo::MakeA8(kProfileTextureWidth, 1))) {
            return {};
        }

        if (useHalfPlaneApprox) {
            create_half_plane_profile(bm.getAddr8(0, 0), kProfileTextureWidth);
        } else {
            // Rescale params to the size of the texture we're creating.
            SkScalar scale = kProfileTextureWidth / *textureRadius;
            create_circle_profile(bm.getAddr8(0, 0), sigma * scale, circleR * scale,
                    kProfileTextureWidth);
        }

        bm.setImmutable();

        GrBitmapTextureMaker maker(context, bm, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
        auto blurView = maker.view(GrMipMapped::kNo);
        if (!blurView) {
            return {};
        }
        proxyProvider->assignUniqueKeyToProxy(key, blurView.asTextureProxy());
        return blurView;
    }

    std::unique_ptr<GrFragmentProcessor> GrCircleBlurFragmentProcessor::Make(
            GrRecordingContext* context, const SkRect& circle, float sigma) {
        float solidRadius;
        float textureRadius;
        GrSurfaceProxyView profile = create_profile_texture(context, circle, sigma,
                                                            &solidRadius, &textureRadius);
        if (!profile) {
            return nullptr;
        }
        return std::unique_ptr<GrFragmentProcessor>(new GrCircleBlurFragmentProcessor(
                circle, textureRadius, solidRadius, std::move(profile)));
    }
}

void main() {
    // We just want to compute "(length(vec) - circleData.z + 0.5) * circleData.w" but need to
    // rearrange for precision.
    half2 vec = half2(half((sk_FragCoord.x - circleData.x) * circleData.w),
                      half((sk_FragCoord.y - circleData.y) * circleData.w));
    half dist = length(vec) + (0.5 - circleData.z) * circleData.w;
    sk_OutColor = sk_InColor * sample(blurProfileSampler, half2(dist, 0.5)).a;
}

@test(testData) {
    SkScalar wh = testData->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar sigma = testData->fRandom->nextRangeF(1.f,10.f);
    SkRect circle = SkRect::MakeWH(wh, wh);
    return GrCircleBlurFragmentProcessor::Make(testData->context(), circle, sigma);
}
