/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConvexPolyEffect_DEFINED
#define GrConvexPolyEffect_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"

class GrInvariantOutput;
class SkPath;

/**
 * An effect that renders a convex polygon. It is intended to be used as a coverage effect.
 * Bounding geometry is rendered and the effect computes coverage based on the fragment's
 * position relative to the polygon.
 */
class GrConvexPolyEffect : public GrFragmentProcessor {
public:
    inline static constexpr int kMaxEdges = 8;

    /**
     * edges is a set of n edge equations where n is limited to kMaxEdges. It contains 3*n values.
     * The edges should form a convex polygon. The positive half-plane is considered to be the
     * inside. The equations should be normalized such that the first two coefficients are a unit
     * 2d vector.
     *
     * Currently the edges are specified in device space. In the future we may prefer to specify
     * them in src space. There are a number of ways this could be accomplished but we'd probably
     * have to modify the effect/shaderbuilder interface to make it possible (e.g. give access
     * to the view matrix or untransformed positions in the fragment shader).
     */
    static GrFPResult Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                           GrClipEdgeType edgeType,
                           int n,
                           const float edges[]) {
        if (n <= 0 || n > kMaxEdges) {
            return GrFPFailure(std::move(inputFP));
        }

        return GrFPSuccess(std::unique_ptr<GrFragmentProcessor>(
                    new GrConvexPolyEffect(std::move(inputFP), edgeType, n, edges)));
    }

    /**
     * Creates an effect that clips against the path. If the path is not a convex polygon, is
     * inverse filled, or has too many edges, creation will fail.
     */
    static GrFPResult Make(std::unique_ptr<GrFragmentProcessor>, GrClipEdgeType, const SkPath&);

    ~GrConvexPolyEffect() override;

    const char* name() const override { return "ConvexPoly"; }
    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    GrConvexPolyEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                       GrClipEdgeType edgeType,
                       int n, const SkScalar edges[]);
    GrConvexPolyEffect(const GrConvexPolyEffect&);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    GrClipEdgeType                 fEdgeType;
    int                            fEdgeCount;
    std::array<float, 3*kMaxEdges> fEdges;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};


#endif
