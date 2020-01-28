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
        Mode fHWMode;
        Filter fFilter;
    };

    auto resolve = [adjustForFilter, filter = sampler.filter(), &caps](int size, Mode mode,
                                                                       Span subset, Span domain) {
        float inset;
        Result1D r;
        r.fFilter = filter;
        bool canDoHW = (mode != Mode::kClampToBorder || caps.clampToBorderSupport()) &&
                       (mode == Mode::kClamp || caps.npotTextureTileSupport() || SkIsPow2(size));
        if (canDoHW && size > 0 && subset.fA <= 0 && subset.fB >= size) {
            r.fShaderMode = ShaderMode::kNone;
            r.fHWMode = mode;
            return r;
        }

        inset = (adjustForFilter && filter != Filter::kNearest) ? 0.5 : 0;
        auto insetSubset = subset.makeInset(inset);

        if (canDoHW && insetSubset.contains(domain)) {
            r.fShaderMode = ShaderMode::kNone;
            r.fHWMode = mode;
            return r;
        }

        if (mode == Mode::kRepeat || mode == Mode::kMirrorRepeat) {
            r.fFilter = Filter::kNearest;
            r.fShaderSubset = subset;
        } else {
            r.fShaderSubset = insetSubset;
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

    fHWSampler = {x.fHWMode, y.fHWMode, std::min(x.fFilter, y.fFilter)};
    fShaderModes[0] = x.fShaderMode;
    fShaderModes[1] = y.fShaderMode;
    fShaderSubset = {x.fShaderSubset.fA, y.fShaderSubset.fA,
                     x.fShaderSubset.fB, y.fShaderSubset.fB};
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
        UniformHandle fDecalUni;

    public:
        void emitCode(EmitArgs& args) override {
            auto appendWrap = [](GrGLSLShaderBuilder* builder, ShaderMode mode, const char* inCoord,
                                 const char* domainStart, const char* domainEnd, bool is2D,
                                 const char* out) {
                switch (mode) {
                    case ShaderMode::kNone:
                        builder->codeAppendf("%s = %s;\n", out, inCoord);
                        break;
                    case ShaderMode::kDecal:
                        // The lookup coordinate to use for decal will be clamped just like
                        // kClamp_Mode, it's just that the post-processing will be different, so
                        // fall through
                    case ShaderMode::kClamp:
                        builder->codeAppendf("%s = clamp(%s, %s, %s);", out, inCoord, domainStart,
                                             domainEnd);
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
                const char* subsetName;
                SkString uniName("TexDom");
                fSubsetUni = args.fUniformHandler->addUniform(
                        kFragment_GrShaderFlag, kHalf4_GrSLType, "subset", &subsetName);

                // Always use a local variable for the input coordinates; often callers pass in an
                // expression and we want to cache it across all of its references in the code below
                auto inCoords = fb->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);
                fb->codeAppend("float2 clampedCoord;");
                SkString start;
                SkString end;
                if (te.fShaderModes[0] == te.fShaderModes[1]) {
                    // Doing the domain setup using vectors seems to avoid shader compilation issues
                    // on Chromecast, possibly due to reducing shader length.
                    start.printf("%s.xy", subsetName);
                    end.printf("%s.zw", subsetName);
                    appendWrap(fb, te.fShaderModes[0], inCoords.c_str(), start.c_str(), end.c_str(),
                               true, "clampedCoord");
                } else {
                    SkString origX, origY;
                    // Apply x mode to the x coordinate using the left and right edges of the domain
                    // rect (stored as the x and z components of the domain uniform).
                    start.printf("%s.x", subsetName);
                    end.printf("%s.z", subsetName);
                    origX.printf("%s.x", inCoords.c_str());
                    appendWrap(fb, te.fShaderModes[0], origX.c_str(), start.c_str(), end.c_str(),
                               false, "clampedCoord.x");
                    // Repeat the same logic for y.
                    start.printf("%s.y", subsetName);
                    end.printf("%s.w", subsetName);
                    origY.printf("%s.y", inCoords.c_str());
                    appendWrap(fb, te.fShaderModes[1], origY.c_str(), start.c_str(), end.c_str(),
                               false, "clampedCoord.y");
                }
                SkString textureLookup;
                fb->appendTextureLookup(&textureLookup, args.fTexSamplers[0], "clampedCoord");
                fb->codeAppendf("half4 textureColor = %s;", textureLookup.c_str());

                // Apply decal mode's transparency interpolation if needed
                bool decalX = te.fShaderModes[0] == ShaderMode::kDecal;
                bool decalY = te.fShaderModes[1] == ShaderMode::kDecal;
                if (decalX || decalY) {
                    const char* decalName;
                    // Half3 since this will hold texture width, height, and then a step function
                    // control param
                    fDecalUni = args.fUniformHandler->addUniform(
                            kFragment_GrShaderFlag, kHalf3_GrSLType, uniName.c_str(), &decalName);
                    // The decal err is the max absolute value between the clamped coordinate and
                    // the original pixel coordinate. This will then be clamped to 1.f if it's
                    // greater than the control parameter, which simulates kNearest and kBilerp
                    // behavior depending on if it's 0 or 1.
                    if (decalX && decalY) {
                        fb->codeAppendf(
                                "half err = max(half(abs(clampedCoord.x - %s.x) * %s.x), "
                                "               half(abs(clampedCoord.y - %s.y) * %s.y));",
                                inCoords.c_str(), decalName, inCoords.c_str(), decalName);
                    } else if (decalX) {
                        fb->codeAppendf("half err = half(abs(clampedCoord.x - %s.x) * %s.x);",
                                        inCoords.c_str(), decalName);
                    } else {
                        SkASSERT(decalY);
                        fb->codeAppendf("half err = half(abs(clampedCoord.y - %s.y) * %s.y);",
                                        inCoords.c_str(), decalName);
                    }

                    // Apply a transform to the error rate, which let's us simulate nearest or
                    // bilerp filtering in the same shader. When the texture is nearest filtered,
                    // fSizeName.z is set to 0 so this becomes a step function centered at the
                    // clamped coordinate. When bilerp, fSizeName.z is set to 1 and it becomes
                    // a simple linear blend between texture and transparent.
                    fb->codeAppendf(
                            "if (err > %s.z) { err = 1.0; } else if (%s.z < 1) { err = 0.0; }",
                            decalName, decalName);
                    fb->codeAppend("textureColor = mix(textureColor, half4(0), err);");
                }
                fb->codeAppendf("%s = textureColor * %s;", args.fOutputColor, args.fInputColor);
            }
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdm,
                       const GrFragmentProcessor& fp) override {
            const auto& te = fp.cast<GrTextureEffect>();
            if (fSubsetUni.isValid()) {
                const float w = te.fSampler.peekTexture()->width();
                const float h = te.fSampler.peekTexture()->height();

                const auto& s = te.fSubset;
                float rect[] = {s.fLeft, s.fTop, s.fRight, s.fBottom};
                float decalW[3];

                if (te.fSampler.view().origin() == kBottomLeft_GrSurfaceOrigin) {
                    rect[1] = h - rect[1];
                    rect[3] = h - rect[3];
                    std::swap(rect[1], rect[3]);
                }

                if (te.fSampler.peekTexture()->texturePriv().textureType() !=
                    GrTextureType::kRectangle) {
                    float iw = 1.f / w;
                    float ih = 1.f / h;
                    rect[0] *= iw;
                    rect[2] *= iw;
                    rect[1] *= ih;
                    rect[3] *= ih;
                    decalW[0] = w;
                    decalW[1] = h;
                } else {
                    decalW[0] = 1;
                    decalW[1] = 1;
                }
                pdm.set4fv(fSubsetUni, 1, rect);

                if (fDecalUni.isValid()) {
                    bool filter = te.textureSampler(0).samplerState().filter() !=
                                  GrSamplerState::Filter::kNearest;
                    decalW[2] = filter ? 1.f : 0.f;
                    pdm.set3fv(fDecalUni, 1, decalW);
                }
            }
        }
    };
    return new Impl;
}

void GrTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    bool shaderFilter = (fShaderModes[0] == ShaderMode::kDecal ||
                         fShaderModes[1] == ShaderMode::kDecal) &&
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
