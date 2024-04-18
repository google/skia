/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrTextureEffect.h"

#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <algorithm>
#include <cmath>
#include <utility>

enum SkAlphaType : int;
struct GrShaderCaps;

using Wrap = GrSamplerState::WrapMode;
using Filter = GrSamplerState::Filter;
using MipmapMode = GrSamplerState::MipmapMode;

struct GrTextureEffect::Sampling {
    GrSamplerState fHWSampler;
    ShaderMode fShaderModes[2] = {ShaderMode::kNone, ShaderMode::kNone};
    SkRect fShaderSubset = {0, 0, 0, 0};
    SkRect fShaderClamp  = {0, 0, 0, 0};
    float fBorder[4] = {0, 0, 0, 0};
    Sampling(Filter filter, MipmapMode mm) : fHWSampler(filter, mm) {}
    Sampling(const GrSurfaceProxy& proxy,
             GrSamplerState wrap,
             const SkRect&,
             const SkRect*,
             const float border[4],
             bool alwaysUseShaderTileMode,
             const GrCaps&,
             SkVector linearFilterInset = {0.5f, 0.5f});
    inline bool hasBorderAlpha() const;
};

GrTextureEffect::Sampling::Sampling(const GrSurfaceProxy& proxy,
                                    GrSamplerState sampler,
                                    const SkRect& subset,
                                    const SkRect* domain,
                                    const float border[4],
                                    bool alwaysUseShaderTileMode,
                                    const GrCaps& caps,
                                    SkVector linearFilterInset) {
    struct Span {
        float fA = 0.f, fB = 0.f;

        Span makeInset(float o) const {
            Span r = {fA + o, fB - o};
            if (r.fA > r.fB) {
                r.fA = r.fB = (r.fA + r.fB) / 2;
            }
            return r;
        }

        bool contains(Span r) const { return fA <= r.fA && fB >= r.fB; }
    };
    struct Result1D {
        ShaderMode fShaderMode = ShaderMode::kNone;
        Span fShaderSubset     = {};
        Span fShaderClamp      = {};
        Wrap fHWWrap           = Wrap::kClamp;
    };

    auto type   = proxy.asTextureProxy()->textureType();
    auto filter = sampler.filter();
    auto mm     = sampler.mipmapMode();

    auto canDoWrapInHW = [&](int size, Wrap wrap) {
        if (alwaysUseShaderTileMode) {
            return false;
        }
        // TODO: Use HW border color when available.
        if (wrap == Wrap::kClampToBorder &&
            (!caps.clampToBorderSupport() || border[0] || border[1] || border[2] || border[3])) {
            return false;
        }
        if (wrap != Wrap::kClamp && !caps.npotTextureTileSupport() && !SkIsPow2(size)) {
            return false;
        }
        if (type != GrTextureType::k2D &&
                   !(wrap == Wrap::kClamp || wrap == Wrap::kClampToBorder)) {
            return false;
        }
        return true;
    };

    SkISize dim = proxy.isFullyLazy() ? SkISize{-1, -1} : proxy.backingStoreDimensions();

    // TODO: Right now if we use shader based subsetting for any reason we just completely drop
    // aniso. Longer term allow shader subsetting, reusing the special repeat mode LOD selection
    // logic for mip maps, and simply don't attempt to restrict ansiso's computed samples to the
    // subset. That is use "subsetting" but not "clamping"/insetting in terms of the shader gen
    // logic.
    bool aniso = sampler.isAniso();
    SkASSERT(!aniso || caps.anisoSupport());
    if (aniso) {
        bool anisoSubset = !subset.contains(proxy.backingStoreBoundsRect()) &&
                           (!domain || !subset.contains(*domain));
        bool needsShaderWrap = !canDoWrapInHW(dim.width(),  sampler.wrapModeX()) ||
                               !canDoWrapInHW(dim.height(), sampler.wrapModeY());
        if (needsShaderWrap || anisoSubset) {
            MipmapMode newMM = proxy.asTextureProxy()->mipmapped() == skgpu::Mipmapped::kYes
                                       ? MipmapMode::kLinear
                                       : MipmapMode::kNone;
            sampler = GrSamplerState(sampler.wrapModeX(),
                                     sampler.wrapModeY(),
                                     SkFilterMode::kLinear,
                                     newMM);
            aniso = false;
        }
    }

    auto resolve = [&](int size, Wrap wrap, Span subset, Span domain, float linearFilterInset) {
        Result1D r;
        bool canDoModeInHW = canDoWrapInHW(size, wrap);
        if (canDoModeInHW && size > 0 && subset.fA <= 0 && subset.fB >= size) {
            r.fShaderMode = ShaderMode::kNone;
            r.fHWWrap = wrap;
            r.fShaderSubset = r.fShaderClamp = {0, 0};
            return r;
        }

        r.fShaderSubset = subset;
        bool domainIsSafe = false;
        if (filter == Filter::kNearest) {
            Span isubset{std::floor(subset.fA), std::ceil(subset.fB)};
            if (domain.fA > isubset.fA && domain.fB < isubset.fB) {
                domainIsSafe = true;
            }
            // This inset prevents sampling neighboring texels that could occur when
            // texture coords fall exactly at texel boundaries (depending on precision
            // and GPU-specific snapping at the boundary).
            r.fShaderClamp = isubset.makeInset(0.5f + kInsetEpsilon);
        } else {
            r.fShaderClamp = subset.makeInset(linearFilterInset + kInsetEpsilon);
            if (r.fShaderClamp.contains(domain)) {
                domainIsSafe = true;
            }
        }
        if (!alwaysUseShaderTileMode && domainIsSafe) {
            // The domain of coords that will be used won't access texels outside of the subset.
            // So the wrap mode effectively doesn't matter. We use kClamp since it is always
            // supported.
            r.fShaderMode = ShaderMode::kNone;
            r.fHWWrap = Wrap::kClamp;
            r.fShaderSubset = r.fShaderClamp = {0, 0};
            return r;
        }
        r.fShaderMode = GetShaderMode(wrap, filter, mm);
        r.fHWWrap = Wrap::kClamp;
        return r;
    };

    Result1D x, y;
    if (!aniso) {
        Span subsetX{subset.fLeft, subset.fRight};
        auto domainX = domain ? Span{domain->fLeft, domain->fRight}
                              : Span{SK_FloatNegativeInfinity, SK_FloatInfinity};
        x = resolve(dim.width(), sampler.wrapModeX(), subsetX, domainX, linearFilterInset.fX);

        Span subsetY{subset.fTop, subset.fBottom};
        auto domainY = domain ? Span{domain->fTop, domain->fBottom}
                              : Span{SK_FloatNegativeInfinity, SK_FloatInfinity};
        y = resolve(dim.height(), sampler.wrapModeY(), subsetY, domainY, linearFilterInset.fY);
    } else {
        x.fHWWrap = sampler.wrapModeX();
        y.fHWWrap = sampler.wrapModeY();
    }

    fHWSampler = aniso ? GrSamplerState::Aniso(x.fHWWrap,
                                               y.fHWWrap,
                                               sampler.maxAniso(),
                                               proxy.asTextureProxy()->mipmapped())
                       : GrSamplerState{x.fHWWrap, y.fHWWrap, filter, mm};
    fShaderModes[0] = x.fShaderMode;
    fShaderModes[1] = y.fShaderMode;
    fShaderSubset = {x.fShaderSubset.fA, y.fShaderSubset.fA,
                     x.fShaderSubset.fB, y.fShaderSubset.fB};
    fShaderClamp = {x.fShaderClamp.fA, y.fShaderClamp.fA,
                    x.fShaderClamp.fB, y.fShaderClamp.fB};
    std::copy_n(border, 4, fBorder);
}

