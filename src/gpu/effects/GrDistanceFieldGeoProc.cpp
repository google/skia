/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldGeoProc.h"
#include "GrFontAtlasSizes.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"

#include "SkDistanceFieldGen.h"

#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

// Assuming a radius of a little less than the diagonal of the fragment
#define SK_DistanceFieldAAFactor     "0.65"

struct DistanceFieldBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLDistanceFieldA8TextGeoProc : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldA8TextGeoProc(const GrGeometryProcessor&,
                                   const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL)
#ifdef SK_GAMMA_APPLY_TO_A8
        , fDistanceAdjust(-1.0f)
#endif
        {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldA8TextGeoProc& dfTexEffect =
                args.fGP.cast<GrDistanceFieldA8TextGeoProc>();
        const DistanceFieldBatchTracker& local = args.fBT.cast<DistanceFieldBatchTracker>();
        GrGLGPBuilder* pb = args.fPB;
        GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        SkAssertResult(fsBuilder->enableFeature(
                GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

        // emit attributes
        vsBuilder->emitAttributes(dfTexEffect);

#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* distanceAdjustUniName = NULL;
        // width, height, 1/(3*width)
        fDistanceAdjustUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
            kFloat_GrSLType, kDefault_GrSLPrecision,
            "DistanceAdjust", &distanceAdjustUniName);
#endif

        // Setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor,
                                    dfTexEffect.inColor(), &fColorUniform);

        // Setup position
        this->setupPosition(pb, gpArgs, dfTexEffect.inPosition()->fName, dfTexEffect.viewMatrix());

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, dfTexEffect.inPosition()->fName,
                             args.fTransformsIn, args.fTransformsOut);

        // add varyings
        GrGLVertToFrag recipScale(kFloat_GrSLType);
        GrGLVertToFrag st(kVec2f_GrSLType);
        bool isSimilarity = SkToBool(dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag);
        args.fPB->addVarying("IntTextureCoords", &st, kHigh_GrSLPrecision);
        vsBuilder->codeAppendf("%s = %s;", st.vsOut(), dfTexEffect.inTextureCoords()->fName);

        GrGLVertToFrag uv(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &uv, kHigh_GrSLPrecision);
        // this is only used with text, so our texture bounds always match the glyph atlas
        vsBuilder->codeAppendf("%s = vec2(" GR_FONT_ATLAS_A8_RECIP_WIDTH ", "
                               GR_FONT_ATLAS_RECIP_HEIGHT ")*%s;", uv.vsOut(),
                               dfTexEffect.inTextureCoords()->fName);
        
        // Use highp to work around aliasing issues
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 uv = %s;\n", uv.fsIn());

        fsBuilder->codeAppend("\tfloat texColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[0],
                                       "uv",
                                       kVec2f_GrSLType);
        fsBuilder->codeAppend(".r;\n");
        fsBuilder->codeAppend("\tfloat distance = "
                       SK_DistanceFieldMultiplier "*(texColor - " SK_DistanceFieldThreshold ");");
#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust width based on gamma
        fsBuilder->codeAppendf("distance -= %s;", distanceAdjustUniName);
