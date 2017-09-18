/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomain.h"

#include "GrResourceProvider.h"
#include "GrShaderCaps.h"
#include "GrSimpleTextureEffect.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "SkFloatingPoint.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"

static bool can_ignore_rect(GrTextureProxy* proxy, const SkRect& domain) {
    if (GrResourceProvider::IsFunctionallyExact(proxy)) {
        const SkIRect kFullRect = SkIRect::MakeWH(proxy->width(), proxy->height());

        return domain.contains(kFullRect);
    }

    return false;
}

GrTextureDomain::GrTextureDomain(GrTextureProxy* proxy, const SkRect& domain, Mode mode, int index)
    : fMode(mode)
    , fIndex(index) {

    if (kIgnore_Mode == fMode) {
        return;
    }

    if (kClamp_Mode == mode && can_ignore_rect(proxy, domain)) {
        fMode = kIgnore_Mode;
        return;
    }

    const SkRect kFullRect = SkRect::MakeIWH(proxy->width(), proxy->height());

    // We don't currently handle domains that are empty or don't intersect the texture.
    // It is OK if the domain rect is a line or point, but it should not be inverted. We do not
    // handle rects that do not intersect the [0..1]x[0..1] rect.
    SkASSERT(domain.fLeft <= domain.fRight);
    SkASSERT(domain.fTop <= domain.fBottom);
    fDomain.fLeft = SkScalarPin(domain.fLeft, 0.0f, kFullRect.fRight);
    fDomain.fRight = SkScalarPin(domain.fRight, fDomain.fLeft, kFullRect.fRight);
    fDomain.fTop = SkScalarPin(domain.fTop, 0.0f, kFullRect.fBottom);
    fDomain.fBottom = SkScalarPin(domain.fBottom, fDomain.fTop, kFullRect.fBottom);
    SkASSERT(fDomain.fLeft <= fDomain.fRight);
    SkASSERT(fDomain.fTop <= fDomain.fBottom);
}

//////////////////////////////////////////////////////////////////////////////

