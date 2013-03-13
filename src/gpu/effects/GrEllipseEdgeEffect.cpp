/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrEllipseEdgeEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

class GrGLEllipseEdgeEffect : public GrGLEffect {
public:
    GrGLEllipseEdgeEffect(const GrBackendEffectFactory& factory, const GrEffectRef&)
    : INHERITED (factory) {}

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrEffectStage& stage,
                          EffectKey key,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const char *vsName, *fsName;
        builder->addVarying(kVec4f_GrSLType, "EllipseEdge", &vsName, &fsName);

        const SkString* attrName = builder->getEffectAttributeName(stage.getVertexAttribIndices()[0]);
        builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());

        builder->fsCodeAppend("\tfloat edgeAlpha;\n");
        // translate to origin
        builder->fsCodeAppendf("\tvec2 offset = (%s.xy - %s.xy);\n", builder->fragmentPosition(), fsName);
        // scale y by xRadius/yRadius
        builder->fsCodeAppendf("\toffset.y *= %s.w;\n", fsName);
        builder->fsCodeAppend("\tfloat d = length(offset);\n");
        // compare length against xRadius
        builder->fsCodeAppendf("\tedgeAlpha = smoothstep(d - 0.5, d + 0.5, %s.z);\n", fsName);
        SkString modulate;
        GrGLSLModulate4f(&modulate, inputColor, "edgeAlpha");
        builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
    }

    static inline EffectKey GenKey(const GrEffectStage& stage, const GrGLCaps&) {
        return 0;
    }

    virtual void setData(const GrGLUniformManager& uman, const GrEffectStage& stage) SK_OVERRIDE {
    }

private:
    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrEllipseEdgeEffect::GrEllipseEdgeEffect() : GrEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
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
                                               GrTexture* textures[]) {
    return GrEllipseEdgeEffect::Create();
}
