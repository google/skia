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

namespace {
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
}  // anonymous namespace

GrTextureEffect::Sampling::Sampling(GrSamplerState sampler, SkISize size, const GrCaps& caps)
        : fHWSampler(sampler) {
    if (!caps.clampToBorderSupport()) {
        if (fHWSampler.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder) {
            fHWSampler.setWrapModeX(GrSamplerState::WrapMode::kClamp);
            fShaderModes[0] = ShaderMode::kDecal;
            Span span{0, (float)size.width()};
            if (sampler.filter() != GrSamplerState::Filter::kNearest) {
                span = span.makeInset(0.5f);
            }
            fShaderSubset.fLeft  = span.fA;
            fShaderSubset.fRight = span.fB;
        }
        if (fHWSampler.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder) {
            fHWSampler.setWrapModeY(GrSamplerState::WrapMode::kClamp);
            fShaderModes[1] = ShaderMode::kDecal;
            Span span{0, (float)size.height()};
            if (sampler.filter() != GrSamplerState::Filter::kNearest) {
                span = span.makeInset(0.5f);
            }
            fShaderSubset.fTop    = span.fA;
            fShaderSubset.fBottom = span.fB;
        }
    }
    if (!caps.npotTextureTileSupport()) {
        if (fHWSampler.wrapModeX() != GrSamplerState::WrapMode::kClamp && !SkIsPow2(size.width())) {
            fShaderModes[0] = static_cast<ShaderMode>(fHWSampler.wrapModeX());
            fHWSampler.setWrapModeX(GrSamplerState::WrapMode::kClamp);
            // We don't yet support shader based Mirror or Repeat with filtering.
            fHWSampler.setFilterMode(GrSamplerState::Filter::kNearest);
            fShaderSubset.fLeft  = 0;
            fShaderSubset.fRight = size.width();
        }
        if (fHWSampler.wrapModeY() != GrSamplerState::WrapMode::kClamp &&
            !SkIsPow2(size.height())) {
            fShaderModes[1] = static_cast<ShaderMode>(fHWSampler.wrapModeY());
            fHWSampler.setWrapModeY(GrSamplerState::WrapMode::kClamp);
            fHWSampler.setFilterMode(GrSamplerState::Filter::kNearest);
            fShaderSubset.fTop    = 0;
            fShaderSubset.fBottom = size.height();
        }
    }
}

