/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrTextureEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

using Mode = GrSamplerState::WrapMode;
using Filter = GrSamplerState::Filter;

GrTextureEffect::Sampling::Sampling(const GrSurfaceProxy& proxy,
                                    GrSamplerState sampler,
                                    const SkRect& subset,
                                    const SkRect* domain,
                                    const GrCaps& caps) {
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
        ShaderMode fShaderMode;
        Span fShaderSubset;
        Span fShaderClamp;
        Mode fHWMode;
    };

    auto type = proxy.asTextureProxy()->textureType();
    auto filter = sampler.filter();

    auto resolve = [type, &caps, filter](int size, Mode mode, Span subset, Span domain) {
        Result1D r;
        bool canDoHW = (mode != Mode::kClampToBorder || caps.clampToBorderSupport()) &&
                       (mode == Mode::kClamp || caps.npotTextureTileSupport() || SkIsPow2(size)) &&
                       (mode == Mode::kClamp || mode == Mode::kClampToBorder ||
                        type != GrTextureType::kRectangle);
        if (canDoHW && size > 0 && subset.fA <= 0 && subset.fB >= size) {
            r.fShaderMode = ShaderMode::kNone;
            r.fHWMode = mode;
            r.fShaderSubset = r.fShaderClamp = {0, 0};
            return r;
        }

        r.fShaderSubset = subset;
        bool domainIsSafe = false;
        if (filter == Filter::kNearest) {
            Span isubset{sk_float_floor(subset.fA), sk_float_ceil(subset.fB)};
            if (domain.fA > isubset.fA && domain.fB < isubset.fB) {
                domainIsSafe = true;
            }
            // This inset prevents sampling neighboring texels that could occur when
            // texture coords fall exactly at texel boundaries (depending on precision
            // and GPU-specific snapping at the boundary).
            r.fShaderClamp = isubset.makeInset(0.5f);
        } else {
            r.fShaderClamp = subset.makeInset(0.5f);
            if (r.fShaderClamp.contains(domain)) {
                domainIsSafe = true;
            }
        }
        if (domainIsSafe) {
            r.fShaderMode = ShaderMode::kNone;
            r.fHWMode = Mode::kClamp;
            r.fShaderSubset = r.fShaderClamp = {0, 0};
            return r;
        }
        r.fShaderMode = static_cast<ShaderMode>(mode);
        r.fHWMode = Mode::kClamp;
        return r;
    };

    SkISize dim = proxy.isFullyLazy() ? SkISize{-1, -1} : proxy.backingStoreDimensions();

    Span subsetX{subset.fLeft, subset.fRight};
    auto domainX = domain ? Span{domain->fLeft, domain->fRight}
                          : Span{SK_FloatNegativeInfinity, SK_FloatInfinity};
    auto x = resolve(dim.width(), sampler.wrapModeX(), subsetX, domainX);

    Span subsetY{subset.fTop, subset.fBottom};
    auto domainY = domain ? Span{domain->fTop, domain->fBottom}
                          : Span{SK_FloatNegativeInfinity, SK_FloatInfinity};
    auto y = resolve(dim.height(), sampler.wrapModeY(), subsetY, domainY);

    fHWSampler = {x.fHWMode, y.fHWMode, sampler.filter()};
    fShaderModes[0] = x.fShaderMode;
    fShaderModes[1] = y.fShaderMode;
    fShaderSubset = {x.fShaderSubset.fA, y.fShaderSubset.fA,
                     x.fShaderSubset.fB, y.fShaderSubset.fB};
    fShaderClamp = {x.fShaderClamp.fA, y.fShaderClamp.fA,
                    x.fShaderClamp.fB, y.fShaderClamp.fB};
}

