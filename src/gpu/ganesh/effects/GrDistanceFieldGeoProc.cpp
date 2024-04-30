/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrDistanceFieldGeoProc.h"

#include "include/core/SkSamplingOptions.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#include "src/gpu/ganesh/effects/GrAtlasedShaderHelpers.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"

#include <algorithm>

#if !defined(SK_DISABLE_SDF_TEXT)

// Assuming a radius of a little less than the diagonal of the fragment
#define SK_DistanceFieldAAFactor     "0.65"

class GrDistanceFieldA8TextGeoProc::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps& shaderCaps,
                 const GrGeometryProcessor& geomProc) override {
        const GrDistanceFieldA8TextGeoProc& dfa8gp = geomProc.cast<GrDistanceFieldA8TextGeoProc>();

#ifdef SK_GAMMA_APPLY_TO_A8
        float distanceAdjust = dfa8gp.fDistanceAdjust;
        if (distanceAdjust != fDistanceAdjust) {
            fDistanceAdjust = distanceAdjust;
            pdman.set1f(fDistanceAdjustUni, distanceAdjust);
        }
#endif

        const SkISize& atlasDimensions = dfa8gp.fAtlasDimensions;
        SkASSERT(SkIsPow2(atlasDimensions.fWidth) && SkIsPow2(atlasDimensions.fHeight));

        if (fAtlasDimensions != atlasDimensions) {
            pdman.set2f(fAtlasDimensionsInvUniform,
                        1.0f / atlasDimensions.fWidth,
                        1.0f / atlasDimensions.fHeight);
            fAtlasDimensions = atlasDimensions;
        }
        SetTransform(pdman, shaderCaps, fLocalMatrixUniform, dfa8gp.fLocalMatrix, &fLocalMatrix);
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldA8TextGeoProc& dfTexEffect =
                args.fGeomProc.cast<GrDistanceFieldA8TextGeoProc>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfTexEffect);

        const char* atlasDimensionsInvName;
        fAtlasDimensionsInvUniform = uniformHandler->addUniform(nullptr,
                                                                kVertex_GrShaderFlag,
                                                                SkSLType::kFloat2,
                                                                "AtlasDimensionsInv",
                                                                &atlasDimensionsInvName);
#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* distanceAdjustUniName = nullptr;
        // width, height, 1/(3*width)
        fDistanceAdjustUni = uniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                        SkSLType::kHalf, "DistanceAdjust",
                                                        &distanceAdjustUniName);
#endif

        // Setup pass through color
        fragBuilder->codeAppendf("half4 %s;\n", args.fOutputColor);
        varyingHandler->addPassThroughAttribute(dfTexEffect.fInColor.asShaderVar(),
                                                args.fOutputColor);

        // Setup position
        gpArgs->fPositionVar = dfTexEffect.fInPosition.asShaderVar();
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        gpArgs->fPositionVar,
                        dfTexEffect.fLocalMatrix,
                        &fLocalMatrixUniform);

        // add varyings
        GrGLSLVarying uv, texIdx, st;
        append_index_uv_varyings(args,
                                 dfTexEffect.numTextureSamplers(),
                                 dfTexEffect.fInTextureCoords.name(),
                                 atlasDimensionsInvName,
                                 &uv,
                                 &texIdx,
                                 &st);

        bool isUniformScale = (dfTexEffect.fFlags & kUniformScale_DistanceFieldEffectMask) ==
                                                    kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity   = SkToBool(dfTexEffect.fFlags & kSimilarity_DistanceFieldEffectFlag  );
        bool isGammaCorrect = SkToBool(dfTexEffect.fFlags & kGammaCorrect_DistanceFieldEffectFlag);
        bool isAliased      = SkToBool(dfTexEffect.fFlags & kAliased_DistanceFieldEffectFlag     );

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
            if (args.fShaderCaps->fAvoidDfDxForGradientsWhenPossible) {
                fragBuilder->codeAppendf(
                        "afwidth = abs(" SK_DistanceFieldAAFactor "*half(dFdy(%s.y)));", st.fsIn());
            } else {
                fragBuilder->codeAppendf(
                        "afwidth = abs(" SK_DistanceFieldAAFactor "*half(dFdx(%s.x)));", st.fsIn());
            }
        } else if (isSimilarity) {
            // For similarity transform, we adjust the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.
            // We use the y gradient because there is a bug in the Mali 400 in the x direction.

            // this gives us a smooth step across approximately one fragment
            if (args.fShaderCaps->fAvoidDfDxForGradientsWhenPossible) {
                fragBuilder->codeAppendf("half st_grad_len = length(half2(dFdy(%s)));", st.fsIn());
            } else {
                fragBuilder->codeAppendf("half st_grad_len = length(half2(dFdx(%s)));", st.fsIn());
            }
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*st_grad_len);");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("half2 dist_grad = half2(dFdx(distance), dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("half dg_len2 = dot(dist_grad, dist_grad);"
                                    "if (dg_len2 < 0.0001) {"
                                        "dist_grad = half2(0.7071, 0.7071);"
                                    "} else {"
                                        "dist_grad = dist_grad*half(inversesqrt(dg_len2));"
                                    "}");

            fragBuilder->codeAppendf("half4 jacobian = half4(dFdx(%s), dFdy(%s));",
                                     st.fsIn(), st.fsIn());
            fragBuilder->codeAppend("half2 grad = half2(dot(dist_grad, jacobian.xz),"
                                                       "dot(dist_grad, jacobian.yw));");

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

        fragBuilder->codeAppendf("half4 %s = half4(val);", args.fOutputCoverage);
    }

