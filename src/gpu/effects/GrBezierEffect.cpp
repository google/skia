/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBezierEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"

class GrGLConicEffect : public GrGLVertexEffect {
public:
    GrGLConicEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrEffectEdgeType fEdgeType;

    typedef GrGLVertexEffect INHERITED;
};

GrGLConicEffect::GrGLConicEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrConicEffect& ce = drawEffect.castEffect<GrConicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLConicEffect::emitCode(GrGLFullShaderBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               const GrEffectKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    builder->addVarying(kVec4f_GrSLType, "ConicCoeffs",
                              &vsName, &fsName);
    const SkString* attr0Name =
        builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    builder->vsCodeAppendf("\t%s = %s;\n", vsName, attr0Name->c_str());

    builder->fsCodeAppend("\t\tfloat edgeAlpha;\n");

    switch (fEdgeType) {
        case kHairlineAA_GrEffectEdgeType: {
            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
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
            builder->fsCodeAppendf("\t\tfloat func = %s.x*%s.x - %s.y*%s.z;\n", fsName, fsName,
                                   fsName, fsName);
            builder->fsCodeAppend("\t\tfunc = abs(func);\n");
            builder->fsCodeAppend("\t\tedgeAlpha = func / gFM;\n");
            builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
            break;
        }
        case kFillAA_GrEffectEdgeType: {
            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
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
            builder->fsCodeAppendf("\t\tfloat func = %s.x*%s.x - %s.y*%s.z;\n", fsName, fsName,
                                   fsName, fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = func / gFM;\n");
            builder->fsCodeAppend("\t\tedgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
            break;
        }
        case kFillBW_GrEffectEdgeType: {
            builder->fsCodeAppendf("\t\tedgeAlpha = %s.x*%s.x - %s.y*%s.z;\n", fsName, fsName,
                                   fsName, fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = float(edgeAlpha < 0.0);\n");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
}

void GrGLConicEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                             GrEffectKeyBuilder* b) {
    const GrConicEffect& ce = drawEffect.castEffect<GrConicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() {}

const GrBackendEffectFactory& GrConicEffect::getFactory() const {
    return GrTBackendEffectFactory<GrConicEffect>::getInstance();
}

GrConicEffect::GrConicEffect(GrEffectEdgeType edgeType) : GrVertexEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrConicEffect::onIsEqual(const GrEffect& other) const {
    const GrConicEffect& ce = CastEffect<GrConicEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrConicEffect);

GrEffect* GrConicEffect::TestCreate(SkRandom* random,
                                    GrContext*,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture*[]) {
    GrEffect* effect;
    do {
        GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(
                                                    random->nextULessThan(kGrEffectEdgeTypeCnt));
        effect = GrConicEffect::Create(edgeType, caps);
    } while (NULL == effect);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrGLQuadEffect : public GrGLVertexEffect {
public:
    GrGLQuadEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrEffectEdgeType fEdgeType;

    typedef GrGLVertexEffect INHERITED;
};

GrGLQuadEffect::GrGLQuadEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrQuadEffect& ce = drawEffect.castEffect<GrQuadEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLQuadEffect::emitCode(GrGLFullShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              const GrEffectKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    const SkString* attrName =
        builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    builder->fsCodeAppendf("\t\tfloat edgeAlpha;\n");

    builder->addVarying(kVec4f_GrSLType, "HairQuadEdge", &vsName, &fsName);

    switch (fEdgeType) {
        case kHairlineAA_GrEffectEdgeType: {
            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
            builder->fsCodeAppendf("\t\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                   "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                   fsName, fsName);
            builder->fsCodeAppendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                                   fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / dot(gF, gF));\n");
            builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
            break;
        }
        case kFillAA_GrEffectEdgeType: {
            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
            builder->fsCodeAppendf("\t\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                   "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                   fsName, fsName);
            builder->fsCodeAppendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                                   fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha / sqrt(dot(gF, gF));\n");
            builder->fsCodeAppend("\t\tedgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
            break;
        }
        case kFillBW_GrEffectEdgeType: {
            builder->fsCodeAppendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                                   fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = float(edgeAlpha < 0.0);\n");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());


    builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());
}

void GrGLQuadEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                            GrEffectKeyBuilder* b) {
    const GrQuadEffect& ce = drawEffect.castEffect<GrQuadEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() {}

const GrBackendEffectFactory& GrQuadEffect::getFactory() const {
    return GrTBackendEffectFactory<GrQuadEffect>::getInstance();
}

GrQuadEffect::GrQuadEffect(GrEffectEdgeType edgeType) : GrVertexEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrQuadEffect::onIsEqual(const GrEffect& other) const {
    const GrQuadEffect& ce = CastEffect<GrQuadEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrQuadEffect);

GrEffect* GrQuadEffect::TestCreate(SkRandom* random,
                                   GrContext*,
                                   const GrDrawTargetCaps& caps,
                                   GrTexture*[]) {
    GrEffect* effect;
    do {
        GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(
                                                    random->nextULessThan(kGrEffectEdgeTypeCnt));
        effect = GrQuadEffect::Create(edgeType, caps);
    } while (NULL == effect);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////
// Cubic
//////////////////////////////////////////////////////////////////////////////

class GrGLCubicEffect : public GrGLVertexEffect {
public:
    GrGLCubicEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrEffectEdgeType fEdgeType;

    typedef GrGLVertexEffect INHERITED;
};

GrGLCubicEffect::GrGLCubicEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrCubicEffect& ce = drawEffect.castEffect<GrCubicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLCubicEffect::emitCode(GrGLFullShaderBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               const GrEffectKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    builder->addVarying(kVec4f_GrSLType, "CubicCoeffs",
                              &vsName, &fsName);
    const SkString* attr0Name =
        builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    builder->vsCodeAppendf("\t%s = %s;\n", vsName, attr0Name->c_str());

    builder->fsCodeAppend("\t\tfloat edgeAlpha;\n");

    switch (fEdgeType) {
        case kHairlineAA_GrEffectEdgeType: {
            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
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
            builder->fsCodeAppendf("\t\tfloat func = %s.x*%s.x*%s.x - %s.y*%s.z;\n",
                                   fsName, fsName, fsName, fsName, fsName);
            builder->fsCodeAppend("\t\tfunc = abs(func);\n");
            builder->fsCodeAppend("\t\tedgeAlpha = func / gFM;\n");
            builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
            break;
        }
        case kFillAA_GrEffectEdgeType: {
            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
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
            builder->fsCodeAppendf("\t\tfloat func = %s.x*%s.x*%s.x - %s.y*%s.z;\n",
                                   fsName, fsName, fsName, fsName, fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = func / gFM;\n");
            builder->fsCodeAppend("\t\tedgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");
            break;
        }
        case kFillBW_GrEffectEdgeType: {
            builder->fsCodeAppendf("\t\tedgeAlpha = %s.x*%s.x*%s.x - %s.y*%s.z;\n",
                                   fsName, fsName, fsName, fsName, fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = float(edgeAlpha < 0.0);\n");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
}

void GrGLCubicEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                             GrEffectKeyBuilder* b) {
    const GrCubicEffect& ce = drawEffect.castEffect<GrCubicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrCubicEffect::~GrCubicEffect() {}

const GrBackendEffectFactory& GrCubicEffect::getFactory() const {
    return GrTBackendEffectFactory<GrCubicEffect>::getInstance();
}

GrCubicEffect::GrCubicEffect(GrEffectEdgeType edgeType) : GrVertexEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrCubicEffect::onIsEqual(const GrEffect& other) const {
    const GrCubicEffect& ce = CastEffect<GrCubicEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrCubicEffect);

GrEffect* GrCubicEffect::TestCreate(SkRandom* random,
                                    GrContext*,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture*[]) {
    GrEffect* effect;
    do {
        GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(
                                                    random->nextULessThan(kGrEffectEdgeTypeCnt));
        effect = GrCubicEffect::Create(edgeType, caps);
    } while (NULL == effect);
    return effect;
}