#endif

        fsBuilder->codeAppend("float afwidth;");
        if (isSimilarity) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
            // we use y to work around a Mali400 bug in the x direction
            fsBuilder->codeAppendf("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdy(%s.y));",
                                       st.fsIn());
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fsBuilder->codeAppend("vec2 dist_grad = vec2(dFdx(distance), dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fsBuilder->codeAppend("float dg_len2 = dot(dist_grad, dist_grad);");
            fsBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fsBuilder->codeAppend("dist_grad = vec2(0.7071, 0.7071);");
            fsBuilder->codeAppend("} else {");
            fsBuilder->codeAppend("dist_grad = dist_grad*inversesqrt(dg_len2);");
            fsBuilder->codeAppend("}");

            fsBuilder->codeAppendf("vec2 Jdx = dFdx(%s);", st.fsIn());
            fsBuilder->codeAppendf("vec2 Jdy = dFdy(%s);", st.fsIn());
            fsBuilder->codeAppend("vec2 grad = vec2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fsBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }
        fsBuilder->codeAppend("float val = smoothstep(-afwidth, afwidth, distance);");

        fsBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& proc,
                         const GrBatchTracker& bt) override {
#ifdef SK_GAMMA_APPLY_TO_A8
        const GrDistanceFieldA8TextGeoProc& dfTexEffect = proc.cast<GrDistanceFieldA8TextGeoProc>();
        float distanceAdjust = dfTexEffect.getDistanceAdjust();
        if (distanceAdjust != fDistanceAdjust) {
            pdman.set1f(fDistanceAdjustUni, distanceAdjust);
            fDistanceAdjust = distanceAdjust;
        }
#endif

        const GrDistanceFieldA8TextGeoProc& dfa8gp = proc.cast<GrDistanceFieldA8TextGeoProc>();
        this->setUniformViewMatrix(pdman, dfa8gp.viewMatrix());

        const DistanceFieldBatchTracker& local = bt.cast<DistanceFieldBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrBatchTracker& bt,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldA8TextGeoProc& dfTexEffect = gp.cast<GrDistanceFieldA8TextGeoProc>();
        const DistanceFieldBatchTracker& local = bt.cast<DistanceFieldBatchTracker>();
        uint32_t key = dfTexEffect.getFlags();
        key |= local.fInputColorType << 16;
        key |= ComputePosKey(dfTexEffect.viewMatrix()) << 25;
        b->add32(key);
    }

private:
    GrColor       fColor;
    UniformHandle fColorUniform;
#ifdef SK_GAMMA_APPLY_TO_A8
    float         fDistanceAdjust;
    UniformHandle fDistanceAdjustUni;
#endif

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldA8TextGeoProc::GrDistanceFieldA8TextGeoProc(GrColor color,
                                                           const SkMatrix& viewMatrix,
                                                           GrTexture* texture,
                                                           const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                           float distanceAdjust,
#endif
                                                           uint32_t flags)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fTextureAccess(texture, params)
#ifdef SK_GAMMA_APPLY_TO_A8
    , fDistanceAdjust(distanceAdjust)
#endif
    , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
    , fInColor(NULL) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->initClassID<GrDistanceFieldA8TextGeoProc>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    if (flags & kColorAttr_DistanceFieldEffectFlag) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                          kVec2s_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

void GrDistanceFieldA8TextGeoProc::getGLProcessorKey(const GrBatchTracker& bt,
                                                     const GrGLSLCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldA8TextGeoProc::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrDistanceFieldA8TextGeoProc::createGLInstance(const GrBatchTracker& bt,
                                               const GrGLSLCaps&) const {
    return SkNEW_ARGS(GrGLDistanceFieldA8TextGeoProc, (*this, bt));
}

void GrDistanceFieldA8TextGeoProc::initBatchTracker(GrBatchTracker* bt,
                                                    const GrPipelineInfo& init) const {
    DistanceFieldBatchTracker* local = bt->cast<DistanceFieldBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                               SkToBool(fInColor));
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldA8TextGeoProc);

GrGeometryProcessor* GrDistanceFieldA8TextGeoProc::TestCreate(SkRandom* random,
                                                              GrContext*,
                                                              const GrDrawTargetCaps&,
                                                              GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                           GrTextureParams::kNone_FilterMode);

    return GrDistanceFieldA8TextGeoProc::Create(GrRandomColor(random),
                                                GrTest::TestMatrix(random),
                                                textures[texIdx], params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                random->nextF(),
#endif
                                                random->nextBool() ?
                                                    kSimilarity_DistanceFieldEffectFlag : 0);
}

///////////////////////////////////////////////////////////////////////////////