private:
#ifdef SK_GAMMA_APPLY_TO_A8
    float    fDistanceAdjust  = -1.f;
#endif
    SkISize  fAtlasDimensions = {-1, -1};
    SkMatrix fLocalMatrix     = SkMatrix::InvalidMatrix();

    UniformHandle fDistanceAdjustUni;
    UniformHandle fAtlasDimensionsInvUniform;
    UniformHandle fLocalMatrixUniform;

    using INHERITED = ProgramImpl;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldA8TextGeoProc::GrDistanceFieldA8TextGeoProc(const GrShaderCaps& caps,
                                                           const GrSurfaceProxyView* views,
                                                           int numViews,
                                                           GrSamplerState params,
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
    SkASSERT(numViews <= kMaxTextures);
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));

    if (flags & kPerspective_DistanceFieldEffectFlag) {
        fInPosition = {"inPosition", kFloat3_GrVertexAttribType, SkSLType::kFloat3};
    } else {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    }
    fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, SkSLType::kHalf4 };
    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.fIntegerSupport ? SkSLType::kUShort2 : SkSLType::kFloat2};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);

    if (numViews) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }
    for (int i = 0; i < numViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
    }
    this->setTextureSamplerCnt(numViews);
}

void GrDistanceFieldA8TextGeoProc::addNewViews(const GrSurfaceProxyView* views,
                                               int numViews,
                                               GrSamplerState params) {
    SkASSERT(numViews <= kMaxTextures);
    // Just to make sure we don't try to add too many proxies
    numViews = std::min(numViews, kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }

    for (int i = 0; i < numViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
        }
    }
    this->setTextureSamplerCnt(numViews);
}

void GrDistanceFieldA8TextGeoProc::addToKey(const GrShaderCaps& caps,
                                            skgpu::KeyBuilder* b) const {
    uint32_t key = 0;
    key |= fFlags;
    key |= ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix) << 16;
    b->add32(key);
    b->add32(this->numTextureSamplers());
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrDistanceFieldA8TextGeoProc::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldA8TextGeoProc)