bool GrTextureEffect::Sampling::hasBorderAlpha() const {
    if (fHWSampler.wrapModeX() == Wrap::kClampToBorder ||
        fHWSampler.wrapModeY() == Wrap::kClampToBorder) {
        return true;
    }
    if (ShaderModeIsClampToBorder(fShaderModes[0]) || ShaderModeIsClampToBorder(fShaderModes[1])) {
        return fBorder[3] < 1.f;
    }
    return false;
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           Filter filter,
                                                           MipmapMode mm) {
    Sampling sampling = Sampling(filter, mm);
    std::unique_ptr<GrFragmentProcessor> te(new GrTextureEffect(std::move(view),
                                                                alphaType,
                                                                sampling));
    return GrMatrixEffect::Make(matrix, std::move(te));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           GrSamplerState sampler,
                                                           const GrCaps& caps,
                                                           const float border[4]) {
    Sampling sampling(*view.proxy(),
                      sampler,
                      SkRect::Make(view.proxy()->dimensions()),
                      nullptr,
                      border,
                      false,
                      caps);
    std::unique_ptr<GrFragmentProcessor> te(new GrTextureEffect(std::move(view),
                                                                alphaType,
                                                                sampling));
    return GrMatrixEffect::Make(matrix, std::move(te));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeSubset(GrSurfaceProxyView view,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler,
                                                                 const SkRect& subset,
                                                                 const GrCaps& caps,
                                                                 const float border[4],
                                                                 bool alwaysUseShaderTileMode) {
    Sampling sampling(*view.proxy(),
                      sampler,
                      subset,
                      nullptr,
                      border,
                      alwaysUseShaderTileMode,
                      caps);
    std::unique_ptr<GrFragmentProcessor> te(new GrTextureEffect(std::move(view),
                                                                alphaType,
                                                                sampling));
    return GrMatrixEffect::Make(matrix, std::move(te));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeSubset(GrSurfaceProxyView view,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler,
                                                                 const SkRect& subset,
                                                                 const SkRect& domain,
                                                                 const GrCaps& caps,
                                                                 const float border[4]) {
    Sampling sampling(*view.proxy(), sampler, subset, &domain, border, false, caps);
    std::unique_ptr<GrFragmentProcessor> te(new GrTextureEffect(std::move(view),
                                                                alphaType,
                                                                sampling));
    return GrMatrixEffect::Make(matrix, std::move(te));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeCustomLinearFilterInset(
        GrSurfaceProxyView view,
        SkAlphaType alphaType,
        const SkMatrix& matrix,
        Wrap wx,
        Wrap wy,
        const SkRect& subset,
        const SkRect* domain,
        SkVector inset,
        const GrCaps& caps,
        const float border[4]) {
    GrSamplerState sampler(wx, wy, Filter::kLinear);
    Sampling sampling(*view.proxy(), sampler, subset, domain, border, false, caps, inset);
    std::unique_ptr<GrFragmentProcessor> te(new GrTextureEffect(std::move(view),
                                                                alphaType,
                                                                sampling));
    return GrMatrixEffect::Make(matrix, std::move(te));
}

SkMatrix GrTextureEffect::coordAdjustmentMatrix() const {
    SkMatrix m;
    GrTexture* texture = this->texture();
    SkISize d = texture->dimensions();
    if (this->matrixEffectShouldNormalize()) {
        if (fView.origin() == kBottomLeft_GrSurfaceOrigin) {
            m.setScaleTranslate(1.f / d.width(), -1.f / d.height(), 0, 1);
        } else {
            m.setScale(1.f / d.width(), 1.f / d.height());
        }
    } else {
        if (fView.origin() == kBottomLeft_GrSurfaceOrigin) {
            m.setScaleTranslate(1.f, -1.f, 0, d.height());
        }
    }
    return m;
}

GrTextureEffect::ShaderMode GrTextureEffect::GetShaderMode(Wrap wrap,
                                                           Filter filter,
                                                           MipmapMode mm) {
    switch (wrap) {
        case Wrap::kMirrorRepeat:
            return ShaderMode::kMirrorRepeat;
        case Wrap::kClamp:
            return ShaderMode::kClamp;
        case Wrap::kRepeat:
            switch (mm) {
                case MipmapMode::kNone:
                    switch (filter) {
                        case Filter::kNearest: return ShaderMode::kRepeat_Nearest_None;
                        case Filter::kLinear:  return ShaderMode::kRepeat_Linear_None;
                    }
                    SkUNREACHABLE;
                case MipmapMode::kNearest:
                case MipmapMode::kLinear:
                    switch (filter) {
                        case Filter::kNearest: return ShaderMode::kRepeat_Nearest_Mipmap;
                        case Filter::kLinear:  return ShaderMode::kRepeat_Linear_Mipmap;
                    }
                    SkUNREACHABLE;
            }
            SkUNREACHABLE;
        case Wrap::kClampToBorder:
            return filter == Filter::kNearest ? ShaderMode::kClampToBorder_Nearest
                                              : ShaderMode::kClampToBorder_Filter;
    }
    SkUNREACHABLE;
}

inline bool GrTextureEffect::ShaderModeIsClampToBorder(ShaderMode m) {
    return m == ShaderMode::kClampToBorder_Nearest || m == ShaderMode::kClampToBorder_Filter;
}

bool GrTextureEffect::ShaderModeRequiresUnormCoord(ShaderMode m) {
    switch (m) {
        case ShaderMode::kNone:                     return false;
        case ShaderMode::kClamp:                    return false;
        case ShaderMode::kRepeat_Nearest_None:      return false;
        case ShaderMode::kRepeat_Linear_None:       return true;
        case ShaderMode::kRepeat_Nearest_Mipmap:    return true;
        case ShaderMode::kRepeat_Linear_Mipmap:     return true;
        case ShaderMode::kMirrorRepeat:             return false;
        case ShaderMode::kClampToBorder_Nearest:    return true;
        case ShaderMode::kClampToBorder_Filter:     return true;
    }
    SkUNREACHABLE;
}

void GrTextureEffect::Impl::emitCode(EmitArgs& args) {
    using ShaderMode = GrTextureEffect::ShaderMode;

    auto& te = args.fFp.cast<GrTextureEffect>();
    auto* fb = args.fFragBuilder;

    if (te.fShaderModes[0] == ShaderMode::kNone &&
        te.fShaderModes[1] == ShaderMode::kNone) {
        fb->codeAppendf("return ");
        fb->appendTextureLookup(fSamplerHandle, args.fSampleCoord);
        fb->codeAppendf(";");
    } else {
        // Here is the basic flow of the various ShaderModes are implemented in a series of
        // steps. Not all the steps apply to all the modes. We try to emit only the steps
        // that are necessary for the given x/y shader modes.
        //
        // 0) Start with interpolated coordinates (unnormalize if doing anything
        //    complicated).
        // 1) Map the coordinates into the subset range [Repeat and MirrorRepeat], or pass
        //    through output of 0).
        // 2) Clamp the coordinates to a 0.5 inset of the subset rect [Clamp, Repeat, and
        //    MirrorRepeat always or ClampToBorder only when filtering] or pass through
        //    output of 1). The clamp rect collapses to a line or point it if the subset
        //    rect is less than one pixel wide/tall.
        // 3) Look up texture with output of 2) [All]
        // 3) Use the difference between 1) and 2) to apply filtering at edge [Repeat or
        //    ClampToBorder]. In the Repeat case this requires extra texture lookups on the
        //    other side of the subset (up to 3 more reads). Or if ClampToBorder and not
        //    filtering do a hard less than/greater than test with the subset rect.

        // Convert possible projective texture coordinates into non-homogeneous half2.
        fb->codeAppendf("float2 inCoord = %s;", args.fSampleCoord);

        const auto& m = te.fShaderModes;

        const char* borderName = nullptr;
        if (te.hasClampToBorderShaderMode()) {
            fBorderUni = args.fUniformHandler->addUniform(
                    &te, kFragment_GrShaderFlag, SkSLType::kHalf4, "border", &borderName);
        }
        auto modeUsesSubset = [](ShaderMode m) {
          switch (m) {
              case ShaderMode::kNone:                     return false;
              case ShaderMode::kClamp:                    return false;
              case ShaderMode::kRepeat_Nearest_None:      return true;
              case ShaderMode::kRepeat_Linear_None:       return true;
              case ShaderMode::kRepeat_Nearest_Mipmap:    return true;
              case ShaderMode::kRepeat_Linear_Mipmap:     return true;
              case ShaderMode::kMirrorRepeat:             return true;
              case ShaderMode::kClampToBorder_Nearest:    return true;
              case ShaderMode::kClampToBorder_Filter:     return true;
          }
          SkUNREACHABLE;
        };

        auto modeUsesClamp = [](ShaderMode m) {
          switch (m) {
              case ShaderMode::kNone:                     return false;
              case ShaderMode::kClamp:                    return true;
              case ShaderMode::kRepeat_Nearest_None:      return true;
              case ShaderMode::kRepeat_Linear_None:       return true;
              case ShaderMode::kRepeat_Nearest_Mipmap:    return true;
              case ShaderMode::kRepeat_Linear_Mipmap:     return true;
              case ShaderMode::kMirrorRepeat:             return true;
              case ShaderMode::kClampToBorder_Nearest:    return false;
              case ShaderMode::kClampToBorder_Filter:     return true;
          }
          SkUNREACHABLE;
        };

        bool useSubset[2] = {modeUsesSubset(m[0]), modeUsesSubset(m[1])};
        bool useClamp [2] = {modeUsesClamp (m[0]), modeUsesClamp (m[1])};

        const char* subsetName = nullptr;
        if (useSubset[0] || useSubset[1]) {
            fSubsetUni = args.fUniformHandler->addUniform(
                    &te, kFragment_GrShaderFlag, SkSLType::kFloat4, "subset", &subsetName);
        }

        const char* clampName = nullptr;
        if (useClamp[0] || useClamp[1]) {
            fClampUni = args.fUniformHandler->addUniform(
                    &te, kFragment_GrShaderFlag, SkSLType::kFloat4, "clamp", &clampName);
        }

        bool unormCoordsRequiredForShaderMode = ShaderModeRequiresUnormCoord(m[0]) ||
                                                ShaderModeRequiresUnormCoord(m[1]);
        // We should not pre-normalize the input coords with GrMatrixEffect if we're going to
        // operate on unnormalized coords and then normalize after the shader mode.
        SkASSERT(!(unormCoordsRequiredForShaderMode && te.matrixEffectShouldNormalize()));
        bool sampleCoordsMustBeNormalized =
                te.fView.asTextureProxy()->textureType() != GrTextureType::kRectangle;

        const char* idims = nullptr;
        if (unormCoordsRequiredForShaderMode && sampleCoordsMustBeNormalized) {
            // TODO: Detect support for textureSize() or polyfill textureSize() in SkSL and
            // always use?
            fIDimsUni = args.fUniformHandler->addUniform(&te, kFragment_GrShaderFlag,
                                                         SkSLType::kFloat2, "idims", &idims);
        }

        // Generates a string to read at a coordinate, normalizing coords if necessary.
        auto read = [&](const char* coord) {
            SkString result;
            SkString normCoord;
            if (idims) {
                normCoord.printf("(%s) * %s", coord, idims);
            } else {
                normCoord = coord;
            }
            fb->appendTextureLookup(&result, fSamplerHandle, normCoord.c_str());
            return result;
        };

        // Implements coord wrapping for kRepeat and kMirrorRepeat
        auto subsetCoord = [&](ShaderMode mode,
                               const char* coordSwizzle,
                               const char* subsetStartSwizzle,
                               const char* subsetStopSwizzle,
                               const char* extraCoord,
                               const char* coordWeight) {
            switch (mode) {
                // These modes either don't use the subset rect or don't need to map the
                // coords to be within the subset.
                case ShaderMode::kNone:
                case ShaderMode::kClampToBorder_Nearest:
                case ShaderMode::kClampToBorder_Filter:
                case ShaderMode::kClamp:
                    fb->codeAppendf("subsetCoord.%s = inCoord.%s;", coordSwizzle, coordSwizzle);
                    break;
                case ShaderMode::kRepeat_Nearest_None:
                case ShaderMode::kRepeat_Linear_None:
                    fb->codeAppendf(
                            "subsetCoord.%s = mod(inCoord.%s - %s.%s, %s.%s - %s.%s) + %s.%s;",
                            coordSwizzle, coordSwizzle, subsetName, subsetStartSwizzle, subsetName,
                            subsetStopSwizzle, subsetName, subsetStartSwizzle, subsetName,
                            subsetStartSwizzle);
                    break;
                case ShaderMode::kRepeat_Nearest_Mipmap:
                case ShaderMode::kRepeat_Linear_Mipmap:
                    // The approach here is to generate two sets of texture coords that
                    // are both "moving" at the same speed (if not direction) as
                    // inCoords. We accomplish that by using two out of phase mirror
                    // repeat coords. We will always sample using both coords but the
                    // read from the upward sloping one is selected using a weight
                    // that transitions from one set to the other near the reflection
                    // point. Like the coords, the weight is a saw-tooth function,
                    // phase-shifted, vertically translated, and then clamped to 0..1.
                    // TODO: Skip this and use textureGrad() when available.
                    SkASSERT(extraCoord);
                    SkASSERT(coordWeight);
                    fb->codeAppend("{");
                    fb->codeAppendf("float w = %s.%s - %s.%s;", subsetName, subsetStopSwizzle,
                                    subsetName, subsetStartSwizzle);
                    fb->codeAppendf("float w2 = 2 * w;");
                    fb->codeAppendf("float d = inCoord.%s - %s.%s;", coordSwizzle, subsetName,
                                    subsetStartSwizzle);
                    fb->codeAppend("float m = mod(d, w2);");
                    fb->codeAppend("float o = mix(m, w2 - m, step(w, m));");
                    fb->codeAppendf("subsetCoord.%s = o + %s.%s;", coordSwizzle, subsetName,
                                    subsetStartSwizzle);
                    fb->codeAppendf("%s = w - o + %s.%s;", extraCoord, subsetName,
                                    subsetStartSwizzle);
                    // coordWeight is used as the third param of mix() to blend between a
                    // sample taken using subsetCoord and a sample at extraCoord.
                    fb->codeAppend("float hw = w/2;");
                    fb->codeAppend("float n = mod(d - hw, w2);");
                    fb->codeAppendf("%s = saturate(half(mix(n, w2 - n, step(w, n)) - hw + 0.5));",
                                    coordWeight);
                    fb->codeAppend("}");
                    break;
                case ShaderMode::kMirrorRepeat:
                    fb->codeAppend("{");
                    fb->codeAppendf("float w = %s.%s - %s.%s;", subsetName, subsetStopSwizzle,
                                    subsetName, subsetStartSwizzle);
                    fb->codeAppendf("float w2 = 2 * w;");
                    fb->codeAppendf("float m = mod(inCoord.%s - %s.%s, w2);", coordSwizzle,
                                    subsetName, subsetStartSwizzle);
                    fb->codeAppendf("subsetCoord.%s = mix(m, w2 - m, step(w, m)) + %s.%s;",
                                    coordSwizzle, subsetName, subsetStartSwizzle);
                    fb->codeAppend("}");
                    break;
            }
        };

        auto clampCoord = [&](bool clamp,
                              const char* coordSwizzle,
                              const char* clampStartSwizzle,
                              const char* clampStopSwizzle) {
            if (clamp) {
                fb->codeAppendf("clampedCoord%s = clamp(subsetCoord%s, %s%s, %s%s);",
                                coordSwizzle, coordSwizzle,
                                clampName, clampStartSwizzle,
                                clampName, clampStopSwizzle);
            } else {
                fb->codeAppendf("clampedCoord%s = subsetCoord%s;", coordSwizzle, coordSwizzle);
            }
        };

        // Insert vars for extra coords and blending weights for repeat + mip map.
        const char* extraRepeatCoordX  = nullptr;
        const char* repeatCoordWeightX = nullptr;
        const char* extraRepeatCoordY  = nullptr;
        const char* repeatCoordWeightY = nullptr;

        bool mipmapRepeatX = m[0] == ShaderMode::kRepeat_Nearest_Mipmap ||
                             m[0] == ShaderMode::kRepeat_Linear_Mipmap;
        bool mipmapRepeatY = m[1] == ShaderMode::kRepeat_Nearest_Mipmap ||
                             m[1] == ShaderMode::kRepeat_Linear_Mipmap;

        if (mipmapRepeatX || mipmapRepeatY) {
            fb->codeAppend("float2 extraRepeatCoord;");
        }
        if (mipmapRepeatX) {
            fb->codeAppend("half repeatCoordWeightX;");
            extraRepeatCoordX   = "extraRepeatCoord.x";
            repeatCoordWeightX  = "repeatCoordWeightX";
        }
        if (mipmapRepeatY) {
            fb->codeAppend("half repeatCoordWeightY;");
            extraRepeatCoordY   = "extraRepeatCoord.y";
            repeatCoordWeightY  = "repeatCoordWeightY";
        }

        // Apply subset rect and clamp rect to coords.
        fb->codeAppend("float2 subsetCoord;");
        subsetCoord(te.fShaderModes[0], "x", "x", "z", extraRepeatCoordX, repeatCoordWeightX);
        subsetCoord(te.fShaderModes[1], "y", "y", "w", extraRepeatCoordY, repeatCoordWeightY);
        fb->codeAppend("float2 clampedCoord;");
        if (useClamp[0] == useClamp[1]) {
            clampCoord(useClamp[0], "", ".xy", ".zw");
        } else {
            clampCoord(useClamp[0], ".x", ".x", ".z");
            clampCoord(useClamp[1], ".y", ".y", ".w");
        }
        // Additional clamping for the extra coords for kRepeat with mip maps.
        if (mipmapRepeatX && mipmapRepeatY) {
            fb->codeAppendf("extraRepeatCoord = clamp(extraRepeatCoord, %s.xy, %s.zw);",
                            clampName, clampName);
        } else if (mipmapRepeatX) {
            fb->codeAppendf("extraRepeatCoord.x = clamp(extraRepeatCoord.x, %s.x, %s.z);",
                            clampName, clampName);
        } else if (mipmapRepeatY) {
            fb->codeAppendf("extraRepeatCoord.y = clamp(extraRepeatCoord.y, %s.y, %s.w);",
                            clampName, clampName);
        }

        // Do the 2 or 4 texture reads for kRepeatMipMap and then apply the weight(s)
        // to blend between them. If neither direction is repeat or not using mip maps do a single
        // read at clampedCoord.
        if (mipmapRepeatX && mipmapRepeatY) {
            fb->codeAppendf(
                    "half4 textureColor ="
                    "   mix(mix(%s, %s, repeatCoordWeightX),"
                    "       mix(%s, %s, repeatCoordWeightX),"
                    "       repeatCoordWeightY);",
                    read("clampedCoord").c_str(),
                    read("float2(extraRepeatCoord.x, clampedCoord.y)").c_str(),
                    read("float2(clampedCoord.x, extraRepeatCoord.y)").c_str(),
                    read("float2(extraRepeatCoord.x, extraRepeatCoord.y)").c_str());

        } else if (mipmapRepeatX) {
            fb->codeAppendf("half4 textureColor = mix(%s, %s, repeatCoordWeightX);",
                            read("clampedCoord").c_str(),
                            read("float2(extraRepeatCoord.x, clampedCoord.y)").c_str());
        } else if (mipmapRepeatY) {
            fb->codeAppendf("half4 textureColor = mix(%s, %s, repeatCoordWeightY);",
                            read("clampedCoord").c_str(),
                            read("float2(clampedCoord.x, extraRepeatCoord.y)").c_str());
        } else {
            fb->codeAppendf("half4 textureColor = %s;", read("clampedCoord").c_str());
        }

        // Strings for extra texture reads used only in kRepeatLinear
        SkString repeatLinearReadX;
        SkString repeatLinearReadY;

        // Calculate the amount the coord moved for clamping. This will be used
        // to implement shader-based filtering for kClampToBorder and kRepeat.
        bool repeatLinearFilterX = m[0] == ShaderMode::kRepeat_Linear_None ||
                                   m[0] == ShaderMode::kRepeat_Linear_Mipmap;
        bool repeatLinearFilterY = m[1] == ShaderMode::kRepeat_Linear_None ||
                                   m[1] == ShaderMode::kRepeat_Linear_Mipmap;
        if (repeatLinearFilterX || m[0] == ShaderMode::kClampToBorder_Filter) {
            fb->codeAppend("half errX = half(subsetCoord.x - clampedCoord.x);");
            if (repeatLinearFilterX) {
                fb->codeAppendf("float repeatCoordX = errX > 0 ? %s.x : %s.z;",
                                clampName, clampName);
                repeatLinearReadX = read("float2(repeatCoordX, clampedCoord.y)");
            }
        }
        if (repeatLinearFilterY || m[1] == ShaderMode::kClampToBorder_Filter) {
            fb->codeAppend("half errY = half(subsetCoord.y - clampedCoord.y);");
            if (repeatLinearFilterY) {
                fb->codeAppendf("float repeatCoordY = errY > 0 ? %s.y : %s.w;",
                                clampName, clampName);
                repeatLinearReadY = read("float2(clampedCoord.x, repeatCoordY)");
            }
        }

        // Add logic for kRepeat + linear filter. Do 1 or 3 more texture reads depending
        // on whether both modes are kRepeat and whether we're near a single subset edge
        // or a corner. Then blend the multiple reads using the err values calculated
        // above.
        const char* ifStr = "if";
        if (repeatLinearFilterX && repeatLinearFilterY) {
            auto repeatLinearReadXY = read("float2(repeatCoordX, repeatCoordY)");
            fb->codeAppendf(
                    "if (errX != 0 && errY != 0) {"
                    "    errX = abs(errX);"
                    "    textureColor = mix(mix(textureColor, %s, errX),"
                    "                       mix(%s, %s, errX),"
                    "                       abs(errY));"
                    "}",
                    repeatLinearReadX.c_str(), repeatLinearReadY.c_str(),
                    repeatLinearReadXY.c_str());
            ifStr = "else if";
        }
        if (repeatLinearFilterX) {
            fb->codeAppendf(
                    "%s (errX != 0) {"
                    "    textureColor = mix(textureColor, %s, abs(errX));"
                    "}",
                    ifStr, repeatLinearReadX.c_str());
        }
        if (repeatLinearFilterY) {
            fb->codeAppendf(
                    "%s (errY != 0) {"
                    "    textureColor = mix(textureColor, %s, abs(errY));"
                    "}",
                    ifStr, repeatLinearReadY.c_str());
        }

        // Do soft edge shader filtering against border color for kClampToBorderFilter using
        // the err values calculated above.
        if (m[0] == ShaderMode::kClampToBorder_Filter) {
            fb->codeAppendf("textureColor = mix(textureColor, %s, min(abs(errX), 1));", borderName);
        }
        if (m[1] == ShaderMode::kClampToBorder_Filter) {
            fb->codeAppendf("textureColor = mix(textureColor, %s, min(abs(errY), 1));", borderName);
        }

        // Do hard-edge shader transition to border color for kClampToBorderNearest at the
        // subset boundaries. Snap the input coordinates to nearest neighbor (with an
        // epsilon) before comparing to the subset rect to avoid GPU interpolation errors
        if (m[0] == ShaderMode::kClampToBorder_Nearest) {
            fb->codeAppendf(
                    "float snappedX = floor(inCoord.x + 0.001) + 0.5;"
                    "if (snappedX < %s.x || snappedX > %s.z) {"
                    "    textureColor = %s;"
                    "}",
                    subsetName, subsetName, borderName);
        }
        if (m[1] == ShaderMode::kClampToBorder_Nearest) {
            fb->codeAppendf(
                    "float snappedY = floor(inCoord.y + 0.001) + 0.5;"
                    "if (snappedY < %s.y || snappedY > %s.w) {"
                    "    textureColor = %s;"
                    "}",
                    subsetName, subsetName, borderName);
        }
        fb->codeAppendf("return textureColor;");
    }
}

void GrTextureEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdm,
                                      const GrFragmentProcessor& fp) {
    const auto& te = fp.cast<GrTextureEffect>();

    const float w = te.texture()->width();
    const float h = te.texture()->height();
    const auto& s = te.fSubset;
    const auto& c = te.fClamp;

    auto type = te.texture()->textureType();

    float idims[2] = {1.f/w, 1.f/h};

    if (fIDimsUni.isValid()) {
        pdm.set2fv(fIDimsUni, 1, idims);
        SkASSERT(type != GrTextureType::kRectangle);
    }

    auto pushRect = [&](float rect[4], UniformHandle uni) {
        if (te.view().origin() == kBottomLeft_GrSurfaceOrigin) {
            rect[1] = h - rect[1];
            rect[3] = h - rect[3];
            std::swap(rect[1], rect[3]);
        }
        if (!fIDimsUni.isValid() && type != GrTextureType::kRectangle) {
            rect[0] *= idims[0];
            rect[2] *= idims[0];
            rect[1] *= idims[1];
            rect[3] *= idims[1];
        }
        pdm.set4fv(uni, 1, rect);
    };

    if (fSubsetUni.isValid()) {
        float subset[] = {s.fLeft, s.fTop, s.fRight, s.fBottom};
        pushRect(subset, fSubsetUni);
    }
    if (fClampUni.isValid()) {
        float subset[] = {c.fLeft, c.fTop, c.fRight, c.fBottom};
        pushRect(subset, fClampUni);
    }
    if (fBorderUni.isValid()) {
        pdm.set4fv(fBorderUni, 1, te.fBorder);
    }
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrTextureEffect::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

void GrTextureEffect::onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const {
    auto m0 = static_cast<uint32_t>(fShaderModes[0]);
    b->addBits(8, m0, "shaderMode0");

    auto m1 = static_cast<uint32_t>(fShaderModes[1]);
    b->addBits(8, m1, "shaderMode1");
}

bool GrTextureEffect::onIsEqual(const GrFragmentProcessor& other) const {
    auto& that = other.cast<GrTextureEffect>();
    if (fView != that.fView) {
        return false;
    }
    if (fSamplerState != that.fSamplerState) {
        return false;
    }
    if (fShaderModes[0] != that.fShaderModes[0] || fShaderModes[1] != that.fShaderModes[1]) {
        return false;
    }
    if (fSubset != that.fSubset) {
        return false;
    }
    if (this->hasClampToBorderShaderMode() && !std::equal(fBorder, fBorder + 4, that.fBorder)) {
        return false;
    }
    return true;
}

bool GrTextureEffect::matrixEffectShouldNormalize() const {
    return fView.asTextureProxy()->textureType() != GrTextureType::kRectangle &&
           !ShaderModeRequiresUnormCoord(fShaderModes[0])                     &&
           !ShaderModeRequiresUnormCoord(fShaderModes[1]);
}

GrTextureEffect::GrTextureEffect(GrSurfaceProxyView view,
                                 SkAlphaType alphaType,
                                 const Sampling& sampling)
        : GrFragmentProcessor(kGrTextureEffect_ClassID,
                              ModulateForSamplerOptFlags(alphaType, sampling.hasBorderAlpha()))
        , fView(std::move(view))
        , fSamplerState(sampling.fHWSampler)
        , fSubset(sampling.fShaderSubset)
        , fClamp(sampling.fShaderClamp)
        , fShaderModes{sampling.fShaderModes[0], sampling.fShaderModes[1]} {
    // We always compare the range even when it isn't used so assert we have canonical don't care
    // values.
    SkASSERT(fShaderModes[0] != ShaderMode::kNone || (fSubset.fLeft == 0 && fSubset.fRight == 0));
    SkASSERT(fShaderModes[1] != ShaderMode::kNone || (fSubset.fTop == 0 && fSubset.fBottom == 0));
    this->setUsesSampleCoordsDirectly();
    std::copy_n(sampling.fBorder, 4, fBorder);
}

GrTextureEffect::GrTextureEffect(const GrTextureEffect& src)
        : INHERITED(kGrTextureEffect_ClassID, src.optimizationFlags())
        , fView(src.fView)
        , fSamplerState(src.fSamplerState)
        , fSubset(src.fSubset)
        , fClamp(src.fClamp)
        , fShaderModes{src.fShaderModes[0], src.fShaderModes[1]} {
    std::copy_n(src.fBorder, 4, fBorder);
    this->setUsesSampleCoordsDirectly();
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrTextureEffect(*this));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTextureEffect)
#if defined(GR_TEST_UTILS)
std::unique_ptr<GrFragmentProcessor> GrTextureEffect::TestCreate(GrProcessorTestData* testData) {
    auto [view, ct, at] = testData->randomView();
    Wrap wrapModes[2];
    GrTest::TestWrapModes(testData->fRandom, wrapModes);

    Filter filter = testData->fRandom->nextBool() ? Filter::kLinear : Filter::kNearest;
    MipmapMode mm = MipmapMode::kNone;
    if (view.asTextureProxy()->mipmapped() == skgpu::Mipmapped::kYes) {
        mm = testData->fRandom->nextBool() ? MipmapMode::kLinear : MipmapMode::kNone;
    }
    GrSamplerState params(wrapModes, filter, mm);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    return GrTextureEffect::Make(std::move(view), at, matrix, params, *testData->caps());
}
#endif
