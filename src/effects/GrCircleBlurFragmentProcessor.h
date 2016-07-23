/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCircleBlurFragmentProcessor_DEFINED
#define GrCircleBlurFragmentProcessor_DEFINED

#include "SkString.h"
#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrFragmentProcessor.h"
#include "GrProcessorUnitTest.h"

class GrTextureProvider;

// This FP handles the special case of a blurred circle. It uses a 1D
// profile that is just rotated about the origin of the circle.
class GrCircleBlurFragmentProcessor : public GrFragmentProcessor {
public:
    ~GrCircleBlurFragmentProcessor() override {};

    const char* name() const override { return "CircleBlur"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], Sigma %.2f, solidR: %.2f, "
                    "textureR: %.2f",
                    fCircle.fLeft, fCircle.fTop, fCircle.fRight, fCircle.fBottom,
                    fSigma, fSolidRadius, fTextureRadius);
        return str;
    }

    static sk_sp<GrFragmentProcessor> Make(GrTextureProvider*textureProvider,
                                           const SkRect& circle, float sigma) {
        float solidRadius;
        float textureRadius;

        SkAutoTUnref<GrTexture> profile(CreateCircleBlurProfileTexture(textureProvider,
                                                                       circle,
                                                                       sigma,
                                                                       &solidRadius,
                                                                       &textureRadius));
        if (!profile) {
           return nullptr;
        }
        return sk_sp<GrFragmentProcessor>(
            new GrCircleBlurFragmentProcessor(circle, sigma, solidRadius, textureRadius, profile));
    }

private:
    // This nested GLSL processor implementation is defined in the cpp file.
    class GLSLProcessor;

    /**
     * Creates a profile texture for the circle and sigma. The texture will have a height of 1.
     * The x texture coord should map from 0 to 1 across the radius range of solidRadius to
     * solidRadius + textureRadius.
     */
    GrCircleBlurFragmentProcessor(const SkRect& circle, float sigma,
                                  float solidRadius, float textureRadius, GrTexture* blurProfile);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const GrCircleBlurFragmentProcessor& cbfp = other.cast<GrCircleBlurFragmentProcessor>();
        // fOffset is computed from the circle width and the sigma
        return this->fCircle == cbfp.fCircle && fSigma == cbfp.fSigma;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    static GrTexture* CreateCircleBlurProfileTexture(GrTextureProvider*,
                                                     const SkRect& circle,
                                                     float sigma,
                                                     float* solidRadius,
                                                     float* textureRadius);

    SkRect              fCircle;
    float               fSigma;
    float               fSolidRadius;
    float               fTextureRadius;
    GrTextureAccess     fBlurProfileAccess;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
#endif