void GrTextureDomain::GLDomain::sampleTexture(GrGLSLShaderBuilder* builder,
                                              GrGLSLUniformHandler* uniformHandler,
                                              const GrShaderCaps* shaderCaps,
                                              const GrTextureDomain& textureDomain,
                                              const char* outColor,
                                              const SkString& inCoords,
                                              GrGLSLFragmentProcessor::SamplerHandle sampler,
                                              const char* inModulateColor,
                                              GrGLSLColorSpaceXformHelper* colorXformHelper) {
    SkASSERT((Mode)-1 == fMode || textureDomain.mode() == fMode);
    SkDEBUGCODE(fMode = textureDomain.mode();)

    if (textureDomain.mode() != kIgnore_Mode && !fDomainUni.isValid()) {
        const char* name;
        SkString uniName("TexDom");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        fDomainUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                uniName.c_str(), &name);
        fDomainName = name;
    }

    switch (textureDomain.mode()) {
        case kIgnore_Mode: {
            builder->codeAppendf("%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler, inCoords.c_str(),
                                                    kHighFloat2_GrSLType, colorXformHelper);
            builder->codeAppend(";");
            break;
        }
        case kClamp_Mode: {
            SkString clampedCoords;
            clampedCoords.appendf("clamp(%s, %s.xy, %s.zw)",
                                  inCoords.c_str(), fDomainName.c_str(), fDomainName.c_str());

            builder->codeAppendf("%s = ", outColor);
            builder->appendTextureLookupAndModulate(inModulateColor, sampler, clampedCoords.c_str(),
                                                    kHighFloat2_GrSLType, colorXformHelper);
            builder->codeAppend(";");
            break;
        }
        case kDecal_Mode: {
            // Add a block since we're going to declare variables.
            GrGLSLShaderBuilder::ShaderBlock block(builder);

            const char* domain = fDomainName.c_str();
            if (!shaderCaps->canUseAnyFunctionInShader()) {
                // On the NexusS and GalaxyNexus, the other path (with the 'any'
                // call) causes the compilation error "Calls to any function that
                // may require a gradient calculation inside a conditional block
                // may return undefined results". This appears to be an issue with
                // the 'any' call since even the simple "result=black; if (any())
                // result=white;" code fails to compile.
                builder->codeAppend("half4 outside = half4(0.0, 0.0, 0.0, 0.0);");
                builder->codeAppend("half4 inside = ");
                builder->appendTextureLookupAndModulate(inModulateColor, sampler, inCoords.c_str(),
                                                        kHighFloat2_GrSLType, colorXformHelper);
                builder->codeAppend(";");

                builder->codeAppendf("highfloat x = (%s).x;", inCoords.c_str());
                builder->codeAppendf("highfloat y = (%s).y;", inCoords.c_str());

                builder->codeAppendf("x = abs(2.0*(x - %s.x)/(%s.z - %s.x) - 1.0);",
                                     domain, domain, domain);
                builder->codeAppendf("y = abs(2.0*(y - %s.y)/(%s.w - %s.y) - 1.0);",
                                     domain, domain, domain);
                builder->codeAppend("half blend = step(1.0, max(x, y));");
                builder->codeAppendf("%s = mix(inside, outside, blend);", outColor);
            } else {
                builder->codeAppend("bool4 outside;\n");
                builder->codeAppendf("outside.xy = lessThan(%s, %s.xy);", inCoords.c_str(),
                                       domain);
                builder->codeAppendf("outside.zw = greaterThan(%s, %s.zw);", inCoords.c_str(),
                                       domain);
                builder->codeAppendf("%s = any(outside) ? half4(0.0, 0.0, 0.0, 0.0) : ",
                                       outColor);
                builder->appendTextureLookupAndModulate(inModulateColor, sampler, inCoords.c_str(),
                                                        kHighFloat2_GrSLType, colorXformHelper);
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
            builder->appendTextureLookupAndModulate(inModulateColor, sampler, clampedCoords.c_str(),
                                                    kHighFloat2_GrSLType, colorXformHelper);
            builder->codeAppend(";");
            break;
        }
    }
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        GrSurfaceProxy* proxy) {
    GrTexture* tex = proxy->priv().peekTexture();
    SkASSERT(textureDomain.mode() == fMode);
    if (kIgnore_Mode != textureDomain.mode()) {
        SkScalar wInv = SK_Scalar1 / tex->width();
        SkScalar hInv = SK_Scalar1 / tex->height();

        float values[kPrevDomainCount] = {
            SkScalarToFloat(textureDomain.domain().fLeft * wInv),
            SkScalarToFloat(textureDomain.domain().fTop * hInv),
            SkScalarToFloat(textureDomain.domain().fRight * wInv),
            SkScalarToFloat(textureDomain.domain().fBottom * hInv)
        };

        SkASSERT(values[0] >= 0.0f && values[0] <= 1.0f);
        SkASSERT(values[1] >= 0.0f && values[1] <= 1.0f);
        SkASSERT(values[2] >= 0.0f && values[2] <= 1.0f);
        SkASSERT(values[3] >= 0.0f && values[3] <= 1.0f);

        // vertical flip if necessary
        if (kBottomLeft_GrSurfaceOrigin == proxy->origin()) {
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
inline GrFragmentProcessor::OptimizationFlags GrTextureDomainEffect::OptFlags(
        GrPixelConfig config, GrTextureDomain::Mode mode) {
    if (mode == GrTextureDomain::kDecal_Mode || !GrPixelConfigIsOpaque(config)) {
        return GrFragmentProcessor::kCompatibleWithCoverageAsAlpha_OptimizationFlag;
    } else {
        return GrFragmentProcessor::kCompatibleWithCoverageAsAlpha_OptimizationFlag |
               GrFragmentProcessor::kPreservesOpaqueInput_OptimizationFlag;
    }
}

std::unique_ptr<GrFragmentProcessor> GrTextureDomainEffect::Make(
        sk_sp<GrTextureProxy> proxy,
        sk_sp<GrColorSpaceXform> colorSpaceXform,
        const SkMatrix& matrix,
        const SkRect& domain,
        GrTextureDomain::Mode mode,
        GrSamplerState::Filter filterMode) {
    if (GrTextureDomain::kIgnore_Mode == mode ||
        (GrTextureDomain::kClamp_Mode == mode && can_ignore_rect(proxy.get(), domain))) {
        return GrSimpleTextureEffect::Make(std::move(proxy),
                                           std::move(colorSpaceXform), matrix, filterMode);
    } else {
        return std::unique_ptr<GrFragmentProcessor>(new GrTextureDomainEffect(
                std::move(proxy), std::move(colorSpaceXform), matrix, domain, mode, filterMode));
    }
}

GrTextureDomainEffect::GrTextureDomainEffect(sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& matrix,
                                             const SkRect& domain,
                                             GrTextureDomain::Mode mode,
                                             GrSamplerState::Filter filterMode)
        : INHERITED(OptFlags(proxy->config(), mode))
        , fCoordTransform(matrix, proxy.get())
        , fTextureDomain(proxy.get(), domain, mode)
        , fTextureSampler(std::move(proxy), filterMode)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    SkASSERT(mode != GrTextureDomain::kRepeat_Mode ||
             filterMode == GrSamplerState::Filter::kNearest);
    this->initClassID<GrTextureDomainEffect>();
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrTextureDomainEffect::GrTextureDomainEffect(const GrTextureDomainEffect& that)
        : INHERITED(that.optimizationFlags())
        , fCoordTransform(that.fCoordTransform)
        , fTextureDomain(that.fTextureDomain)
        , fTextureSampler(that.fTextureSampler)
        , fColorSpaceXform(that.fColorSpaceXform) {
    this->initClassID<GrTextureDomainEffect>();
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

void GrTextureDomainEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    b->add32(GrTextureDomain::GLDomain::DomainKey(fTextureDomain));
    b->add32(GrColorSpaceXform::XformKey(this->colorSpaceXform()));
}

GrGLSLFragmentProcessor* GrTextureDomainEffect::onCreateGLSLInstance() const  {
    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const GrTextureDomainEffect& tde = args.fFp.cast<GrTextureDomainEffect>();
            const GrTextureDomain& domain = tde.fTextureDomain;

            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);

            fColorSpaceHelper.emitCode(args.fUniformHandler, tde.colorSpaceXform());
            fGLDomain.sampleTexture(fragBuilder,
                                    args.fUniformHandler,
                                    args.fShaderCaps,
                                    domain,
                                    args.fOutputColor,
                                    coords2D,
                                    args.fTexSamplers[0],
                                    args.fInputColor,
                                    &fColorSpaceHelper);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const GrTextureDomainEffect& tde = fp.cast<GrTextureDomainEffect>();
            const GrTextureDomain& domain = tde.fTextureDomain;
            GrSurfaceProxy* proxy = tde.textureSampler(0).proxy();

            fGLDomain.setData(pdman, domain, proxy);
            if (SkToBool(tde.colorSpaceXform())) {
                fColorSpaceHelper.setData(pdman, tde.colorSpaceXform());
            }
        }

    private:
        GrTextureDomain::GLDomain         fGLDomain;
        GrGLSLColorSpaceXformHelper       fColorSpaceHelper;
    };

    return new GLSLProcessor;
}

