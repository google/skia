/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrTextureDomain.h"
#include "GrSimpleTextureEffect.h"
#include "GrTBackendProcessorFactory.h"
#include "gl/GrGLProcessor.h"
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
                                              const GrGLProcessor::TextureSampler sampler,
                                              const char* inModulateColor) {
    SkASSERT((Mode)-1 == fMode || textureDomain.mode() == fMode);
    SkDEBUGCODE(fMode = textureDomain.mode();)

    GrGLProgramBuilder* program = builder->getProgramBuilder();

    if (textureDomain.mode() != kIgnore_Mode && !fDomainUni.isValid()) {
        const char* name;
        SkString uniName("TexDom");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        fDomainUni = program->addUniform(GrGLProgramBuilder::kFragment_Visibility, kVec4f_GrSLType,
                uniName.c_str(), &name);
        fDomainName = name;
    }

    switch (textureDomain.mode()) {
        case kIgnore_Mode: {
            builder->codeAppendf("\t%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                      inCoords.c_str());
            builder->codeAppend(";\n");
            break;
        }
        case kClamp_Mode: {
            SkString clampedCoords;
            clampedCoords.appendf("\tclamp(%s, %s.xy, %s.zw)",
                                  inCoords.c_str(), fDomainName.c_str(), fDomainName.c_str());

            builder->codeAppendf("\t%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                      clampedCoords.c_str());
            builder->codeAppend(";\n");
            break;
        }
        case kDecal_Mode: {
            // Add a block since we're going to declare variables.
            GrGLShaderBuilder::ShaderBlock block(builder);

            const char* domain = fDomainName.c_str();
            if (kImagination_GrGLVendor == program->ctxInfo().vendor()) {
                // On the NexusS and GalaxyNexus, the other path (with the 'any'
                // call) causes the compilation error "Calls to any function that
                // may require a gradient calculation inside a conditional block
                // may return undefined results". This appears to be an issue with
                // the 'any' call since even the simple "result=black; if (any())
                // result=white;" code fails to compile.
                builder->codeAppend("\tvec4 outside = vec4(0.0, 0.0, 0.0, 0.0);\n");
                builder->codeAppend("\tvec4 inside = ");
                builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                          inCoords.c_str());
                builder->codeAppend(";\n");
                builder->codeAppendf("\tfloat x = (%s).x;\n", inCoords.c_str());
                builder->codeAppendf("\tfloat y = (%s).y;\n", inCoords.c_str());

                builder->codeAppendf("\tx = abs(2.0*(x - %s.x)/(%s.z - %s.x) - 1.0);\n",
                                       domain, domain, domain);
                builder->codeAppendf("\ty = abs(2.0*(y - %s.y)/(%s.w - %s.y) - 1.0);\n",
                                       domain, domain, domain);
                builder->codeAppend("\tfloat blend = step(1.0, max(x, y));\n");
                builder->codeAppendf("\t%s = mix(inside, outside, blend);\n", outColor);
            } else {
                builder->codeAppend("\tbvec4 outside;\n");
                builder->codeAppendf("\toutside.xy = lessThan(%s, %s.xy);\n", inCoords.c_str(),
                                       domain);
                builder->codeAppendf("\toutside.zw = greaterThan(%s, %s.zw);\n", inCoords.c_str(),
                                       domain);
                builder->codeAppendf("\t%s = any(outside) ? vec4(0.0, 0.0, 0.0, 0.0) : ",
                                       outColor);
                builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                          inCoords.c_str());
                builder->codeAppend(";\n");
            }
            break;
        }
        case kRepeat_Mode: {
            SkString clampedCoords;
            clampedCoords.printf("\tmod(%s - %s.xy, %s.zw - %s.xy) + %s.xy",
                                 inCoords.c_str(), fDomainName.c_str(), fDomainName.c_str(),
                                 fDomainName.c_str(), fDomainName.c_str());

            builder->codeAppendf("\t%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                      clampedCoords.c_str());
            builder->codeAppend(";\n");
            break;
        }
    }
}

void GrTextureDomain::GLDomain::setData(const GrGLProgramDataManager& pdman,
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
            pdman.set4fv(fDomainUni, 1, values);
            memcpy(fPrevDomain, values, 4 * sizeof(GrGLfloat));
        }
    }
}


