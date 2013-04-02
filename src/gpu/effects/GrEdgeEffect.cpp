/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrEdgeEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

class GrGLEdgeEffect : public GrGLEffect {
public:
    GrGLEdgeEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
    : INHERITED (factory) {}

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrEdgeEffect& edgeEffect = drawEffect.castEffect<GrEdgeEffect>();
        GrEdgeEffect::EdgeType type = edgeEffect.edgeType();

        const char *vsName, *fsName;
        const SkString* attrName =
            builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
        builder->fsCodeAppendf("\t\tfloat edgeAlpha;\n");

        switch (type) {
        case GrEdgeEffect::kHairLine_EdgeType:
            builder->addVarying(kVec4f_GrSLType, "HairEdge", &vsName, &fsName);

            builder->fsCodeAppendf("\t\tedgeAlpha = abs(dot(vec3(%s.xy,1), %s.xyz));\n",
                                   builder->fragmentPosition(), fsName);
            builder->fsCodeAppendf("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            break;
        case GrEdgeEffect::kQuad_EdgeType:
            SkAssertResult(builder->enableFeature(GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
            builder->addVarying(kVec4f_GrSLType, "QuadEdge", &vsName, &fsName);

            // keep the derivative instructions outside the conditional
            builder->fsCodeAppendf("\t\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tif (%s.z > 0.0 && %s.w > 0.0) {\n", fsName, fsName);
            // today we know z and w are in device space. We could use derivatives
            builder->fsCodeAppendf("\t\t\tedgeAlpha = min(min(%s.z, %s.w) + 0.5, 1.0);\n", fsName,
                                   fsName);
            builder->fsCodeAppendf ("\t\t} else {\n");
            builder->fsCodeAppendf("\t\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                   "\t\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                    fsName, fsName);
            builder->fsCodeAppendf("\t\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                                   fsName);
            builder->fsCodeAppendf("\t\t\tedgeAlpha = "
                                   "clamp(0.5 - edgeAlpha / length(gF), 0.0, 1.0);\n\t\t}\n");
            break;
        case GrEdgeEffect::kHairQuad_EdgeType:
            SkAssertResult(builder->enableFeature(GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
            builder->addVarying(kVec4f_GrSLType, "HairQuadEdge", &vsName, &fsName);

            builder->fsCodeAppendf("\t\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                   "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                   fsName, fsName);
            builder->fsCodeAppendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                                   fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / dot(gF, gF));\n");
            builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            break;
        };

        SkString modulate;
        GrGLSLModulate4f(&modulate, inputColor, "edgeAlpha");
        builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());

        builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());
    }

    static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
        const GrEdgeEffect& QuadEffect = drawEffect.castEffect<GrEdgeEffect>();

        return QuadEffect.edgeType();
    }

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {
    }

private:
    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrEdgeEffect::GrEdgeEffect(EdgeType edgeType) : GrEffect() {
    if (edgeType == kQuad_EdgeType) {
        this->addVertexAttrib(kVec4f_GrSLType);
    } else {
        this->addVertexAttrib(kVec4f_GrSLType);  // TODO: use different vec sizes for differnt edge
                                                 // types.
    }
    fEdgeType = edgeType;
}

void GrEdgeEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& GrEdgeEffect::getFactory() const {
    return GrTBackendEffectFactory<GrEdgeEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrEdgeEffect);

GrEffectRef* GrEdgeEffect::TestCreate(SkMWCRandom* random,
                                      GrContext*,
                                      const GrDrawTargetCaps& caps,
                                      GrTexture*[]) {
    // Only kHairLine works without derivative instructions.
    EdgeType edgeType;
    if (caps.shaderDerivativeSupport()) {
        edgeType = static_cast<EdgeType>(random->nextULessThan(kEdgeTypeCount));
    } else {
        edgeType = kHairLine_EdgeType;
    }
    return GrEdgeEffect::Create(edgeType);
}
