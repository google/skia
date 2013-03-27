/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrEdgeEffect_DEFINED
#define GrEdgeEffect_DEFINED

#include "GrEffect.h"

class GrGLEdgeEffect;

/**
 * The output of this effect is one of three different edge types: hairlines, quads,
 * and hairline quads.
 */

class GrEdgeEffect : public GrEffect {
public:
    enum EdgeType {
        /* 1-pixel wide line
           2D implicit device coord line eq (a*x + b*y +c = 0). 4th component unused. */
        kHairLine_EdgeType = 0,
        /* Quadratic specified by 0=u^2-v canonical coords. u and v are the first
           two components of the vertex attribute. Coverage is based on signed
           distance with negative being inside, positive outside. The edge is specified in
           window space (y-down). If either the third or fourth component of the interpolated
           vertex coord is > 0 then the pixel is considered outside the edge. This is used to
           attempt to trim to a portion of the infinite quad. Requires shader derivative
           instruction support. */
        kQuad_EdgeType,
        /* Similar to above but for hairline quadratics. Uses unsigned distance.
           Coverage is min(0, 1-distance). 3rd & 4th component unused. Requires
           shader derivative instruction support. */
        kHairQuad_EdgeType,

        kLast_EdgeType = kHairQuad_EdgeType
    };
    static const int kEdgeTypeCount = kLast_EdgeType + 1;

    static GrEffectRef* Create(EdgeType type) {
        // we go through this so we only have one copy of each effect
        static GrEffectRef* gEdgeEffectRef[kEdgeTypeCount] = {
            CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrEdgeEffect, (kHairLine_EdgeType)))),
            CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrEdgeEffect, (kQuad_EdgeType)))),
            CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrEdgeEffect, (kHairQuad_EdgeType)))),
        };
        static SkAutoTUnref<GrEffectRef> gUnref0(gEdgeEffectRef[0]);
        static SkAutoTUnref<GrEffectRef> gUnref1(gEdgeEffectRef[1]);
        static SkAutoTUnref<GrEffectRef> gUnref2(gEdgeEffectRef[2]);

        gEdgeEffectRef[type]->ref();
        return gEdgeEffectRef[type];
    }

    virtual ~GrEdgeEffect() {}

    static const char* Name() { return "Edge"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLEdgeEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    EdgeType edgeType() const { return fEdgeType; }

private:
    GrEdgeEffect(EdgeType edgeType);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const GrEdgeEffect& qee = CastEffect<GrEdgeEffect>(other);
        return qee.fEdgeType == fEdgeType;
    }

    EdgeType fEdgeType;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

#endif
