/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrEllipseEdgeEffect_DEFINED
#define GrEllipseEdgeEffect_DEFINED

#include "GrEffect.h"

class GrGLEllipseEdgeEffect;

/**
 * The output of this effect is a modulation of the input color and coverage for an axis-aligned
 * ellipse, specified as center_x, center_y, x_radius, x_radius/y_radius in window space (y-down).
 */

class GrEllipseEdgeEffect : public GrEffect {
public:
    static GrEffectRef* Create() {
        // maybe only have one static copy?
        AutoEffectUnref effect(SkNEW(GrEllipseEdgeEffect));
        return CreateEffectRef(effect);
    }

    virtual ~GrEllipseEdgeEffect() {}

    static const char* Name() { return "EllipseEdge"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLEllipseEdgeEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrEllipseEdgeEffect();

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        return true;
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

#endif
