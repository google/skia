/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldGeoProc.h"
#include "GrAtlasedShaderHelpers.h"
#include "GrCaps.h"
#include "GrShaderCaps.h"
#include "GrTexture.h"
#include "SkDistanceFieldGen.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

// Assuming a radius of a little less than the diagonal of the fragment
#define SK_DistanceFieldAAFactor     "0.65"

class GrGLDistanceFieldA8TextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLDistanceFieldA8TextGeoProc() = default;

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldA8TextGeoProc& dfTexEffect =
                args.fGP.cast<GrDistanceFieldA8TextGeoProc>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfTexEffect);

        const char* atlasSizeInvName;
        fAtlasSizeInvUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                          kFloat2_GrSLType,
                                                          kHigh_GrSLPrecision,
                                                          "AtlasSizeInv",
                                                          &atlasSizeInvName);
#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* distanceAdjustUniName = nullptr;
        // width, height, 1/(3*width)
        fDistanceAdjustUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                        "DistanceAdjust", &distanceAdjustUniName);
#endif

        // Setup pass through color
        varyingHandler->addPassThroughAttribute(dfTexEffect.inColor(), args.fOutputColor);

        // Setup position
        gpArgs->fPositionVar = dfTexEffect.inPosition().asShaderVar();

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             dfTexEffect.inPosition().asShaderVar(),
                             dfTexEffect.localMatrix(),
                             args.fFPCoordTransformHandler);

        // add varyings
        GrGLSLVarying uv(kFloat2_GrSLType);
        GrSLType texIdxType = args.fShaderCaps->integerSupport() ? kInt_GrSLType : kFloat_GrSLType;
        GrGLSLVarying texIdx(texIdxType);
        GrGLSLVarying st(kFloat2_GrSLType);
        append_index_uv_varyings(args, dfTexEffect.inTextureCoords().name(), atlasSizeInvName, &uv,
                                 &texIdx, &st);

        bool isUniformScale = (dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask) ==
                              kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity = SkToBool(dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        bool isGammaCorrect =
            SkToBool(dfTexEffect.getFlags() & kGammaCorrect_DistanceFieldEffectFlag);
        bool isAliased =
            SkToBool(dfTexEffect.getFlags() & kAliased_DistanceFieldEffectFlag);

        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("float2 uv = %s;\n", uv.fsIn());
        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, dfTexEffect.numTextureSamplers(),
                                   texIdx, "uv", "texColor");

        fragBuilder->codeAppend("half distance = "
                      SK_DistanceFieldMultiplier "*(texColor.r - " SK_DistanceFieldThreshold ");");
#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust width based on gamma
        fragBuilder->codeAppendf("distance -= %s;", distanceAdjustUniName);
#endif

        fragBuilder->codeAppend("half afwidth;");
        if (isUniformScale) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the t coordinate in the y direction.
            // We use st coordinates to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor
                                    "*half(dFdx(%s.x)));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor
                                     "*half(dFdy(%s.y)));", st.fsIn());
