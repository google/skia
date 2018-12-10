/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomain.h"

#include "GrProxyProvider.h"
#include "GrShaderCaps.h"
#include "GrSimpleTextureEffect.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "SkFloatingPoint.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"

#include <utility>

static GrTextureDomain::Mode optimize_mode_for_axis(bool proxyIsExact, SkScalar proxySize,
                                                    SkScalar domainStart, SkScalar domainEnd,
                                                    GrTextureDomain::Mode mode) {
    // Clamp-like modes where the proxy is exact and the domain's bounds on the particular axis
    // cover the entirety of the proxy can be converted to kIgnore_Mode. Everything else remains
    return (mode == GrTextureDomain::kClamp_Mode || mode == GrTextureDomain::kDecal_Mode) &&
           (domainStart < 0.f && domainEnd > proxySize) && proxyIsExact ?
           GrTextureDomain::kIgnore_Mode : mode;
}

// Possibly updates modeX and modeY to kIgnore_Mode if the axis can be simplified based on the proxy
// Returns true if both modeX and modeY have been set to kIgnore_Mode at the end.
static bool optimize_modes_for_domain(GrTextureProxy* proxy, const SkRect& domain,
                                      GrTextureDomain::Mode* modeX, GrTextureDomain::Mode* modeY) {
    bool exact = GrProxyProvider::IsFunctionallyExact(proxy);
    *modeX = optimize_mode_for_axis(exact, proxy->width(), domain.fLeft, domain.fRight, *modeX);
    *modeY = optimize_mode_for_axis(exact, proxy->height(), domain.fTop, domain.fBottom, *modeY);
    return *modeX == GrTextureDomain::kIgnore_Mode && *modeY == GrTextureDomain::kIgnore_Mode;
}

static SkString clamped_expression(GrTextureDomain::Mode mode, const SkString& inCoord,
                                   const char* coordSwizzle, const SkString& domain,
                                   const char* minSwizzle, const char* maxSwizzle) {
    // All inCoord references should be surrounded by () to make sure the evaluated expression
    // is swizzled, since it comes from outside code. No need to do so for the domain since that
    // will always be a simple variable name.
    SkString clampedExpr;
    switch(mode) {
        case GrTextureDomain::kIgnore_Mode:
            clampedExpr.printf("(%s).%s\n", inCoord.c_str(), coordSwizzle);
            break;
        case GrTextureDomain::kDecal_Mode:
            // The lookup coordinate to use for decal will be clamped just like kClamp_Mode,
            // it's just that the post-processing will be different, so fall through
        case GrTextureDomain::kClamp_Mode:
            clampedExpr.printf("clamp((%s).%s, %s.%s, %s.%s)", inCoord.c_str(), coordSwizzle,
                          domain.c_str(), minSwizzle, domain.c_str(), maxSwizzle);
            break;
        case GrTextureDomain::kRepeat_Mode:
            clampedExpr.printf("mod((%s).%s - %s.%s, %s.%s - %s.%s) + %s.%s",
                          inCoord.c_str(), coordSwizzle, domain.c_str(), minSwizzle,
                          domain.c_str(), maxSwizzle, domain.c_str(), minSwizzle,
                          domain.c_str(), minSwizzle);
            break;
        default:
            SkASSERTF(false, "Unknown texture domain mode\n");
            break;
    }
    return clampedExpr;
}