#if defined(GR_TEST_UTILS)
GrGeometryProcessor* GrDistanceFieldA8TextGeoProc::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomAlphaOnlyView();

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kLinear
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
    return GrDistanceFieldA8TextGeoProc::Make(d->allocator(), *d->caps()->shaderCaps(),
                                              &view, 1,
                                              samplerState,
#ifdef SK_GAMMA_APPLY_TO_A8
                                              lum,
#endif
                                              flags, localMatrix);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class GrDistanceFieldPathGeoProc::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps& shaderCaps,
                 const GrGeometryProcessor& geomProc) override {
        const GrDistanceFieldPathGeoProc& dfpgp = geomProc.cast<GrDistanceFieldPathGeoProc>();

        // We always set the matrix uniform. It's used to transform from from device to local
        // for the local coord variable.
        SetTransform(pdman, shaderCaps, fLocalMatrixUniform, dfpgp.fLocalMatrix, &fLocalMatrix);

        const SkISize& atlasDimensions = dfpgp.fAtlasDimensions;
        SkASSERT(SkIsPow2(atlasDimensions.fWidth) && SkIsPow2(atlasDimensions.fHeight));
        if (fAtlasDimensions != atlasDimensions) {
            pdman.set2f(fAtlasDimensionsInvUniform,
                        1.0f / atlasDimensions.fWidth,
                        1.0f / atlasDimensions.fHeight);
            fAtlasDimensions = atlasDimensions;
        }
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldPathGeoProc& dfPathEffect =
                args.fGeomProc.cast<GrDistanceFieldPathGeoProc>();

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfPathEffect);

        const char* atlasDimensionsInvName;
        fAtlasDimensionsInvUniform = uniformHandler->addUniform(nullptr,
                                                                kVertex_GrShaderFlag,
                                                                SkSLType::kFloat2,
                                                                "AtlasDimensionsInv",
                                                                &atlasDimensionsInvName);

        GrGLSLVarying uv, texIdx, st;
        append_index_uv_varyings(args,
                                 dfPathEffect.numTextureSamplers(),
                                 dfPathEffect.fInTextureCoords.name(),
                                 atlasDimensionsInvName,
                                 &uv,
                                 &texIdx,
                                 &st);

        // setup pass through color
        fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        varyingHandler->addPassThroughAttribute(dfPathEffect.fInColor.asShaderVar(),
                                                args.fOutputColor);

        // Setup position (output position is pass through, local coords are transformed)
        gpArgs->fPositionVar = dfPathEffect.fInPosition.asShaderVar();
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        gpArgs->fPositionVar,
                        dfPathEffect.fLocalMatrix,
                        &fLocalMatrixUniform);

        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("float2 uv = %s;", uv.fsIn());
        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, dfPathEffect.numTextureSamplers(), texIdx, "uv",
                                   "texColor");

        fragBuilder->codeAppend("half distance = "
            SK_DistanceFieldMultiplier "*(texColor.r - " SK_DistanceFieldThreshold ");");

        fragBuilder->codeAppend("half afwidth;");
        bool isUniformScale = (dfPathEffect.fFlags & kUniformScale_DistanceFieldEffectMask) ==
                                                     kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity   = SkToBool(dfPathEffect.fFlags & kSimilarity_DistanceFieldEffectFlag  );
        bool isGammaCorrect = SkToBool(dfPathEffect.fFlags & kGammaCorrect_DistanceFieldEffectFlag);
        if (isUniformScale) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the t coordinate in the y direction.
            // We use st coordinates to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
            if (args.fShaderCaps->fAvoidDfDxForGradientsWhenPossible) {
                fragBuilder->codeAppendf(
                        "afwidth = abs(" SK_DistanceFieldAAFactor "*half(dFdy(%s.y)));", st.fsIn());
            } else {
                fragBuilder->codeAppendf(
                        "afwidth = abs(" SK_DistanceFieldAAFactor "*half(dFdx(%s.x)));", st.fsIn());
            }
        } else if (isSimilarity) {
            // For similarity transform, we adjust the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
            if (args.fShaderCaps->fAvoidDfDxForGradientsWhenPossible) {
                fragBuilder->codeAppendf("half st_grad_len = half(length(dFdy(%s)));", st.fsIn());
            } else {
                fragBuilder->codeAppendf("half st_grad_len = half(length(dFdx(%s)));", st.fsIn());
            }
            fragBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*st_grad_len);");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fragBuilder->codeAppend("half2 dist_grad = half2(dFdx(distance), "
                                                            "dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("half dg_len2 = dot(dist_grad, dist_grad);"
                                    "if (dg_len2 < 0.0001) {"
                                        "dist_grad = half2(0.7071, 0.7071);"
                                    "} else {"
                                        "dist_grad = dist_grad*half(inversesqrt(dg_len2));"
                                    "}");

            fragBuilder->codeAppendf("half4 jacobian = half4(dFdx(%s), dFdy(%s));",
                                     st.fsIn(), st.fsIn());
            fragBuilder->codeAppend("half2 grad = half2(dot(dist_grad, jacobian.xz),"
                                                       "dot(dist_grad, jacobian.yw));");

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

        fragBuilder->codeAppendf("half4 %s = half4(val);", args.fOutputCoverage);
    }

    SkMatrix      fLocalMatrix;
    UniformHandle fLocalMatrixUniform;

    SkISize       fAtlasDimensions;
    UniformHandle fAtlasDimensionsInvUniform;

    using INHERITED = ProgramImpl;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldPathGeoProc::GrDistanceFieldPathGeoProc(const GrShaderCaps& caps,
                                                       const GrSurfaceProxyView* views,
                                                       int numViews,
                                                       GrSamplerState params,
                                                       const SkMatrix& localMatrix,
                                                       uint32_t flags)
        : INHERITED(kGrDistanceFieldPathGeoProc_ClassID)
        , fLocalMatrix(localMatrix)
        , fFlags(flags & kPath_DistanceFieldEffectMask) {
    SkASSERT(numViews <= kMaxTextures);
    SkASSERT(!(flags & ~kPath_DistanceFieldEffectMask));

    fInPosition = {"inPosition", kFloat3_GrVertexAttribType, SkSLType::kFloat3};
    fInColor = MakeColorAttribute("inColor", SkToBool(flags & kWideColor_DistanceFieldEffectFlag));
    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.fIntegerSupport ? SkSLType::kUShort2 : SkSLType::kFloat2};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);

    if (numViews) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }

    for (int i = 0; i < numViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
    }
    this->setTextureSamplerCnt(numViews);
}

