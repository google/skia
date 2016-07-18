/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomain.h"
#include "GrInvariantOutput.h"
#include "GrSimpleTextureEffect.h"
#include "SkFloatingPoint.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLSampler.h"
#include "glsl/GrGLSLShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"

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
        fDomain.fLeft = SkScalarPin(domain.fLeft, kFullRect.fLeft, kFullRect.fRight);
        fDomain.fRight = SkScalarPin(domain.fRight, kFullRect.fLeft, kFullRect.fRight);
        fDomain.fTop = SkScalarPin(domain.fTop, kFullRect.fTop, kFullRect.fBottom);
        fDomain.fBottom = SkScalarPin(domain.fBottom, kFullRect.fTop, kFullRect.fBottom);
        SkASSERT(fDomain.fLeft <= fDomain.fRight);
        SkASSERT(fDomain.fTop <= fDomain.fBottom);
    }
}

//////////////////////////////////////////////////////////////////////////////

void GrTextureDomain::GLDomain::sampleTexture(GrGLSLShaderBuilder* builder,
                                              GrGLSLUniformHandler* uniformHandler,
                                              const GrGLSLCaps* glslCaps,
                                              const GrTextureDomain& textureDomain,
                                              const char* outColor,
                                              const SkString& inCoords,
                                              GrGLSLFragmentProcessor::SamplerHandle sampler,
                                              const char* inModulateColor) {
    SkASSERT((Mode)-1 == fMode || textureDomain.mode() == fMode);
    SkDEBUGCODE(fMode = textureDomain.mode();)

    if (textureDomain.mode() != kIgnore_Mode && !fDomainUni.isValid()) {
        const char* name;
        SkString uniName("TexDom");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        fDomainUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                uniName.c_str(), &name);
        fDomainName = name;
    }

    switch (textureDomain.mode()) {
        case kIgnore_Mode: {
            builder->codeAppendf("%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                      inCoords.c_str());
            builder->codeAppend(";");
            break;
        }
        case kClamp_Mode: {
            SkString clampedCoords;
            clampedCoords.appendf("clamp(%s, %s.xy, %s.zw)",
                                  inCoords.c_str(), fDomainName.c_str(), fDomainName.c_str());

            builder->codeAppendf("%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                      clampedCoords.c_str());
            builder->codeAppend(";");
            break;
        }
        case kDecal_Mode: {
            // Add a block since we're going to declare variables.
            GrGLSLShaderBuilder::ShaderBlock block(builder);

            const char* domain = fDomainName.c_str();
            if (!glslCaps->canUseAnyFunctionInShader()) {
                // On the NexusS and GalaxyNexus, the other path (with the 'any'
                // call) causes the compilation error "Calls to any function that
                // may require a gradient calculation inside a conditional block
                // may return undefined results". This appears to be an issue with
                // the 'any' call since even the simple "result=black; if (any())
                // result=white;" code fails to compile.
                builder->codeAppend("vec4 outside = vec4(0.0, 0.0, 0.0, 0.0);");
                builder->codeAppend("vec4 inside = ");
                builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                          inCoords.c_str());
                builder->codeAppend(";");

                builder->appendPrecisionModifier(kHigh_GrSLPrecision);
                builder->codeAppendf("float x = (%s).x;", inCoords.c_str());
                builder->appendPrecisionModifier(kHigh_GrSLPrecision);
                builder->codeAppendf("float y = (%s).y;", inCoords.c_str());

                builder->codeAppendf("x = abs(2.0*(x - %s.x)/(%s.z - %s.x) - 1.0);",
                                     domain, domain, domain);
                builder->codeAppendf("y = abs(2.0*(y - %s.y)/(%s.w - %s.y) - 1.0);",
                                     domain, domain, domain);
                builder->codeAppend("float blend = step(1.0, max(x, y));");
                builder->codeAppendf("%s = mix(inside, outside, blend);", outColor);
            } else {
                builder->codeAppend("bvec4 outside;\n");
                builder->codeAppendf("outside.xy = lessThan(%s, %s.xy);", inCoords.c_str(),
                                       domain);
                builder->codeAppendf("outside.zw = greaterThan(%s, %s.zw);", inCoords.c_str(),
                                       domain);
                builder->codeAppendf("%s = any(outside) ? vec4(0.0, 0.0, 0.0, 0.0) : ",
                                       outColor);
                builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                          inCoords.c_str());
                builder->codeAppend(";");
            }
            break;
        }
        case kRepeat_Mode: {
            SkString clampedCoords;
            clampedCoords.printf("mod(%s - %s.xy, %s.zw - %s.xy) + %s.xy",
                                 inCoords.c_str(), fDomainName.c_str(), fDomainName.c_str(),
                                 fDomainName.c_str(), fDomainName.c_str());

            builder->codeAppendf("%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler,
                                                      clampedCoords.c_str());
            builder->codeAppend(";");
            break;
        }
    }
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        GrSurfaceOrigin textureOrigin) {
    SkASSERT(textureDomain.mode() == fMode);
    if (kIgnore_Mode != textureDomain.mode()) {
        float values[kPrevDomainCount] = {
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
        if (0 != memcmp(values, fPrevDomain, kPrevDomainCount * sizeof(float))) {
            pdman.set4fv(fDomainUni, 1, values);
            memcpy(fPrevDomain, values, kPrevDomainCount * sizeof(float));
        }
    }
}


//////////////////////////////////////////////////////////////////////////////

class GrGLTextureDomainEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrTextureDomain::GLDomain         fGLDomain;
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLTextureDomainEffect::emitCode(EmitArgs& args) {
    const GrTextureDomainEffect& textureDomainEffect = args.fFp.cast<GrTextureDomainEffect>();
    const GrTextureDomain& domain = textureDomainEffect.textureDomain();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    fGLDomain.sampleTexture(fragBuilder,
                            args.fUniformHandler,
                            args.fGLSLCaps,
                            domain,
                            args.fOutputColor,
                            coords2D,
                            args.fTexSamplers[0],
                            args.fInputColor);
}

void GrGLTextureDomainEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                        const GrProcessor& processor) {
    const GrTextureDomainEffect& textureDomainEffect = processor.cast<GrTextureDomainEffect>();
    const GrTextureDomain& domain = textureDomainEffect.textureDomain();
    fGLDomain.setData(pdman, domain, processor.texture(0)->origin());
}

void GrGLTextureDomainEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                                     GrProcessorKeyBuilder* b) {
    const GrTextureDomain& domain = processor.cast<GrTextureDomainEffect>().textureDomain();
    b->add32(GrTextureDomain::GLDomain::DomainKey(domain));
}


///////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrTextureDomainEffect::Make(GrTexture* texture,
                                                       const SkMatrix& matrix,
                                                       const SkRect& domain,
                                                       GrTextureDomain::Mode mode,
                                                       GrTextureParams::FilterMode filterMode,
                                                       GrCoordSet coordSet) {
    static const SkRect kFullRect = {0, 0, SK_Scalar1, SK_Scalar1};
    if (GrTextureDomain::kIgnore_Mode == mode ||
        (GrTextureDomain::kClamp_Mode == mode && domain.contains(kFullRect))) {
        return GrSimpleTextureEffect::Make(texture, matrix, filterMode);
    } else {
        return sk_sp<GrFragmentProcessor>(
            new GrTextureDomainEffect(texture, matrix, domain, mode, filterMode, coordSet));
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
    this->initClassID<GrTextureDomainEffect>();
}

GrTextureDomainEffect::~GrTextureDomainEffect() {}

void GrTextureDomainEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    GrGLTextureDomainEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrTextureDomainEffect::onCreateGLSLInstance() const  {
    return new GrGLTextureDomainEffect;
}

bool GrTextureDomainEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrTextureDomainEffect& s = sBase.cast<GrTextureDomainEffect>();
    return this->fTextureDomain == s.fTextureDomain;
}

void GrTextureDomainEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (GrTextureDomain::kDecal_Mode == fTextureDomain.mode()) { // TODO: helper
        if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
            inout->mulByUnknownSingleComponent();
        } else {
            inout->mulByUnknownFourComponents();
        }
    } else {
        this->updateInvariantOutputForModulation(inout);
    }
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTextureDomainEffect);

sk_sp<GrFragmentProcessor> GrTextureDomainEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    SkRect domain;
    domain.fLeft = d->fRandom->nextUScalar1();
    domain.fRight = d->fRandom->nextRangeScalar(domain.fLeft, SK_Scalar1);
    domain.fTop = d->fRandom->nextUScalar1();
    domain.fBottom = d->fRandom->nextRangeScalar(domain.fTop, SK_Scalar1);
    GrTextureDomain::Mode mode =
        (GrTextureDomain::Mode) d->fRandom->nextULessThan(GrTextureDomain::kModeCount);
    const SkMatrix& matrix = GrTest::TestMatrix(d->fRandom);
    bool bilerp = mode != GrTextureDomain::kRepeat_Mode ? d->fRandom->nextBool() : false;
    GrCoordSet coords = d->fRandom->nextBool() ? kLocal_GrCoordSet : kDevice_GrCoordSet;
    return GrTextureDomainEffect::Make(
        d->fTextures[texIdx],
        matrix,
        domain,
        mode,
        bilerp ? GrTextureParams::kBilerp_FilterMode : GrTextureParams::kNone_FilterMode,
        coords);
}