#endif
        } else if (isSimilarity) {
            // For similarity transform, we adjust the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("half st_grad_len = length(half2(dFdx(%s)));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("half st_grad_len = length(half2(dFdy(%s)));", st.fsIn());
#endif
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*st_grad_len);");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("half2 dist_grad = half2(float2(dFdx(distance), "
                                                                   "dFdy(distance)));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("half dg_len2 = dot(dist_grad, dist_grad);");
            fragBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fragBuilder->codeAppend("dist_grad = half2(0.7071, 0.7071);");
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppend("dist_grad = dist_grad*half(inversesqrt(dg_len2));");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppendf("half2 Jdx = half2(dFdx(%s));", st.fsIn());
            fragBuilder->codeAppendf("half2 Jdy = half2(dFdy(%s));", st.fsIn());
            fragBuilder->codeAppend("half2 grad = half2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fragBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }

        if (isAliased) {
            fragBuilder->codeAppend("half val = distance > 0 ? 1.0 : 0.0;");
        } else if (isGammaCorrect) {
            // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
            // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want
            // distance mapped linearly to coverage, so use a linear step:
            fragBuilder->codeAppend(
                "half val = saturate((distance + afwidth) / (2.0 * afwidth));");
        } else {
            fragBuilder->codeAppend("half val = smoothstep(-afwidth, afwidth, distance);");
        }

        fragBuilder->codeAppendf("%s = half4(val);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {
        const GrDistanceFieldA8TextGeoProc& dfa8gp = proc.cast<GrDistanceFieldA8TextGeoProc>();

#ifdef SK_GAMMA_APPLY_TO_A8
        float distanceAdjust = dfa8gp.getDistanceAdjust();
        if (distanceAdjust != fDistanceAdjust) {
            fDistanceAdjust = distanceAdjust;
            pdman.set1f(fDistanceAdjustUni, distanceAdjust);
        }
#endif

        const SkISize& atlasSize = dfa8gp.atlasSize();
        SkASSERT(SkIsPow2(atlasSize.fWidth) && SkIsPow2(atlasSize.fHeight));

        if (fAtlasSize != atlasSize) {
            pdman.set2f(fAtlasSizeInvUniform, 1.0f / atlasSize.fWidth, 1.0f / atlasSize.fHeight);
            fAtlasSize = atlasSize;
        }
        this->setTransformDataHelper(dfa8gp.localMatrix(), pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldA8TextGeoProc& dfTexEffect = gp.cast<GrDistanceFieldA8TextGeoProc>();
        uint32_t key = dfTexEffect.getFlags();
        b->add32(key);
        b->add32(dfTexEffect.numTextureSamplers());
    }

private:
#ifdef SK_GAMMA_APPLY_TO_A8
    float fDistanceAdjust = -1.f;
    UniformHandle fDistanceAdjustUni;
#endif
    SkISize fAtlasSize = {0, 0};
    UniformHandle fAtlasSizeInvUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldA8TextGeoProc::GrDistanceFieldA8TextGeoProc(const GrShaderCaps& caps,
                                                           const sk_sp<GrTextureProxy>* proxies,
                                                           int numProxies,
                                                           const GrSamplerState& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                           float distanceAdjust,
#endif
                                                           uint32_t flags,
                                                           const SkMatrix& localMatrix)
        : INHERITED(kGrDistanceFieldA8TextGeoProc_ClassID)
        , fLocalMatrix(localMatrix)
        , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
#ifdef SK_GAMMA_APPLY_TO_A8
        , fDistanceAdjust(distanceAdjust)
#endif
{
    SkASSERT(numProxies <= kMaxTextures);
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));

    if (flags & kPerspective_DistanceFieldEffectFlag) {
        fInPosition = {"inPosition", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
    } else {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    }
    fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType };
    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.integerSupport() ? kUShort2_GrSLType : kFloat2_GrSLType};
    this->setVertexAttributes(&fInPosition, 3);

    if (numProxies) {
        fAtlasSize = proxies[0]->isize();
    }
    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        SkASSERT(proxies[i]->isize() == fAtlasSize);
        fTextureSamplers[i].reset(proxies[i]->textureType(), proxies[i]->config(), params);
    }
    this->setTextureSamplerCnt(numProxies);
}

void GrDistanceFieldA8TextGeoProc::addNewProxies(const sk_sp<GrTextureProxy>* proxies,
                                                 int numProxies,
                                                 const GrSamplerState& params) {
    SkASSERT(numProxies <= kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasSize = proxies[0]->isize();
    }

    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        SkASSERT(proxies[i]->isize() == fAtlasSize);
        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(proxies[i]->textureType(), proxies[i]->config(), params);
        }
    }
    this->setTextureSamplerCnt(numProxies);
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
    sk_sp<GrTextureProxy> proxies[kMaxTextures] = {
        d->textureProxy(texIdx),
        nullptr,
        nullptr,
        nullptr
    };

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kBilerp
                                                   : GrSamplerState::Filter::kNearest);

    uint32_t flags = 0;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
#ifdef SK_GAMMA_APPLY_TO_A8
    float lum = d->fRandom->nextF();
