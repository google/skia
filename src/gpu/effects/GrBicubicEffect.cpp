/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrBicubicEffect.h"

#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include <cmath>

class GrBicubicEffect::Impl : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrBicubicEffect::Impl::emitCode(EmitArgs& args) {
    const GrBicubicEffect& bicubicEffect = args.fFp.cast<GrBicubicEffect>();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    if (bicubicEffect.fKernel == GrBicubicEffect::Kernel::kMitchell) {
        /*
         * Filter weights come from Don Mitchell & Arun Netravali's 'Reconstruction Filters in\
         * Computer * Graphics', ACM SIGGRAPH Computer Graphics 22, 4 (Aug. 1988).
         * ACM DL: http://dl.acm.org/citation.cfm?id=378514
         * Free:
         * http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/Mitchell.pdf
         *
         * The authors define a family of cubic filters with two free parameters (B and C):
         *
         *            { (12 - 9B - 6C)|x|^3 + (-18 + 12B + 6C)|x|^2 + (6 - 2B)          |x| < 1
         * k(x) = 1/6 { (-B - 6C)|x|^3 + (6B + 30C)|x|^2 + (-12B - 48C)|x| + (8B + 24C) 1 <= |x| < 2
         *            { 0                                                               otherwise
         *
         * Various well-known cubic splines can be generated, and the authors select (1/3, 1/3) as
         * their favorite overall spline - this is now commonly known as the Mitchell filter, and
         * is the source of the specific weights below.
         *
         * This is SkSL, so the matrix is column-major (transposed from standard matrix notation).
         */
        fragBuilder->codeAppend(
                "half4x4 kCoefficients = half4x4("
                " 1.0 / 18.0,  16.0 / 18.0,   1.0 / 18.0,  0.0 / 18.0,"
                "-9.0 / 18.0,   0.0 / 18.0,   9.0 / 18.0,  0.0 / 18.0,"
                "15.0 / 18.0, -36.0 / 18.0,  27.0 / 18.0, -6.0 / 18.0,"
                "-7.0 / 18.0,  21.0 / 18.0, -21.0 / 18.0,  7.0 / 18.0);");
    } else {
        /*
         * Centripetal variant of the Catmull-Rom spline.
         *
         * Catmull, Edwin; Rom, Raphael (1974). "A class of local interpolating splines". In
         * Barnhill, Robert E.; Riesenfeld, Richard F. (eds.). Computer Aided Geometric Design.
         * pp. 317–326.
         */
        SkASSERT(bicubicEffect.fKernel == GrBicubicEffect::Kernel::kCatmullRom);
        fragBuilder->codeAppend(
                "half4x4 kCoefficients = 0.5 * half4x4("
                " 0,  2,  0,  0,"
                "-1,  0,  1,  0,"
                " 2, -5,  4, -1,"
                "-1,  3, -3,  1);");
    }
    // We determine our fractional offset (f) within the texel. We then snap coord to a texel
    // center. The snap prevents cases where the starting coords are near a texel boundary and
    // offsets with imperfect precision would cause us to skip/double hit a texel.
    // The use of "texel" above is somewhat abstract as we're sampling a child processor. It is
    // assumed the child processor represents something akin to a nearest neighbor sampled texture.
    if (bicubicEffect.fDirection == GrBicubicEffect::Direction::kXY) {
        fragBuilder->codeAppendf("float2 coord = %s - float2(0.5);", args.fSampleCoord);
        fragBuilder->codeAppend("half2 f = half2(fract(coord));");
        fragBuilder->codeAppend("coord += 0.5 - f;");
        fragBuilder->codeAppend(
                "half4 wx = kCoefficients * half4(1.0, f.x, f.x * f.x, f.x * f.x * f.x);");
        fragBuilder->codeAppend(
                "half4 wy = kCoefficients * half4(1.0, f.y, f.y * f.y, f.y * f.y * f.y);");
        fragBuilder->codeAppend("half4 rowColors[4];");
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                SkString coord;
                coord.printf("coord + float2(%d, %d)", x - 1, y - 1);
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
        const char* d = bicubicEffect.fDirection == Direction::kX ? "x" : "y";
        fragBuilder->codeAppendf("float coord = %s.%s - 0.5;", args.fSampleCoord, d);
        fragBuilder->codeAppend("half f = half(fract(coord));");
        fragBuilder->codeAppend("coord += 0.5 - f;");
        fragBuilder->codeAppend("half f2 = f * f;");
        fragBuilder->codeAppend("half4 w = kCoefficients * half4(1.0, f, f2, f2 * f);");
        fragBuilder->codeAppend("half4 c[4];");
        for (int i = 0; i < 4; ++i) {
            SkString coord;
            if (bicubicEffect.fDirection == Direction::kX) {
                coord.printf("float2(coord + %d, %s.y)", i - 1, args.fSampleCoord);
            } else {
                coord.printf("float2(%s.x, coord + %d)", args.fSampleCoord, i - 1);
            }
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

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           Kernel kernel,
                                                           Direction direction) {
    auto fp = GrTextureEffect::Make(std::move(view), alphaType, SkMatrix::I());
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), kernel, direction, clamp)));
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           const GrSamplerState::WrapMode wrapX,
                                                           const GrSamplerState::WrapMode wrapY,
                                                           Kernel kernel,
                                                           Direction direction,
                                                           const GrCaps& caps) {
    GrSamplerState sampler(wrapX, wrapY, GrSamplerState::Filter::kNearest);
    std::unique_ptr<GrFragmentProcessor> fp;
    fp = GrTextureEffect::Make(std::move(view), alphaType, SkMatrix::I(), sampler, caps);
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), kernel, direction, clamp)));
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::MakeSubset(
        GrSurfaceProxyView view,
        SkAlphaType alphaType,
        const SkMatrix& matrix,
        const GrSamplerState::WrapMode wrapX,
        const GrSamplerState::WrapMode wrapY,
        const SkRect& subset,
        Kernel kernel,
        Direction direction,
        const GrCaps& caps) {
    GrSamplerState sampler(wrapX, wrapY, GrSamplerState::Filter::kNearest);
    std::unique_ptr<GrFragmentProcessor> fp;
    fp = GrTextureEffect::MakeSubset(
            std::move(view), alphaType, SkMatrix::I(), sampler, subset, caps);
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), kernel, direction, clamp)));
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::MakeSubset(
        GrSurfaceProxyView view,
        SkAlphaType alphaType,
        const SkMatrix& matrix,
        const GrSamplerState::WrapMode wrapX,
        const GrSamplerState::WrapMode wrapY,
        const SkRect& subset,
        const SkRect& domain,
        Kernel kernel,
        Direction direction,
        const GrCaps& caps) {
    auto lowerBound = [](float x) { return std::floor(x - 1.5f) + 0.5f; };
    auto upperBound = [](float x) { return std::floor(x + 1.5f) - 0.5f; };
    SkRect expandedDomain {
            lowerBound(domain.fLeft)  ,
            upperBound(domain.fRight) ,
            lowerBound(domain.fTop)   ,
            upperBound(domain.fBottom)
    };
    GrSamplerState sampler(wrapX, wrapY, GrSamplerState::Filter::kNearest);
    std::unique_ptr<GrFragmentProcessor> fp;
    fp = GrTextureEffect::MakeSubset(
            std::move(view), alphaType, SkMatrix::I(), sampler, subset, expandedDomain, caps);
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), kernel, direction, clamp)));
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           Kernel kernel,
                                                           Direction direction) {
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), kernel, direction, clamp)));
}