GrTextureDomain::GrTextureDomain(GrTextureProxy* proxy, const SkRect& domain, Mode modeX,
                                 Mode modeY, int index)
    : fModeX(modeX)
    , fModeY(modeY)
    , fIndex(index) {

    if (optimize_modes_for_domain(proxy, domain, &fModeX, &fModeY)) {
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
                                              const char* inModulateColor) {
    SkASSERT(!fHasMode || (textureDomain.modeX() == fModeX && textureDomain.modeY() == fModeY));
    SkDEBUGCODE(fModeX = textureDomain.modeX();)
    SkDEBUGCODE(fModeY = textureDomain.modeY();)
    SkDEBUGCODE(fHasMode = true;)

    if ((textureDomain.modeX() != kIgnore_Mode || textureDomain.modeY() != kIgnore_Mode) &&
        !fDomainUni.isValid()) {
        // Must include the domain uniform since at least one axis uses it
        const char* name;
        SkString uniName("TexDom");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        fDomainUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                uniName.c_str(), &name);
        fDomainName = name;
    }
    if ((textureDomain.modeX() == kDecal_Mode || textureDomain.modeY() == kDecal_Mode) &&
        !fDomainSizeUni.isValid()) {
        const char* name;
        SkString uniName("TexSize");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        fDomainSizeUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                    uniName.c_str(), &name);
        fSizeName = name;
    }

    SkString tc;
    bool decalX = textureDomain.modeX() == kDecal_Mode;
    bool decalY = textureDomain.modeY() == kDecal_Mode;
    if (textureDomain.modeX() != textureDomain.modeY()) {
        // The wrap modes differ on the two axes, so build up a coordinate that respects each
        // axis' domain rule independently before sampling the texture.
        SkString tcX = clamped_expression(decalX ? kClamp_Mode : textureDomain.modeX(),
                                          inCoords, "x", fDomainName, "x", "z");
        SkString tcY = clamped_expression(decalY ? kClamp_Mode : textureDomain.modeY(),
                                          inCoords, "y", fDomainName, "y", "w");
        tc.printf("float2(%s, %s)", tcX.c_str(), tcY.c_str());
        SkDebugf("different components: %s\n", tc.c_str());
    } else {
        // Since the x and y axis wrap modes are the same, they can be calculated together
        // using more efficient vector operations
        tc = clamped_expression(decalX ? kClamp_Mode : textureDomain.modeX(),
                                inCoords, "xy", fDomainName, "xy", "zw");
        SkDebugf("same components: %s\n", tc.c_str());
    }

    SkDebugf("dx: %d, dy: %d\n", decalX, decalY);
    if (decalX || decalY) {
        // Cannot just rely on using the clamped texture coordinate with the sampler since we
        // replace the inside color with transparent black when it's outside
        GrGLSLShaderBuilder::ShaderBlock block(builder);
        builder->codeAppendf("float2 clampedCoord = %s;", tc.c_str());
#define SMOOTH_DECAL 1
#if defined(SMOOTH_DECAL)
        if (decalX && decalY) {
            builder->codeAppendf("half err = max(abs(clampedCoord.x - (%s).x) * %s.x, abs(clampedCoord.y - (%s).y) * %s.y);",
                    inCoords.c_str(), fSizeName.c_str(), inCoords.c_str(), fSizeName.c_str());
        } else if (decalX) {
            builder->codeAppendf("half err = abs(clampedCoord.x - (%s).x) * %s.x;", inCoords.c_str(), fSizeName.c_str());
        } else {
            SkASSERT(decalY);
            builder->codeAppendf("half err = abs(clampedCoord.y - (%s).y) * %s.y;", inCoords.c_str(), fSizeName.c_str());
        }

        // If err is non-zero, the input coordinates are larger than the clamped coordinates
        // so mix between 0 (texture color) and 1 (fully transparent)
        // FIXME this mix only makes sense if the texture was bilerp, otherwise we add a nice edge
        //  to a texture that was otherwise kNearest filtered.
        builder->codeAppendf("%s = mix(", outColor);
        builder->appendTextureLookupAndModulate(inModulateColor, sampler, "clampedCoord", kFloat2_GrSLType);
        builder->codeAppend(", half4(0, 0, 0, 0), err);");
#else
        if (decalX && decalY) {
            builder->codeAppendf("bool outside = clampedCoord != %s;", inCoords.c_str());
        } else if (decalX) {
            builder->codeAppendf("bool outside = clampedCoord.x != (%s).x;", inCoords.c_str());
        } else {
            SkASSERT(decalY);
            builder->codeAppendf("bool outside = clampedCoord.y != (%s).y;", inCoords.c_str());
        }
        builder->codeAppendf("%s = outside ? half4(0, 0, 0, 0) : ", outColor);
        builder->appendTextureLookupAndModulate(inModulateColor, sampler, "clampedCoord", kFloat2_GrSLType);
        builder->codeAppend(";");
#endif
    } else {
        builder->codeAppendf("%s = ", outColor);
        builder->appendTextureLookupAndModulate(inModulateColor, sampler, tc.c_str(), kFloat2_GrSLType);
        builder->codeAppend(";");
    }