#endif
    return GrDistanceFieldA8TextGeoProc::Make(*d->caps()->shaderCaps(),
                                              proxies, 1,
                                              samplerState,
#ifdef SK_GAMMA_APPLY_TO_A8
                                              lum,
#endif
                                              flags, localMatrix);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class GrGLDistanceFieldPathGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLDistanceFieldPathGeoProc()
            : fMatrix(SkMatrix::InvalidMatrix())
            , fAtlasSize({0,0}) {
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldPathGeoProc& dfPathEffect =
                args.fGP.cast<GrDistanceFieldPathGeoProc>();

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfPathEffect);

        const char* atlasSizeInvName;
        fAtlasSizeInvUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                          kFloat2_GrSLType,
                                                          kHigh_GrSLPrecision,
                                                          "AtlasSizeInv",
                                                          &atlasSizeInvName);

        GrGLSLVarying uv(kFloat2_GrSLType);
        GrSLType texIdxType = args.fShaderCaps->integerSupport() ? kInt_GrSLType : kFloat_GrSLType;
        GrGLSLVarying texIdx(texIdxType);
        GrGLSLVarying st(kFloat2_GrSLType);
        append_index_uv_varyings(args, dfPathEffect.inTextureCoords().name(), atlasSizeInvName, &uv,
                                 &texIdx, &st);

        // setup pass through color
        varyingHandler->addPassThroughAttribute(dfPathEffect.inColor(), args.fOutputColor);

        if (dfPathEffect.matrix().hasPerspective()) {
            // Setup position
            this->writeOutputPosition(vertBuilder,
                                      uniformHandler,
                                      gpArgs,
                                      dfPathEffect.inPosition().name(),
                                      dfPathEffect.matrix(),
                                      &fMatrixUniform);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 dfPathEffect.inPosition().asShaderVar(),
                                 args.fFPCoordTransformHandler);
        } else {
            // Setup position
            this->writeOutputPosition(vertBuilder, gpArgs, dfPathEffect.inPosition().name());

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 dfPathEffect.inPosition().asShaderVar(),
                                 dfPathEffect.matrix(),
                                 args.fFPCoordTransformHandler);
        }

        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("float2 uv = %s;", uv.fsIn());
        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, dfPathEffect.numTextureSamplers(), texIdx, "uv",
                                   "texColor");

        fragBuilder->codeAppend("half distance = "
            SK_DistanceFieldMultiplier "*(texColor.r - " SK_DistanceFieldThreshold ");");

        fragBuilder->codeAppend("half afwidth;");
        bool isUniformScale = (dfPathEffect.getFlags() & kUniformScale_DistanceFieldEffectMask) ==
                              kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity = SkToBool(dfPathEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        bool isGammaCorrect =
                SkToBool(dfPathEffect.getFlags() & kGammaCorrect_DistanceFieldEffectFlag);
        if (isUniformScale) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the t coordinate in the y direction.
            // We use st coordinates to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor
                                     "*half(dFdx(%s.x)));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor
                                     "*half(dFdy(%s.y)));", st.fsIn());