GrBicubicEffect::GrBicubicEffect(std::unique_ptr<GrFragmentProcessor> fp,
                                 Kernel kernel,
                                 Direction direction,
                                 Clamp clamp)
        : INHERITED(kGrBicubicEffect_ClassID, ProcessorOptimizationFlags(fp.get()))
        , fKernel(kernel)
        , fDirection(direction)
        , fClamp(clamp) {
    this->setUsesSampleCoordsDirectly();
    this->registerChild(std::move(fp), SkSL::SampleUsage::Explicit());
}

GrBicubicEffect::GrBicubicEffect(const GrBicubicEffect& that)
        : INHERITED(kGrBicubicEffect_ClassID, that.optimizationFlags())
        , fKernel(that.fKernel)
        , fDirection(that.fDirection)
        , fClamp(that.fClamp) {
    this->setUsesSampleCoordsDirectly();
    this->cloneAndRegisterAllChildProcessors(that);
}

void GrBicubicEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    uint32_t key = (static_cast<uint32_t>(fKernel)    << 0)
                 | (static_cast<uint32_t>(fDirection) << 1)
                 | (static_cast<uint32_t>(fClamp)     << 3);
    b->add32(key);
}

GrGLSLFragmentProcessor* GrBicubicEffect::onCreateGLSLInstance() const { return new Impl(); }

bool GrBicubicEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const auto& that = other.cast<GrBicubicEffect>();
    return fDirection == that.fDirection && fClamp == that.fClamp;
}

SkPMColor4f GrBicubicEffect::constantOutputForConstantInput(const SkPMColor4f& input) const {
    return GrFragmentProcessor::ConstantOutputForConstantInput(this->childProcessor(0), input);
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
    auto kernel = d->fRandom->nextBool() ? GrBicubicEffect::Kernel::kMitchell
                                         : GrBicubicEffect::Kernel::kCatmullRom;
    auto m = GrTest::TestMatrix(d->fRandom);
    switch (d->fRandom->nextULessThan(3)) {
        case 0: {
            auto [view, ct, at] = d->randomView();
            GrSamplerState::WrapMode wm[2];
            GrTest::TestWrapModes(d->fRandom, wm);

            if (d->fRandom->nextBool()) {
                SkRect subset;
                subset.fLeft = d->fRandom->nextSScalar1() * view.width();
                subset.fTop = d->fRandom->nextSScalar1() * view.height();
                subset.fRight = d->fRandom->nextSScalar1() * view.width();
                subset.fBottom = d->fRandom->nextSScalar1() * view.height();
                subset.sort();
                return MakeSubset(std::move(view),
                                  at,
                                  m,
                                  wm[0],
                                  wm[1],
                                  subset,
                                  kernel,
                                  direction,
                                  *d->caps());
            }
            return Make(std::move(view), at, m, wm[0], wm[1], kernel, direction, *d->caps());
        }
        case 1: {
            auto [view, ct, at] = d->randomView();
            return Make(std::move(view), at, m, kernel, direction);
        }
        default: {
            SkAlphaType at;
            do {
                at = static_cast<SkAlphaType>(d->fRandom->nextULessThan(kLastEnum_SkAlphaType + 1));
            } while (at == kUnknown_SkAlphaType);
            return Make(GrProcessorUnitTest::MakeChildFP(d), at, m, kernel, direction);
        }
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