bool GrTextureDomainEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrTextureDomainEffect& s = sBase.cast<GrTextureDomainEffect>();
    return this->fTextureDomain == s.fTextureDomain;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTextureDomainEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrTextureDomainEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);
    SkRect domain;
    domain.fLeft = d->fRandom->nextRangeScalar(0, proxy->width());
    domain.fRight = d->fRandom->nextRangeScalar(domain.fLeft, proxy->width());
    domain.fTop = d->fRandom->nextRangeScalar(0, proxy->height());
    domain.fBottom = d->fRandom->nextRangeScalar(domain.fTop, proxy->height());
    GrTextureDomain::Mode mode =
        (GrTextureDomain::Mode) d->fRandom->nextULessThan(GrTextureDomain::kModeCount);
    const SkMatrix& matrix = GrTest::TestMatrix(d->fRandom);
    bool bilerp = mode != GrTextureDomain::kRepeat_Mode ? d->fRandom->nextBool() : false;
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrTest::TestColorXform(d->fRandom);
    return GrTextureDomainEffect::Make(
            std::move(proxy),
            std::move(colorSpaceXform),
            matrix,
            domain,
            mode,
            bilerp ? GrSamplerState::Filter::kBilerp : GrSamplerState::Filter::kNearest);
}
#endif

///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<GrFragmentProcessor> GrDeviceSpaceTextureDecalFragmentProcessor::Make(
        sk_sp<GrTextureProxy> proxy, const SkIRect& subset, const SkIPoint& deviceSpaceOffset) {
    return std::unique_ptr<GrFragmentProcessor>(new GrDeviceSpaceTextureDecalFragmentProcessor(
            std::move(proxy), subset, deviceSpaceOffset));
}

