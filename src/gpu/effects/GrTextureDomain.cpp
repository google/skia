/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrTextureDomain.h"

#include "include/private/SkFloatingPoint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/effects/GrTextureEffect.h"
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
    fDomain.fLeft = SkTPin(domain.fLeft, 0.0f, kFullRect.fRight);
    fDomain.fRight = SkTPin(domain.fRight, fDomain.fLeft, kFullRect.fRight);
    fDomain.fTop = SkTPin(domain.fTop, 0.0f, kFullRect.fBottom);
    fDomain.fBottom = SkTPin(domain.fBottom, fDomain.fTop, kFullRect.fBottom);
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

static void append_wrap(GrGLSLShaderBuilder* builder, GrTextureDomain::Mode mode,
                        const char* inCoord, const char* domainStart, const char* domainEnd,
                        bool is2D, const char* out) {
    switch(mode) {
        case GrTextureDomain::kIgnore_Mode:
            builder->codeAppendf("%s = %s;\n", out, inCoord);
            break;
        case GrTextureDomain::kDecal_Mode:
            // The lookup coordinate to use for decal will be clamped just like kClamp_Mode,
            // it's just that the post-processing will be different, so fall through
        case GrTextureDomain::kClamp_Mode:
            builder->codeAppendf("%s = clamp(%s, %s, %s);", out, inCoord, domainStart, domainEnd);
            break;
        case GrTextureDomain::kRepeat_Mode:
            builder->codeAppendf("%s = mod(%s - %s, %s - %s) + %s;", out, inCoord, domainStart,
                                 domainEnd, domainStart, domainStart);
            break;
        case GrTextureDomain::kMirrorRepeat_Mode: {
            const char* type = is2D ? "float2" : "float";
            builder->codeAppend("{");
            builder->codeAppendf("%s w = %s - %s;", type, domainEnd, domainStart);
            builder->codeAppendf("%s w2 = 2 * w;", type);
            builder->codeAppendf("%s m = mod(%s - %s, w2);", type, inCoord, domainStart);
            builder->codeAppendf("%s = mix(m, w2 - m, step(w, m)) + %s;", out, domainStart);
            builder->codeAppend("}");
            break;
        }
    }
}

void GrTextureDomain::GLDomain::sampleProcessor(const GrTextureDomain& textureDomain,
                                                const char* inColor,
                                                const char* outColor,
                                                const SkString& inCoords,
                                                GrGLSLFragmentProcessor* parent,
                                                GrGLSLFragmentProcessor::EmitArgs& args,
                                                int childIndex) {
    auto appendProcessorSample = [parent, &args, childIndex, inColor](const char* coord) {
        return parent->invokeChild(childIndex, inColor, args, coord);
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
        builder->appendTextureLookupAndBlend(inModulateColor, SkBlendMode::kModulate, sampler,
                                             coord);
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
    builder->codeAppend("float2 clampedCoord;");
    SkString start;
    SkString end;
    bool is2D = textureDomain.modeX() == textureDomain.modeY();
    if (is2D) {
        // Doing the domain setup using vectors seems to avoid shader compilation issues on
        // Chromecast, possibly due to reducing shader length.
        start.printf("%s.xy", fDomainName.c_str());
        end.printf("%s.zw", fDomainName.c_str());
        append_wrap(builder, textureDomain.modeX(), "origCoord", start.c_str(), end.c_str(),
                    true, "clampedCoord");
    } else {
        // Apply x mode to the x coordinate using the left and right edges of the domain rect
        // (stored as the x and z components of the domain uniform).
        start.printf("%s.x", fDomainName.c_str());
        end.printf("%s.z", fDomainName.c_str());
        append_wrap(builder, textureDomain.modeX(), "origCoord.x", start.c_str(), end.c_str(),
                    false, "clampedCoord.x");
        // Repeat the same logic for y.
        start.printf("%s.y", fDomainName.c_str());
        end.printf("%s.w", fDomainName.c_str());
        append_wrap(builder, textureDomain.modeY(), "origCoord.y", start.c_str(), end.c_str(),
                    false, "clampedCoord.y");
    }
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
                                        const GrSurfaceProxyView& view,
                                        GrSamplerState state) {
    // We want a hard transition from texture content to trans-black in nearest mode.
    bool filterDecal = state.filter() != GrSamplerState::Filter::kNearest;
    this->setData(pdman, textureDomain, view.proxy(), view.origin(), filterDecal);
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        bool filterIfDecal) {
    // The origin we pass here doesn't matter
    this->setData(pdman, textureDomain, nullptr, kTopLeft_GrSurfaceOrigin, filterIfDecal);
}

void GrTextureDomain::GLDomain::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrTextureDomain& textureDomain,
                                        const GrSurfaceProxy* proxy,
                                        GrSurfaceOrigin origin,
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
        if (kBottomLeft_GrSurfaceOrigin == origin) {
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