void GrDistanceFieldPathGeoProc::addNewViews(const GrSurfaceProxyView* views,
                                             int numViews,
                                             GrSamplerState params) {
    SkASSERT(numViews <= kMaxTextures);
    // Just to make sure we don't try to add too many proxies
    numViews = std::min(numViews, kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }

    for (int i = 0; i < numViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
        }
    }
    this->setTextureSamplerCnt(numViews);
}

void GrDistanceFieldPathGeoProc::addToKey(const GrShaderCaps& caps,
                                          skgpu::KeyBuilder* b) const {
    uint32_t key = fFlags;
    key |= ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix) << 16;
    key |= fLocalMatrix.hasPerspective() << (16 + ProgramImpl::kMatrixKeyBits);
    b->add32(key);
    b->add32(this->numTextureSamplers());
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrDistanceFieldPathGeoProc::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldPathGeoProc)

#if defined(GR_TEST_UTILS)
GrGeometryProcessor* GrDistanceFieldPathGeoProc::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomAlphaOnlyView();

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kLinear
                                                   : GrSamplerState::Filter::kNearest);

    uint32_t flags = 0;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }
    flags |= d->fRandom->nextBool() ? kWideColor_DistanceFieldEffectFlag : 0;
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
    return GrDistanceFieldPathGeoProc::Make(d->allocator(), *d->caps()->shaderCaps(),
                                            &view, 1,
                                            samplerState,
                                            localMatrix,
                                            flags);
}
#endif


///////////////////////////////////////////////////////////////////////////////

