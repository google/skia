/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlurredEdgeFragmentProcessor_DEFINED
#define GrBlurredEdgeFragmentProcessor_DEFINED

#include "GrFragmentProcessor.h"

/**
 * Shader for managing a blurred edge for a shadow.
 *
 * There are two blurring modes supported: Gaussian blur function and smoothstep function.
 *
 * If the primitive supports an implicit distance to the edge, the radius of the blur is specified
 * by r & g values of the color in 14.2 fixed point. For spot shadows, we increase the stroke width
 * to set the shadow against the shape. This pad is specified by b, also in 6.2 fixed point.
 * The a value represents the max final alpha.
 *
 * When not using implicit distance, then b in the input color represents the input to the
 * blur function, and r the max final alpha.
 *
 */
class GrBlurredEdgeFP : public GrFragmentProcessor {
public:
    enum Mode {
        kGaussian_Mode,
        kSmoothstep_Mode,

        kLastMode = kSmoothstep_Mode
    };
    static const int kModeCnt = kLastMode + 1;

    static sk_sp<GrFragmentProcessor> Make(Mode mode = kGaussian_Mode) {
        return sk_sp<GrFragmentProcessor>(new GrBlurredEdgeFP(mode));
    }

    const char* name() const override { return "BlurredEdge"; }

    Mode mode() const { return fMode; }

private:
    GrBlurredEdgeFP(Mode mode)
        : INHERITED(kNone_OptimizationFlags)
        , fMode(mode) {
        // enable output of distance information for shape
        this->setWillUseDistanceVectorField();

        this->initClassID<GrBlurredEdgeFP>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    Mode   fMode;

    typedef GrFragmentProcessor INHERITED;
};

#endif
