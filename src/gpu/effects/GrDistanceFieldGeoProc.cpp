/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldGeoProc.h"

#include "GrTexture.h"
#include "SkDistanceFieldGen.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

// Assuming a radius of a little less than the diagonal of the fragment
#define SK_DistanceFieldAAFactor     "0.65"

class GrGLDistanceFieldA8TextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLDistanceFieldA8TextGeoProc()
        : fViewMatrix(SkMatrix::InvalidMatrix())
#ifdef SK_GAMMA_APPLY_TO_A8
        , fDistanceAdjust(-1.0f)
#endif
        {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldA8TextGeoProc& dfTexEffect =
                args.fGP.cast<GrDistanceFieldA8TextGeoProc>();
        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfTexEffect);

#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* distanceAdjustUniName = nullptr;
        // width, height, 1/(3*width)
        fDistanceAdjustUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                        kFloat_GrSLType, kDefault_GrSLPrecision,
                                                        "DistanceAdjust", &distanceAdjustUniName);
#endif

        // Setup pass through color
        varyingHandler->addPassThroughAttribute(dfTexEffect.inColor(), args.fOutputColor);

        // Setup position
        this->setupPosition(vertBuilder,
                            uniformHandler,
                            gpArgs,
                            dfTexEffect.inPosition()->fName,
                            dfTexEffect.viewMatrix(),
                            &fViewMatrixUniform);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             gpArgs->fPositionVar,
                             dfTexEffect.inPosition()->fName,
                             args.fFPCoordTransformHandler);

        // add varyings
        GrGLSLVertToFrag recipScale(kFloat_GrSLType);
        GrGLSLVertToFrag uv(kVec2f_GrSLType);
        bool isUniformScale = (dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask) ==
                              kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity = SkToBool(dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        bool isGammaCorrect =
            SkToBool(dfTexEffect.getFlags() & kGammaCorrect_DistanceFieldEffectFlag);
        bool isAliased =
            SkToBool(dfTexEffect.getFlags() & kAliased_DistanceFieldEffectFlag);
        varyingHandler->addVarying("TextureCoords", &uv, kHigh_GrSLPrecision);
        vertBuilder->codeAppendf("%s = %s;", uv.vsOut(), dfTexEffect.inTextureCoords()->fName);

        // compute numbers to be hardcoded to convert texture coordinates from float to int
        SkASSERT(dfTexEffect.numTextureSamplers() == 1);
        GrTexture* atlas = dfTexEffect.textureSampler(0).peekTexture();
        SkASSERT(atlas && SkIsPow2(atlas->width()) && SkIsPow2(atlas->height()));

        GrGLSLVertToFrag st(kVec2f_GrSLType);
        varyingHandler->addVarying("IntTextureCoords", &st, kHigh_GrSLPrecision);
        vertBuilder->codeAppendf("%s = vec2(%d, %d) * %s;", st.vsOut(),
                                 atlas->width(), atlas->height(),
                                 dfTexEffect.inTextureCoords()->fName);

        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("highp vec2 uv = %s;\n", uv.fsIn());

        fragBuilder->codeAppend("\tfloat texColor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0],
                                         "uv",
                                         kVec2f_GrSLType);
        fragBuilder->codeAppend(".r;\n");
        fragBuilder->codeAppend("\tfloat distance = "
                       SK_DistanceFieldMultiplier "*(texColor - " SK_DistanceFieldThreshold ");");
#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust width based on gamma
        fragBuilder->codeAppendf("distance -= %s;", distanceAdjustUniName);
#endif

        fragBuilder->codeAppend("float afwidth;");
        if (isUniformScale) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the t coordinate in the y direction.
            // We use st coordinates to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdx(%s.x));",
                                     st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdy(%s.y));",
                                     st.fsIn());