class GrDistanceFieldLCDTextGeoProc::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps& shaderCaps,
                 const GrGeometryProcessor& geomProc) override {
        SkASSERT(fDistanceAdjustUni.isValid());

        const GrDistanceFieldLCDTextGeoProc& dflcd = geomProc.cast<GrDistanceFieldLCDTextGeoProc>();
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust wa = dflcd.fDistanceAdjust;
        if (wa != fDistanceAdjust) {
            pdman.set3f(fDistanceAdjustUni, wa.fR, wa.fG, wa.fB);
            fDistanceAdjust = wa;
        }

        const SkISize& atlasDimensions = dflcd.fAtlasDimensions;
        SkASSERT(SkIsPow2(atlasDimensions.fWidth) && SkIsPow2(atlasDimensions.fHeight));
        if (fAtlasDimensions != atlasDimensions) {
            pdman.set2f(fAtlasDimensionsInvUniform,
                        1.0f / atlasDimensions.fWidth,
                        1.0f / atlasDimensions.fHeight);
            fAtlasDimensions = atlasDimensions;
        }
        SetTransform(pdman, shaderCaps, fLocalMatrixUniform, dflcd.fLocalMatrix, &fLocalMatrix);
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrDistanceFieldLCDTextGeoProc& dfTexEffect =
                args.fGeomProc.cast<GrDistanceFieldLCDTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(dfTexEffect);

        const char* atlasDimensionsInvName;
        fAtlasDimensionsInvUniform = uniformHandler->addUniform(nullptr,
                                                                kVertex_GrShaderFlag,
                                                                SkSLType::kFloat2,
                                                                "AtlasDimensionsInv",
                                                                &atlasDimensionsInvName);

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // setup pass through color
        fragBuilder->codeAppendf("half4 %s;\n", args.fOutputColor);
        varyingHandler->addPassThroughAttribute(dfTexEffect.fInColor.asShaderVar(),
                                                args.fOutputColor);

        // Setup position
        gpArgs->fPositionVar = dfTexEffect.fInPosition.asShaderVar();
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        dfTexEffect.fInPosition.asShaderVar(),
                        dfTexEffect.fLocalMatrix,
                        &fLocalMatrixUniform);

        // set up varyings
        GrGLSLVarying uv, texIdx, st;
        append_index_uv_varyings(args,
                                 dfTexEffect.numTextureSamplers(),
                                 dfTexEffect.fInTextureCoords.name(),
                                 atlasDimensionsInvName,
                                 &uv,
                                 &texIdx,
                                 &st);

        GrGLSLVarying delta(SkSLType::kFloat);
        varyingHandler->addVarying("Delta", &delta);
        if (dfTexEffect.fFlags & kBGR_DistanceFieldEffectFlag) {
            vertBuilder->codeAppendf("%s = -%s.x/3.0;", delta.vsOut(), atlasDimensionsInvName);
        } else {
            vertBuilder->codeAppendf("%s = %s.x/3.0;", delta.vsOut(), atlasDimensionsInvName);
        }

        // add frag shader code
        bool isUniformScale = (dfTexEffect.fFlags & kUniformScale_DistanceFieldEffectMask) ==
                                                    kUniformScale_DistanceFieldEffectMask;
        bool isSimilarity   = SkToBool(dfTexEffect.fFlags & kSimilarity_DistanceFieldEffectFlag  );
        bool isGammaCorrect = SkToBool(dfTexEffect.fFlags & kGammaCorrect_DistanceFieldEffectFlag);

        // create LCD offset adjusted by inverse of transform
        // Use highp to work around aliasing issues
        fragBuilder->codeAppendf("float2 uv = %s;\n", uv.fsIn());

        if (isUniformScale) {
            if (args.fShaderCaps->fAvoidDfDxForGradientsWhenPossible) {
                fragBuilder->codeAppendf("half st_grad_len = half(abs(dFdy(%s.y)));", st.fsIn());
            } else {
                fragBuilder->codeAppendf("half st_grad_len = half(abs(dFdx(%s.x)));", st.fsIn());
            }
            fragBuilder->codeAppendf("half2 offset = half2(half(st_grad_len*%s), 0.0);",
                                     delta.fsIn());
        } else if (isSimilarity) {
            // For a similarity matrix with rotation, the gradient will not be aligned
            // with the texel coordinate axes, so we need to calculate it.
            if (args.fShaderCaps->fAvoidDfDxForGradientsWhenPossible) {
                // We use dFdy instead and rotate -90 degrees to get the gradient in the x
                // direction.
                fragBuilder->codeAppendf("half2 st_grad = half2(dFdy(%s));", st.fsIn());
                fragBuilder->codeAppendf("half2 offset = half2(%s*float2(st_grad.y, -st_grad.x));",
                                         delta.fsIn());
            } else {
                fragBuilder->codeAppendf("half2 st_grad = half2(dFdx(%s));", st.fsIn());
                fragBuilder->codeAppendf("half2 offset = half(%s)*st_grad;", delta.fsIn());
            }
            fragBuilder->codeAppend("half st_grad_len = length(st_grad);");
        } else {
            fragBuilder->codeAppendf("half2 st = half2(%s);\n", st.fsIn());

            fragBuilder->codeAppend("half4 jacobian = half4(dFdx(st), dFdy(st));");
            fragBuilder->codeAppendf("half2 offset = half2(%s)*jacobian.xy;", delta.fsIn());
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
        fDistanceAdjustUni = uniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                        SkSLType::kHalf3, "DistanceAdjust",
                                                        &distanceAdjustUniName);
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
            fragBuilder->codeAppend("half2 dist_grad = half2(dFdx(distance.r), dFdy(distance.r));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fragBuilder->codeAppend("half dg_len2 = dot(dist_grad, dist_grad);"
                                    "if (dg_len2 < 0.0001) {"
                                        "dist_grad = half2(0.7071, 0.7071);"
                                    "} else {"
                                        "dist_grad = dist_grad*half(inversesqrt(dg_len2));"
                                    "}"
                                    "half2 grad = half2(dot(dist_grad, jacobian.xz),"
                                                       "dot(dist_grad, jacobian.yw));");

            // this gives us a smooth step across approximately one fragment
            fragBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }

        // The smoothstep falloff compensates for the non-linear sRGB response curve. If we are
        // doing gamma-correct rendering (to an sRGB or F16 buffer), then we actually want distance
        // mapped linearly to coverage, so use a linear step:
        if (isGammaCorrect) {
            fragBuilder->codeAppendf("half4 %s = "
                    "half4(saturate((distance + half3(afwidth)) / half3(2.0 * afwidth)), 1.0);",
                    args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf(
                    "half4 %s = half4(smoothstep(half3(-afwidth), half3(afwidth), distance), 1.0);",
                    args.fOutputCoverage);
        }
    }

