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

GrTextureDomain::GrTextureDomain(GrSurfaceProxy* proxy, const SkRect& domain, Mode modeX,
                                 Mode modeY, int index)
    : fModeX(modeX)
    , fModeY(modeY)
    , fIndex(index) {

    if (!proxy) {
        SkASSERT(modeX == kIgnore_Mode && modeY == kIgnore_Mode);
        return;
    }

    const SkRect kFullRect = proxy->getBoundsRect();

    // We don't currently handle domains that are empty or don't intersect the texture.
    // It is OK if the domain rect is a line or point, but it should not be inverted. We do not
    // handle rects that do not intersect the [0..1]x[0..1] rect.
    SkASSERT(domain.isSorted());
    fDomain.fLeft = SkScalarPin(domain.fLeft, 0.0f, kFullRect.fRight);
    fDomain.fRight = SkScalarPin(domain.fRight, fDomain.fLeft, kFullRect.fRight);
    fDomain.fTop = SkScalarPin(domain.fTop, 0.0f, kFullRect.fBottom);
    fDomain.fBottom = SkScalarPin(domain.fBottom, fDomain.fTop, kFullRect.fBottom);
    SkASSERT(fDomain.fLeft <= fDomain.fRight);
    SkASSERT(fDomain.fTop <= fDomain.fBottom);
}

