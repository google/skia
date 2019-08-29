/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrTextureDomain.h"

#include "include/gpu/GrTexture.h"
#include "include/private/SkFloatingPoint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/effects/generated/GrSimpleTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

#include <utility>

GrTextureDomain::GrTextureDomain(GrTextureProxy* proxy, const SkRect& domain, Mode modeX,
                                 Mode modeY, int index)
    : fModeX(modeX)
    , fModeY(modeY)
    , fIndex(index) {

    if (!proxy) {
        SkASSERT(modeX == kIgnore_Mode && modeY == kIgnore_Mode);
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

static SkString clamp_expression(GrTextureDomain::Mode mode, const char* inCoord,
                                 const char* coordSwizzle, const char* domain,
                                 const char* minSwizzle, const char* maxSwizzle) {
    SkString clampedExpr;
    switch(mode) {
        case GrTextureDomain::kIgnore_Mode:
            clampedExpr.printf("%s.%s\n", inCoord, coordSwizzle);
            break;
        case GrTextureDomain::kDecal_Mode:
            // The lookup coordinate to use for decal will be clamped just like kClamp_Mode,
            // it's just that the post-processing will be different, so fall through
        case GrTextureDomain::kClamp_Mode:
            clampedExpr.printf("clamp(%s.%s, %s.%s, %s.%s)",
                               inCoord, coordSwizzle, domain, minSwizzle, domain, maxSwizzle);
            break;
        case GrTextureDomain::kRepeat_Mode:
            clampedExpr.printf("mod(%s.%s - %s.%s, %s.%s - %s.%s) + %s.%s",
                               inCoord, coordSwizzle, domain, minSwizzle, domain, maxSwizzle,
                               domain, minSwizzle, domain, minSwizzle);
            break;
        default:
            SkASSERTF(false, "Unknown texture domain mode: %u\n", (uint32_t) mode);
            break;
    }
    return clampedExpr;
}

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

    bool decalX = textureDomain.modeX() == kDecal_Mode;
    bool decalY = textureDomain.modeY() == kDecal_Mode;
    if ((decalX || decalY) && !fDecalUni.isValid()) {
        const char* name;
        SkString uniName("DecalParams");
        if (textureDomain.fIndex >= 0) {
            uniName.appendS32(textureDomain.fIndex);
        }
        // Half3 since this will hold texture width, height, and then a step function control param
        fDecalUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf3_GrSLType,
                                               uniName.c_str(), &name);
        fDecalName = name;
    }

    // Add a block so that we can declare variables
    GrGLSLShaderBuilder::ShaderBlock block(builder);
    // Always use a local variable for the input coordinates; often callers pass in an expression
    // and we want to cache it across all of its references in the code below
    builder->codeAppendf("float2 origCoord = %s;", inCoords.c_str());
    builder->codeAppend("float2 clampedCoord = ");
    if (textureDomain.modeX() != textureDomain.modeY()) {
        // The wrap modes differ on the two axes, so build up a coordinate that respects each axis'
        // domain rule independently before sampling the texture.
        SkString tcX = clamp_expression(textureDomain.modeX(), "origCoord", "x",
                                        fDomainName.c_str(), "x", "z");
        SkString tcY = clamp_expression(textureDomain.modeY(), "origCoord", "y",
                                        fDomainName.c_str(), "y", "w");
        builder->codeAppendf("float2(%s, %s)", tcX.c_str(), tcY.c_str());
    } else {
        // Since the x and y axis wrap modes are the same, they can be calculated together using
        // more efficient vector operations
        SkString tc = clamp_expression(textureDomain.modeX(), "origCoord", "xy",
                                       fDomainName.c_str(), "xy", "zw");
        builder->codeAppend(tc.c_str());
    }
    builder->codeAppend(";");

    // Look up the texture sample at the clamped coordinate location
    builder->codeAppend("half4 inside = ");
    builder->appendTextureLookupAndModulate(inModulateColor, sampler, "clampedCoord",
                                            kFloat2_GrSLType);
    builder->codeAppend(";");

    // Apply decal mode's transparency interpolation if needed
    if (decalX || decalY) {
        // The decal err is the max absoluate value between the clamped coordinate and the original
        // pixel coordinate. This will then be clamped to 1.f if it's greater than the control
        // parameter, which simulates kNearest and kBilerp behavior depending on if it's 0 or 1.
        if (decalX && decalY) {
            builder->codeAppendf("half err = max(half(abs(clampedCoord.x - origCoord.x) * %s.x), "
                                                "half(abs(clampedCoord.y - origCoord.y) * %s.y));",
                                 fDecalName.c_str(), fDecalName.c_str());
        } else if (decalX) {
            builder->codeAppendf("half err = half(abs(clampedCoord.x - origCoord.x) * %s.x);",
                                 fDecalName.c_str());
        } else {
            SkASSERT(decalY);
            builder->codeAppendf("half err = half(abs(clampedCoord.y - origCoord.y) * %s.y);",
                                 fDecalName.c_str());
        }

        // Apply a transform to the error rate, which let's us simulate nearest or bilerp filtering
        // in the same shader. When the texture is nearest filtered, fSizeName.z is set to 1/2 so
        // this becomes a step function centered at .5 away from the clamped coordinate (but the
        // domain for decal is inset by .5 so the edge lines up properly). When bilerp, fSizeName.z
        // is set to 1 and it becomes a simple linear blend between texture and transparent.
        builder->codeAppendf("if (err > %s.z) { err = 1.0; } else if (%s.z < 1) { err = 0.0; }",
                             fDecalName.c_str(), fDecalName.c_str());
        builder->codeAppendf("%s = mix(inside, half4(0, 0, 0, 0), err);", outColor);
    } else {
        // A simple look up
        builder->codeAppendf("%s = inside;", outColor);
    }
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        GrTextureProxy* proxy,
                                        const GrSamplerState& sampler) {
    GrTexture* tex = proxy->peekTexture();
    SkASSERT(fHasMode && textureDomain.modeX() == fModeX && textureDomain.modeY() == fModeY);
    if (kIgnore_Mode != textureDomain.modeX() || kIgnore_Mode != textureDomain.modeY()) {
        bool sendDecalData = textureDomain.modeX() == kDecal_Mode ||
                             textureDomain.modeY() == kDecal_Mode;

        // If the texture is using nearest filtering, then the decal filter weight should step from
        // 0 (texture) to 1 (transparent) one half pixel away from the domain. When doing any other
        // form of filtering, the weight should be 1.0 so that it smoothly interpolates between the
        // texture and transparent.
        SkScalar decalFilterWeight = sampler.filter() == GrSamplerState::Filter::kNearest ?
                SK_ScalarHalf : 1.0f;
        SkScalar wInv, hInv, h;
        if (proxy->textureType() == GrTextureType::kRectangle) {
            wInv = hInv = 1.f;
            h = tex->height();

            // Don't do any scaling by texture size for decal filter rate, it's already in pixels
            if (sendDecalData) {
                pdman.set3f(fDecalUni, 1.f, 1.f, decalFilterWeight);
            }
        } else {
            wInv = SK_Scalar1 / tex->width();
            hInv = SK_Scalar1 / tex->height();
            h = 1.f;

            if (sendDecalData) {
                pdman.set3f(fDecalUni, tex->width(), tex->height(), decalFilterWeight);
            }
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
            pdman.set4fv(fDomainUni, 1, values);
            memcpy(fPrevDomain, values, kPrevDomainCount * sizeof(float));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrTextureDomainEffect::Make(
        sk_sp<GrTextureProxy> proxy,
        const SkMatrix& matrix,
        const SkRect& domain,
        GrTextureDomain::Mode mode,
        GrSamplerState::Filter filterMode) {
    return Make(std::move(proxy), matrix, domain, mode, mode,
                GrSamplerState(GrSamplerState::WrapMode::kClamp, filterMode));
}

std::unique_ptr<GrFragmentProcessor> GrTextureDomainEffect::Make(
        sk_sp<GrTextureProxy> proxy,
        const SkMatrix& matrix,
        const SkRect& domain,
        GrTextureDomain::Mode modeX,
        GrTextureDomain::Mode modeY,
        const GrSamplerState& sampler) {
    // If both domain modes happen to be ignore, it would be faster to just drop the domain logic
    // entirely Technically, we could also use the simple texture effect if the domain modes agree
    // with the sampler modes and the proxy is the same size as the domain. It's a lot easier for
    // calling code to detect these cases and handle it themselves.
    return std::unique_ptr<GrFragmentProcessor>(new GrTextureDomainEffect(
            std::move(proxy), matrix, domain, modeX, modeY, sampler));
}

GrTextureDomainEffect::GrTextureDomainEffect(sk_sp<GrTextureProxy> proxy,
                                             const SkMatrix& matrix,
                                             const SkRect& domain,
                                             GrTextureDomain::Mode modeX,
                                             GrTextureDomain::Mode modeY,
                                             const GrSamplerState& sampler)
        : INHERITED(kGrTextureDomainEffect_ClassID,
                    ModulateForSamplerOptFlags(proxy->config(),
                            GrTextureDomain::IsDecalSampled(sampler, modeX, modeY)))
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
            SkString coords2D =
                              fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);

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
            GrTextureProxy* proxy = tde.textureSampler(0).proxy();

            fGLDomain.setData(pdman, domain, proxy, tde.textureSampler(0).samplerState());
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
            GrSamplerState(GrSamplerState::WrapMode::kClamp, bilerp ?
                           GrSamplerState::Filter::kBilerp : GrSamplerState::Filter::kNearest));
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
        , fTextureDomain(proxy.get(),
                         GrTextureDomain::MakeTexelDomain(subset, GrTextureDomain::kDecal_Mode),
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
            args.fFragBuilder->codeAppendf("half2 coords = half2(sk_FragCoord.xy * %s.xy + %s.zw);",
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
            GrTextureProxy* proxy = dstdfp.textureSampler(0).proxy();
            GrTexture* texture = proxy->peekTexture();

            fGLDomain.setData(pdman, dstdfp.fTextureDomain, proxy,
                              dstdfp.textureSampler(0).samplerState());
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