struct DistanceFieldPathBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLDistanceFieldPathGeoProc : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldPathGeoProc(const GrGeometryProcessor&,
                                          const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL), fTextureSize(SkISize::Make(-1, -1)) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldPathGeoProc& dfTexEffect = args.fGP.cast<GrDistanceFieldPathGeoProc>();

        const DistanceFieldPathBatchTracker& local = args.fBT.cast<DistanceFieldPathBatchTracker>();
        GrGLGPBuilder* pb = args.fPB;
        GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        SkAssertResult(fsBuilder->enableFeature(
                                     GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

        // emit attributes
        vsBuilder->emitAttributes(dfTexEffect);

        GrGLVertToFrag v(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &v, kHigh_GrSLPrecision);

        // setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor,
                                    dfTexEffect.inColor(), &fColorUniform);

        vsBuilder->codeAppendf("%s = %s;", v.vsOut(), dfTexEffect.inTextureCoords()->fName);

        // Setup position
        this->setupPosition(pb, gpArgs, dfTexEffect.inPosition()->fName, dfTexEffect.viewMatrix());

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, dfTexEffect.inPosition()->fName,
                             args.fTransformsIn, args.fTransformsOut);

        const char* textureSizeUniName = NULL;
        fTextureSizeUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                              kVec2f_GrSLType, kDefault_GrSLPrecision,
                                              "TextureSize", &textureSizeUniName);

        // Use highp to work around aliasing issues
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 uv = %s;", v.fsIn());

        fsBuilder->codeAppend("float texColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[0],
                                       "uv",
                                       kVec2f_GrSLType);
        fsBuilder->codeAppend(".r;");
        fsBuilder->codeAppend("float distance = "
            SK_DistanceFieldMultiplier "*(texColor - " SK_DistanceFieldThreshold ");");

        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 st = uv*%s;", textureSizeUniName);
        fsBuilder->codeAppend("float afwidth;");
        if (dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdy(st.y));");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fsBuilder->codeAppend("vec2 dist_grad = vec2(dFdx(distance), dFdy(distance));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fsBuilder->codeAppend("float dg_len2 = dot(dist_grad, dist_grad);");
            fsBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fsBuilder->codeAppend("dist_grad = vec2(0.7071, 0.7071);");
            fsBuilder->codeAppend("} else {");
            fsBuilder->codeAppend("dist_grad = dist_grad*inversesqrt(dg_len2);");
            fsBuilder->codeAppend("}");

            fsBuilder->codeAppend("vec2 Jdx = dFdx(st);");
            fsBuilder->codeAppend("vec2 Jdy = dFdy(st);");
            fsBuilder->codeAppend("vec2 grad = vec2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fsBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }
        fsBuilder->codeAppend("float val = smoothstep(-afwidth, afwidth, distance);");

        fsBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& proc,
                         const GrBatchTracker& bt) override {
        SkASSERT(fTextureSizeUni.isValid());

        GrTexture* texture = proc.texture(0);
        if (texture->width() != fTextureSize.width() || 
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            pdman.set2f(fTextureSizeUni,
                        SkIntToScalar(fTextureSize.width()),
                        SkIntToScalar(fTextureSize.height()));
        }

        const GrDistanceFieldPathGeoProc& dfpgp = proc.cast<GrDistanceFieldPathGeoProc>();
        this->setUniformViewMatrix(pdman, dfpgp.viewMatrix());

        const DistanceFieldPathBatchTracker& local = bt.cast<DistanceFieldPathBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrBatchTracker& bt,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldPathGeoProc& dfTexEffect = gp.cast<GrDistanceFieldPathGeoProc>();

        const DistanceFieldPathBatchTracker& local = bt.cast<DistanceFieldPathBatchTracker>();
        uint32_t key = dfTexEffect.getFlags();
        key |= local.fInputColorType << 16;
        key |= ComputePosKey(dfTexEffect.viewMatrix()) << 25;
        b->add32(key);
    }

