/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrEllipseEdgeEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

class GrGLEllipseEdgeEffect : public GrGLEffect {
public:
    GrGLEllipseEdgeEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
    : INHERITED (factory) {}

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrEllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<GrEllipseEdgeEffect>();

        const char *vsCenterName, *fsCenterName;
        const char *vsEdgeName, *fsEdgeName;

        builder->addVarying(kVec2f_GrSLType, "EllipseCenter", &vsCenterName, &fsCenterName);
        const SkString* attr0Name =
            builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
        builder->vsCodeAppendf("\t%s = %s;\n", vsCenterName, attr0Name->c_str());

        builder->addVarying(kVec4f_GrSLType, "EllipseEdge", &vsEdgeName, &fsEdgeName);
        const SkString* attr1Name =
            builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
        builder->vsCodeAppendf("\t%s = %s;\n", vsEdgeName, attr1Name->c_str());

        // translate to origin
        builder->fsCodeAppendf("\tvec2 outerOffset = (%s.xy - %s.xy);\n",
                               builder->fragmentPosition(), fsCenterName);
        builder->fsCodeAppend("\tvec2 innerOffset = outerOffset;\n");
        // scale y by xRadius/yRadius
        builder->fsCodeAppendf("\touterOffset.y *= %s.y;\n", fsEdgeName);
        builder->fsCodeAppend("\tfloat dOuter = length(outerOffset);\n");
        // compare outer lengths against xOuterRadius
        builder->fsCodeAppendf("\tfloat edgeAlpha = clamp(%s.x-dOuter, 0.0, 1.0);\n", fsEdgeName);

        if (ellipseEffect.isStroked()) {
            builder->fsCodeAppendf("\tinnerOffset.y *= %s.w;\n", fsEdgeName);
            builder->fsCodeAppend("\tfloat dInner = length(innerOffset);\n");

            // compare inner lengths against xInnerRadius
            builder->fsCodeAppendf("\tfloat innerAlpha = clamp(dInner-%s.z, 0.0, 1.0);\n", fsEdgeName);
            builder->fsCodeAppend("\tedgeAlpha *= innerAlpha;\n");
        }

        SkString modulate;
        GrGLSLModulate4f(&modulate, inputColor, "edgeAlpha");
        builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
    }

    static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
        const GrEllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<GrEllipseEdgeEffect>();

        return ellipseEffect.isStroked() ? 0x1 : 0x0;
    }

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {
    }

private:
    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrEllipseEdgeEffect::GrEllipseEdgeEffect(bool stroke) : GrEffect() {
    this->addVertexAttrib(kVec2f_GrSLType);
    this->addVertexAttrib(kVec4f_GrSLType);

    fStroke = stroke;
}

void GrEllipseEdgeEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& GrEllipseEdgeEffect::getFactory() const {
    return GrTBackendEffectFactory<GrEllipseEdgeEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrEllipseEdgeEffect);

GrEffectRef* GrEllipseEdgeEffect::TestCreate(SkMWCRandom* random,
                                               GrContext* context,
                                               const GrDrawTargetCaps&,
                                               GrTexture* textures[]) {
    return GrEllipseEdgeEffect::Create(random->nextBool());
}