#endif
        } else if (isSimilarity) {
            // For similarity transform, we adjust the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("float st_grad_len = length(dFdx(%s));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("float st_grad_len = length(dFdy(%s));", st.fsIn());
#endif
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*st_grad_len);");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("vec2 dist_grad = vec2(dFdx(distance), dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("float dg_len2 = dot(dist_grad, dist_grad);");
            fragBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fragBuilder->codeAppend("dist_grad = vec2(0.7071, 0.7071);");
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppend("dist_grad = dist_grad*inversesqrt(dg_len2);");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppendf("vec2 Jdx = dFdx(%s);", st.fsIn());
            fragBuilder->codeAppendf("vec2 Jdy = dFdy(%s);", st.fsIn());
            fragBuilder->codeAppend("vec2 grad = vec2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fragBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }

        if (isAliased) {
            fragBuilder->codeAppend("float val = distance > 0 ? 1.0 : 0.0;");
        } else if (isGammaCorrect) {
            // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
            // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want
            // distance mapped linearly to coverage, so use a linear step:
            fragBuilder->codeAppend(
                "float val = clamp((distance + afwidth) / (2.0 * afwidth), 0.0, 1.0);");
        } else {
            fragBuilder->codeAppend("float val = smoothstep(-afwidth, afwidth, distance);");
        }

        fragBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {
#ifdef SK_GAMMA_APPLY_TO_A8
        const GrDistanceFieldA8TextGeoProc& dfTexEffect = proc.cast<GrDistanceFieldA8TextGeoProc>();
        float distanceAdjust = dfTexEffect.getDistanceAdjust();
        if (distanceAdjust != fDistanceAdjust) {
            pdman.set1f(fDistanceAdjustUni, distanceAdjust);
            fDistanceAdjust = distanceAdjust;
        }
#endif
        const GrDistanceFieldA8TextGeoProc& dfa8gp = proc.cast<GrDistanceFieldA8TextGeoProc>();

        if (!dfa8gp.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(dfa8gp.viewMatrix())) {
            fViewMatrix = dfa8gp.viewMatrix();
            float viewMatrix[3 * 3];
            GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
            pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
        }
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldA8TextGeoProc& dfTexEffect = gp.cast<GrDistanceFieldA8TextGeoProc>();
        uint32_t key = dfTexEffect.getFlags();
        key |= ComputePosKey(dfTexEffect.viewMatrix()) << 16;
        b->add32(key);

        // Currently we hardcode numbers to convert atlas coordinates to normalized floating point
        SkASSERT(gp.numTextureSamplers() == 1);
        GrTextureProxy* atlas = gp.textureSampler(0).proxy();
        if (atlas) {
            b->add32(atlas->width());
            b->add32(atlas->height());
        }
    }

private:
    SkMatrix      fViewMatrix;
    UniformHandle fViewMatrixUniform;
#ifdef SK_GAMMA_APPLY_TO_A8
    float         fDistanceAdjust;
    UniformHandle fDistanceAdjustUni;
#endif

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldA8TextGeoProc::GrDistanceFieldA8TextGeoProc(GrColor color,
                                                           const SkMatrix& viewMatrix,
                                                           sk_sp<GrTextureProxy> proxy,
                                                           const GrSamplerParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                           float distanceAdjust,
#endif
                                                           uint32_t flags,
                                                           bool usesLocalCoords)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fTextureSampler(std::move(proxy), params)
#ifdef SK_GAMMA_APPLY_TO_A8
    , fDistanceAdjust(distanceAdjust)
#endif
    , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
    , fInColor(nullptr)
    , fUsesLocalCoords(usesLocalCoords) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->initClassID<GrDistanceFieldA8TextGeoProc>();
    fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                         kHigh_GrSLPrecision);
    fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
    fInTextureCoords = &this->addVertexAttrib("inTextureCoords", kVec2us_GrVertexAttribType,
                                              kHigh_GrSLPrecision);
    this->addTextureSampler(&fTextureSampler);
}

void GrDistanceFieldA8TextGeoProc::getGLSLProcessorKey(const GrShaderCaps& caps,
                                                       GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldA8TextGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor*
GrDistanceFieldA8TextGeoProc::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLDistanceFieldA8TextGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldA8TextGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> GrDistanceFieldA8TextGeoProc::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrSamplerParams params(tileModes, d->fRandom->nextBool() ? GrSamplerParams::kBilerp_FilterMode
                                                             : GrSamplerParams::kNone_FilterMode);

    uint32_t flags = 0;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }

    return GrDistanceFieldA8TextGeoProc::Make(GrRandomColor(d->fRandom),
                                              GrTest::TestMatrix(d->fRandom),
                                              std::move(proxy), params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                              d->fRandom->nextF(),
#endif
                                              flags,
                                              d->fRandom->nextBool());
}
#endif

