/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConvexPolyEffect_DEFINED
#define GrConvexPolyEffect_DEFINED

#include "GrCaps.h"
#include "GrFragmentProcessor.h"
#include "GrProcessor.h"
#include "GrTypesPriv.h"

class GrInvariantOutput;
class SkPath;

/**
 * An effect that renders a convex polygon. It is intended to be used as a coverage effect.
 * Bounding geometry is rendered and the effect computes coverage based on the fragment's
 * position relative to the polygon.
 */
class GrConvexPolyEffect : public GrFragmentProcessor {
public:
    enum {
        kMaxEdges = 8,
    };

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
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType edgeType, int n,
                                       const SkScalar edges[]) {
        if (n <= 0 || n > kMaxEdges || kHairlineAA_GrProcessorEdgeType == edgeType) {
            return nullptr;
        }
        return new GrConvexPolyEffect(edgeType, n, edges);
    }

    /**
     * Creates an effect that clips against the path. If the path is not a convex polygon, is
     * inverse filled, or has too many edges, this will return nullptr. If offset is non-nullptr, then
     * the path is translated by the vector.
     */
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkPath&,
                                       const SkVector* offset = nullptr);

    /**
     * Creates an effect that fills inside the rect with AA edges..
     */
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkRect&);

    virtual ~GrConvexPolyEffect();

    const char* name() const override { return "ConvexPoly"; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    int getEdgeCount() const { return fEdgeCount; }

    const SkScalar* getEdges() const { return fEdges; }

private:
    GrConvexPolyEffect(GrPrimitiveEdgeType edgeType, int n, const SkScalar edges[]);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GrPrimitiveEdgeType    fEdgeType;
    int                    fEdgeCount;
    SkScalar               fEdges[3 * kMaxEdges];

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};


#endif
