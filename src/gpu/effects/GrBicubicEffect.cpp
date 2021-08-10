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

class GrBicubicEffect::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    SkImage::CubicResampler fKernel = {-1, -1};
    UniformHandle fCoefficientUni;
};

void GrBicubicEffect::Impl::emitCode(EmitArgs& args) {
    const GrBicubicEffect& bicubicEffect = args.fFp.cast<GrBicubicEffect>();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    const char* coeffs;
    fCoefficientUni = args.fUniformHandler->addUniform(&args.fFp, kFragment_GrShaderFlag,
                                                       kHalf4x4_GrSLType, "coefficients", &coeffs);
    // We determine our fractional offset (f) within the texel. We then snap coord to a texel
    // center. The snap prevents cases where the starting coords are near a texel boundary and
    // offsets with imperfect precision would cause us to skip/double hit a texel.
    // The use of "texel" above is somewhat abstract as we're sampling a child processor. It is
    // assumed the child processor represents something akin to a nearest neighbor sampled texture.
    if (bicubicEffect.fDirection == GrBicubicEffect::Direction::kXY) {
        fragBuilder->codeAppendf("float2 coord = %s - float2(0.5);", args.fSampleCoord);
        fragBuilder->codeAppend("half2 f = half2(fract(coord));");
        fragBuilder->codeAppend("coord += 0.5 - f;");
        fragBuilder->codeAppendf("half4 wx = %s * half4(1.0, f.x, f.x * f.x, f.x * f.x * f.x);",
                                 coeffs);
        fragBuilder->codeAppendf("half4 wy = %s * half4(1.0, f.y, f.y * f.y, f.y * f.y * f.y);",
                                 coeffs);
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
        fragBuilder->codeAppendf("half4 w = %s * half4(1.0, f, f2, f2 * f);", coeffs);
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
    fragBuilder->codeAppendf("return bicubicColor;");
}

#include "src/shaders/SkImageShader.h"

void GrBicubicEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdm,
                                      const GrFragmentProcessor& fp) {
    auto& bicubicEffect = fp.cast<GrBicubicEffect>();

    if (fKernel.B != bicubicEffect.fKernel.B || fKernel.C != bicubicEffect.fKernel.C) {
        fKernel = bicubicEffect.fKernel;
        pdm.setSkM44(fCoefficientUni, SkImageShader::CubicResamplerMatrix(fKernel.B, fKernel.C));
    }
}

std::unique_ptr<GrFragmentProcessor> GrBicubicEffect::Make(GrSurfaceProxyView view,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           SkImage::CubicResampler kernel,
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
                                                           SkImage::CubicResampler kernel,
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
        SkImage::CubicResampler kernel,
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
        SkImage::CubicResampler kernel,
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
                                                           SkImage::CubicResampler kernel,
                                                           Direction direction) {
    auto clamp = kPremul_SkAlphaType == alphaType ? Clamp::kPremul : Clamp::kUnpremul;
    return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
            new GrBicubicEffect(std::move(fp), kernel, direction, clamp)));
}

GrBicubicEffect::GrBicubicEffect(std::unique_ptr<GrFragmentProcessor> fp,
                                 SkImage::CubicResampler kernel,
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
        : INHERITED(that)
        , fKernel(that.fKernel)
        , fDirection(that.fDirection)
        , fClamp(that.fClamp) {}

void GrBicubicEffect::onAddToKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    uint32_t key = (static_cast<uint32_t>(fDirection) << 0) | (static_cast<uint32_t>(fClamp) << 2);
    b->add32(key);
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrBicubicEffect::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

bool GrBicubicEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const auto& that = other.cast<GrBicubicEffect>();
    return fDirection == that.fDirection &&
           fClamp == that.fClamp         &&
           fKernel.B == that.fKernel.B   &&
           fKernel.C == that.fKernel.C;
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
    auto kernel = d->fRandom->nextBool() ? GrBicubicEffect::gMitchell
                                         : GrBicubicEffect::gCatmullRom;
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
