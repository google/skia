/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBicubicEffect.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "../private/GrGLSL.h"

/*
 * Filter weights come from Don Mitchell & Arun Netravali's 'Reconstruction Filters in Computer
 * Graphics', ACM SIGGRAPH Computer Graphics 22, 4 (Aug. 1988).
 * ACM DL: http://dl.acm.org/citation.cfm?id=378514
 * Free  : http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/Mitchell.pdf
 *
 * The authors define a family of cubic filters with two free parameters (B and C):
 *
 *            { (12 - 9B - 6C)|x|^3 + (-18 + 12B + 6C)|x|^2 + (6 - 2B)            if |x| < 1
 * k(x) = 1/6 { (-B - 6C)|x|^3 + (6B + 30C)|x|^2 + (-12B - 48C)|x| + (8B + 24C)   if 1 <= |x| < 2
 *            { 0                                                                 otherwise
 *
 * Various well-known cubic splines can be generated, and the authors select (1/3, 1/3) as their
 * favorite overall spline - this is now commonly known as the Mitchell filter, and is the source
 * of the specific weights below.
 *
 * These weights are in column-major order (ie this matrix is transposed from what you'd expect),
 * so we can upload them directly via setMatrix4f.
 */
static constexpr float kMitchellCoefficients[16] = {
     1.0f / 18.0f,  16.0f / 18.0f,   1.0f / 18.0f,  0.0f / 18.0f,
    -9.0f / 18.0f,   0.0f / 18.0f,   9.0f / 18.0f,  0.0f / 18.0f,
    15.0f / 18.0f, -36.0f / 18.0f,  27.0f / 18.0f, -6.0f / 18.0f,
    -7.0f / 18.0f,  21.0f / 18.0f, -21.0f / 18.0f,  7.0f / 18.0f,
};

class GrGLBicubicEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor& effect, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBicubicEffect& bicubicEffect = effect.cast<GrBicubicEffect>();
        b->add32(GrTextureDomain::GLDomain::DomainKey(bicubicEffect.domain()));
        b->add32(GrColorSpaceXform::XformKey(bicubicEffect.colorSpaceXform()));
    }

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    UniformHandle               fCoefficientsUni;
    UniformHandle               fImageIncrementUni;
    UniformHandle               fColorSpaceXformUni;
    GrTextureDomain::GLDomain   fDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLBicubicEffect::emitCode(EmitArgs& args) {
    const GrBicubicEffect& bicubicEffect = args.fFp.cast<GrBicubicEffect>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fCoefficientsUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                  kMat44f_GrSLType, kDefault_GrSLPrecision,
                                                  "Coefficients");
    fImageIncrementUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                    "ImageIncrement");

    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);
    const char* coeff = uniformHandler->getUniformCStr(fCoefficientsUni);

    GrGLSLColorSpaceXformHelper colorSpaceHelper(uniformHandler, bicubicEffect.colorSpaceXform(),
                                                 &fColorSpaceXformUni);

    SkString cubicBlendName;

    static const GrShaderVar gCubicBlendArgs[] = {
        GrShaderVar("coefficients",  kMat44f_GrSLType),
        GrShaderVar("t",             kFloat_GrSLType),
        GrShaderVar("c0",            kVec4f_GrSLType),
        GrShaderVar("c1",            kVec4f_GrSLType),
        GrShaderVar("c2",            kVec4f_GrSLType),
        GrShaderVar("c3",            kVec4f_GrSLType),
    };
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    fragBuilder->emitFunction(kVec4f_GrSLType,
                              "cubicBlend",
                              SK_ARRAY_COUNT(gCubicBlendArgs),
                              gCubicBlendArgs,
                              "\tvec4 ts = vec4(1.0, t, t * t, t * t * t);\n"
                              "\tvec4 c = coefficients * ts;\n"
                              "\treturn c.x * c0 + c.y * c1 + c.z * c2 + c.w * c3;\n",
                              &cubicBlendName);
    fragBuilder->codeAppendf("\tvec2 coord = %s - %s * vec2(0.5);\n", coords2D.c_str(), imgInc);
    // We unnormalize the coord in order to determine our fractional offset (f) within the texel
    // We then snap coord to a texel center and renormalize. The snap prevents cases where the
    // starting coords are near a texel boundary and accumulations of imgInc would cause us to skip/
    // double hit a texel.
    fragBuilder->codeAppendf("\tcoord /= %s;\n", imgInc);
    fragBuilder->codeAppend("\tvec2 f = fract(coord);\n");
    fragBuilder->codeAppendf("\tcoord = (coord - f + vec2(0.5)) * %s;\n", imgInc);
    fragBuilder->codeAppend("\tvec4 rowColors[4];\n");
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            SkString coord;
            coord.printf("coord + %s * vec2(%d, %d)", imgInc, x - 1, y - 1);
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
            "\tvec4 s%d = %s(%s, f.x, rowColors[0], rowColors[1], rowColors[2], rowColors[3]);\n",
            y, cubicBlendName.c_str(), coeff);
    }
    SkString bicubicColor;
    bicubicColor.printf("%s(%s, f.y, s0, s1, s2, s3)", cubicBlendName.c_str(), coeff);
    if (colorSpaceHelper.getXformMatrix()) {
        SkString xformedColor;
        fragBuilder->appendColorGamutXform(&xformedColor, bicubicColor.c_str(), &colorSpaceHelper);
        bicubicColor.swap(xformedColor);
    }
    fragBuilder->codeAppendf("\t%s = %s;\n",
                             args.fOutputColor, (GrGLSLExpr4(bicubicColor.c_str()) *
                                                 GrGLSLExpr4(args.fInputColor)).c_str());
}

void GrGLBicubicEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                  const GrProcessor& processor) {
    const GrBicubicEffect& bicubicEffect = processor.cast<GrBicubicEffect>();
    GrTexture* texture = processor.textureSampler(0).texture();
    float imageIncrement[2];
    imageIncrement[0] = 1.0f / texture->width();
    imageIncrement[1] = 1.0f / texture->height();
    pdman.set2fv(fImageIncrementUni, 1, imageIncrement);
    pdman.setMatrix4f(fCoefficientsUni, kMitchellCoefficients);
    fDomain.setData(pdman, bicubicEffect.domain(), texture->origin());
    if (SkToBool(bicubicEffect.colorSpaceXform())) {
        pdman.setSkMatrix44(fColorSpaceXformUni, bicubicEffect.colorSpaceXform()->srcToDst());
    }
}

GrBicubicEffect::GrBicubicEffect(GrTexture* texture,
                                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                                 const SkMatrix &matrix,
                                 const SkShader::TileMode tileModes[2])
  : INHERITED(texture, std::move(colorSpaceXform), matrix,
              GrSamplerParams(tileModes, GrSamplerParams::kNone_FilterMode))
  , fDomain(GrTextureDomain::IgnoredDomain()) {
    this->initClassID<GrBicubicEffect>();
}

GrBicubicEffect::GrBicubicEffect(GrTexture* texture,
                                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                                 const SkMatrix &matrix,
                                 const SkRect& domain)
  : INHERITED(texture, std::move(colorSpaceXform), matrix,
              GrSamplerParams(SkShader::kClamp_TileMode, GrSamplerParams::kNone_FilterMode))
  , fDomain(domain, GrTextureDomain::kClamp_Mode) {
    this->initClassID<GrBicubicEffect>();
}

GrBicubicEffect::~GrBicubicEffect() {
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
    return fDomain == s.fDomain;
}

void GrBicubicEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    // FIXME: Perhaps we can do better.
    inout->mulByUnknownSingleComponent();
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrBicubicEffect);

sk_sp<GrFragmentProcessor> GrBicubicEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    auto colorSpaceXform = GrTest::TestColorXform(d->fRandom);
    static const SkShader::TileMode kClampClamp[] =
        { SkShader::kClamp_TileMode, SkShader::kClamp_TileMode };
    return GrBicubicEffect::Make(d->fTextures[texIdx], colorSpaceXform,
                                 GrCoordTransform::MakeDivByTextureWHMatrix(d->fTextures[texIdx]),
                                 kClampClamp);
}

//////////////////////////////////////////////////////////////////////////////

bool GrBicubicEffect::ShouldUseBicubic(const SkMatrix& matrix,
                                       GrSamplerParams::FilterMode* filterMode) {
    if (matrix.isIdentity()) {
        *filterMode = GrSamplerParams::kNone_FilterMode;
        return false;
    }

    SkScalar scales[2];
    if (!matrix.getMinMaxScales(scales) || scales[0] < SK_Scalar1) {
        // Bicubic doesn't handle arbitrary minimization well, as src texels can be skipped
        // entirely,
        *filterMode = GrSamplerParams::kMipMap_FilterMode;
        return false;
    }
    // At this point if scales[1] == SK_Scalar1 then the matrix doesn't do any scaling.
    if (scales[1] == SK_Scalar1) {
        if (matrix.rectStaysRect() && SkScalarIsInt(matrix.getTranslateX()) &&
            SkScalarIsInt(matrix.getTranslateY())) {
            *filterMode = GrSamplerParams::kNone_FilterMode;
        } else {
            // Use bilerp to handle rotation or fractional translation.
            *filterMode = GrSamplerParams::kBilerp_FilterMode;
        }
        return false;
    }
    // When we use the bicubic filtering effect each sample is read from the texture using
    // nearest neighbor sampling.
    *filterMode = GrSamplerParams::kNone_FilterMode;
    return true;
}