GrTextureEffect::Sampling::Sampling(const GrSurfaceProxy& proxy,
                                    GrSamplerState sampler,
                                    const SkRect& subset,
                                    bool adjustForFilter,
                                    const SkRect* domain,
                                    const GrCaps& caps) {
    using Mode = GrSamplerState::WrapMode;
    using Filter = GrSamplerState::Filter;

    struct Result1D {
        ShaderMode fShaderMode;
        Span fShaderSubset;
        Span fShaderClamp;
        Mode fHWMode;
    };

    auto resolve = [adjustForFilter, filter = sampler.filter(), &caps](int size, Mode mode,
                                                                       Span subset, Span domain) {
        Result1D r;
        bool canDoHW = (mode != Mode::kClampToBorder || caps.clampToBorderSupport()) &&
                       (mode == Mode::kClamp || caps.npotTextureTileSupport() || SkIsPow2(size));
        if (canDoHW && size > 0 && subset.fA <= 0 && subset.fB >= size) {
            r.fShaderMode = ShaderMode::kNone;
            r.fHWMode = mode;
            r.fShaderSubset = r.fShaderClamp = {0, 0};
            return r;
        }

        // TODO: Ignoring domain for now.
        // TODO: Assuming texel rect.
        r.fShaderClamp  = subset.makeInset(0.5f);
        r.fShaderSubset = subset;
        r.fShaderMode = static_cast<ShaderMode>(mode);
        r.fHWMode = GrSamplerState::WrapMode::kClamp;
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
    if ((x.fShaderMode != ShaderMode::kNone && x.fShaderMode != ShaderMode::kClamp && x.fShaderMode != ShaderMode::kRepeat) ||
        (y.fShaderMode != ShaderMode::kNone && y.fShaderMode != ShaderMode::kClamp && y.fShaderMode != ShaderMode::kRepeat)) {
        fHWSampler.setFilterMode(GrSamplerState::Filter::kNearest);
    }
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

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(sk_sp<GrSurfaceProxy> proxy,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           GrSamplerState::Filter filter) {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, Sampling(filter)));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(sk_sp<GrSurfaceProxy> proxy,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           GrSamplerState sampler,
                                                           const GrCaps& caps) {
    Sampling sampling(sampler, proxy->dimensions(), caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeTexelSubset(sk_sp<GrSurfaceProxy> proxy,
                                                                      SkAlphaType alphaType,
                                                                      const SkMatrix& matrix,
                                                                      GrSamplerState sampler,
                                                                      const SkIRect& subset,
                                                                      const GrCaps& caps) {
    Sampling sampling(*proxy, sampler, SkRect::Make(subset), true, nullptr, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeTexelSubset(sk_sp<GrSurfaceProxy> proxy,
                                                                      SkAlphaType alphaType,
                                                                      const SkMatrix& matrix,
                                                                      GrSamplerState sampler,
                                                                      const SkIRect& subset,
                                                                      const SkRect& domain,
                                                                      const GrCaps& caps) {
    Sampling sampling(*proxy, sampler, SkRect::Make(subset), true, &domain, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeSubset(sk_sp<GrSurfaceProxy> proxy,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler,
                                                                 const SkRect& subset,
                                                                 const GrCaps& caps) {
    Sampling sampling(*proxy, sampler, subset, false, nullptr, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, sampling));
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::MakeSubset(sk_sp<GrSurfaceProxy> proxy,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler,
                                                                 const SkRect& subset,
                                                                 const SkRect& domain,
                                                                 const GrCaps& caps) {
    Sampling sampling(*proxy, sampler, subset, false, &domain, caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, sampling));
}

GrGLSLFragmentProcessor* GrTextureEffect::onCreateGLSLInstance() const {
    class Impl : public GrGLSLFragmentProcessor {
        UniformHandle fSubsetUni;
        UniformHandle fClampUni;
        UniformHandle fNormUni;

    public:
        void emitCode(EmitArgs& args) override {
            auto appendWrap = [](GrGLSLShaderBuilder* builder, ShaderMode mode, const char* inCoord,
                                 const char* domainStart, const char* domainEnd, bool is2D,
                                 const char* out) {
                switch (mode) {
                    // These only use the clamp step, not the subset step.
                    case ShaderMode::kDecal:
                    case ShaderMode::kClamp:
                    // This uses neither subset nor clamp.
                    case ShaderMode::kNone:
                        builder->codeAppendf("%s = %s;\n", out, inCoord);
                        break;
                    case ShaderMode::kRepeat:
                        builder->codeAppendf("%s = mod(%s - %s, %s - %s) + %s;", out, inCoord,
                                             domainStart, domainEnd, domainStart, domainStart);
                        break;
                    case ShaderMode::kMirrorRepeat: {
                        const char* type = is2D ? "float2" : "float";
                        builder->codeAppend("{");
                        builder->codeAppendf("%s w = %s - %s;", type, domainEnd, domainStart);
                        builder->codeAppendf("%s w2 = 2 * w;", type);
                        builder->codeAppendf("%s m = mod(%s - %s, w2);", type, inCoord,
                                             domainStart);
                        builder->codeAppendf("%s = mix(m, w2 - m, step(w, m)) + %s;", out,
                                             domainStart);
                        builder->codeAppend("}");
                        break;
                    }
                }
            };
            auto clampOrNone = [](ShaderMode m) {
                return m == ShaderMode::kClamp || m == ShaderMode::kNone;
            };

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
                const char* subsetName ="<bogus>";
                bool useSubsetX = !clampOrNone(te.fShaderModes[0]);
                bool useSubsetY = !clampOrNone(te.fShaderModes[1]);
                if (useSubsetX || useSubsetY) {
                    fSubsetUni = args.fUniformHandler->addUniform(
                            kFragment_GrShaderFlag, kFloat4_GrSLType, "subset", &subsetName);
                }
                const char* norm = "float2(1)";
                if (te.fSampler.samplerState().filter() > GrSamplerState::Filter::kNearest &&
                    te.fSampler.proxy()->peekTexture()->texturePriv().textureType() != GrTextureType::kRectangle &&
                    (useSubsetX || useSubsetY)) {
                    // TODO: detect support for textureSize()? polyfill textureSize() in SkSL?
                    fNormUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                kFloat2_GrSLType, "norm", &norm);
                }
                auto read = [&fb, norm, &sampler = args.fTexSamplers[0]] (const char* coord) {
                    SkString result;
                    SkString normCoord;
                    normCoord.printf("((%s) * (%s)).xy", coord, norm);
                    fb->appendTextureLookup(&result, sampler, normCoord.c_str());
                    return result;
                };

                const char* clampName;
                fClampUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kFloat4_GrSLType, "clamp", &clampName);

                // Always use a local variable for the input coordinates; often callers pass in an
                // expression and we want to cache it across all of its references in the code below
                auto inCoords = fb->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);
                fb->codeAppend("float2 subsetCoord;");
                SkString start;
                SkString end;
                if (te.fShaderModes[0] == te.fShaderModes[1]) {
                    // Doing the domain setup using vectors seems to avoid shader compilation issues
                    // on Chromecast, possibly due to reducing shader length.
                    start.printf("%s.xy", subsetName);
                    end.printf("%s.zw", subsetName);
                    appendWrap(fb, te.fShaderModes[0], inCoords.c_str(), start.c_str(), end.c_str(),
                               true, "subsetCoord");
                    fb->codeAppendf("float2 clampedCoord = clamp(subsetCoord, %s.xy, %s.zw);", clampName, clampName);
                } else {
                    SkString origX, origY;
                    // Apply x mode to the x coordinate using the left and right edges of the domain
                    // rect (stored as the x and z components of the domain uniform).
                    start.printf("%s.x", subsetName);
                    end.printf("%s.z", subsetName);
                    origX.printf("%s.x", inCoords.c_str());
                    appendWrap(fb, te.fShaderModes[0], origX.c_str(), start.c_str(), end.c_str(),
                               false, "subsetCoord.x");
                    // Repeat the same logic for y.
                    start.printf("%s.y", subsetName);
                    end.printf("%s.w", subsetName);
                    origY.printf("%s.y", inCoords.c_str());
                    appendWrap(fb, te.fShaderModes[1], origY.c_str(), start.c_str(), end.c_str(),
                               false, "subsetCoord.y");
                    if (te.fShaderModes[0] == ShaderMode::kNone) {
                        fb->codeAppendf("float2 clampedCoord = float2(subsetCoord.x, clamp(subsetCoord, %s.x, %s.z));",
                                        clampName, clampName);
                    } else if (te.fShaderModes[1] == ShaderMode::kNone) {
                        fb->codeAppendf("float2 clampedCoord = float2(subsetCoord.x, clamp(subsetCoord, %s.y, %s.w));",
                                        clampName, clampName);
                    } else {
                        fb->codeAppendf("float2 clampedCoord = clamp(subsetCoord, %s.xy, %s.zw);", clampName, clampName);

                    }

                }
                SkString textureLookup = read("clampedCoord");
                fb->codeAppendf("half4 textureColor = %s;", textureLookup.c_str());

                if (te.fSampler.samplerState().filter() > GrSamplerState::Filter::kNearest) {
                    SkString wrappedCoord[2];
                    const char* comp[2]  = {".x", ".y"};
                    const char* spanA[2] = {".x", ".z"};
                    const char* spanB[2] = {".y", ".w"};
                    fb->codeAppend("half2 err;");
                    for (int i = 0; i < 2; ++i) {
                        if (!clampOrNone(te.fShaderModes[i])) {
                            fb->codeAppendf("err%s = half(clampedCoord%s - subsetCoord%s);", comp[i],
                                            comp[i], comp[i]);
                            switch (te.fShaderModes[i]) {
                                case ShaderMode::kDecal:
                                    fb->codeAppendf(
                                            "textureColor = mix(textureColor, half4(0), min(abs(err%s), 1));",
                                            comp[i]);
                                    break;
                                case ShaderMode::kRepeat:
                                    wrappedCoord[i].printf(
                                            "mix(%s%s, %s%s, err%s > half(0))",
                                            clampName, spanA[i], clampName, spanB[i], comp[i]);
                                    break;
                                case ShaderMode::kMirrorRepeat:
                                    SkASSERT(false);
                                    wrappedCoord[i].printf(
                                            "mix(%s%s, %s%s, err%s < half(0))",
                                            clampName, spanA[i], clampName, spanB[i], comp[i]);
                                    break;
                                default:
                                    SkUNREACHABLE;
                            }
                        }
                    }
                    const char* singleWrapIf = "if";
                    SkString coord10, coord01, coord11;
                    coord10.printf("float2(%s, clampedCoord.y)", wrappedCoord[0].c_str());
                    coord01.printf("float2(clampedCoord.x, %s)", wrappedCoord[1].c_str());
                    coord11.printf("float2(%s, %s)", wrappedCoord[0].c_str(), wrappedCoord[1].c_str());
                    if (!wrappedCoord[0].isEmpty() && !wrappedCoord[1].isEmpty()) {
                        singleWrapIf = "else if";
                        auto read10 = read(coord10.c_str());
                        auto read01 = read(coord01.c_str());
                        auto read11 = read(coord11.c_str());
                        fb->codeAppendf("if (err.x != 0 && err.y != 0) {"
                                        "    err = 2*abs(err);"
                                        "    textureColor = mix(mix(textureColor, %s, err.x),"
                                        "                       mix(%s, %s, err.x),"
                                        "                       err.y);"
                                        "}", read10.c_str(), read01.c_str(), read11.c_str());
                    }
                    if (!wrappedCoord[0].isEmpty()) {
                        auto read10 = read(coord10.c_str());
                        fb->codeAppendf("%s (err.x != 0) {"
                                        "    textureColor = mix(textureColor, %s, 2*abs(err.x));"
                                        "}", singleWrapIf, read10.c_str());
                    }
                    if (!wrappedCoord[1].isEmpty()) {
                        auto read01 = read(coord01.c_str());
                        fb->codeAppendf("%s (err.y != 0) {"
                                        "    textureColor = mix(textureColor, %s, 2*abs(err.y));"
                                        "}", singleWrapIf, read01.c_str());
                    }
                } else {
                    // TODO: DON'T SAMPLE THE TEXTURE AT ALL IF OUTSIDE?
                    SkString errX;
                    if (te.fShaderModes[0] == ShaderMode::kDecal) {
                        fb->codeAppend(
                                "if (subsetCoord.x != clampedCoord.x) {"
                                "    textureColor = half4(0);"
                                "}");
                    }
                    SkString errY;
                    if (te.fShaderModes[1] == ShaderMode::kDecal) {
                        fb->codeAppend(
                                "if (subsetCoord.y != clampedCoord.y) {"
                                "    textureColor = half4(0);"
                                "}");
                    }
                }
                fb->codeAppendf("%s = %s * textureColor;", args.fOutputColor, args.fInputColor);;
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
                    float iwh[2] = {1.f / w, 1.f / h};
                    if (fNormUni.isValid()) {
                        pdm.set2fv(fNormUni, 1, iwh);
                        iwh[0] = iwh[1] = 1;
                    }
                    subset[0] *= iwh[0];
                    subset[2] *= iwh[0];
                    subset[1] *= iwh[1];
                    subset[3] *= iwh[1];
                    clamp[0] *= iwh[0];
                    clamp[2] *= iwh[0];
                    clamp[1] *= iwh[1];
                    clamp[3] *= iwh[1];
                } else {
                    SkASSERT(!fNormUni.isValid());
                }
                if (fSubsetUni.isValid()) {
                    pdm.set4fv(fSubsetUni, 1, subset);
                }
                pdm.set4fv(fClampUni,  1, clamp);
            } else {
                SkASSERT(!fSubsetUni.isValid());
                SkASSERT(!fNormUni.isValid());
            }
        }
    };
    return new Impl;
}

void GrTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    bool shaderFilter = (fShaderModes[0] == ShaderMode::kDecal || fShaderModes[0] == ShaderMode::kRepeat ||
                         fShaderModes[1] == ShaderMode::kDecal || fShaderModes[1] == ShaderMode::kRepeat) &&
                        fSampler.samplerState().filter() != GrSamplerState::Filter::kNearest;
    auto m0 = static_cast<uint32_t>(fShaderModes[0]);
    auto m1 = static_cast<uint32_t>(fShaderModes[1]);
    b->add32(shaderFilter << 31 | (m0 << 16) | m1);
}

