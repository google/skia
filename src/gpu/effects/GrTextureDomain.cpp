/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomain.h"
#include "GrSimpleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "SkFloatingPoint.h"


GrTextureDomain::GrTextureDomain(const SkRect& domain, Mode mode, int index)
    : fIndex(index) {

    static const SkRect kFullRect = {0, 0, SK_Scalar1, SK_Scalar1};
    if (domain.contains(kFullRect) && kClamp_Mode == mode) {
        fMode = kIgnore_Mode;
    } else {
        fMode = mode;
    }

    if (fMode != kIgnore_Mode) {
        // We don't currently handle domains that are empty or don't intersect the texture.
        // It is OK if the domain rect is a line or point, but it should not be inverted. We do not
        // handle rects that do not intersect the [0..1]x[0..1] rect.
        SkASSERT(domain.fLeft <= domain.fRight);
        SkASSERT(domain.fTop <= domain.fBottom);
        fDomain.fLeft = SkMaxScalar(domain.fLeft, kFullRect.fLeft);
        fDomain.fRight = SkMinScalar(domain.fRight, kFullRect.fRight);
        fDomain.fTop = SkMaxScalar(domain.fTop, kFullRect.fTop);
        fDomain.fBottom = SkMinScalar(domain.fBottom, kFullRect.fBottom);
        SkASSERT(fDomain.fLeft <= fDomain.fRight);
        SkASSERT(fDomain.fTop <= fDomain.fBottom);
    }
}

//////////////////////////////////////////////////////////////////////////////

void GrTextureDomain::GLDomain::sampleTexture(GrGLShaderBuilder* builder,
                                              const GrTextureDomain& textureDomain,
                                              const char* outColor,
                                              const SkString& inCoords,
                                              const GrGLEffect::TextureSampler sampler,
                                              const char* inModulateColor) {
    SkASSERT((Mode)-1 == fMode || textureDomain.mode() == fMode);
    SkDEBUGCODE(fMode = textureDomain.mode();)

    if (kIgnore_Mode == textureDomain.mode()) {
        builder->fsCodeAppendf("\t%s = ", outColor);
        builder->fsAppendTextureLookupAndModulate(inModulateColor, sampler,
                                                    inCoords.c_str());
        builder->fsCodeAppend(";\n");
        return;
    }

    if (!fDomainUni.isValid()) {
        const char* name;
        SkString uniName("TexDom");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        fDomainUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec4f_GrSLType, uniName.c_str(), &name);
        fDomainName = name;
    }
    if (kClamp_Mode == textureDomain.mode()) {
        SkString clampedCoords;
        clampedCoords.appendf("\tclamp(%s, %s.xy, %s.zw)",
                                inCoords.c_str(), fDomainName.c_str(), fDomainName.c_str());

        builder->fsCodeAppendf("\t%s = ", outColor);
        builder->fsAppendTextureLookupAndModulate(inModulateColor, sampler, clampedCoords.c_str());
        builder->fsCodeAppend(";\n");
    } else {
        SkASSERT(GrTextureDomain::kDecal_Mode == textureDomain.mode());
        // Add a block since we're going to declare variables.
        GrGLShaderBuilder::FSBlock block(builder);

        const char* domain = fDomainName.c_str();
        if (kImagination_GrGLVendor == builder->ctxInfo().vendor()) {
            // On the NexusS and GalaxyNexus, the other path (with the 'any'
            // call) causes the compilation error "Calls to any function that
            // may require a gradient calculation inside a conditional block
            // may return undefined results". This appears to be an issue with
            // the 'any' call since even the simple "result=black; if (any())
            // result=white;" code fails to compile.
            builder->fsCodeAppend("\tvec4 outside = vec4(0.0, 0.0, 0.0, 0.0);\n");
            builder->fsCodeAppend("\tvec4 inside = ");
            builder->fsAppendTextureLookupAndModulate(inModulateColor, sampler, inCoords.c_str());
            builder->fsCodeAppend(";\n");

            builder->fsCodeAppendf("\tfloat x = abs(2.0*(%s.x - %s.x)/(%s.z - %s.x) - 1.0);\n",
                                    inCoords.c_str(), domain, domain, domain);
            builder->fsCodeAppendf("\tfloat y = abs(2.0*(%s.y - %s.y)/(%s.w - %s.y) - 1.0);\n",
                                    inCoords.c_str(), domain, domain, domain);
            builder->fsCodeAppend("\tfloat blend = step(1.0, max(x, y));\n");
            builder->fsCodeAppendf("\t%s = mix(inside, outside, blend);\n", outColor);
        } else {
            builder->fsCodeAppend("\tbvec4 outside;\n");
            builder->fsCodeAppendf("\toutside.xy = lessThan(%s, %s.xy);\n", inCoords.c_str(),
                                   domain);
            builder->fsCodeAppendf("\toutside.zw = greaterThan(%s, %s.zw);\n", inCoords.c_str(),
                                   domain);
            builder->fsCodeAppendf("\t%s = any(outside) ? vec4(0.0, 0.0, 0.0, 0.0) : ", outColor);
            builder->fsAppendTextureLookupAndModulate(inModulateColor, sampler, inCoords.c_str());
            builder->fsCodeAppend(";\n");
        }
    }
}