#endif
        } else if (isSimilarity) {
            // For similarity transform, we adjust the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("half st_grad_len = half(length(dFdx(%s)));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("half st_grad_len = half(length(dFdy(%s)));", st.fsIn());
#endif
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*st_grad_len);");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("half2 dist_grad = half2(dFdx(distance), "
                                                            "dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("half dg_len2 = dot(dist_grad, dist_grad);");
            fragBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fragBuilder->codeAppend("dist_grad = half2(0.7071, 0.7071);");
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppend("dist_grad = dist_grad*half(inversesqrt(dg_len2));");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppendf("half2 Jdx = half2(dFdx(%s));", st.fsIn());
            fragBuilder->codeAppendf("half2 Jdy = half2(dFdy(%s));", st.fsIn());
            fragBuilder->codeAppend("half2 grad = half2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fragBuilder->codeAppend("                   dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }
        // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
        // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want distance
        // mapped linearly to coverage, so use a linear step:
        if (isGammaCorrect) {
            fragBuilder->codeAppend(
                "half val = saturate((distance + afwidth) / (2.0 * afwidth));");
        } else {
            fragBuilder->codeAppend("half val = smoothstep(-afwidth, afwidth, distance);");
        }

        fragBuilder->codeAppendf("%s = half4(val);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {

        const GrDistanceFieldPathGeoProc& dfpgp = proc.cast<GrDistanceFieldPathGeoProc>();

        if (dfpgp.matrix().hasPerspective() && !fMatrix.cheapEqualTo(dfpgp.matrix())) {
            fMatrix = dfpgp.matrix();
            float matrix[3 * 3];
            GrGLSLGetMatrix<3>(matrix, fMatrix);
            pdman.setMatrix3f(fMatrixUniform, matrix);
        }

        const SkISize& atlasSize = dfpgp.atlasSize();
        SkASSERT(SkIsPow2(atlasSize.fWidth) && SkIsPow2(atlasSize.fHeight));
        if (fAtlasSize != atlasSize) {
            pdman.set2f(fAtlasSizeInvUniform, 1.0f / atlasSize.fWidth, 1.0f / atlasSize.fHeight);
            fAtlasSize = atlasSize;
        }

        if (dfpgp.matrix().hasPerspective()) {
            this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
        } else {
            this->setTransformDataHelper(dfpgp.matrix(), pdman, &transformIter);
        }
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldPathGeoProc& dfTexEffect = gp.cast<GrDistanceFieldPathGeoProc>();

        uint32_t key = dfTexEffect.getFlags();
        key |= ComputePosKey(dfTexEffect.matrix()) << 16;
        b->add32(key);
        b->add32(dfTexEffect.matrix().hasPerspective());
        b->add32(dfTexEffect.numTextureSamplers());
    }

private:
    SkMatrix      fMatrix;        // view matrix if perspective, local matrix otherwise
    UniformHandle fMatrixUniform;

    SkISize       fAtlasSize;
    UniformHandle fAtlasSizeInvUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldPathGeoProc::GrDistanceFieldPathGeoProc(const GrShaderCaps& caps,
                                                       const SkMatrix& matrix,
                                                       bool wideColor,
                                                       const sk_sp<GrTextureProxy>* proxies,
                                                       int numProxies,
                                                       const GrSamplerState& params,
                                                       uint32_t flags)
        : INHERITED(kGrDistanceFieldPathGeoProc_ClassID)
        , fMatrix(matrix)
        , fFlags(flags & kNonLCD_DistanceFieldEffectMask) {
    SkASSERT(numProxies <= kMaxTextures);
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));

    fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    fInColor = MakeColorAttribute("inColor", wideColor);
    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.integerSupport() ? kUShort2_GrSLType : kFloat2_GrSLType};
    this->setVertexAttributes(&fInPosition, 3);

    if (numProxies) {
        fAtlasSize = proxies[0]->isize();
    }

    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        SkASSERT(proxies[i]->isize() == fAtlasSize);
        fTextureSamplers[i].reset(proxies[i]->textureType(), proxies[i]->config(), params);
    }
    this->setTextureSamplerCnt(numProxies);
}