private:
    UniformHandle fColorUniform;
    UniformHandle fTextureSizeUni;
    GrColor       fColor;
    SkISize       fTextureSize;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldPathGeoProc::GrDistanceFieldPathGeoProc(
        GrColor color,
        const SkMatrix& viewMatrix,
        GrTexture* texture,
        const GrTextureParams& params,
        uint32_t flags)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fTextureAccess(texture, params)
    , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
    , fInColor(NULL) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->initClassID<GrDistanceFieldPathGeoProc>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    if (flags & kColorAttr_DistanceFieldEffectFlag) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                          kVec2f_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

void GrDistanceFieldPathGeoProc::getGLProcessorKey(const GrBatchTracker& bt,
                                                   const GrGLSLCaps& caps,
                                                   GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldPathGeoProc::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrDistanceFieldPathGeoProc::createGLInstance(const GrBatchTracker& bt, const GrGLSLCaps&) const {
    return SkNEW_ARGS(GrGLDistanceFieldPathGeoProc, (*this, bt));
}

void GrDistanceFieldPathGeoProc::initBatchTracker(GrBatchTracker* bt,
                                                  const GrPipelineInfo& init) const {
    DistanceFieldPathBatchTracker* local = bt->cast<DistanceFieldPathBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                               SkToBool(fInColor));
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldPathGeoProc);

GrGeometryProcessor* GrDistanceFieldPathGeoProc::TestCreate(SkRandom* random,
                                                            GrContext*,
                                                            const GrDrawTargetCaps&,
                                                            GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx 
                                    : GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode 
                                                         : GrTextureParams::kNone_FilterMode);

    return GrDistanceFieldPathGeoProc::Create(GrRandomColor(random),
                                              GrTest::TestMatrix(random),
                                              textures[texIdx],
                                              params,
        random->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0);
}

///////////////////////////////////////////////////////////////////////////////

struct DistanceFieldLCDBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLDistanceFieldLCDTextGeoProc : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldLCDTextGeoProc(const GrGeometryProcessor&, const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL) {
        fDistanceAdjust = GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(1.0f, 1.0f, 1.0f);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrDistanceFieldLCDTextGeoProc& dfTexEffect =
                args.fGP.cast<GrDistanceFieldLCDTextGeoProc>();
        const DistanceFieldLCDBatchTracker& local = args.fBT.cast<DistanceFieldLCDBatchTracker>();
        GrGLGPBuilder* pb = args.fPB;

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

        // emit attributes
        vsBuilder->emitAttributes(dfTexEffect);

        // setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL,
                                    &fColorUniform);

        // Setup position
        this->setupPosition(pb, gpArgs, dfTexEffect.inPosition()->fName, dfTexEffect.viewMatrix());

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, dfTexEffect.inPosition()->fName,
                             args.fTransformsIn, args.fTransformsOut);

        // set up varyings
        bool isUniformScale = SkToBool(dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask);
        GrGLVertToFrag recipScale(kFloat_GrSLType);
        GrGLVertToFrag st(kVec2f_GrSLType);
        args.fPB->addVarying("IntTextureCoords", &st, kHigh_GrSLPrecision);
        vsBuilder->codeAppendf("%s = %s;", st.vsOut(), dfTexEffect.inTextureCoords()->fName);

        GrGLVertToFrag uv(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &uv, kHigh_GrSLPrecision);
        // this is only used with text, so our texture bounds always match the glyph atlas
        vsBuilder->codeAppendf("%s = vec2(" GR_FONT_ATLAS_A8_RECIP_WIDTH ", "
                               GR_FONT_ATLAS_RECIP_HEIGHT ")*%s;", uv.vsOut(),
                               dfTexEffect.inTextureCoords()->fName);

        // add frag shader code
        GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

        SkAssertResult(fsBuilder->enableFeature(
                GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        // create LCD offset adjusted by inverse of transform
        // Use highp to work around aliasing issues
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 uv = %s;\n", uv.fsIn());
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        if (dfTexEffect.getFlags() & kBGR_DistanceFieldEffectFlag) {
            fsBuilder->codeAppend("float delta = -" GR_FONT_ATLAS_LCD_DELTA ";\n");
        } else {
            fsBuilder->codeAppend("float delta = " GR_FONT_ATLAS_LCD_DELTA ";\n");
        }
        if (isUniformScale) {
            fsBuilder->codeAppendf("float dy = abs(dFdy(%s.y));", st.fsIn());
            fsBuilder->codeAppend("vec2 offset = vec2(dy*delta, 0.0);");
        } else {
            fsBuilder->codeAppendf("vec2 st = %s;\n", st.fsIn());

            fsBuilder->codeAppend("vec2 Jdx = dFdx(st);");
            fsBuilder->codeAppend("vec2 Jdy = dFdy(st);");
            fsBuilder->codeAppend("vec2 offset = delta*Jdx;");
        }

        // green is distance to uv center
        fsBuilder->codeAppend("\tvec4 texColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[0], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tvec3 distance;\n");
        fsBuilder->codeAppend("\tdistance.y = texColor.r;\n");
        // red is distance to left offset
        fsBuilder->codeAppend("\tvec2 uv_adjusted = uv - offset;\n");
        fsBuilder->codeAppend("\ttexColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[0], "uv_adjusted", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tdistance.x = texColor.r;\n");
        // blue is distance to right offset
        fsBuilder->codeAppend("\tuv_adjusted = uv + offset;\n");
        fsBuilder->codeAppend("\ttexColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[0], "uv_adjusted", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tdistance.z = texColor.r;\n");

        fsBuilder->codeAppend("\tdistance = "
           "vec3(" SK_DistanceFieldMultiplier ")*(distance - vec3(" SK_DistanceFieldThreshold"));");

        // adjust width based on gamma
        const char* distanceAdjustUniName = NULL;
        fDistanceAdjustUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
            kVec3f_GrSLType, kDefault_GrSLPrecision,
            "DistanceAdjust", &distanceAdjustUniName);
        fsBuilder->codeAppendf("distance -= %s;", distanceAdjustUniName);

        // To be strictly correct, we should compute the anti-aliasing factor separately
        // for each color component. However, this is only important when using perspective
        // transformations, and even then using a single factor seems like a reasonable
        // trade-off between quality and speed.
        fsBuilder->codeAppend("float afwidth;");
        if (isUniformScale) {
            // For uniform scale, we adjust for the effect of the transformation on the distance
            // by using the length of the gradient of the texture coordinates. We use st coordinates
            // to ensure we're mapping 1:1 from texel space to pixel space.

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*dy;");
        } else {
            // For general transforms, to determine the amount of correction we multiply a unit
            // vector pointing along the SDF gradient direction by the Jacobian of the st coords
            // (which is the inverse transform for this fragment) and take the length of the result.
            fsBuilder->codeAppend("vec2 dist_grad = vec2(dFdx(distance.r), dFdy(distance.r));");
            // the length of the gradient may be 0, so we need to check for this
            // this also compensates for the Adreno, which likes to drop tiles on division by 0
            fsBuilder->codeAppend("float dg_len2 = dot(dist_grad, dist_grad);");
            fsBuilder->codeAppend("if (dg_len2 < 0.0001) {");
            fsBuilder->codeAppend("dist_grad = vec2(0.7071, 0.7071);");
            fsBuilder->codeAppend("} else {");
            fsBuilder->codeAppend("dist_grad = dist_grad*inversesqrt(dg_len2);");
            fsBuilder->codeAppend("}");
            fsBuilder->codeAppend("vec2 grad = vec2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,");
            fsBuilder->codeAppend("                 dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }

        fsBuilder->codeAppend(
                      "vec4 val = vec4(smoothstep(vec3(-afwidth), vec3(afwidth), distance), 1.0);");

        fsBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& processor,
                         const GrBatchTracker& bt) override {
        SkASSERT(fDistanceAdjustUni.isValid());

        const GrDistanceFieldLCDTextGeoProc& dfTexEffect =
                processor.cast<GrDistanceFieldLCDTextGeoProc>();
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust wa = dfTexEffect.getDistanceAdjust();
        if (wa != fDistanceAdjust) {
            pdman.set3f(fDistanceAdjustUni,
                        wa.fR,
                        wa.fG,
                        wa.fB);
            fDistanceAdjust = wa;
        }

        this->setUniformViewMatrix(pdman, dfTexEffect.viewMatrix());

        const DistanceFieldLCDBatchTracker& local = bt.cast<DistanceFieldLCDBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrBatchTracker& bt,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldLCDTextGeoProc& dfTexEffect = gp.cast<GrDistanceFieldLCDTextGeoProc>();

        const DistanceFieldLCDBatchTracker& local = bt.cast<DistanceFieldLCDBatchTracker>();
        uint32_t key = dfTexEffect.getFlags();
        key |= local.fInputColorType << 16;
        key |= ComputePosKey(dfTexEffect.viewMatrix()) << 25;
        b->add32(key);
    }

private:
    GrColor                                      fColor;
    UniformHandle                                fColorUniform;
    GrDistanceFieldLCDTextGeoProc::DistanceAdjust fDistanceAdjust;
    UniformHandle                                fDistanceAdjustUni;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldLCDTextGeoProc::GrDistanceFieldLCDTextGeoProc(
                                                  GrColor color, const SkMatrix& viewMatrix,
                                                  GrTexture* texture, const GrTextureParams& params,
                                                  DistanceAdjust distanceAdjust,
                                                  uint32_t flags)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fTextureAccess(texture, params)
    , fDistanceAdjust(distanceAdjust)
    , fFlags(flags & kLCD_DistanceFieldEffectMask){
    SkASSERT(!(flags & ~kLCD_DistanceFieldEffectMask) && (flags & kUseLCD_DistanceFieldEffectFlag));
    this->initClassID<GrDistanceFieldLCDTextGeoProc>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                          kVec2s_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

void GrDistanceFieldLCDTextGeoProc::getGLProcessorKey(const GrBatchTracker& bt,
                                                      const GrGLSLCaps& caps,
                                                      GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldLCDTextGeoProc::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrDistanceFieldLCDTextGeoProc::createGLInstance(const GrBatchTracker& bt,
                                                const GrGLSLCaps&) const {
    return SkNEW_ARGS(GrGLDistanceFieldLCDTextGeoProc, (*this, bt));
}

void GrDistanceFieldLCDTextGeoProc::initBatchTracker(GrBatchTracker* bt,
                                                     const GrPipelineInfo& init) const {
    DistanceFieldLCDBatchTracker* local = bt->cast<DistanceFieldLCDBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldLCDTextGeoProc);

GrGeometryProcessor* GrDistanceFieldLCDTextGeoProc::TestCreate(SkRandom* random,
                                                                 GrContext*,
                                                                 const GrDrawTargetCaps&,
                                                                 GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                           GrTextureParams::kNone_FilterMode);
    DistanceAdjust wa = { 0.0f, 0.1f, -0.1f };
    uint32_t flags = kUseLCD_DistanceFieldEffectFlag;
    flags |= random->nextBool() ? kUniformScale_DistanceFieldEffectMask : 0;
    flags |= random->nextBool() ? kBGR_DistanceFieldEffectFlag : 0;
    return GrDistanceFieldLCDTextGeoProc::Create(GrRandomColor(random),
                                                 GrTest::TestMatrix(random),
                                                 textures[texIdx], params,
                                                 wa,
                                                 flags);
}
