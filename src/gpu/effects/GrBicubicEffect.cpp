/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrBicubicEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrBicubicEffect::Impl : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    UniformHandle fDimensions;
    GrTextureDomain::GLDomain   fDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrBicubicEffect::Impl::emitCode(EmitArgs& args) {
    const GrBicubicEffect& bicubicEffect = args.fFp.cast<GrBicubicEffect>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fDimensions = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "Dimensions");

    const char* dims = uniformHandler->getUniformCStr(fDimensions);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);

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
     * This is SkSL, so the matrix is column-major (transposed from standard matrix notation).
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
    if (bicubicEffect.fDirection == GrBicubicEffect::Direction::kXY) {
        fragBuilder->codeAppend(
                "half4 wx = kMitchellCoefficients * half4(1.0, f.x, f.x * f.x, f.x * f.x * f.x);");
        fragBuilder->codeAppend(
                "half4 wy = kMitchellCoefficients * half4(1.0, f.y, f.y * f.y, f.y * f.y * f.y);");
        fragBuilder->codeAppend("half4 rowColors[4];");
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                SkString coord;
                coord.printf("coord + %s.xy * float2(%d, %d)", dims, x - 1, y - 1);
                auto childStr =
                        this->invokeChild(0, args, SkSL::String(coord.c_str(), coord.size()));
                fragBuilder->codeAppendf("rowColors[%d] = %s;", x, childStr.c_str());
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
            auto childStr = this->invokeChild(0, args, SkSL::String(coord.c_str(), coord.size()));
            fragBuilder->codeAppendf("c[%d] = %s;", i, childStr.c_str());
        }
        fragBuilder->codeAppend(
                "half4 bicubicColor = c[0] * w.x + c[1] * w.y + c[2] * w.z + c[3] * w.w;");
    }
    // Bicubic can send colors out of range, so clamp to get them back in (source) gamut.
    // The kind of clamp we have to do depends on the alpha type.
    switch (bicubicEffect.fClamp) {
        case Clamp::kUnpremul:
            fragBuilder->codeAppend("bicubicColor = saturate(bicubicColor);");
            break;
        case Clamp::kPremul:
            fragBuilder->codeAppend(
                    "bicubicColor.rgb = max(half3(0.0), min(bicubicColor.rgb, bicubicColor.aaa));");
            break;
    }
    fragBuilder->codeAppendf("%s = bicubicColor * %s;", args.fOutputColor, args.fInputColor);
}

void GrBicubicEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                      const GrFragmentProcessor& processor) {
    const GrBicubicEffect& bicubicEffect = processor.cast<GrBicubicEffect>();
    // Currently we only ever construct with GrTextureEffect and always take its
    // coord transform as our own.
    SkASSERT(bicubicEffect.fCoordTransform.peekTexture());
    SkISize textureDims = bicubicEffect.fCoordTransform.peekTexture()->dimensions();

    float dims[4] = {0, 0, 0, 0};
    if (bicubicEffect.fDirection != GrBicubicEffect::Direction::kY) {
        if (bicubicEffect.fCoordTransform.normalize()) {
            dims[0] = 1.f / textureDims.width();
            dims[2] = textureDims.width();
        } else {
            dims[0] = dims[2] = 1.f;
        }
    }
    if (bicubicEffect.fDirection != GrBicubicEffect::Direction::kX) {
        if (bicubicEffect.fCoordTransform.normalize()) {
            dims[1] = 1.f / textureDims.height();
            dims[3] = textureDims.height();
        } else {
            dims[1] = dims[3] = 1.f;
        }
    }
    pdman.set4fv(fDimensions, 1, dims);
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           Direction direction) {
    auto fp = GrTextureEffect::Make(std::move(view), alphaType, matrix);
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), direction, clamp));
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           const GrSamplerState::WrapMode wrapX,
                                                           const GrSamplerState::WrapMode wrapY,
                                                           Direction direction,
                                                           const GrCaps& caps) {
    GrSamplerState sampler(wrapX, wrapY, GrSamplerState::Filter::kNearest);
    std::unique_ptr<GrFragmentProcessor> fp;
    fp = GrTextureEffect::Make(std::move(view), alphaType, matrix, sampler, caps);
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), direction, clamp));
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::MakeSubset(
        GrSurfaceProxyView view,
        SkAlphaType alphaType,
        const SkMatrix& matrix,
        const GrSamplerState::WrapMode wrapX,
        const GrSamplerState::WrapMode wrapY,
        const SkRect& subset,
        Direction direction,
        const GrCaps& caps) {
    GrSamplerState sampler(wrapX, wrapY, GrSamplerState::Filter::kNearest);
    std::unique_ptr<GrFragmentProcessor> fp;
    fp = GrTextureEffect::MakeSubset(std::move(view), alphaType, matrix, sampler, subset, caps);
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), direction, clamp));
}