///////////////////////////////////////////////////////////////////////////////

class GrGLDistanceFieldPathGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLDistanceFieldPathGeoProc()
        : fViewMatrix(SkMatrix::InvalidMatrix())
        , fTextureSize(SkISize::Make(-1, -1)) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldPathGeoProc& dfTexEffect = args.fGP.cast<GrDistanceFieldPathGeoProc>();

        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfTexEffect);

        GrGLSLVertToFrag v(kVec2f_GrSLType);
        varyingHandler->addVarying("TextureCoords", &v, kHigh_GrSLPrecision);

        // setup pass through color
        varyingHandler->addPassThroughAttribute(dfTexEffect.inColor(), args.fOutputColor);
        vertBuilder->codeAppendf("%s = %s;", v.vsOut(), dfTexEffect.inTextureCoords()->fName);

        // Setup position
        this->setupPosition(vertBuilder,
                            uniformHandler,
                            gpArgs,
                            dfTexEffect.inPosition()->fName,
                            dfTexEffect.viewMatrix(),
                            &fViewMatrixUniform);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             gpArgs->fPositionVar,
                             dfTexEffect.inPosition()->fName,
                             args.fFPCoordTransformHandler);

        const char* textureSizeUniName = nullptr;
        fTextureSizeUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                     kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                     "TextureSize", &textureSizeUniName);

        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("highp vec2 uv = %s;", v.fsIn());

        fragBuilder->codeAppend("float texColor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0],
                                         "uv",
                                         kVec2f_GrSLType);
        fragBuilder->codeAppend(".r;");
        fragBuilder->codeAppend("float distance = "
            SK_DistanceFieldMultiplier "*(texColor - " SK_DistanceFieldThreshold ");");

        fragBuilder->codeAppendf("highp vec2 st = uv*%s;", textureSizeUniName);
        fragBuilder->codeAppend("float afwidth;");
        bool isUniformScale = (dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask) ==
                               kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity = SkToBool(dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        bool isGammaCorrect =
            SkToBool(dfTexEffect.getFlags() & kGammaCorrect_DistanceFieldEffectFlag);
        if (isUniformScale) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the t coordinate in the y direction.
            // We use st coordinates to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdx(st.x));");
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdy(st.y));");
#endif
        } else if (isSimilarity) {
            // For similarity transform, we adjust the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppend("float st_grad_len = length(dFdx(st));");
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppend("float st_grad_len = length(dFdy(st));");
#endif
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*st_grad_len);");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("vec2 dist_grad = vec2(dFdx(distance), dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("float dg_len2 = dot(dist_grad, dist_grad);");
            fragBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fragBuilder->codeAppend("dist_grad = vec2(0.7071, 0.7071);");
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppend("dist_grad = dist_grad*inversesqrt(dg_len2);");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppend("vec2 Jdx = dFdx(st);");
            fragBuilder->codeAppend("vec2 Jdy = dFdy(st);");
            fragBuilder->codeAppend("vec2 grad = vec2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fragBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }
        // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
        // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want distance
        // mapped linearly to coverage, so use a linear step:
        if (isGammaCorrect) {
            fragBuilder->codeAppend(
                "float val = clamp((distance + afwidth) / (2.0 * afwidth), 0.0, 1.0);");
        } else {
            fragBuilder->codeAppend("float val = smoothstep(-afwidth, afwidth, distance);");
        }

        fragBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {
        SkASSERT(fTextureSizeUni.isValid());

        GrTexture* texture = proc.textureSampler(0).peekTexture();

        if (texture->width() != fTextureSize.width() ||
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            pdman.set2f(fTextureSizeUni,
                        SkIntToScalar(fTextureSize.width()),
                        SkIntToScalar(fTextureSize.height()));
        }

        const GrDistanceFieldPathGeoProc& dfpgp = proc.cast<GrDistanceFieldPathGeoProc>();

        if (!dfpgp.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(dfpgp.viewMatrix())) {
            fViewMatrix = dfpgp.viewMatrix();
            float viewMatrix[3 * 3];
            GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
            pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
        }
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldPathGeoProc& dfTexEffect = gp.cast<GrDistanceFieldPathGeoProc>();

        uint32_t key = dfTexEffect.getFlags();
        key |= ComputePosKey(dfTexEffect.viewMatrix()) << 16;
        b->add32(key);
    }

private:
    UniformHandle fTextureSizeUni;
    UniformHandle fViewMatrixUniform;
    SkMatrix      fViewMatrix;
    SkISize       fTextureSize;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
GrDistanceFieldPathGeoProc::GrDistanceFieldPathGeoProc(GrColor color,
                                                       const SkMatrix& viewMatrix,
                                                       sk_sp<GrTextureProxy> proxy,
                                                       const GrSamplerParams& params,
                                                       uint32_t flags,
                                                       bool usesLocalCoords)
        : fColor(color)
        , fViewMatrix(viewMatrix)
        , fTextureSampler(std::move(proxy), params)
        , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
        , fInColor(nullptr)
        , fUsesLocalCoords(usesLocalCoords) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->initClassID<GrDistanceFieldPathGeoProc>();
    fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                         kHigh_GrSLPrecision);
    fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
    fInTextureCoords = &this->addVertexAttrib("inTextureCoords", kVec2us_GrVertexAttribType);
    this->addTextureSampler(&fTextureSampler);
}