bool GrTextureEffect::Sampling::usesDecal() const {
    return fShaderModes[0] == ShaderMode::kDecal || fShaderModes[1] == ShaderMode::kDecal ||
           fHWSampler.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
           fHWSampler.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder;
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           Filter filter) {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(view), alphaType, matrix, Sampling(filter)));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           GrSamplerState sampler,
                                                           const GrCaps& caps) {
    Sampling sampling(*view.proxy(), sampler, SkRect::Make(view.proxy()->dimensions()), nullptr,
                      caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(view), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeTexelSubset(GrSurfaceProxyView view,
                                                                      SkAlphaType alphaType,
                                                                      const SkMatrix& matrix,
                                                                      GrSamplerState sampler,
                                                                      const SkIRect& subset,
                                                                      const GrCaps& caps) {
    Sampling sampling(*view.proxy(), sampler, SkRect::Make(subset), nullptr, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(view), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeTexelSubset(GrSurfaceProxyView view,
                                                                      SkAlphaType alphaType,
                                                                      const SkMatrix& matrix,
                                                                      GrSamplerState sampler,
                                                                      const SkIRect& subset,
                                                                      const SkRect& domain,
                                                                      const GrCaps& caps) {
    Sampling sampling(*view.proxy(), sampler, SkRect::Make(subset), &domain, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(view), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeSubset(GrSurfaceProxyView view,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler,
                                                                 const SkRect& subset,
                                                                 const GrCaps& caps) {
    Sampling sampling(*view.proxy(), sampler, subset, nullptr, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(view), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeSubset(GrSurfaceProxyView view,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler,
                                                                 const SkRect& subset,
                                                                 const SkRect& domain,
                                                                 const GrCaps& caps) {
    Sampling sampling(*view.proxy(), sampler, subset, &domain, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(view), alphaType, matrix, sampling));
}

bool GrTextureEffect::SmoothInShader(GrSamplerState::Filter filter, ShaderMode mode) {
    return filter > Filter::kNearest && (mode == ShaderMode::kDecal || mode == ShaderMode::kRepeat);
}

GrGLSLFragmentProcessor* GrTextureEffect::onCreateGLSLInstance() const {
    class Impl : public GrGLSLFragmentProcessor {
        UniformHandle fSubsetUni;
        UniformHandle fClampUni;
        UniformHandle fNormUni;

    public:
        void emitCode(EmitArgs& args) override {
            auto te = args.fFp.cast<GrTextureEffect>();
            const char* coords;
            if (args.fFp.coordTransformsApplyToLocalCoords()) {
                coords = args.fTransformedCoords[0].fVaryingPoint.c_str();
            } else {
                coords = "_coords";
            }
            auto* fb = args.fFragBuilder;
            if (te.fShaderModes[0] == ShaderMode::kNone &&
                te.fShaderModes[1] == ShaderMode::kNone) {
                fb->codeAppendf("%s = ", args.fOutputColor);
                fb->appendTextureLookupAndBlend(args.fInputColor, SkBlendMode::kModulate,
                                                args.fTexSamplers[0], coords);
                fb->codeAppendf(";");
            } else {
                // Here is the basic flow of the various ShaderModes are implemented in a series of
                // steps. Not all the steps apply to all the modes. We try to emit only the steps
                // that are necessary for the x/y modes and try to do vector ops when the steps are
                // shared by both the x and y mode. The latter makes the shader shorter which seems
                // to avoid some spurious shader compilation failures on Chromecast.
                //
                // 0) Start with interpolated coordinates.
                // 1) Map the coordinates into the subset range [Repeat and MirrorRepeat], or pass
                //    through 0).
                // 2) Clamp the coordinates to a 0.5 inset of the subset rect [Clamp and
                //    MirrorRepeat or Decal only when filtering] or pass through output of 1).
                // 3) Look up texture with output of 2) [All]
                // 3) Use the difference between 1) and 2) to apply filtering at edge [Repeat or
                //    Decal]. In the Repeat case this requires extra texture lookups on the other
                //    side of the subset (up to 3 more reads).
                // 4) [Decal only] If not filtering set the color to 0 if outside the subset.

                // Convert possible projective texture coordinates into non-homogeneous half2.
                fb->codeAppendf(
                        "float2 inCoord = %s;",
                        fb->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint).c_str());

                const auto& m = te.fShaderModes;
                const auto* texture = te.fSampler.proxy()->peekTexture();
                bool normCoords = texture->texturePriv().textureType() != GrTextureType::kRectangle;
                auto filter = te.fSampler.samplerState().filter();

                auto modeUsesSubset = [](ShaderMode m) {
                    return m == ShaderMode::kRepeat || m == ShaderMode::kMirrorRepeat ||
                           m == ShaderMode::kDecal;
                };

                auto modeUsesClamp = [filter](ShaderMode m) {
                    return m != ShaderMode::kNone &&
                           (m != ShaderMode::kDecal || filter >= Filter::kNearest);
                };

                bool useSubset[2] = {modeUsesSubset(m[0]), modeUsesSubset(m[1])};
                bool useClamp [2] = {modeUsesClamp (m[0]), modeUsesClamp (m[1])};

                const char* subsetName = nullptr;
                if (useSubset[0] || useSubset[1]) {
                    fSubsetUni = args.fUniformHandler->addUniform(
                            kFragment_GrShaderFlag, kFloat4_GrSLType, "subset", &subsetName);
                }

                const char* clampName = nullptr;
                if (useClamp[0] || useClamp[1]) {
                    fClampUni = args.fUniformHandler->addUniform(
                            kFragment_GrShaderFlag, kFloat4_GrSLType, "clamp", &clampName);
                }

                bool smoothEdge[2] = {SmoothInShader(filter, m[0]), SmoothInShader(filter, m[1])};
                const char* norm = nullptr;
                if (normCoords && (smoothEdge[0] || smoothEdge[1])) {
                    // TODO: Detect support for textureSize() or polyfill textureSize() in SkSL and
                    // always use?
                    fNormUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                kFloat4_GrSLType, "norm", &norm);
                    // TODO: Remove the normalization from the CoordTransform to skip denorm here.
                    fb->codeAppendf("inCoord *= %s.xy;", norm);
                }

                // Generates a string to read at a coordinate, normalizing coords if necessary.
                auto read = [fb, norm, &sampler = args.fTexSamplers[0]](const char* coord) {
                    SkString result;
                    SkString normCoord;
                    if (norm) {
                        normCoord.printf("(%s) * %s.zw", coord, norm);
                    } else {
                        normCoord = coord;
                    }
                    fb->appendTextureLookup(&result, sampler, normCoord.c_str());
                    return result;
                };

                // Implements coord wrapping for kRepeat and kMirrorRepeat
                auto subsetCoord = [fb, subsetName](ShaderMode mode,
                                                    const char* coordSwizzle,
                                                    const char* subsetStartSwizzle,
                                                    const char* subsetStopSwizzle) {
                    switch (mode) {
                        // These modes either don't use the subset rect or don't need to map the
                        // coords to be within the subset.
                        case ShaderMode::kNone:
                        case ShaderMode::kDecal:
                        case ShaderMode::kClamp:
                            fb->codeAppendf("subsetCoord.%s = inCoord.%s;", coordSwizzle,
                                            coordSwizzle);
                            break;
                        case ShaderMode::kRepeat:
                            fb->codeAppendf(
                                    "subsetCoord.%s = mod(inCoord.%s - %s.%s, %s.%s - %s.%s) + "
                                    "%s.%s;",
                                    coordSwizzle, coordSwizzle, subsetName, subsetStartSwizzle,
                                    subsetName, subsetStopSwizzle, subsetName, subsetStartSwizzle,
                                    subsetName, subsetStartSwizzle);
                            break;
                        case ShaderMode::kMirrorRepeat: {
                            fb->codeAppend("{");
                            fb->codeAppendf("float w = %s.%s - %s.%s;", subsetName,
                                            subsetStopSwizzle, subsetName, subsetStartSwizzle);
                            fb->codeAppendf("float w2 = 2 * w;");
                            fb->codeAppendf("float m = mod(inCoord.%s - %s.%s, w2);", coordSwizzle,
                                            subsetName, subsetStartSwizzle);
                            fb->codeAppendf("subsetCoord.%s = mix(m, w2 - m, step(w, m)) + %s.%s;",
                                            coordSwizzle, subsetName, subsetStartSwizzle);
                            fb->codeAppend("}");
                            break;
                        }
                    }
                };

                auto clampCoord = [fb, clampName](bool clamp,
                                                  const char* coordSwizzle,
                                                  const char* clampStartSwizzle,
                                                  const char* clampStopSwizzle) {
                    if (clamp) {
                        fb->codeAppendf("clampedCoord.%s = clamp(subsetCoord.%s, %s.%s, %s.%s);",
                                        coordSwizzle, coordSwizzle, clampName, clampStartSwizzle,
                                        clampName, clampStopSwizzle);
                    } else {
                        fb->codeAppendf("clampedCoord.%s = subsetCoord.%s;", coordSwizzle,
                                        coordSwizzle);
                    }
                };

                fb->codeAppend("float2 subsetCoord;");
                subsetCoord(te.fShaderModes[0], "x", "x", "z");
                subsetCoord(te.fShaderModes[1], "y", "y", "w");
                fb->codeAppend("float2 clampedCoord;");
                clampCoord(useClamp[0], "x", "x", "z");
                clampCoord(useClamp[1], "y", "y", "w");

                fb->codeAppendf("half4 textureColor = %s;", read("clampedCoord").c_str());
                if (smoothEdge[0] || smoothEdge[1]) {
                    SkString readX;
                    SkString readY;
                    if (smoothEdge[0]) {
                        fb->codeAppend("half errX = half(subsetCoord.x - clampedCoord.x);");
                        fb->codeAppendf("float repeatCoordX = errX > 0 ? %s.x : %s.z;", clampName,
                                        clampName);
                        readX = read("float2(repeatCoordX, clampedCoord.y)");
                    }
                    if (smoothEdge[1]) {
                        fb->codeAppend("half errY = half(subsetCoord.y - clampedCoord.y);");
                        fb->codeAppendf("float repeatCoordY = errY > 0 ? %s.y : %s.w;", clampName,
                                        clampName);
                        readY = read("float2(clampedCoord.x, repeatCoordY)");
                    }
                    const char* ifStr = "if";
                    if ((m[0] == ShaderMode::kRepeat) && (m[1] == ShaderMode::kRepeat)) {
                        auto readXY = read("float2(repeatCoordX, repeatCoordY)");
                        fb->codeAppendf(
                                "if (errX != 0 && errY != 0) {"
                                "    textureColor = mix(mix(textureColor, %s, errX),"
                                "                       mix(%s, %s, errX),"
                                "                       errY);"
                                "}",
                                readX.c_str(), readY.c_str(), readXY.c_str());
                        ifStr = "else if";
                    }
                    if (m[0] == ShaderMode::kRepeat) {
                        fb->codeAppendf(
                                "%s (errX != 0) {"
                                "    textureColor = mix(textureColor, %s, abs(errX));"
                                "}",
                                ifStr, readX.c_str());
                    }
                    if (m[1] == ShaderMode::kRepeat) {
                        fb->codeAppendf(
                                "%s (errY != 0) {"
                                "    textureColor = mix(textureColor, %s, abs(errY));"
                                "}",
                                ifStr, readY.c_str());
                    }
                    if (m[0] == ShaderMode::kDecal) {
                        fb->codeAppendf(
                                "textureColor = mix(textureColor, half4(0), min(abs(errX), 1));");
                    }
                    if (m[1] == ShaderMode::kDecal) {
                        fb->codeAppendf(
                                "textureColor = mix(textureColor, half4(0), min(abs(errY), 1));");
                    }
                } else {
                    // TODO: Do this first and avoid sampling the texture if outside?
                    // The subset edges are right on texel boundaries. Thus we need to check if the
                    // coordinates are exclusively inside the subset or we may sample a texel
                    // outside the subset. Otherwise we'd do something like clamp(inCoord,
                    // subset.xy, subset.zw) != inCoord.
                    if (m[0] == ShaderMode::kDecal) {
                        fb->codeAppendf(
                                "if ((inCoord.x - min(inCoord.x, %s.x)) *"
                                "    (max(inCoord.x, %s.z) - inCoord.x) <= 0) {"
                                "    textureColor = half4(0);"
                                "}",
                                subsetName, subsetName);
                    }
                    if (m[1] == ShaderMode::kDecal) {
                        fb->codeAppendf(
                                "if ((inCoord.y - min(inCoord.y, %s.y)) *"
                                "    (max(inCoord.y, %s.w) - inCoord.y) <= 0) {"
                                "    textureColor = half4(0);"
                                "}",
                                subsetName, subsetName);
                    }
                }
                fb->codeAppendf("%s = %s * textureColor;", args.fOutputColor, args.fInputColor);
            }
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdm,
                       const GrFragmentProcessor& fp) override {
            const auto& te = fp.cast<GrTextureEffect>();
            if (fClampUni.isValid()) {
                const float w = te.fSampler.peekTexture()->width();
                const float h = te.fSampler.peekTexture()->height();

                const auto& s = te.fSubset;
                const auto& c = te.fClamp;
                float subset[] = {s.fLeft, s.fTop, s.fRight, s.fBottom};
                float clamp[]  = {c.fLeft, c.fTop, c.fRight, c.fBottom};

                if (te.fSampler.view().origin() == kBottomLeft_GrSurfaceOrigin) {
                    subset[1] = h - subset[1];
                    subset[3] = h - subset[3];
                    std::swap(subset[1], subset[3]);
                    clamp[1] = h - clamp[1];
                    clamp[3] = h - clamp[3];
                    std::swap(clamp[1], clamp[3]);
                }

                if (te.fSampler.peekTexture()->texturePriv().textureType() !=
                    GrTextureType::kRectangle) {
                    float norm[4] = {w, h, 1.f/w, 1.f/h};
                    if (fNormUni.isValid()) {
                        pdm.set4fv(fNormUni, 1, norm);
                        norm[2] = norm[3] = 1;
                    }
                    subset[0] *= norm[2];
                    subset[2] *= norm[2];
                    subset[1] *= norm[3];
                    subset[3] *= norm[3];
                    clamp[0] *= norm[2];
                    clamp[2] *= norm[2];
                    clamp[1] *= norm[3];
                    clamp[3] *= norm[3];
                } else {
                    SkASSERT(!fNormUni.isValid());
                }
                if (fSubsetUni.isValid()) {
                    pdm.set4fv(fSubsetUni, 1, subset);
                }
                pdm.set4fv(fClampUni, 1, clamp);
            } else {
                SkASSERT(!fSubsetUni.isValid());
                SkASSERT(!fNormUni.isValid());
            }
        }
    };
    return new Impl;
}

void GrTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    bool smooth = SmoothInShader(fSampler.samplerState().filter(), fShaderModes[0]) ||
                  SmoothInShader(fSampler.samplerState().filter(), fShaderModes[1]);
    auto m0 = static_cast<uint32_t>(fShaderModes[0]);
    auto m1 = static_cast<uint32_t>(fShaderModes[1]);
    b->add32(smooth << 31 | (m0 << 16) | m1);
}

bool GrTextureEffect::onIsEqual(const GrFragmentProcessor& other) const {
    auto that = other.cast<GrTextureEffect>();
    return fShaderModes[0] == that.fShaderModes[1] && fShaderModes[1] == that.fShaderModes[1] &&
           fSubset == that.fSubset;
}

GrTextureEffect::GrTextureEffect(GrSurfaceProxyView view, SkAlphaType alphaType,
                                 const SkMatrix& matrix, const Sampling& sampling)
        : GrFragmentProcessor(kGrTextureEffect_ClassID,
                              ModulateForSamplerOptFlags(alphaType, sampling.usesDecal()))
        , fCoordTransform(matrix, view.proxy())
        , fSampler(std::move(view), sampling.fHWSampler)
        , fSubset(sampling.fShaderSubset)
        , fClamp(sampling.fShaderClamp)
        , fShaderModes{sampling.fShaderModes[0], sampling.fShaderModes[1]} {
    // We always compare the range even when it isn't used so assert we have canonical don't care
    // values.
    SkASSERT(fShaderModes[0] != ShaderMode::kNone || (fSubset.fLeft == 0 && fSubset.fRight == 0));
    SkASSERT(fShaderModes[1] != ShaderMode::kNone || (fSubset.fTop == 0 && fSubset.fBottom == 0));
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fCoordTransform);
}

GrTextureEffect::GrTextureEffect(const GrTextureEffect& src)
        : INHERITED(kGrTextureEffect_ClassID, src.optimizationFlags())
        , fCoordTransform(src.fCoordTransform)
        , fSampler(src.fSampler)
        , fSubset(src.fSubset)
        , fShaderModes{src.fShaderModes[0], src.fShaderModes[1]} {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fCoordTransform);
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrTextureEffect(*this));
}

const GrFragmentProcessor::TextureSampler& GrTextureEffect::onTextureSampler(int) const {
    return fSampler;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTextureEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrTextureEffect::TestCreate(GrProcessorTestData* testData) {
    auto [proxy, ct, at] = testData->randomProxy();
    Mode wrapModes[2];
    GrTest::TestWrapModes(testData->fRandom, wrapModes);

    Filter filter;
    if (proxy->mipMapped() == GrMipMapped::kYes) {
        switch (testData->fRandom->nextULessThan(3)) {
            case 0:
                filter = Filter::kNearest;
                break;
            case 1:
                filter = Filter::kBilerp;
                break;
            default:
                filter = Filter::kMipMap;
                break;
        }
    } else {
        filter = testData->fRandom->nextBool() ? Filter::kBilerp : Filter::kNearest;
    }
    GrSamplerState params(wrapModes, filter);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    GrSurfaceOrigin origin = proxy->origin();
    GrSwizzle swizzle = proxy->textureSwizzle();
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return GrTextureEffect::Make(std::move(view), at, matrix, params, *testData->caps());
}
#endif
