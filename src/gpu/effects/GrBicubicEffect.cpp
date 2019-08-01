/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixPriv.h"
#include "src/gpu/effects/GrBicubicEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrGLBicubicEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor& effect, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBicubicEffect& bicubicEffect = effect.cast<GrBicubicEffect>();
        b->add32(GrTextureDomain::GLDomain::DomainKey(bicubicEffect.domain()));
        uint32_t bidir = bicubicEffect.direction() == GrBicubicEffect::Direction::kXY ? 1 : 0;
        b->add32(bidir | (bicubicEffect.alphaType() << 1));
    }

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    UniformHandle fDimensions;
    GrTextureDomain::GLDomain   fDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLBicubicEffect::emitCode(EmitArgs& args) {
    const GrBicubicEffect& bicubicEffect = args.fFp.cast<GrBicubicEffect>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fDimensions = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "Dimensions");

    const char* dims = uniformHandler->getUniformCStr(fDimensions);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);

    /*
     * Filter weights come from Don Mitchell & Arun Netravali's 'Reconstruction Filters in Computer
     * Graphics', ACM SIGGRAPH Computer Graphics 22, 4 (Aug. 1988).
     * ACM DL: http://dl.acm.org/citation.cfm?id=378514
     * Free  : http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/Mitchell.pdf
     *
     * The authors define a family of cubic filters with two free parameters (B and C):
     *
     *            { (12 - 9B - 6C)|x|^3 + (-18 + 12B + 6C)|x|^2 + (6 - 2B)          if |x| < 1
     * k(x) = 1/6 { (-B - 6C)|x|^3 + (6B + 30C)|x|^2 + (-12B - 48C)|x| + (8B + 24C) if 1 <= |x| < 2
     *            { 0                                                               otherwise
     *
     * Various well-known cubic splines can be generated, and the authors select (1/3, 1/3) as their
     * favorite overall spline - this is now commonly known as the Mitchell filter, and is the
     * source of the specific weights below.
     *
     * This is GLSL, so the matrix is column-major (transposed from standard matrix notation).
     */
    fragBuilder->codeAppend("half4x4 kMitchellCoefficients = half4x4("
                            " 1.0 / 18.0,  16.0 / 18.0,   1.0 / 18.0,  0.0 / 18.0,"
                            "-9.0 / 18.0,   0.0 / 18.0,   9.0 / 18.0,  0.0 / 18.0,"
                            "15.0 / 18.0, -36.0 / 18.0,  27.0 / 18.0, -6.0 / 18.0,"
                            "-7.0 / 18.0,  21.0 / 18.0, -21.0 / 18.0,  7.0 / 18.0);");
    fragBuilder->codeAppendf("float2 coord = %s - %s.xy * float2(0.5);", coords2D.c_str(), dims);
    // We unnormalize the coord in order to determine our fractional offset (f) within the texel
    // We then snap coord to a texel center and renormalize. The snap prevents cases where the
    // starting coords are near a texel boundary and accumulations of dims would cause us to skip/
    // double hit a texel.
    fragBuilder->codeAppendf("half2 f = half2(fract(coord * %s.zw));", dims);
    fragBuilder->codeAppendf("coord = coord + (half2(0.5) - f) * %s.xy;", dims);
    if (bicubicEffect.direction() == GrBicubicEffect::Direction::kXY) {
        fragBuilder->codeAppend(
                "half4 wx = kMitchellCoefficients * half4(1.0, f.x, f.x * f.x, f.x * f.x * f.x);");
        fragBuilder->codeAppend(
                "half4 wy = kMitchellCoefficients * half4(1.0, f.y, f.y * f.y, f.y * f.y * f.y);");
        fragBuilder->codeAppend("half4 rowColors[4];");
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                SkString coord;
                coord.printf("coord + %s.xy * float2(%d, %d)", dims, x - 1, y - 1);
                SkString sampleVar;
                sampleVar.printf("rowColors[%d]", x);
                fDomain.sampleTexture(fragBuilder,
                                      args.fUniformHandler,
                                      args.fShaderCaps,
                                      bicubicEffect.domain(),
                                      sampleVar.c_str(),
                                      coord,
                                      args.fTexSamplers[0]);
            }
            fragBuilder->codeAppendf(
                    "half4 s%d = wx.x * rowColors[0] + wx.y * rowColors[1] + wx.z * rowColors[2] + "
                    "wx.w * rowColors[3];",
                    y);
        }
        fragBuilder->codeAppend(
                "half4 bicubicColor = wy.x * s0 + wy.y * s1 + wy.z * s2 + wy.w * s3;");
    } else {
        // One of the dims.xy values will be zero. So v here selects the nonzero value of f.
        fragBuilder->codeAppend("half v = f.x + f.y;");
        fragBuilder->codeAppend("half v2 = v * v;");
        fragBuilder->codeAppend("half4 w = kMitchellCoefficients * half4(1.0, v, v2, v2 * v);");
        fragBuilder->codeAppend("half4 c[4];");
        for (int i = 0; i < 4; ++i) {
            SkString coord;
            coord.printf("coord + %s.xy * half(%d)", dims, i - 1);
            SkString samplerVar;
            samplerVar.printf("c[%d]", i);
            // With added complexity we could apply the domain once in X or Y depending on
            // direction rather than for each of the four lookups, but then we might not be
            // be able to share code for Direction::kX and ::kY.
            fDomain.sampleTexture(fragBuilder,
                                  args.fUniformHandler,
                                  args.fShaderCaps,
                                  bicubicEffect.domain(),
                                  samplerVar.c_str(),
                                  coord,
                                  args.fTexSamplers[0]);
        }
        fragBuilder->codeAppend(
                "half4 bicubicColor = c[0] * w.x + c[1] * w.y + c[2] * w.z + c[3] * w.w;");
    }
    // Bicubic can send colors out of range, so clamp to get them back in (source) gamut.
    // The kind of clamp we have to do depends on the alpha type.
    if (kPremul_SkAlphaType == bicubicEffect.alphaType()) {
        fragBuilder->codeAppend("bicubicColor.a = saturate(bicubicColor.a);");
        fragBuilder->codeAppend(
                "bicubicColor.rgb = max(half3(0.0), min(bicubicColor.rgb, bicubicColor.aaa));");
    } else {
        fragBuilder->codeAppend("bicubicColor = saturate(bicubicColor);");
    }
    fragBuilder->codeAppendf("%s = bicubicColor * %s;", args.fOutputColor, args.fInputColor);
}

void GrGLBicubicEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                  const GrFragmentProcessor& processor) {
    const GrBicubicEffect& bicubicEffect = processor.cast<GrBicubicEffect>();
    GrTextureProxy* proxy = processor.textureSampler(0).proxy();
    GrTexture* texture = proxy->peekTexture();

    float dims[4] = {0, 0, 0, 0};
    if (bicubicEffect.direction() != GrBicubicEffect::Direction::kY) {
        dims[0] = 1.0f / texture->width();
        dims[2] = texture->width();
    }
    if (bicubicEffect.direction() != GrBicubicEffect::Direction::kX) {
        dims[1] = 1.0f / texture->height();
        dims[3] = texture->height();
    }
    pdman.set4fv(fDimensions, 1, dims);
    fDomain.setData(pdman, bicubicEffect.domain(), proxy,
                    processor.textureSampler(0).samplerState());
}

GrBicubicEffect::GrBicubicEffect(sk_sp<GrTextureProxy> proxy, const SkMatrix& matrix,
                                 const SkRect& domain, const GrSamplerState::WrapMode wrapModes[2],
                                 GrTextureDomain::Mode modeX, GrTextureDomain::Mode modeY,
                                 Direction direction, SkAlphaType alphaType)
        : INHERITED{kGrBicubicEffect_ClassID,
                    ModulateForSamplerOptFlags(
                            proxy->config(),
                            GrTextureDomain::IsDecalSampled(wrapModes, modeX, modeY))}
        , fCoordTransform(matrix, proxy.get())
        , fDomain(proxy.get(), domain, modeX, modeY)
        , fTextureSampler(std::move(proxy),
                          GrSamplerState(wrapModes, GrSamplerState::Filter::kNearest))
        , fAlphaType(alphaType)
        , fDirection(direction) {
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(1);
}

GrBicubicEffect::GrBicubicEffect(const GrBicubicEffect& that)
        : INHERITED(kGrBicubicEffect_ClassID, that.optimizationFlags())
        , fCoordTransform(that.fCoordTransform)
        , fDomain(that.fDomain)
        , fTextureSampler(that.fTextureSampler)
        , fAlphaType(that.fAlphaType)
        , fDirection(that.fDirection) {
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(1);
}

void GrBicubicEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GrGLBicubicEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrBicubicEffect::onCreateGLSLInstance() const  {
    return new GrGLBicubicEffect;
}

bool GrBicubicEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrBicubicEffect& s = sBase.cast<GrBicubicEffect>();
    return fDomain == s.fDomain && fDirection == s.fDirection && fAlphaType == s.fAlphaType;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrBicubicEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    static const GrSamplerState::WrapMode kClampClamp[] = {GrSamplerState::WrapMode::kClamp,
                                                           GrSamplerState::WrapMode::kClamp};
    SkAlphaType alphaType = d->fRandom->nextBool() ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;
    Direction direction = Direction::kX;
    switch (d->fRandom->nextULessThan(3)) {
        case 0:
            direction = Direction::kX;
            break;
        case 1:
            direction = Direction::kY;
            break;
        case 2:
            direction = Direction::kXY;
            break;
    }
    return GrBicubicEffect::Make(d->textureProxy(texIdx), SkMatrix::I(), kClampClamp, direction,
                                 alphaType);
}
#endif

//////////////////////////////////////////////////////////////////////////////

bool GrBicubicEffect::ShouldUseBicubic(const SkMatrix& matrix, GrSamplerState::Filter* filterMode) {
    switch (SkMatrixPriv::ShouldUseBicubic(matrix)) {
        case kNone_SkFilterQuality:
            *filterMode = GrSamplerState::Filter::kNearest;
            break;
        case kLow_SkFilterQuality:
            *filterMode = GrSamplerState::Filter::kBilerp;
            break;
        case kMedium_SkFilterQuality:
            *filterMode = GrSamplerState::Filter::kMipMap;
            break;
        case kHigh_SkFilterQuality:
            // When we use the bicubic filtering effect each sample is read from the texture using
            // nearest neighbor sampling.
            *filterMode = GrSamplerState::Filter::kNearest;
            return true;
    }
    return false;
}