GrTextureDomain::GrTextureDomain(const SkRect& domain, Mode modeX, Mode modeY, int index)
        : fDomain(domain), fModeX(modeX), fModeY(modeY), fIndex(index) {
    // We don't currently handle domains that are empty or don't intersect the texture.
    // It is OK if the domain rect is a line or point, but it should not be inverted.
    SkASSERT(domain.isSorted());
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

void GrTextureDomain::GLDomain::sampleProcessor(const GrTextureDomain& textureDomain,
                                                const char* inColor,
                                                const char* outColor,
                                                const SkString& inCoords,
                                                GrGLSLFragmentProcessor* parent,
                                                GrGLSLFragmentProcessor::EmitArgs& args,
                                                int childIndex) {
    auto appendProcessorSample = [parent, &args, childIndex, inColor](const char* coord) {
        SkString outColor("childColor");
        parent->invokeChild(childIndex, inColor, &outColor, args, coord);
        return outColor;
    };
    this->sample(args.fFragBuilder, args.fUniformHandler, textureDomain, outColor, inCoords,
                 appendProcessorSample);
}

void GrTextureDomain::GLDomain::sampleTexture(GrGLSLShaderBuilder* builder,
                                              GrGLSLUniformHandler* uniformHandler,
                                              const GrShaderCaps* shaderCaps,
                                              const GrTextureDomain& textureDomain,
                                              const char* outColor,
                                              const SkString& inCoords,
                                              GrGLSLFragmentProcessor::SamplerHandle sampler,
                                              const char* inModulateColor) {
    auto appendTextureSample = [&sampler, inModulateColor, builder](const char* coord) {
        builder->codeAppend("half4 textureColor = ");
        builder->appendTextureLookupAndModulate(inModulateColor, sampler, coord);
        builder->codeAppend(";");
        return SkString("textureColor");
    };
    this->sample(builder, uniformHandler, textureDomain, outColor, inCoords, appendTextureSample);
}

void GrTextureDomain::GLDomain::sample(GrGLSLShaderBuilder* builder,
                                       GrGLSLUniformHandler* uniformHandler,
                                       const GrTextureDomain& textureDomain,
                                       const char* outColor,
                                       const SkString& inCoords,
                                       const std::function<AppendSample>& appendSample) {
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

    // Sample 'appendSample' at the clamped coordinate location.
    SkString color = appendSample("clampedCoord");

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
        builder->codeAppendf("%s = mix(%s, half4(0, 0, 0, 0), err);", outColor, color.c_str());
    } else {
        // A simple look up
        builder->codeAppendf("%s = %s;", outColor, color.c_str());
    }
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        const GrSurfaceProxy* proxy,
                                        const GrSamplerState& state) {
    // We want a hard transition from texture content to trans-black in nearest mode.
    bool filterDecal = state.filter() != GrSamplerState::Filter::kNearest;
    this->setData(pdman, textureDomain, proxy, filterDecal);
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        bool filterIfDecal) {
    this->setData(pdman, textureDomain, nullptr, filterIfDecal);
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        const GrSurfaceProxy* proxy,
                                        bool filterIfDecal) {
    SkASSERT(fHasMode && textureDomain.modeX() == fModeX && textureDomain.modeY() == fModeY);
    if (kIgnore_Mode == textureDomain.modeX() && kIgnore_Mode == textureDomain.modeY()) {
        return;
    }
    // If the texture is using nearest filtering, then the decal filter weight should step from
    // 0 (texture) to 1 (transparent) one half pixel away from the domain. When doing any other
    // form of filtering, the weight should be 1.0 so that it smoothly interpolates between the
    // texture and transparent.
    // Start off assuming we're in pixel units and later adjust if we have to deal with normalized
    // texture coords.
    float decalFilterWeights[3] = {1.f, 1.f, filterIfDecal ? 1.f : 0.5f};
    bool sendDecalData = textureDomain.modeX() == kDecal_Mode ||
                         textureDomain.modeY() == kDecal_Mode;
    float tempDomainValues[4];
    const float* values;
    if (proxy) {
        SkScalar wInv, hInv, h;
        GrTexture* tex = proxy->peekTexture();
        if (proxy->backendFormat().textureType() == GrTextureType::kRectangle) {
            wInv = hInv = 1.f;
            h = tex->height();
            // Don't do any scaling by texture size for decal filter rate, it's already in
            // pixels
        } else {
            wInv = SK_Scalar1 / tex->width();
            hInv = SK_Scalar1 / tex->height();
            h = 1.f;

            // Account for texture coord normalization in decal filter weights.
            decalFilterWeights[0] = tex->width();
            decalFilterWeights[1] = tex->height();
        }

        tempDomainValues[0] = SkScalarToFloat(textureDomain.domain().fLeft * wInv);
        tempDomainValues[1] = SkScalarToFloat(textureDomain.domain().fTop * hInv);
        tempDomainValues[2] = SkScalarToFloat(textureDomain.domain().fRight * wInv);
        tempDomainValues[3] = SkScalarToFloat(textureDomain.domain().fBottom * hInv);

        if (proxy->backendFormat().textureType() == GrTextureType::kRectangle) {
            SkASSERT(tempDomainValues[0] >= 0.0f && tempDomainValues[0] <= proxy->width());
            SkASSERT(tempDomainValues[1] >= 0.0f && tempDomainValues[1] <= proxy->height());
            SkASSERT(tempDomainValues[2] >= 0.0f && tempDomainValues[2] <= proxy->width());
            SkASSERT(tempDomainValues[3] >= 0.0f && tempDomainValues[3] <= proxy->height());
        } else {
            SkASSERT(tempDomainValues[0] >= 0.0f && tempDomainValues[0] <= 1.0f);
            SkASSERT(tempDomainValues[1] >= 0.0f && tempDomainValues[1] <= 1.0f);
            SkASSERT(tempDomainValues[2] >= 0.0f && tempDomainValues[2] <= 1.0f);
            SkASSERT(tempDomainValues[3] >= 0.0f && tempDomainValues[3] <= 1.0f);
        }

        // vertical flip if necessary
        if (kBottomLeft_GrSurfaceOrigin == proxy->origin()) {
            tempDomainValues[1] = h - tempDomainValues[1];
            tempDomainValues[3] = h - tempDomainValues[3];

            // The top and bottom were just flipped, so correct the ordering
            // of elements so that values = (l, t, r, b).
            using std::swap;
            swap(tempDomainValues[1], tempDomainValues[3]);
        }
        values = tempDomainValues;
    } else {
        values = textureDomain.domain().asScalars();
    }
    if (!std::equal(values, values + 4, fPrevDomain)) {
        pdman.set4fv(fDomainUni, 1, values);
        std::copy_n(values, 4, fPrevDomain);
    }
    if (sendDecalData &&
        !std::equal(decalFilterWeights, decalFilterWeights + 3, fPrevDeclFilterWeights)) {
        pdman.set3fv(fDecalUni, 1, decalFilterWeights);
        std::copy_n(decalFilterWeights, 3, fPrevDeclFilterWeights);
    }
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrDomainEffect::Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                          const SkRect& domain,
                                                          GrTextureDomain::Mode mode,
                                                          bool decalIsFiltered) {
    return Make(std::move(fp), domain, mode, mode, decalIsFiltered);
}

