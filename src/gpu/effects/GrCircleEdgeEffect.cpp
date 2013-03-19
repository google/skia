/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCircleEdgeEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

#include "SkRTConf.h"

class GrGLCircleEdgeEffect : public GrGLEffect {
public:
    GrGLCircleEdgeEffect(const GrBackendEffectFactory& factory, const GrEffectRef&)
    : INHERITED (factory) {}

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrEffectStage& stage,
                          EffectKey key,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrCircleEdgeEffect& effect = GetEffectFromStage<GrCircleEdgeEffect>(stage);
        
        const char *vsName, *fsName;
        builder->addVarying(kVec4f_GrSLType, "CircleEdge", &vsName, &fsName);

        const SkString* attrName =
            builder->getEffectAttributeName(stage.getVertexAttribIndices()[0]);
        builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());

        builder->fsCodeAppendf("\tfloat d = distance(%s.xy, %s.xy);\n",
                               builder->fragmentPosition(), fsName);
        builder->fsCodeAppendf("\tfloat edgeAlpha = clamp(%s.z - d, 0.0, 1.0);\n", fsName);
        if (effect.isStroked()) {
            builder->fsCodeAppendf("\tfloat innerAlpha = clamp(d - %s.w, 0.0, 1.0);\n", fsName);
            builder->fsCodeAppend("\tedgeAlpha *= innerAlpha;\n");
        }
        SkString modulate;
        GrGLSLModulate4f(&modulate, inputColor, "edgeAlpha");
        builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
    }

    static inline EffectKey GenKey(const GrEffectStage& stage, const GrGLCaps&) {
        const GrCircleEdgeEffect& effect = GetEffectFromStage<GrCircleEdgeEffect>(stage);

        return effect.isStroked() ? 0x1 : 0x0;
    }

    virtual void setData(const GrGLUniformManager& uman, const GrEffectStage& stage) SK_OVERRIDE {
    }

private:
    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrCircleEdgeEffect::GrCircleEdgeEffect(bool stroke) : GrEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fStroke = stroke;
}

void GrCircleEdgeEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& GrCircleEdgeEffect::getFactory() const {
    return GrTBackendEffectFactory<GrCircleEdgeEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrCircleEdgeEffect);

GrEffectRef* GrCircleEdgeEffect::TestCreate(SkMWCRandom* random,
                                            GrContext* context,
                                            GrTexture* textures[]) {
    return GrCircleEdgeEffect::Create(random->nextBool());
}