/*
    switch (textureDomain.mode()) {

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
                                                        kFloat2_GrSLType);
                builder->codeAppend(";");

                builder->codeAppendf("float x = (%s).x;", inCoords.c_str());
                builder->codeAppendf("float y = (%s).y;", inCoords.c_str());

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
                                                        kFloat2_GrSLType);
                builder->codeAppend(";");
            }
            break;
        }
    }*/
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        GrSurfaceProxy* proxy) {
    GrTexture* tex = proxy->peekTexture();
    SkASSERT(fHasMode && textureDomain.modeX() == fModeX && textureDomain.modeY() == fModeY);
    if (kIgnore_Mode != textureDomain.modeX() || kIgnore_Mode != textureDomain.modeY()) {
        SkScalar wInv, hInv, h;
        if (proxy->textureType() == GrTextureType::kRectangle) {
            wInv = hInv = 1.f;
            h = tex->height();
        } else {
            wInv = SK_Scalar1 / tex->width();
            hInv = SK_Scalar1 / tex->height();
            h = 1.f;
        }

        if (textureDomain.modeX() == kDecal_Mode || textureDomain.modeY() == kDecal_Mode) {
            pdman.set2f(fDomainSizeUni, tex->width(), tex->height());
        }

        float values[kPrevDomainCount] = {
            SkScalarToFloat(textureDomain.domain().fLeft * wInv),
            SkScalarToFloat(textureDomain.domain().fTop * hInv),
            SkScalarToFloat(textureDomain.domain().fRight * wInv),
            SkScalarToFloat(textureDomain.domain().fBottom * hInv)
        };

        if (proxy->textureType() == GrTextureType::kRectangle) {
            SkASSERT(values[0] >= 0.0f && values[0] <= proxy->height());
            SkASSERT(values[1] >= 0.0f && values[1] <= proxy->height());
            SkASSERT(values[2] >= 0.0f && values[2] <= proxy->height());
            SkASSERT(values[3] >= 0.0f && values[3] <= proxy->height());
        } else {
            SkASSERT(values[0] >= 0.0f && values[0] <= 1.0f);
            SkASSERT(values[1] >= 0.0f && values[1] <= 1.0f);
            SkASSERT(values[2] >= 0.0f && values[2] <= 1.0f);
            SkASSERT(values[3] >= 0.0f && values[3] <= 1.0f);
        }

        // vertical flip if necessary
        if (kBottomLeft_GrSurfaceOrigin == proxy->origin()) {
            values[1] = h - values[1];
            values[3] = h - values[3];

            // The top and bottom were just flipped, so correct the ordering
            // of elements so that values = (l, t, r, b).
            using std::swap;
            swap(values[1], values[3]);
        }
        if (0 != memcmp(values, fPrevDomain, kPrevDomainCount * sizeof(float))) {
            SkDebugf("domain set to %.4f %.4f %.4f %.4f\n", values[0], values[1], values[2], values[3]);
            pdman.set4fv(fDomainUni, 1, values);
            memcpy(fPrevDomain, values, kPrevDomainCount * sizeof(float));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
inline GrFragmentProcessor::OptimizationFlags GrTextureDomainEffect::OptFlags(
        GrPixelConfig config, GrTextureDomain::Mode modeX, GrTextureDomain::Mode modeY) {
    if (modeX == GrTextureDomain::kDecal_Mode || modeY == GrTextureDomain::kDecal_Mode ||
        !GrPixelConfigIsOpaque(config)) {
        return GrFragmentProcessor::kCompatibleWithCoverageAsAlpha_OptimizationFlag;
    } else {
        return GrFragmentProcessor::kCompatibleWithCoverageAsAlpha_OptimizationFlag |
               GrFragmentProcessor::kPreservesOpaqueInput_OptimizationFlag;
    }
}

std::unique_ptr<GrFragmentProcessor> GrTextureDomainEffect::Make(
        sk_sp<GrTextureProxy> proxy,
        const SkMatrix& matrix,
        const SkRect& domain,
        GrTextureDomain::Mode mode,
        GrSamplerState::Filter filterMode) {
    return Make(std::move(proxy), matrix, domain, mode, mode, GrSamplerState(GrSamplerState::WrapMode::kClamp, filterMode));
}

std::unique_ptr<GrFragmentProcessor> GrTextureDomainEffect::Make(
        sk_sp<GrTextureProxy> proxy,
        const SkMatrix& matrix,
        const SkRect& domain,
        GrTextureDomain::Mode modeX,
        GrTextureDomain::Mode modeY,
        const GrSamplerState& sampler) {
    if (optimize_modes_for_domain(proxy.get(), domain, &modeX, &modeY)) {
        SkDebugf("Domain effect optimized away\n");
        return GrSimpleTextureEffect::Make(std::move(proxy), matrix, sampler);
    } else {
        SkDebugf("Domain effect instantiated\n");
        return std::unique_ptr<GrFragmentProcessor>(new GrTextureDomainEffect(
                std::move(proxy), matrix, domain, modeX, modeY, sampler));
    }
}

GrTextureDomainEffect::GrTextureDomainEffect(sk_sp<GrTextureProxy> proxy,
                                             const SkMatrix& matrix,
                                             const SkRect& domain,
                                             GrTextureDomain::Mode modeX,
                                             GrTextureDomain::Mode modeY,
                                             const GrSamplerState& sampler)
        : INHERITED(kGrTextureDomainEffect_ClassID, OptFlags(proxy->config(), modeX, modeY))
        , fCoordTransform(matrix, proxy.get())
        , fTextureDomain(proxy.get(), domain, modeX, modeY)
        , fTextureSampler(std::move(proxy), sampler) {
    SkASSERT((modeX != GrTextureDomain::kRepeat_Mode && modeY != GrTextureDomain::kRepeat_Mode) ||
             sampler.filter() == GrSamplerState::Filter::kNearest);
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(1);
}

GrTextureDomainEffect::GrTextureDomainEffect(const GrTextureDomainEffect& that)
        : INHERITED(kGrTextureDomainEffect_ClassID, that.optimizationFlags())
        , fCoordTransform(that.fCoordTransform)
        , fTextureDomain(that.fTextureDomain)
        , fTextureSampler(that.fTextureSampler) {
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(1);
}

void GrTextureDomainEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
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
                                    args.fShaderCaps,
                                    domain,
                                    args.fOutputColor,
                                    coords2D,
                                    args.fTexSamplers[0],
                                    args.fInputColor);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const GrTextureDomainEffect& tde = fp.cast<GrTextureDomainEffect>();
            const GrTextureDomain& domain = tde.fTextureDomain;
            GrSurfaceProxy* proxy = tde.textureSampler(0).proxy();

            fGLDomain.setData(pdman, domain, proxy);
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
    GrTextureDomain::Mode modeX =
        (GrTextureDomain::Mode) d->fRandom->nextULessThan(GrTextureDomain::kModeCount);
    GrTextureDomain::Mode modeY =
        (GrTextureDomain::Mode) d->fRandom->nextULessThan(GrTextureDomain::kModeCount);
    const SkMatrix& matrix = GrTest::TestMatrix(d->fRandom);
    bool bilerp = modeX != GrTextureDomain::kRepeat_Mode && modeY != GrTextureDomain::kRepeat_Mode ?
            d->fRandom->nextBool() : false;
    return GrTextureDomainEffect::Make(
            std::move(proxy),
            matrix,
            domain,
            modeX,
            modeY,
            GrSamplerState(GrSamplerState::WrapMode::kClamp, bilerp ? GrSamplerState::Filter::kBilerp : GrSamplerState::Filter::kNearest));
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
        : INHERITED(kGrDeviceSpaceTextureDecalFragmentProcessor_ClassID,
                    kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fTextureSampler(proxy, GrSamplerState::ClampNearest())
        , fTextureDomain(proxy.get(), GrTextureDomain::MakeTexelDomain(subset),
                         GrTextureDomain::kDecal_Mode, GrTextureDomain::kDecal_Mode) {
    this->setTextureSamplerCnt(1);
    fDeviceSpaceOffset.fX = deviceSpaceOffset.fX - subset.fLeft;
    fDeviceSpaceOffset.fY = deviceSpaceOffset.fY - subset.fTop;
}

GrDeviceSpaceTextureDecalFragmentProcessor::GrDeviceSpaceTextureDecalFragmentProcessor(
        const GrDeviceSpaceTextureDecalFragmentProcessor& that)
        : INHERITED(kGrDeviceSpaceTextureDecalFragmentProcessor_ClassID,
                    kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fTextureSampler(that.fTextureSampler)
        , fTextureDomain(that.fTextureDomain)
        , fDeviceSpaceOffset(that.fDeviceSpaceOffset) {
    this->setTextureSamplerCnt(1);
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
            GrTexture* texture = proxy->peekTexture();

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
