/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConvexPolyEffect_DEFINED
#define GrConvexPolyEffect_DEFINED

#include "GrDrawTargetCaps.h"
#include "GrEffect.h"
#include "GrVertexEffect.h"

class GrGLConvexPolyEffect;
class SkPath;

/**
 * An effect that renders a convex polygon. It is intended to be used as a coverage effect.
 * Bounding geometry is rendered and the effect computes coverage based on the fragment's
 * position relative to the polygon.
 */
class GrConvexPolyEffect : public GrEffect {
public:
    /** This could be expanded to include a AA hairline mode. If so, unify with GrBezierEffect's
        enum. */
    enum EdgeType {
        kFillNoAA_EdgeType,
        kFillAA_EdgeType,

        kLastEdgeType = kFillAA_EdgeType,
    };

    enum {
        kEdgeTypeCnt = kLastEdgeType + 1,
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
    static GrEffectRef* Create(EdgeType edgeType, int n, const SkScalar edges[]) {
        if (n <= 0 || n > kMaxEdges) {
            return NULL;
        }
        return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrConvexPolyEffect,
                                                          (edgeType, n, edges))));
    }

    /**
     * Creates an effect that clips against the path. If the path is not a convex polygon, is
     * inverse filled, or has too many edges, this will return NULL. If offset is non-NULL, then
     * the path is translated by the vector.
     */
    static GrEffectRef* Create(EdgeType, const SkPath&, const SkVector* offset= NULL);

    /**
     * Creates an effect that fills inside the rect with AA edges..
     */
    static GrEffectRef* CreateForAAFillRect(const SkRect&);

    virtual ~GrConvexPolyEffect();

    static const char* Name() { return "ConvexPoly"; }

    EdgeType getEdgeType() const { return fEdgeType; }

    int getEdgeCount() const { return fEdgeCount; }

    const SkScalar* getEdges() const { return fEdges; }

    typedef GrGLConvexPolyEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrConvexPolyEffect(EdgeType edgeType, int n, const SkScalar edges[]);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    EdgeType fEdgeType;
    int      fEdgeCount;
    SkScalar fEdges[3 * kMaxEdges];

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};


#endif