GrDeviceSpaceTextureDecalFragmentProcessor::GrDeviceSpaceTextureDecalFragmentProcessor(
        sk_sp<GrTextureProxy> proxy, const SkIRect& subset, const SkIPoint& deviceSpaceOffset)
        : INHERITED(kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fTextureSampler(proxy, GrSamplerState::ClampNearest())
        , fTextureDomain(proxy.get(), GrTextureDomain::MakeTexelDomain(subset),
                         GrTextureDomain::kDecal_Mode) {
    this->addTextureSampler(&fTextureSampler);
    fDeviceSpaceOffset.fX = deviceSpaceOffset.fX - subset.fLeft;
    fDeviceSpaceOffset.fY = deviceSpaceOffset.fY - subset.fTop;
    this->initClassID<GrDeviceSpaceTextureDecalFragmentProcessor>();
}

GrDeviceSpaceTextureDecalFragmentProcessor::GrDeviceSpaceTextureDecalFragmentProcessor(
        const GrDeviceSpaceTextureDecalFragmentProcessor& that)
        : INHERITED(kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fTextureSampler(that.fTextureSampler)
        , fTextureDomain(that.fTextureDomain)
        , fDeviceSpaceOffset(that.fDeviceSpaceOffset) {
    this->initClassID<GrDeviceSpaceTextureDecalFragmentProcessor>();
    this->addTextureSampler(&fTextureSampler);
}

std::unique_ptr<GrFragmentProcessor> GrDeviceSpaceTextureDecalFragmentProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrDeviceSpaceTextureDecalFragmentProcessor(*this));
}

GrGLSLFragmentProcessor* GrDeviceSpaceTextureDecalFragmentProcessor::onCreateGLSLInstance() const  {
    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const GrDeviceSpaceTextureDecalFragmentProcessor& dstdfp =
                    args.fFp.cast<GrDeviceSpaceTextureDecalFragmentProcessor>();
            const char* scaleAndTranslateName;
            fScaleAndTranslateUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                     kHalf4_GrSLType,
                                                                     "scaleAndTranslate",
                                                                     &scaleAndTranslateName);
            args.fFragBuilder->codeAppendf("half2 coords = sk_FragCoord.xy * %s.xy + %s.zw;",
                                           scaleAndTranslateName, scaleAndTranslateName);
            fGLDomain.sampleTexture(args.fFragBuilder,
                                    args.fUniformHandler,
                                    args.fShaderCaps,
                                    dstdfp.fTextureDomain,
                                    args.fOutputColor,
                                    SkString("coords"),
                                    args.fTexSamplers[0],
                                    args.fInputColor);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const GrDeviceSpaceTextureDecalFragmentProcessor& dstdfp =
                    fp.cast<GrDeviceSpaceTextureDecalFragmentProcessor>();
            GrSurfaceProxy* proxy = dstdfp.textureSampler(0).proxy();
            GrTexture* texture = proxy->priv().peekTexture();

            fGLDomain.setData(pdman, dstdfp.fTextureDomain, proxy);
            float iw = 1.f / texture->width();
            float ih = 1.f / texture->height();
            float scaleAndTransData[4] = {
                iw, ih,
                -dstdfp.fDeviceSpaceOffset.fX * iw, -dstdfp.fDeviceSpaceOffset.fY * ih
            };
            if (proxy->origin() == kBottomLeft_GrSurfaceOrigin) {
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
    return dstdfp.fTextureSampler.proxy()->underlyingUniqueID() ==
                   fTextureSampler.proxy()->underlyingUniqueID() &&
           dstdfp.fDeviceSpaceOffset == fDeviceSpaceOffset &&
           dstdfp.fTextureDomain == fTextureDomain;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDeviceSpaceTextureDecalFragmentProcessor);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrDeviceSpaceTextureDecalFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);
    SkIRect subset;
    subset.fLeft = d->fRandom->nextULessThan(proxy->width() - 1);
    subset.fRight = d->fRandom->nextRangeU(subset.fLeft, proxy->width());
    subset.fTop = d->fRandom->nextULessThan(proxy->height() - 1);
    subset.fBottom = d->fRandom->nextRangeU(subset.fTop, proxy->height());
    SkIPoint pt;
    pt.fX = d->fRandom->nextULessThan(2048);
    pt.fY = d->fRandom->nextULessThan(2048);
    return GrDeviceSpaceTextureDecalFragmentProcessor::Make(std::move(proxy), subset, pt);
}
#endif
