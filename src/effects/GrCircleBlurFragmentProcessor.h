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

class GrResourceProvider;

// This FP handles the special case of a blurred circle. It uses a 1D
// profile that is just rotated about the origin of the circle.
class GrCircleBlurFragmentProcessor : public GrFragmentProcessor {
public:
    ~GrCircleBlurFragmentProcessor() override {}

    const char* name() const override { return "CircleBlur"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], solidR: %.2f, textureR: %.2f",
                    fCircle.fLeft, fCircle.fTop, fCircle.fRight, fCircle.fBottom,
                    fSolidRadius, fTextureRadius);
        return str;
    }

    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider*, const SkRect& circle, float sigma);

private:
    // This nested GLSL processor implementation is defined in the cpp file.
    class GLSLProcessor;

    /**
     * Creates a profile texture for the circle and sigma. The texture will have a height of 1.
     * The x texture coord should map from 0 to 1 across the radius range of solidRadius to
     * solidRadius + textureRadius.
     */
    GrCircleBlurFragmentProcessor(GrResourceProvider*, const SkRect& circle,
                                  float textureRadius, float innerRadius,
                                  sk_sp<GrTextureProxy> blurProfile);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const GrCircleBlurFragmentProcessor& cbfp = other.cast<GrCircleBlurFragmentProcessor>();
        return fCircle == cbfp.fCircle && fSolidRadius == cbfp.fSolidRadius &&
               fTextureRadius == cbfp.fTextureRadius;
    }

    SkRect              fCircle;
    SkScalar            fSolidRadius;
    float               fTextureRadius;
    TextureSampler      fBlurProfileSampler;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
#endif