private:
    DistanceAdjust fDistanceAdjust  = DistanceAdjust::Make(1.0f, 1.0f, 1.0f);
    SkISize        fAtlasDimensions = {-1, -1};
    SkMatrix       fLocalMatrix     = SkMatrix::InvalidMatrix();

    UniformHandle fDistanceAdjustUni;
    UniformHandle fAtlasDimensionsInvUniform;
    UniformHandle fLocalMatrixUniform;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldLCDTextGeoProc::GrDistanceFieldLCDTextGeoProc(const GrShaderCaps& caps,
                                                             const GrSurfaceProxyView* views,
                                                             int numViews,
                                                             GrSamplerState params,
                                                             DistanceAdjust distanceAdjust,
                                                             uint32_t flags,
                                                             const SkMatrix& localMatrix)
        : INHERITED(kGrDistanceFieldLCDTextGeoProc_ClassID)
        , fLocalMatrix(localMatrix)
        , fDistanceAdjust(distanceAdjust)
        , fFlags(flags & kLCD_DistanceFieldEffectMask) {
    SkASSERT(numViews <= kMaxTextures);
    SkASSERT(!(flags & ~kLCD_DistanceFieldEffectMask) && (flags & kUseLCD_DistanceFieldEffectFlag));

    if (fFlags & kPerspective_DistanceFieldEffectFlag) {
        fInPosition = {"inPosition", kFloat3_GrVertexAttribType, SkSLType::kFloat3};
    } else {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    }
    fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, SkSLType::kHalf4};
    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.fIntegerSupport ? SkSLType::kUShort2 : SkSLType::kFloat2};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);

    if (numViews) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }

    for (int i = 0; i < numViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
    }
    this->setTextureSamplerCnt(numViews);
}

void GrDistanceFieldLCDTextGeoProc::addNewViews(const GrSurfaceProxyView* views,
                                                int numViews,
                                                GrSamplerState params) {
    SkASSERT(numViews <= kMaxTextures);
    // Just to make sure we don't try to add too many proxies
    numViews = std::min(numViews, kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }

    for (int i = 0; i < numViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
        }
    }
    this->setTextureSamplerCnt(numViews);
}

void GrDistanceFieldLCDTextGeoProc::addToKey(const GrShaderCaps& caps,
                                             skgpu::KeyBuilder* b) const {
    uint32_t key = 0;
    key |= ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix);
    key |= fFlags << 16;
    b->add32(key);
    b->add32(this->numTextureSamplers());
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrDistanceFieldLCDTextGeoProc::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldLCDTextGeoProc)

#if defined(GR_TEST_UTILS)
GrGeometryProcessor* GrDistanceFieldLCDTextGeoProc::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kLinear
                                                   : GrSamplerState::Filter::kNearest);
    DistanceAdjust wa = { 0.0f, 0.1f, -0.1f };
    uint32_t flags = kUseLCD_DistanceFieldEffectFlag;
    flags |= d->fRandom->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0;
    if (flags & kSimilarity_DistanceFieldEffectFlag) {
        flags |= d->fRandom->nextBool() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    }
    flags |= d->fRandom->nextBool() ? kBGR_DistanceFieldEffectFlag : 0;
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);

    return GrDistanceFieldLCDTextGeoProc::Make(d->allocator(), *d->caps()->shaderCaps(), &view,
                                               1, samplerState, wa, flags, localMatrix);
}
#endif

#endif // !defined(SK_DISABLE_SDF_TEXT)