bool GrTextureEffect::onIsEqual(const GrFragmentProcessor& other) const {
    auto that = other.cast<GrTextureEffect>();
    return fShaderModes[0] == that.fShaderModes[1] && fShaderModes[1] == that.fShaderModes[1] &&
           fSubset == that.fSubset;
}

GrTextureEffect::GrTextureEffect(sk_sp<GrSurfaceProxy> texture, SkAlphaType alphaType,
                                 const SkMatrix& matrix, const Sampling& sampling)
        : GrFragmentProcessor(kGrTextureEffect_ClassID,
                              ModulateForSamplerOptFlags(alphaType, sampling.usesDecal()))
        , fCoordTransform(matrix, texture.get())
        , fSampler(std::move(texture), sampling.fHWSampler)
        , fSubset(sampling.fShaderSubset)
        , fClamp(sampling.fShaderClamp)
        , fShaderModes{sampling.fShaderModes[0], sampling.fShaderModes[1]} {
    SkASSERT((int)sampling.fHWSampler.wrapModeX() >= 0 && sampling.fHWSampler.wrapModeX() <= GrSamplerState::WrapMode::kLast);
    SkASSERT((int)sampling.fHWSampler.wrapModeY() >= 0 && sampling.fHWSampler.wrapModeY() <= GrSamplerState::WrapMode::kLast);
    // We always compare the range even when it isn't used so assert we have canonical don't care
    // values.
    SkASSERT(fShaderModes[0] != ShaderMode::kNone || (fSubset.fLeft == 0 && fSubset.fRight == 0));
    SkASSERT(fShaderModes[1] != ShaderMode::kNone || (fSubset.fTop == 0 && fSubset.fBottom == 0));
    this->setTextureSamplerCnt(1);
    if (fSampler.samplerState().filter() > GrSamplerState::Filter::kNearest &&
        (fShaderModes[0] == ShaderMode::kRepeat ||
         fShaderModes[1] == ShaderMode::kRepeat)) {
        fCoordTransform.fOverrideNorm = true;
    }
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
    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(testData->fRandom, wrapModes);
    if (!testData->caps()->npotTextureTileSupport()) {
        // Performing repeat sampling on npot textures will cause asserts on HW
        // that lacks support.
        wrapModes[0] = GrSamplerState::WrapMode::kClamp;
        wrapModes[1] = GrSamplerState::WrapMode::kClamp;
    }

    GrSamplerState params(wrapModes, testData->fRandom->nextBool()
                                             ? GrSamplerState::Filter::kBilerp
                                             : GrSamplerState::Filter::kNearest);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    return GrTextureEffect::Make(std::move(proxy), at, matrix, params, *testData->caps());
}
#endif
