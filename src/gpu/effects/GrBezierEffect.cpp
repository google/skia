/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBezierEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

class GrGLConicEffect : public GrGLEffect {
public:
    GrGLConicEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrBezierEdgeType fEdgeType;

    typedef GrGLEffect INHERITED;
};
    
GrGLConicEffect::GrGLConicEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrConicEffect& ce = drawEffect.castEffect<GrConicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLConicEffect::emitCode(GrGLShaderBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               EffectKey key,
                               const char* outputColor,
                               const char* inputColor,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    const bool antiAlias = GrBezierEdgeTypeIsAA(fEdgeType);
    const bool fill = GrBezierEdgeTypeIsFill(fEdgeType);

    if (antiAlias) {
        SkAssertResult(builder->enableFeature(
                GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
    }
    builder->addVarying(kVec4f_GrSLType, "ConicCoeffs",
                        &vsName, &fsName);
    const SkString* attr0Name =
        builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    builder->vsCodeAppendf("\t%s = %s;\n", vsName, attr0Name->c_str());

    builder->fsCodeAppend("\t\tfloat edgeAlpha;\n");

    if (antiAlias) {
        builder->fsCodeAppendf("\t\tvec3 dklmdx = dFdx(%s.xyz);\n", fsName);
        builder->fsCodeAppendf("\t\tvec3 dklmdy = dFdy(%s.xyz);\n", fsName);
        builder->fsCodeAppendf("\t\tfloat dfdx =\n"
                               "\t\t\t2.0*%s.x*dklmdx.x - %s.y*dklmdx.z - %s.z*dklmdx.y;\n",
                               fsName, fsName, fsName);
        builder->fsCodeAppendf("\t\tfloat dfdy =\n"
                               "\t\t\t2.0*%s.x*dklmdy.x - %s.y*dklmdy.z - %s.z*dklmdy.y;\n",
                               fsName, fsName, fsName);
        builder->fsCodeAppend("\t\tvec2 gF = vec2(dfdx, dfdy);\n");
        builder->fsCodeAppend("\t\tfloat gFM = sqrt(dot(gF, gF));\n");
    }
    builder->fsCodeAppendf("\t\tfloat func = %s.x*%s.x - %s.y*%s.z;\n", fsName, fsName,
                           fsName, fsName);
    if (!fill) {
        builder->fsCodeAppend("\t\tfunc = abs(func);\n");
    }
    if (antiAlias) {
        builder->fsCodeAppend("\t\tedgeAlpha = func / gFM;\n");
    }
    if (fill) {
        if (antiAlias) {
            builder->fsCodeAppend("\t\tedgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
        } else {
            builder->fsCodeAppend("\t\tedgeAlpha = float(func < 0.0);\n");
        }
    } else {
        builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
        // Add line below for smooth cubic ramp
        // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
    }

    SkString modulate;
    GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
    builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
}

GrGLEffect::EffectKey GrGLConicEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
    const GrConicEffect& ce = drawEffect.castEffect<GrConicEffect>();
    return ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() {}

const GrBackendEffectFactory& GrConicEffect::getFactory() const {
    return GrTBackendEffectFactory<GrConicEffect>::getInstance();
}

GrConicEffect::GrConicEffect(GrBezierEdgeType edgeType) : GrEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrConicEffect::onIsEqual(const GrEffect& other) const {
    const GrConicEffect& ce = CastEffect<GrConicEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrConicEffect);

GrEffectRef* GrConicEffect::TestCreate(SkMWCRandom* random,
                                             GrContext*,
                                             const GrDrawTargetCaps& caps,
                                             GrTexture*[]) {
    const GrBezierEdgeType edgeType = static_cast<GrBezierEdgeType>(random->nextULessThan(3));
    return GrConicEffect::Create(edgeType, caps);
}

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrGLQuadEffect : public GrGLEffect {
public:
    GrGLQuadEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrBezierEdgeType fEdgeType;

    typedef GrGLEffect INHERITED;
};
    
GrGLQuadEffect::GrGLQuadEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrQuadEffect& ce = drawEffect.castEffect<GrQuadEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLQuadEffect::emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    bool antiAlias = GrBezierEdgeTypeIsAA(fEdgeType);
    bool fill = GrBezierEdgeTypeIsFill(fEdgeType);

    const SkString* attrName =
        builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    builder->fsCodeAppendf("\t\tfloat edgeAlpha;\n");

    if (antiAlias) {
        SkAssertResult(builder->enableFeature(
                GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
    }
    builder->addVarying(kVec4f_GrSLType, "HairQuadEdge", &vsName, &fsName);

    if (antiAlias) {
        builder->fsCodeAppendf("\t\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
        builder->fsCodeAppendf("\t\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
        builder->fsCodeAppendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                               "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                               fsName, fsName);
    }
    builder->fsCodeAppendf("\t\tfloat func = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                           fsName);
    if (!fill) {
        builder->fsCodeAppend("\t\tfunc = abs(func);\n");
    }
    if (antiAlias) {
        builder->fsCodeAppend("\t\tedgeAlpha = func / sqrt(dot(gF, gF));\n");
    }
    if (fill) {
        if (antiAlias) {
            builder->fsCodeAppend("\t\tedgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
        } else {
            builder->fsCodeAppend("\t\tedgeAlpha = float(func < 0.0);\n");
        }
    } else {
        builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
        // Add line below for smooth cubic ramp
        // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
    }

    SkString modulate;
    GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
    builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());

    builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());
}

GrGLEffect::EffectKey GrGLQuadEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
    const GrQuadEffect& ce = drawEffect.castEffect<GrQuadEffect>();
    return ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() {}

const GrBackendEffectFactory& GrQuadEffect::getFactory() const {
    return GrTBackendEffectFactory<GrQuadEffect>::getInstance();
}

GrQuadEffect::GrQuadEffect(GrBezierEdgeType edgeType) : GrEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrQuadEffect::onIsEqual(const GrEffect& other) const {
    const GrQuadEffect& ce = CastEffect<GrQuadEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrQuadEffect);

GrEffectRef* GrQuadEffect::TestCreate(SkMWCRandom* random,
                                             GrContext*,
                                             const GrDrawTargetCaps& caps,
                                             GrTexture*[]) {
    const GrBezierEdgeType edgeType = static_cast<GrBezierEdgeType>(random->nextULessThan(3));
    return GrQuadEffect::Create(edgeType, caps);
}

//////////////////////////////////////////////////////////////////////////////
// Cubic
//////////////////////////////////////////////////////////////////////////////

class GrGLCubicEffect : public GrGLEffect {
public:
    GrGLCubicEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrBezierEdgeType fEdgeType;

    typedef GrGLEffect INHERITED;
};
    
GrGLCubicEffect::GrGLCubicEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrCubicEffect& ce = drawEffect.castEffect<GrCubicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLCubicEffect::emitCode(GrGLShaderBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               EffectKey key,
                               const char* outputColor,
                               const char* inputColor,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    bool antiAlias = GrBezierEdgeTypeIsAA(fEdgeType);
    bool fill = GrBezierEdgeTypeIsFill(fEdgeType);

    if (antiAlias) {
        SkAssertResult(builder->enableFeature(
                GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
    }
    builder->addVarying(kVec4f_GrSLType, "CubicCoeffs",
                        &vsName, &fsName);
    const SkString* attr0Name =
        builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    builder->vsCodeAppendf("\t%s = %s;\n", vsName, attr0Name->c_str());

    builder->fsCodeAppend("\t\tfloat edgeAlpha;\n");

    if (antiAlias) {
    builder->fsCodeAppendf("\t\tvec3 dklmdx = dFdx(%s.xyz);\n", fsName);
    builder->fsCodeAppendf("\t\tvec3 dklmdy = dFdy(%s.xyz);\n", fsName);
    builder->fsCodeAppendf("\t\tfloat dfdx =\n"
                           "\t\t3.0*%s.x*%s.x*dklmdx.x - %s.y*dklmdx.z - %s.z*dklmdx.y;\n",
                           fsName, fsName, fsName, fsName);
    builder->fsCodeAppendf("\t\tfloat dfdy =\n"
                           "\t\t3.0*%s.x*%s.x*dklmdy.x - %s.y*dklmdy.z - %s.z*dklmdy.y;\n",
                           fsName, fsName, fsName, fsName);
    builder->fsCodeAppend("\t\tvec2 gF = vec2(dfdx, dfdy);\n");
    builder->fsCodeAppend("\t\tfloat gFM = sqrt(dot(gF, gF));\n");
    }
    builder->fsCodeAppendf("\t\tfloat func = %s.x*%s.x*%s.x - %s.y*%s.z;\n",
                           fsName, fsName, fsName, fsName, fsName);
    if (!fill) {
        builder->fsCodeAppend("\t\tfunc = abs(func);\n");
    }
    if (antiAlias) {
        builder->fsCodeAppend("\t\tedgeAlpha = func / gFM;\n");
    }
    if (fill) {
        if (antiAlias) {
            builder->fsCodeAppend("\t\tedgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
        } else {
            builder->fsCodeAppend("\t\tedgeAlpha = float(func < 0.0);\n");
        }
    } else {
        builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
        // Add line below for smooth cubic ramp
        // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
    }

    SkString modulate;
    GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
    builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
}

GrGLEffect::EffectKey GrGLCubicEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
    const GrCubicEffect& ce = drawEffect.castEffect<GrCubicEffect>();
    return ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
}

//////////////////////////////////////////////////////////////////////////////

GrCubicEffect::~GrCubicEffect() {}

const GrBackendEffectFactory& GrCubicEffect::getFactory() const {
    return GrTBackendEffectFactory<GrCubicEffect>::getInstance();
}

GrCubicEffect::GrCubicEffect(GrBezierEdgeType edgeType) : GrEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrCubicEffect::onIsEqual(const GrEffect& other) const {
    const GrCubicEffect& ce = CastEffect<GrCubicEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrCubicEffect);

GrEffectRef* GrCubicEffect::TestCreate(SkMWCRandom* random,
                                             GrContext*,
                                             const GrDrawTargetCaps& caps,
                                             GrTexture*[]) {
    const GrBezierEdgeType edgeType = static_cast<GrBezierEdgeType>(random->nextULessThan(3));
    return GrCubicEffect::Create(edgeType, caps);
}