std::unique_ptr<GrFragmentProcessor> GrDomainEffect::Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                          const SkRect& domain,
                                                          GrTextureDomain::Mode modeX,
                                                          GrTextureDomain::Mode modeY,
                                                          bool decalIsFiltered) {
    if (modeX == GrTextureDomain::kIgnore_Mode && modeY == GrTextureDomain::kIgnore_Mode) {
        return fp;
    }
    int count = 0;
    GrCoordTransform* coordTransform = nullptr;
    for (auto [transform, ignored] : GrFragmentProcessor::FPCoordTransformRange(*fp)) {
        ++count;
        coordTransform = &transform;
    }
    // If there are no coord transforms on the passed FP or it's children then there's no need to
    // enforce a domain.
    // We have a limitation that only one coord transform is support when overriding local coords.
    // If that limit were relaxed we would need to add a coord transform for each descendent FP
    // transform and possibly have multiple domain rects to account for different proxy
    // normalization and y-reversals.
    if (count != 1) {
        return fp;
    }
    GrCoordTransform transformCopy = *coordTransform;
    // Reset the child FP's coord transform.
    *coordTransform = {};
    // If both domain modes happen to be ignore, it would be faster to just drop the domain logic
    // entirely and return the original FP. We'd need a GrMatrixProcessor if the matrix is not
    // identity, though.
    return std::unique_ptr<GrFragmentProcessor>(new GrDomainEffect(
            std::move(fp), transformCopy, domain, modeX, modeY, decalIsFiltered));
}

std::unique_ptr<GrFragmentProcessor> GrDomainEffect::Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                          const SkRect& domain,
                                                          GrTextureDomain::Mode mode,
                                                          GrSamplerState::Filter filter) {
    bool filterIfDecal = filter != GrSamplerState::Filter::kNearest;
    return Make(std::move(fp), domain, mode, filterIfDecal);
}

std::unique_ptr<GrFragmentProcessor> GrDomainEffect::Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                          const SkRect& domain,
                                                          GrTextureDomain::Mode modeX,
                                                          GrTextureDomain::Mode modeY,
                                                          GrSamplerState::Filter filter) {
    bool filterIfDecal = filter != GrSamplerState::Filter::kNearest;
    return Make(std::move(fp), domain, modeX, modeY, filterIfDecal);
}
GrFragmentProcessor::OptimizationFlags GrDomainEffect::Flags(GrFragmentProcessor* fp,
                                                             GrTextureDomain::Mode modeX,
                                                             GrTextureDomain::Mode modeY) {
    auto fpFlags = GrFragmentProcessor::ProcessorOptimizationFlags(fp);
    if (modeX == GrTextureDomain::kDecal_Mode || modeY == GrTextureDomain::kDecal_Mode) {
        return fpFlags & ~kPreservesOpaqueInput_OptimizationFlag;
    }
    return fpFlags;
}

GrDomainEffect::GrDomainEffect(std::unique_ptr<GrFragmentProcessor> fp,
                               const GrCoordTransform& coordTransform,
                               const SkRect& domain,
                               GrTextureDomain::Mode modeX,
                               GrTextureDomain::Mode modeY,
                               bool decalIsFiltered)
        : INHERITED(kGrDomainEffect_ClassID, Flags(fp.get(), modeX, modeY))
        , fCoordTransform(coordTransform)
        , fDomain(domain, modeX, modeY)
        , fDecalIsFiltered(decalIsFiltered) {
    SkASSERT(fp);
    fp->setSampledWithExplicitCoords(true);
    this->registerChildProcessor(std::move(fp));
    this->addCoordTransform(&fCoordTransform);
    if (fDomain.modeX() != GrTextureDomain::kDecal_Mode &&
        fDomain.modeY() != GrTextureDomain::kDecal_Mode) {
        // Canonicalize this don't care value so we don't have to worry about it elsewhere.
        fDecalIsFiltered = false;
    }
}

GrDomainEffect::GrDomainEffect(const GrDomainEffect& that)
        : INHERITED(kGrDomainEffect_ClassID, that.optimizationFlags())
        , fCoordTransform(that.fCoordTransform)
        , fDomain(that.fDomain)
        , fDecalIsFiltered(that.fDecalIsFiltered) {
    auto child = that.childProcessor(0).clone();
    child->setSampledWithExplicitCoords(true);
    this->registerChildProcessor(std::move(child));
    this->addCoordTransform(&fCoordTransform);
}

void GrDomainEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                           GrProcessorKeyBuilder* b) const {
    b->add32(GrTextureDomain::GLDomain::DomainKey(fDomain));
}

GrGLSLFragmentProcessor* GrDomainEffect::onCreateGLSLInstance() const {
    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const GrDomainEffect& de = args.fFp.cast<GrDomainEffect>();
            const GrTextureDomain& domain = de.fDomain;

            SkString coords2D =
                    args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);

            fGLDomain.sampleProcessor(domain, args.fInputColor, args.fOutputColor, coords2D, this,
                                      args, 0);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const GrDomainEffect& de = fp.cast<GrDomainEffect>();
            const GrTextureDomain& domain = de.fDomain;
            fGLDomain.setData(pdman, domain, de.fCoordTransform.proxy(), de.fDecalIsFiltered);
        }

    private:
        GrTextureDomain::GLDomain         fGLDomain;
    };

    return new GLSLProcessor;
}

bool GrDomainEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    auto& td = sBase.cast<GrDomainEffect>();
    return fDomain == td.fDomain && fDecalIsFiltered == td.fDecalIsFiltered;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDomainEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrDomainEffect::TestCreate(GrProcessorTestData* d) {
    do {
        GrTextureDomain::Mode modeX =
                (GrTextureDomain::Mode)d->fRandom->nextULessThan(GrTextureDomain::kModeCount);
        GrTextureDomain::Mode modeY =
                (GrTextureDomain::Mode)d->fRandom->nextULessThan(GrTextureDomain::kModeCount);
        auto child = GrProcessorUnitTest::MakeChildFP(d);
        const auto* childPtr = child.get();
        SkRect domain;
        // We assert if the child's coord transform has a proxy and the domain rect is outside its
        // bounds.
        GrFragmentProcessor::CoordTransformIter ctIter(*child);
        if (!ctIter) {
            continue;
        }
        auto [transform, fp] = *ctIter;
        if (auto proxy = transform.proxy()) {
            auto [w, h] = proxy->backingStoreDimensions();
            domain.fLeft   = d->fRandom->nextRangeScalar(0, w);
            domain.fRight  = d->fRandom->nextRangeScalar(0, w);
            domain.fTop    = d->fRandom->nextRangeScalar(0, h);
            domain.fBottom = d->fRandom->nextRangeScalar(0, h);
        } else {
            domain.fLeft   = d->fRandom->nextRangeScalar(-100.f, 100.f);
            domain.fRight  = d->fRandom->nextRangeScalar(-100.f, 100.f);
            domain.fTop    = d->fRandom->nextRangeScalar(-100.f, 100.f);
            domain.fBottom = d->fRandom->nextRangeScalar(-100.f, 100.f);
        }
        domain.sort();
        bool filterIfDecal = d->fRandom->nextBool();
        auto result = GrDomainEffect::Make(std::move(child), domain, modeX, modeY, filterIfDecal);
        if (result && result.get() != childPtr) {
            return result;
        }
    } while (true);
}
#endif

///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<GrFragmentProcessor> GrDeviceSpaceTextureDecalFragmentProcessor::Make(
        sk_sp<GrSurfaceProxy> proxy, const SkIRect& subset, const SkIPoint& deviceSpaceOffset) {
    return std::unique_ptr<GrFragmentProcessor>(new GrDeviceSpaceTextureDecalFragmentProcessor(
            std::move(proxy), subset, deviceSpaceOffset));
}

GrDeviceSpaceTextureDecalFragmentProcessor::GrDeviceSpaceTextureDecalFragmentProcessor(
        sk_sp<GrSurfaceProxy> proxy, const SkIRect& subset, const SkIPoint& deviceSpaceOffset)
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
            GrSurfaceProxy* proxy = dstdfp.textureSampler(0).proxy();
            SkISize textureDims = proxy->backingStoreDimensions();

            fGLDomain.setData(pdman, dstdfp.fTextureDomain, proxy,
                              dstdfp.textureSampler(0).samplerState());
            float iw = 1.f / textureDims.width();
            float ih = 1.f / textureDims.height();
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