void GrTextureDomain::GLDomain::setData(const GrGLUniformManager& uman,
                                        const GrTextureDomain& textureDomain,
                                        GrSurfaceOrigin textureOrigin) {
    SkASSERT(textureDomain.mode() == fMode);
    if (kIgnore_Mode != textureDomain.mode()) {
        GrGLfloat values[4] = {
            SkScalarToFloat(textureDomain.domain().left()),
            SkScalarToFloat(textureDomain.domain().top()),
            SkScalarToFloat(textureDomain.domain().right()),
            SkScalarToFloat(textureDomain.domain().bottom())
        };
        // vertical flip if necessary
        if (kBottomLeft_GrSurfaceOrigin == textureOrigin) {
            values[1] = 1.0f - values[1];
            values[3] = 1.0f - values[3];
            // The top and bottom were just flipped, so correct the ordering
            // of elements so that values = (l, t, r, b).
            SkTSwap(values[1], values[3]);
        }
        if (0 != memcmp(values, fPrevDomain, 4 * sizeof(GrGLfloat))) {
            uman.set4fv(fDomainUni, 1, values);
            memcpy(fPrevDomain, values, 4 * sizeof(GrGLfloat));
        }
    }
}


//////////////////////////////////////////////////////////////////////////////

class GrGLTextureDomainEffect : public GrGLEffect {
public:
    GrGLTextureDomainEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

private:
    GrTextureDomain::GLDomain         fGLDomain;
    typedef GrGLEffect INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrBackendEffectFactory& factory,
                                                 const GrDrawEffect&)
    : INHERITED(factory) {
}

void GrGLTextureDomainEffect::emitCode(GrGLShaderBuilder* builder,
                                       const GrDrawEffect& drawEffect,
                                       const GrEffectKey& key,
                                       const char* outputColor,
                                       const char* inputColor,
                                       const TransformedCoordsArray& coords,
                                       const TextureSamplerArray& samplers) {
    const GrTextureDomainEffect& effect = drawEffect.castEffect<GrTextureDomainEffect>();
    const GrTextureDomain& domain = effect.textureDomain();

    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    fGLDomain.sampleTexture(builder, domain, outputColor, coords2D, samplers[0], inputColor);
}

void GrGLTextureDomainEffect::setData(const GrGLUniformManager& uman,
                                      const GrDrawEffect& drawEffect) {
    const GrTextureDomainEffect& effect = drawEffect.castEffect<GrTextureDomainEffect>();
    const GrTextureDomain& domain = effect.textureDomain();
    fGLDomain.setData(uman, domain, effect.texture(0)->origin());
}

void GrGLTextureDomainEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                                     GrEffectKeyBuilder* b) {
    const GrTextureDomain& domain = drawEffect.castEffect<GrTextureDomainEffect>().textureDomain();
    b->add32(GrTextureDomain::GLDomain::DomainKey(domain));
}


///////////////////////////////////////////////////////////////////////////////

GrEffect* GrTextureDomainEffect::Create(GrTexture* texture,
                                           const SkMatrix& matrix,
                                           const SkRect& domain,
                                           GrTextureDomain::Mode mode,
                                           GrTextureParams::FilterMode filterMode,
                                           GrCoordSet coordSet) {
    static const SkRect kFullRect = {0, 0, SK_Scalar1, SK_Scalar1};
    if (GrTextureDomain::kIgnore_Mode == mode ||
        (GrTextureDomain::kClamp_Mode == mode && domain.contains(kFullRect))) {
        return GrSimpleTextureEffect::Create(texture, matrix, filterMode);
    } else {

        return SkNEW_ARGS(GrTextureDomainEffect, (texture,
                                                  matrix,
                                                  domain,
                                                  mode,
                                                  filterMode,
                                                  coordSet));
    }
}

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture,
                                             const SkMatrix& matrix,
                                             const SkRect& domain,
                                             GrTextureDomain::Mode mode,
                                             GrTextureParams::FilterMode filterMode,
                                             GrCoordSet coordSet)
    : GrSingleTextureEffect(texture, matrix, filterMode, coordSet)
    , fTextureDomain(domain, mode) {
}

GrTextureDomainEffect::~GrTextureDomainEffect() {

}

const GrBackendEffectFactory& GrTextureDomainEffect::getFactory() const {
    return GrTBackendEffectFactory<GrTextureDomainEffect>::getInstance();
}

bool GrTextureDomainEffect::onIsEqual(const GrEffect& sBase) const {
    const GrTextureDomainEffect& s = CastEffect<GrTextureDomainEffect>(sBase);
    return this->hasSameTextureParamsMatrixAndSourceCoords(s) &&
           this->fTextureDomain == s.fTextureDomain;
}

void GrTextureDomainEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    if (GrTextureDomain::kDecal_Mode == fTextureDomain.mode()) { // TODO: helper
        *validFlags = 0;
    } else {
        this->updateConstantColorComponentsForModulation(color, validFlags);
    }
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrTextureDomainEffect);

GrEffect* GrTextureDomainEffect::TestCreate(SkRandom* random,
                                            GrContext*,
                                            const GrDrawTargetCaps&,
                                            GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    SkRect domain;
    domain.fLeft = random->nextUScalar1();
    domain.fRight = random->nextRangeScalar(domain.fLeft, SK_Scalar1);
    domain.fTop = random->nextUScalar1();
    domain.fBottom = random->nextRangeScalar(domain.fTop, SK_Scalar1);
    GrTextureDomain::Mode mode =
        (GrTextureDomain::Mode) random->nextULessThan(GrTextureDomain::kModeCount);
    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    bool bilerp = random->nextBool();
    GrCoordSet coords = random->nextBool() ? kLocal_GrCoordSet : kPosition_GrCoordSet;
    return GrTextureDomainEffect::Create(textures[texIdx],
                                         matrix,
                                         domain,
                                         mode,
                                         bilerp ? GrTextureParams::kBilerp_FilterMode : GrTextureParams::kNone_FilterMode,
                                         coords);
}