void GrDistanceFieldPathGeoProc::addNewProxies(const sk_sp<GrTextureProxy>* proxies,
                                               int numProxies,
                                               const GrSamplerState& params) {
    SkASSERT(numProxies <= kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasSize = proxies[0]->isize();
    }

    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        SkASSERT(proxies[i]->isize() == fAtlasSize);

        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(proxies[i]->textureType(), proxies[i]->config(), params);
        }
    }
    this->setTextureSamplerCnt(numProxies);
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
    sk_sp<GrTextureProxy> proxies[kMaxTextures] = {
        d->textureProxy(texIdx),
        nullptr,
        nullptr,
        nullptr
    };

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kBilerp
                                                   : GrSamplerState::Filter::kNearest);

    uint32_t flags = 0;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }

    return GrDistanceFieldPathGeoProc::Make(*d->caps()->shaderCaps(),
                                            GrTest::TestMatrix(d->fRandom),
                                            d->fRandom->nextBool(),
                                            proxies, 1,
                                            samplerState,
                                            flags);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class GrGLDistanceFieldLCDTextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLDistanceFieldLCDTextGeoProc() : fAtlasSize({0, 0}) {
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

        const char* atlasSizeInvName;
        fAtlasSizeInvUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                          kFloat2_GrSLType,
                                                          kHigh_GrSLPrecision,
                                                          "AtlasSizeInv",
                                                          &atlasSizeInvName);

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // setup pass through color
        varyingHandler->addPassThroughAttribute(dfTexEffect.inColor(), args.fOutputColor);

        // Setup position
        gpArgs->fPositionVar = dfTexEffect.inPosition().asShaderVar();

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             dfTexEffect.inPosition().asShaderVar(),
                             dfTexEffect.localMatrix(),
                             args.fFPCoordTransformHandler);

        // set up varyings
        GrGLSLVarying uv(kFloat2_GrSLType);
        GrSLType texIdxType = args.fShaderCaps->integerSupport() ? kInt_GrSLType : kFloat_GrSLType;
        GrGLSLVarying texIdx(texIdxType);
        GrGLSLVarying st(kFloat2_GrSLType);
        append_index_uv_varyings(args, dfTexEffect.inTextureCoords().name(), atlasSizeInvName, &uv,
                                 &texIdx, &st);

        GrGLSLVarying delta(kFloat_GrSLType);
        varyingHandler->addVarying("Delta", &delta);
        if (dfTexEffect.getFlags() & kBGR_DistanceFieldEffectFlag) {
            vertBuilder->codeAppendf("%s = -%s.x/3.0;", delta.vsOut(), atlasSizeInvName);
        } else {
            vertBuilder->codeAppendf("%s = %s.x/3.0;", delta.vsOut(), atlasSizeInvName);
        }

        // add frag shader code
        bool isUniformScale = (dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask) ==
                              kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity = SkToBool(dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        bool isGammaCorrect =
            SkToBool(dfTexEffect.getFlags() & kGammaCorrect_DistanceFieldEffectFlag);

        // create LCD offset adjusted by inverse of transform
        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("float2 uv = %s;\n", uv.fsIn());

        if (isUniformScale) {
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("half st_grad_len = half(abs(dFdx(%s.x)));", st.fsIn());
#else
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.
            fragBuilder->codeAppendf("half st_grad_len = half(abs(dFdy(%s.y)));", st.fsIn());
#endif
            fragBuilder->codeAppendf("half2 offset = half2(half(st_grad_len*%s), 0.0);",
                                     delta.fsIn());
        } else if (isSimilarity) {
            // For a similarity matrix with rotation, the gradient will not be aligned
            // with the texel coordinate axes, so we need to calculate it.
#ifdef SK_VULKAN
            fragBuilder->codeAppendf("half2 st_grad = half2(dFdx(%s));", st.fsIn());
            fragBuilder->codeAppendf("half2 offset = half(%s)*st_grad;", delta.fsIn());
#else
            // We use dFdy because of a Mali 400 bug, and rotate -90 degrees to
            // get the gradient in the x direction.
            fragBuilder->codeAppendf("half2 st_grad = half2(dFdy(%s));", st.fsIn());
            fragBuilder->codeAppendf("half2 offset = half2(%s*float2(st_grad.y, -st_grad.x));",
                                     delta.fsIn());
#endif
            fragBuilder->codeAppend("half st_grad_len = length(st_grad);");
        } else {
            fragBuilder->codeAppendf("half2 st = half2(%s);\n", st.fsIn());

            fragBuilder->codeAppend("half2 Jdx = half2(dFdx(st));");
            fragBuilder->codeAppend("half2 Jdy = half2(dFdy(st));");
            fragBuilder->codeAppendf("half2 offset = half2(half(%s))*Jdx;", delta.fsIn());
        }

        // sample the texture by index
        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, dfTexEffect.numTextureSamplers(),
                                   texIdx, "uv", "texColor");

        // green is distance to uv center
        fragBuilder->codeAppend("half3 distance;");
        fragBuilder->codeAppend("distance.y = texColor.r;");
        // red is distance to left offset
        fragBuilder->codeAppend("half2 uv_adjusted = half2(uv) - offset;");
        append_multitexture_lookup(args, dfTexEffect.numTextureSamplers(),
                                   texIdx, "uv_adjusted", "texColor");
        fragBuilder->codeAppend("distance.x = texColor.r;");
        // blue is distance to right offset
        fragBuilder->codeAppend("uv_adjusted = half2(uv) + offset;");
        append_multitexture_lookup(args, dfTexEffect.numTextureSamplers(),
                                   texIdx, "uv_adjusted", "texColor");
        fragBuilder->codeAppend("distance.z = texColor.r;");

        fragBuilder->codeAppend("distance = "
           "half3(" SK_DistanceFieldMultiplier ")*(distance - half3(" SK_DistanceFieldThreshold"));");

        // adjust width based on gamma
        const char* distanceAdjustUniName = nullptr;
        fDistanceAdjustUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf3_GrSLType,
                                                        "DistanceAdjust", &distanceAdjustUniName);
        fragBuilder->codeAppendf("distance -= %s;", distanceAdjustUniName);

        // To be strictly correct, we should compute the anti-aliasing factor separately
        // for each color component. However, this is only important when using perspective
        // transformations, and even then using a single factor seems like a reasonable
        // trade-off between quality and speed.
        fragBuilder->codeAppend("half afwidth;");
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
            fragBuilder->codeAppend("half2 dist_grad = half2(half(dFdx(distance.r)), "
                                                            "half(dFdy(distance.r)));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("half dg_len2 = dot(dist_grad, dist_grad);");
            fragBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fragBuilder->codeAppend("dist_grad = half2(0.7071, 0.7071);");
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppend("dist_grad = dist_grad*half(inversesqrt(dg_len2));");
            fragBuilder->codeAppend("}");
            fragBuilder->codeAppend("half2 grad = half2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fragBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }

        // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
        // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want distance
        // mapped linearly to coverage, so use a linear step:
        if (isGammaCorrect) {
            fragBuilder->codeAppendf("%s = "
                "half4(saturate((distance + half3(afwidth)) / half3(2.0 * afwidth)), 1.0);",
                args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf(
                "%s = half4(smoothstep(half3(-afwidth), half3(afwidth), distance), 1.0);",
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

        const SkISize& atlasSize = dflcd.atlasSize();
        SkASSERT(SkIsPow2(atlasSize.fWidth) && SkIsPow2(atlasSize.fHeight));
        if (fAtlasSize != atlasSize) {
            pdman.set2f(fAtlasSizeInvUniform, 1.0f / atlasSize.fWidth, 1.0f / atlasSize.fHeight);
            fAtlasSize = atlasSize;
        }
        this->setTransformDataHelper(dflcd.localMatrix(), pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldLCDTextGeoProc& dfTexEffect = gp.cast<GrDistanceFieldLCDTextGeoProc>();

        uint32_t key = dfTexEffect.getFlags();
        b->add32(key);
        b->add32(dfTexEffect.numTextureSamplers());
    }

private:
    GrDistanceFieldLCDTextGeoProc::DistanceAdjust fDistanceAdjust;
    UniformHandle                                 fDistanceAdjustUni;

    SkISize                                       fAtlasSize;
    UniformHandle                                 fAtlasSizeInvUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldLCDTextGeoProc::GrDistanceFieldLCDTextGeoProc(const GrShaderCaps& caps,
                                                             const sk_sp<GrTextureProxy>* proxies,
                                                             int numProxies,
                                                             const GrSamplerState& params,
                                                             DistanceAdjust distanceAdjust,
                                                             uint32_t flags,
                                                             const SkMatrix& localMatrix)
        : INHERITED(kGrDistanceFieldLCDTextGeoProc_ClassID)
        , fLocalMatrix(localMatrix)
        , fDistanceAdjust(distanceAdjust)
        , fFlags(flags & kLCD_DistanceFieldEffectMask) {
    SkASSERT(numProxies <= kMaxTextures);
    SkASSERT(!(flags & ~kLCD_DistanceFieldEffectMask) && (flags & kUseLCD_DistanceFieldEffectFlag));

    if (fFlags & kPerspective_DistanceFieldEffectFlag) {
        fInPosition = {"inPosition", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
    } else {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    }
    fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.integerSupport() ? kUShort2_GrSLType : kFloat2_GrSLType};
    this->setVertexAttributes(&fInPosition, 3);

    if (numProxies) {
        fAtlasSize = proxies[0]->isize();
    }

    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        SkASSERT(proxies[i]->isize() == fAtlasSize);
        fTextureSamplers[i].reset(proxies[i]->textureType(), proxies[i]->config(), params);
    }
    this->setTextureSamplerCnt(numProxies);
}

void GrDistanceFieldLCDTextGeoProc::addNewProxies(const sk_sp<GrTextureProxy>* proxies,
                                                  int numProxies,
                                                  const GrSamplerState& params) {
    SkASSERT(numProxies <= kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasSize = proxies[0]->isize();
    }

    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        SkASSERT(proxies[i]->isize() == fAtlasSize);

        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(proxies[i]->textureType(), proxies[i]->config(), params);
        }
    }
    this->setTextureSamplerCnt(numProxies);
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
    sk_sp<GrTextureProxy> proxies[kMaxTextures] = {
        d->textureProxy(texIdx),
        nullptr,
        nullptr,
        nullptr
    };

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kBilerp
                                                   : GrSamplerState::Filter::kNearest);
    DistanceAdjust wa = { 0.0f, 0.1f, -0.1f };
    uint32_t flags = kUseLCD_DistanceFieldEffectFlag;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }
    flags |= d->fRandom->nextBool() ? kBGR_DistanceFieldEffectFlag : 0;
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
    return GrDistanceFieldLCDTextGeoProc::Make(*d->caps()->shaderCaps(), proxies, 1, samplerState,
                                               wa, flags, localMatrix);
}
#endif