//////////////////////////////////////////////////////////////////////////////

class GrGLTextureDomainEffect : public GrGLFragmentProcessor {
public:
    GrGLTextureDomainEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder*,
                          const GrFragmentProcessor&,
                          const GrProcessorKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

private:
    GrTextureDomain::GLDomain         fGLDomain;
    typedef GrGLFragmentProcessor INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrBackendProcessorFactory& factory,
                                                 const GrProcessor&)
    : INHERITED(factory) {
}

void GrGLTextureDomainEffect::emitCode(GrGLProgramBuilder* builder,
                                       const GrFragmentProcessor& fp,
                                       const GrProcessorKey& key,
                                       const char* outputColor,
                                       const char* inputColor,
                                       const TransformedCoordsArray& coords,
                                       const TextureSamplerArray& samplers) {
    const GrTextureDomainEffect& textureDomainEffect = fp.cast<GrTextureDomainEffect>();
    const GrTextureDomain& domain = textureDomainEffect.textureDomain();

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);
    fGLDomain.sampleTexture(fsBuilder, domain, outputColor, coords2D, samplers[0], inputColor);
}

void GrGLTextureDomainEffect::setData(const GrGLProgramDataManager& pdman,
                                      const GrProcessor& processor) {
    const GrTextureDomainEffect& textureDomainEffect = processor.cast<GrTextureDomainEffect>();
    const GrTextureDomain& domain = textureDomainEffect.textureDomain();
    fGLDomain.setData(pdman, domain, processor.texture(0)->origin());
}

void GrGLTextureDomainEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                                     GrProcessorKeyBuilder* b) {
    const GrTextureDomain& domain = processor.cast<GrTextureDomainEffect>().textureDomain();
    b->add32(GrTextureDomain::GLDomain::DomainKey(domain));
}


///////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor* GrTextureDomainEffect::Create(GrTexture* texture,
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
    SkASSERT(mode != GrTextureDomain::kRepeat_Mode ||
            filterMode == GrTextureParams::kNone_FilterMode);
}

GrTextureDomainEffect::~GrTextureDomainEffect() {

}

const GrBackendFragmentProcessorFactory& GrTextureDomainEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrTextureDomainEffect>::getInstance();
}

bool GrTextureDomainEffect::onIsEqual(const GrProcessor& sBase) const {
    const GrTextureDomainEffect& s = sBase.cast<GrTextureDomainEffect>();
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

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTextureDomainEffect);

GrFragmentProcessor* GrTextureDomainEffect::TestCreate(SkRandom* random,
                                                       GrContext*,
                                                       const GrDrawTargetCaps&,
                                                       GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    SkRect domain;
    domain.fLeft = random->nextUScalar1();
    domain.fRight = random->nextRangeScalar(domain.fLeft, SK_Scalar1);
    domain.fTop = random->nextUScalar1();
    domain.fBottom = random->nextRangeScalar(domain.fTop, SK_Scalar1);
    GrTextureDomain::Mode mode =
        (GrTextureDomain::Mode) random->nextULessThan(GrTextureDomain::kModeCount);
    const SkMatrix& matrix = GrProcessorUnitTest::TestMatrix(random);
    bool bilerp = mode != GrTextureDomain::kRepeat_Mode ? random->nextBool() : false;
    GrCoordSet coords = random->nextBool() ? kLocal_GrCoordSet : kPosition_GrCoordSet;
    return GrTextureDomainEffect::Create(textures[texIdx],
                                         matrix,
                                         domain,
                                         mode,
                                         bilerp ? GrTextureParams::kBilerp_FilterMode : GrTextureParams::kNone_FilterMode,
                                         coords);
}
