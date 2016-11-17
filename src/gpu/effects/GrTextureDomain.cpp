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

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrTextureDomainEffect::Make(GrTexture* texture,
                                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                                       const SkMatrix& matrix,
                                                       const SkRect& domain,
                                                       GrTextureDomain::Mode mode,
                                                       GrTextureParams::FilterMode filterMode) {
    static const SkRect kFullRect = {0, 0, SK_Scalar1, SK_Scalar1};
    if (GrTextureDomain::kIgnore_Mode == mode ||
        (GrTextureDomain::kClamp_Mode == mode && domain.contains(kFullRect))) {
        return GrSimpleTextureEffect::Make(texture, std::move(colorSpaceXform), matrix, filterMode);
    } else {
        return sk_sp<GrFragmentProcessor>(
            new GrTextureDomainEffect(texture, std::move(colorSpaceXform), matrix, domain, mode,
                                      filterMode));
    }
}

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& matrix,
                                             const SkRect& domain,
                                             GrTextureDomain::Mode mode,
                                             GrTextureParams::FilterMode filterMode)
    : GrSingleTextureEffect(texture, std::move(colorSpaceXform), matrix, filterMode)
    , fTextureDomain(domain, mode) {
    SkASSERT(mode != GrTextureDomain::kRepeat_Mode ||
            filterMode == GrTextureParams::kNone_FilterMode);
    this->initClassID<GrTextureDomainEffect>();
}

void GrTextureDomainEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    b->add32(GrTextureDomain::GLDomain::DomainKey(fTextureDomain));
}

GrGLSLFragmentProcessor* GrTextureDomainEffect::onCreateGLSLInstance() const  {
    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const GrTextureDomainEffect& tde = args.fFp.cast<GrTextureDomainEffect>();
            const GrTextureDomain& domain = tde.fTextureDomain;

            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
            fGLDomain.sampleTexture(fragBuilder,
                                    args.fUniformHandler,
                                    args.fGLSLCaps,
                                    domain,
                                    args.fOutputColor,
                                    coords2D,
                                    args.fTexSamplers[0],
                                    args.fInputColor);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& fp) override {
            const GrTextureDomainEffect& tde = fp.cast<GrTextureDomainEffect>();
            const GrTextureDomain& domain = tde.fTextureDomain;
            fGLDomain.setData(pdman, domain, tde.textureSampler(0).texture()->origin());
        }

    private:
        GrTextureDomain::GLDomain         fGLDomain;

    };

    return new GLSLProcessor;
}

bool GrTextureDomainEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrTextureDomainEffect& s = sBase.cast<GrTextureDomainEffect>();
    return this->fTextureDomain == s.fTextureDomain;
}

void GrTextureDomainEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (GrTextureDomain::kDecal_Mode == fTextureDomain.mode()) {
        if (GrPixelConfigIsAlphaOnly(this->textureSampler(0).texture()->config())) {
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
    auto colorSpaceXform = GrTest::TestColorXform(d->fRandom);
    return GrTextureDomainEffect::Make(
        d->fTextures[texIdx],
        colorSpaceXform,
        matrix,
        domain,
        mode,
        bilerp ? GrTextureParams::kBilerp_FilterMode : GrTextureParams::kNone_FilterMode);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrDeviceSpaceTextureDecalFragmentProcessor::Make(GrTexture* texture,
        const SkIRect& subset, const SkIPoint& deviceSpaceOffset) {
    return sk_sp<GrFragmentProcessor>(new GrDeviceSpaceTextureDecalFragmentProcessor(
            texture, subset, deviceSpaceOffset));
}

GrDeviceSpaceTextureDecalFragmentProcessor::GrDeviceSpaceTextureDecalFragmentProcessor(
        GrTexture* texture, const SkIRect& subset, const SkIPoint& deviceSpaceOffset)
        : fTextureSampler(texture, GrTextureParams::ClampNoFilter())
        , fTextureDomain(GrTextureDomain::MakeTexelDomain(texture, subset),
                         GrTextureDomain::kDecal_Mode) {
    this->addTextureSampler(&fTextureSampler);
    fDeviceSpaceOffset.fX = deviceSpaceOffset.fX - subset.fLeft;
    fDeviceSpaceOffset.fY = deviceSpaceOffset.fY - subset.fTop;
    this->initClassID<GrDeviceSpaceTextureDecalFragmentProcessor>();
    this->setWillReadFragmentPosition();
}

GrGLSLFragmentProcessor* GrDeviceSpaceTextureDecalFragmentProcessor::onCreateGLSLInstance() const  {
    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const GrDeviceSpaceTextureDecalFragmentProcessor& dstdfp =
                    args.fFp.cast<GrDeviceSpaceTextureDecalFragmentProcessor>();
            const char* scaleAndTranslateName;
            fScaleAndTranslateUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                     kVec4f_GrSLType,
                                                                     kDefault_GrSLPrecision,
                                                                     "scaleAndTranslate",
                                                                     &scaleAndTranslateName);
            args.fFragBuilder->codeAppendf("vec2 coords = %s.xy * %s.xy + %s.zw;",
                                           args.fFragBuilder->fragmentPosition(),
                                           scaleAndTranslateName, scaleAndTranslateName);
            fGLDomain.sampleTexture(args.fFragBuilder,
                                    args.fUniformHandler,
                                    args.fGLSLCaps,
                                    dstdfp.fTextureDomain,
                                    args.fOutputColor,
                                    SkString("coords"),
                                    args.fTexSamplers[0],
                                    args.fInputColor);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& fp) override {
            const GrDeviceSpaceTextureDecalFragmentProcessor& dstdfp =
                    fp.cast<GrDeviceSpaceTextureDecalFragmentProcessor>();
            GrTexture* texture = dstdfp.textureSampler(0).texture();
            fGLDomain.setData(pdman, dstdfp.fTextureDomain, texture->origin());
            float iw = 1.f / texture->width();
            float ih = 1.f / texture->height();
            float scaleAndTransData[4] = {
                iw, ih,
                -dstdfp.fDeviceSpaceOffset.fX * iw, -dstdfp.fDeviceSpaceOffset.fY * ih
            };
            if (texture->origin() == kBottomLeft_GrSurfaceOrigin) {
                scaleAndTransData[1] = -scaleAndTransData[1];
                scaleAndTransData[3] = 1 - scaleAndTransData[3];
            }
            pdman.set4fv(fScaleAndTranslateUni, 1, scaleAndTransData);
        }

    private:
        GrTextureDomain::GLDomain   fGLDomain;
        UniformHandle               fScaleAndTranslateUni;
    };

    return new GLSLProcessor;
}

bool GrDeviceSpaceTextureDecalFragmentProcessor::onIsEqual(const GrFragmentProcessor& fp) const {
    const GrDeviceSpaceTextureDecalFragmentProcessor& dstdfp =
            fp.cast<GrDeviceSpaceTextureDecalFragmentProcessor>();
    return dstdfp.fTextureSampler.texture() == fTextureSampler.texture() &&
           dstdfp.fDeviceSpaceOffset == fDeviceSpaceOffset &&
           dstdfp.fTextureDomain == fTextureDomain;
}

void GrDeviceSpaceTextureDecalFragmentProcessor::onComputeInvariantOutput(
        GrInvariantOutput* inout) const {
    if (GrPixelConfigIsAlphaOnly(this->textureSampler(0).texture()->config())) {
        inout->mulByUnknownSingleComponent();
    } else {
        inout->mulByUnknownFourComponents();
    }
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDeviceSpaceTextureDecalFragmentProcessor);

sk_sp<GrFragmentProcessor> GrDeviceSpaceTextureDecalFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    SkIRect subset;
    subset.fLeft = d->fRandom->nextULessThan(d->fTextures[texIdx]->width() - 1);
    subset.fRight = d->fRandom->nextRangeU(subset.fLeft, d->fTextures[texIdx]->width());
    subset.fTop = d->fRandom->nextULessThan(d->fTextures[texIdx]->height() - 1);
    subset.fBottom = d->fRandom->nextRangeU(subset.fTop, d->fTextures[texIdx]->height());
    SkIPoint pt;
    pt.fX = d->fRandom->nextULessThan(2048);
    pt.fY = d->fRandom->nextULessThan(2048);
    return GrDeviceSpaceTextureDecalFragmentProcessor::Make(d->fTextures[texIdx], subset, pt);
}