GrBicubicEffect::GrBicubicEffect(std::unique_ptr<GrFragmentProcessor> fp,
                                 Direction direction,
                                 Clamp clamp)
        : INHERITED(kGrBicubicEffect_ClassID, ProcessorOptimizationFlags(fp.get()))
        , fDirection(direction)
        , fClamp(clamp) {
    SkASSERT(fp->numCoordTransforms() == 1);
    fCoordTransform = fp->coordTransform(0);
    this->addCoordTransform(&fCoordTransform);
    fp->coordTransform(0) = {};
    fp->setSampledWithExplicitCoords(true);
    this->registerChildProcessor(std::move(fp));
}

GrBicubicEffect::GrBicubicEffect(const GrBicubicEffect& that)
        : INHERITED(kGrBicubicEffect_ClassID, that.optimizationFlags())
        , fCoordTransform(that.fCoordTransform)
        , fDirection(that.fDirection)
        , fClamp(that.fClamp) {
    this->addCoordTransform(&fCoordTransform);
    auto child = that.childProcessor(0).clone();
    child->setSampledWithExplicitCoords(true);
    this->registerChildProcessor(std::move(child));
}

void GrBicubicEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    uint32_t key = (fDirection == GrBicubicEffect::Direction::kXY)
                 | (static_cast<uint32_t>(fClamp) << 1);
    b->add32(key);
}

GrGLSLFragmentProcessor* GrBicubicEffect::onCreateGLSLInstance() const { return new Impl(); }

bool GrBicubicEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const auto& that = other.cast<GrBicubicEffect>();
    return fDirection == that.fDirection && fClamp == that.fClamp;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrBicubicEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::TestCreate(GrProcessorTestData* d) {
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
    auto [view, ct, at] = d->randomView();
    auto m = GrTest::TestMatrix(d->fRandom);
    if (d->fRandom->nextBool()) {
        GrSamplerState::WrapMode wm[2];
        GrTest::TestWrapModes(d->fRandom, wm);

        if (d->fRandom->nextBool()) {
            SkRect subset;
            subset.fLeft   = d->fRandom->nextSScalar1() * view.width();
            subset.fTop    = d->fRandom->nextSScalar1() * view.height();
            subset.fRight  = d->fRandom->nextSScalar1() * view.width();
            subset.fBottom = d->fRandom->nextSScalar1() * view.height();
            subset.sort();
            return MakeSubset(std::move(view), at, m, wm[0], wm[1], subset, direction, *d->caps());
        }
        return Make(std::move(view), at, m, wm[0], wm[1], direction, *d->caps());
    } else {
        return Make(std::move(view), at, m, direction);
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

bool GrBicubicEffect::ShouldUseBicubic(const SkMatrix& matrix, GrSamplerState::Filter* filterMode) {
    switch (SkMatrixPriv::AdjustHighQualityFilterLevel(matrix)) {
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
