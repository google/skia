/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCircleEdgeEffect_DEFINED
#define GrCircleEdgeEffect_DEFINED

#include "GrEffect.h"

class GrGLCircleEdgeEffect;

/**
 * The output of this effect is a modulation of the input color and coverage for a circle,
 * specified as center_x, center_y, x_radius, inner radius and outer radius in window space
 * (y-down).
 */

class GrCircleEdgeEffect : public GrEffect {
public:
    static GrEffectRef* Create(bool stroke) {
        // we go through this so we only have one copy of each effect (stroked/filled)
        static SkAutoTUnref<GrEffectRef> gCircleStrokeEdgeEffectRef(
                        CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrCircleEdgeEffect, (true)))));
        static SkAutoTUnref<GrEffectRef> gCircleFillEdgeEffectRef(
                        CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrCircleEdgeEffect, (false)))));

        if (stroke) {
            gCircleStrokeEdgeEffectRef.get()->ref();
            return gCircleStrokeEdgeEffectRef;
        } else {
            gCircleFillEdgeEffectRef.get()->ref();
            return gCircleFillEdgeEffectRef;
        }
    }

    virtual ~GrCircleEdgeEffect() {}

    static const char* Name() { return "CircleEdge"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLCircleEdgeEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    inline bool isStroked() const { return fStroke; }

private:
    GrCircleEdgeEffect(bool stroke);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const GrCircleEdgeEffect& cee = CastEffect<GrCircleEdgeEffect>(other);
        return cee.fStroke == fStroke;
    }

    bool fStroke;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

#endif