void GrDistanceFieldPathGeoProc::getGLSLProcessorKey(const GrShaderCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldPathGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor*
GrDistanceFieldPathGeoProc::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLDistanceFieldPathGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldPathGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> GrDistanceFieldPathGeoProc::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrSamplerParams params(tileModes, d->fRandom->nextBool() ? GrSamplerParams::kBilerp_FilterMode
                                                             : GrSamplerParams::kNone_FilterMode);

    uint32_t flags = 0;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }

    return GrDistanceFieldPathGeoProc::Make(GrRandomColor(d->fRandom),
                                            GrTest::TestMatrix(d->fRandom),
                                            std::move(proxy),
                                            params,
                                            flags,
                                            d->fRandom->nextBool());
}
#endif

///////////////////////////////////////////////////////////////////////////////

class GrGLDistanceFieldLCDTextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLDistanceFieldLCDTextGeoProc()
        : fViewMatrix(SkMatrix::InvalidMatrix()) {
        fDistanceAdjust = GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(1.0f, 1.0f, 1.0f);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldLCDTextGeoProc& dfTexEffect =
                args.fGP.cast<GrDistanceFieldLCDTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfTexEffect);

        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // setup pass through color
        varyingHandler->addPassThroughAttribute(dfTexEffect.inColor(), args.fOutputColor);

        // Setup position
        this->setupPosition(vertBuilder,
                            uniformHandler,
                            gpArgs,
                            dfTexEffect.inPosition()->fName,
                            dfTexEffect.viewMatrix(),
                            &fViewMatrixUniform);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             gpArgs->fPositionVar,
                             dfTexEffect.inPosition()->fName,
                             args.fFPCoordTransformHandler);

        // set up varyings
        bool isUniformScale = (dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask) ==
                              kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity = SkToBool(dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        bool isGammaCorrect =
            SkToBool(dfTexEffect.getFlags() & kGammaCorrect_DistanceFieldEffectFlag);
        GrGLSLVertToFrag recipScale(kFloat_GrSLType);
        GrGLSLVertToFrag uv(kVec2f_GrSLType);
        varyingHandler->addVarying("TextureCoords", &uv, kHigh_GrSLPrecision);
        vertBuilder->codeAppendf("%s = %s;", uv.vsOut(), dfTexEffect.inTextureCoords()->fName);

        // compute numbers to be hardcoded to convert texture coordinates from float to int
        SkASSERT(dfTexEffect.numTextureSamplers() == 1);
        GrTexture* atlas = dfTexEffect.textureSampler(0).peekTexture();
        SkASSERT(atlas && SkIsPow2(atlas->width()) && SkIsPow2(atlas->height()));

        GrGLSLVertToFrag st(kVec2f_GrSLType);
        varyingHandler->addVarying("IntTextureCoords", &st, kHigh_GrSLPrecision);
        vertBuilder->codeAppendf("%s = vec2(%d, %d) * %s;", st.vsOut(),
                                 atlas->width(), atlas->height(),
                                 dfTexEffect.inTextureCoords()->fName);

        // add frag shader code

        // create LCD offset adjusted by inverse of transform
        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("highp vec2 uv = %s;\n", uv.fsIn());

        SkScalar lcdDelta = 1.0f / (3.0f * atlas->width());
        if (dfTexEffect.getFlags() & kBGR_DistanceFieldEffectFlag) {
            fragBuilder->codeAppendf("highp float delta = -%.*f;\n", SK_FLT_DECIMAL_DIG, lcdDelta);
        } else {
            fragBuilder->codeAppendf("highp float delta = %.*f;\n", SK_FLT_DECIMAL_DIG, lcdDelta);
        }
        if (isUniformScale) {
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("float st_grad_len = abs(dFdx(%s.x));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("float st_grad_len = abs(dFdy(%s.y));", st.fsIn());
#endif
            fragBuilder->codeAppend("vec2 offset = vec2(st_grad_len*delta, 0.0);");
        } else if (isSimilarity) {
            // For a similarity matrix with rotation, the gradient will not be aligned
            // with the texel coordinate axes, so we need to calculate it.
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("vec2 st_grad = dFdx(%s);", st.fsIn());
            fragBuilder->codeAppend("vec2 offset = delta*st_grad;");
#else
            // We use dFdy because of a Mali 400 bug, and rotate -90 degrees to
            // get the gradient in the x direction.
            fragBuilder->codeAppendf("vec2 st_grad = dFdy(%s);", st.fsIn());
            fragBuilder->codeAppend("vec2 offset = delta*vec2(st_grad.y, -st_grad.x);");
#endif
            fragBuilder->codeAppend("float st_grad_len = length(st_grad);");
        } else {
            fragBuilder->codeAppendf("vec2 st = %s;\n", st.fsIn());

            fragBuilder->codeAppend("vec2 Jdx = dFdx(st);");
            fragBuilder->codeAppend("vec2 Jdy = dFdy(st);");
            fragBuilder->codeAppend("vec2 offset = delta*Jdx;");
        }

        // green is distance to uv center
        fragBuilder->codeAppend("\tvec4 texColor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "uv", kVec2f_GrSLType);
        fragBuilder->codeAppend(";\n");
        fragBuilder->codeAppend("\tvec3 distance;\n");
        fragBuilder->codeAppend("\tdistance.y = texColor.r;\n");
        // red is distance to left offset
        fragBuilder->codeAppend("\tvec2 uv_adjusted = uv - offset;\n");
        fragBuilder->codeAppend("\ttexColor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "uv_adjusted", kVec2f_GrSLType);
        fragBuilder->codeAppend(";\n");
        fragBuilder->codeAppend("\tdistance.x = texColor.r;\n");
        // blue is distance to right offset
        fragBuilder->codeAppend("\tuv_adjusted = uv + offset;\n");
        fragBuilder->codeAppend("\ttexColor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "uv_adjusted", kVec2f_GrSLType);
        fragBuilder->codeAppend(";\n");
        fragBuilder->codeAppend("\tdistance.z = texColor.r;\n");

        fragBuilder->codeAppend("\tdistance = "
           "vec3(" SK_DistanceFieldMultiplier ")*(distance - vec3(" SK_DistanceFieldThreshold"));");

        // adjust width based on gamma
        const char* distanceAdjustUniName = nullptr;
        fDistanceAdjustUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                        kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                        "DistanceAdjust", &distanceAdjustUniName);
        fragBuilder->codeAppendf("distance -= %s;", distanceAdjustUniName);

        // To be strictly correct, we should compute the anti-aliasing factor separately
        // for each color component. However, this is only important when using perspective
        // transformations, and even then using a single factor seems like a reasonable
        // trade-off between quality and speed.
        fragBuilder->codeAppend("float afwidth;");
        if (isSimilarity) {
            // For similarity transform (uniform scale-only is a subset of this), we adjust for the
            // effect of the transformation on the distance by using the length of the gradient of
            // the texture coordinates. We use st coordinates to ensure we're mapping 1:1 from texel
            // space to pixel space.

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*st_grad_len;");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("vec2 dist_grad = vec2(dFdx(distance.r), dFdy(distance.r));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("float dg_len2 = dot(dist_grad, dist_grad);");
            fragBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fragBuilder->codeAppend("dist_grad = vec2(0.7071, 0.7071);");
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppend("dist_grad = dist_grad*inversesqrt(dg_len2);");
            fragBuilder->codeAppend("}");
            fragBuilder->codeAppend("vec2 grad = vec2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fragBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }

        // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
        // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want distance
        // mapped linearly to coverage, so use a linear step:
        if (isGammaCorrect) {
            fragBuilder->codeAppendf("%s = "
                "vec4(clamp((distance + vec3(afwidth)) / vec3(2.0 * afwidth), 0.0, 1.0), 1.0);",
                args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf(
                "%s = vec4(smoothstep(vec3(-afwidth), vec3(afwidth), distance), 1.0);",
                args.fOutputCoverage);
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& processor,
                 FPCoordTransformIter&& transformIter) override {
        SkASSERT(fDistanceAdjustUni.isValid());

        const GrDistanceFieldLCDTextGeoProc& dflcd = processor.cast<GrDistanceFieldLCDTextGeoProc>();
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust wa = dflcd.getDistanceAdjust();
        if (wa != fDistanceAdjust) {
            pdman.set3f(fDistanceAdjustUni,
                        wa.fR,
                        wa.fG,
                        wa.fB);
            fDistanceAdjust = wa;
        }

        if (!dflcd.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(dflcd.viewMatrix())) {
            fViewMatrix = dflcd.viewMatrix();
            float viewMatrix[3 * 3];
            GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
            pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
        }
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldLCDTextGeoProc& dfTexEffect = gp.cast<GrDistanceFieldLCDTextGeoProc>();

        uint32_t key = dfTexEffect.getFlags();
        key |= ComputePosKey(dfTexEffect.viewMatrix()) << 16;
        b->add32(key);

        // Currently we hardcode numbers to convert atlas coordinates to normalized floating point
        SkASSERT(gp.numTextureSamplers() == 1);
        GrTextureProxy* atlas = gp.textureSampler(0).proxy();
        if (atlas) {
            b->add32(atlas->width());
            b->add32(atlas->height());
        }
    }

private:
    SkMatrix                                     fViewMatrix;
    UniformHandle                                fViewMatrixUniform;
    UniformHandle                                fColorUniform;
    GrDistanceFieldLCDTextGeoProc::DistanceAdjust fDistanceAdjust;
    UniformHandle                                fDistanceAdjustUni;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
GrDistanceFieldLCDTextGeoProc::GrDistanceFieldLCDTextGeoProc(
                                                  GrColor color, const SkMatrix& viewMatrix,
                                                  sk_sp<GrTextureProxy> proxy,
                                                  const GrSamplerParams& params,
                                                  DistanceAdjust distanceAdjust,
                                                  uint32_t flags, bool usesLocalCoords)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fTextureSampler(std::move(proxy), params)
    , fDistanceAdjust(distanceAdjust)
    , fFlags(flags & kLCD_DistanceFieldEffectMask)
    , fUsesLocalCoords(usesLocalCoords) {
    SkASSERT(!(flags & ~kLCD_DistanceFieldEffectMask) && (flags & kUseLCD_DistanceFieldEffectFlag));
    this->initClassID<GrDistanceFieldLCDTextGeoProc>();
    fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                         kHigh_GrSLPrecision);
    fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
    fInTextureCoords = &this->addVertexAttrib("inTextureCoords", kVec2us_GrVertexAttribType,
                                              kHigh_GrSLPrecision);
    this->addTextureSampler(&fTextureSampler);
}

void GrDistanceFieldLCDTextGeoProc::getGLSLProcessorKey(const GrShaderCaps& caps,
                                                        GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldLCDTextGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrDistanceFieldLCDTextGeoProc::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLDistanceFieldLCDTextGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldLCDTextGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> GrDistanceFieldLCDTextGeoProc::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrSamplerParams params(tileModes, d->fRandom->nextBool() ? GrSamplerParams::kBilerp_FilterMode
                                                             : GrSamplerParams::kNone_FilterMode);
    DistanceAdjust wa = { 0.0f, 0.1f, -0.1f };
    uint32_t flags = kUseLCD_DistanceFieldEffectFlag;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }
    flags |= d->fRandom->nextBool() ? kBGR_DistanceFieldEffectFlag : 0;
    return GrDistanceFieldLCDTextGeoProc::Make(GrRandomColor(d->fRandom),
                                               GrTest::TestMatrix(d->fRandom),
                                               std::move(proxy), params,
                                               wa,
                                               flags,
                                               d->fRandom->nextBool());
}
#endif
