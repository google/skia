/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldTextureEffect.h"
#include "GrFontAtlasSizes.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"
#include "SkDistanceFieldGen.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

// Assuming a radius of the diagonal of the fragment, hence a factor of sqrt(2)/2
#define SK_DistanceFieldAAFactor     "0.7071"

struct DistanceFieldBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLDistanceFieldTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldTextureEffect(const GrGeometryProcessor&,
                                   const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL)
#ifdef SK_GAMMA_APPLY_TO_A8
        , fLuminance(-1.0f)
#endif
        {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
        const GrDistanceFieldTextureEffect& dfTexEffect =
                args.fGP.cast<GrDistanceFieldTextureEffect>();
        const DistanceFieldBatchTracker& local = args.fBT.cast<DistanceFieldBatchTracker>();
        GrGLGPBuilder* pb = args.fPB;
        GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        SkAssertResult(fsBuilder->enableFeature(
                GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

        // emit attributes
        vsBuilder->emitAttributes(dfTexEffect);

        GrGLVertToFrag st(kVec2f_GrSLType);
        args.fPB->addVarying("IntTextureCoords", &st, kHigh_GrSLPrecision);
        vsBuilder->codeAppendf("%s = %s;", st.vsOut(), dfTexEffect.inTextureCoords()->fName);
        
        GrGLVertToFrag uv(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &uv, kHigh_GrSLPrecision);
        // this is only used with text, so our texture bounds always match the glyph atlas
        vsBuilder->codeAppendf("%s = vec2(" GR_FONT_ATLAS_A8_RECIP_WIDTH ", "
                               GR_FONT_ATLAS_RECIP_HEIGHT ")*%s;", uv.vsOut(),
                               dfTexEffect.inTextureCoords()->fName);

        // Setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor,
                                    dfTexEffect.inColor(), &fColorUniform);

        // Setup position
        this->setupPosition(pb, gpArgs, dfTexEffect.inPosition()->fName, dfTexEffect.viewMatrix());

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, dfTexEffect.inPosition()->fName,
                             dfTexEffect.localMatrix(), args.fTransformsIn, args.fTransformsOut);

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

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 st = %s;\n", st.fsIn());
        fsBuilder->codeAppend("\tfloat afwidth;\n");
        if (dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag) {
            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = abs(" SK_DistanceFieldAAFactor "*dFdx(st.x));\n");
        } else {
            fsBuilder->codeAppend("\tvec2 Jdx = dFdx(st);\n");
            fsBuilder->codeAppend("\tvec2 Jdy = dFdy(st);\n");

            fsBuilder->codeAppend("\tvec2 uv_grad;\n");
            if (args.fPB->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                fsBuilder->codeAppend("\tfloat uv_len2 = dot(uv, uv);\n");
                fsBuilder->codeAppend("\tif (uv_len2 < 0.0001) {\n");
                fsBuilder->codeAppend("\t\tuv_grad = vec2(0.7071, 0.7071);\n");
                fsBuilder->codeAppend("\t} else {\n");
                fsBuilder->codeAppend("\t\tuv_grad = uv*inversesqrt(uv_len2);\n");
                fsBuilder->codeAppend("\t}\n");
            } else {
                fsBuilder->codeAppend("\tuv_grad = normalize(uv);\n");
            }
            fsBuilder->codeAppend("\tvec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,\n");
            fsBuilder->codeAppend("\t                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);\n");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*length(grad);\n");
        }
        fsBuilder->codeAppend("\tfloat val = smoothstep(-afwidth, afwidth, distance);\n");

#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* luminanceUniName = NULL;
        // width, height, 1/(3*width)
        fLuminanceUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                             kFloat_GrSLType, kDefault_GrSLPrecision,
                                             "Luminance", &luminanceUniName);

        fsBuilder->codeAppendf("\tuv = vec2(val, %s);\n", luminanceUniName);
        fsBuilder->codeAppend("\tvec4 gammaColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tval = gammaColor.r;\n");
#endif

        fsBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& proc,
                         const GrBatchTracker& bt) SK_OVERRIDE {
#ifdef SK_GAMMA_APPLY_TO_A8
        const GrDistanceFieldTextureEffect& dfTexEffect =
                proc.cast<GrDistanceFieldTextureEffect>();
        float luminance = dfTexEffect.getLuminance();
        if (luminance != fLuminance) {
            pdman.set1f(fLuminanceUni, luminance);
            fLuminance = luminance;
        }
#endif

        this->setUniformViewMatrix(pdman, proc.viewMatrix());

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
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldTextureEffect& dfTexEffect = gp.cast<GrDistanceFieldTextureEffect>();
        const DistanceFieldBatchTracker& local = bt.cast<DistanceFieldBatchTracker>();
        uint32_t key = dfTexEffect.getFlags();
        key |= local.fInputColorType << 16;
        key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 << 24: 0x0;
        key |= ComputePosKey(gp.viewMatrix()) << 25;
        b->add32(key);
    }

private:
    GrColor       fColor;
    UniformHandle fColorUniform;
#ifdef SK_GAMMA_APPLY_TO_A8
    UniformHandle fLuminanceUni;
    float         fLuminance;
#endif

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldTextureEffect::GrDistanceFieldTextureEffect(GrColor color,
                                                           const SkMatrix& viewMatrix,
                                                           GrTexture* texture,
                                                           const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                           GrTexture* gamma,
                                                           const GrTextureParams& gammaParams,
                                                           float luminance,
#endif
                                                           uint32_t flags, bool opaqueVertexColors)
    : INHERITED(color, viewMatrix, SkMatrix::I(), opaqueVertexColors)
    , fTextureAccess(texture, params)
#ifdef SK_GAMMA_APPLY_TO_A8
    , fGammaTextureAccess(gamma, gammaParams)
    , fLuminance(luminance)
#endif
    , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
    , fInColor(NULL) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->initClassID<GrDistanceFieldTextureEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    if (flags & kColorAttr_DistanceFieldEffectFlag) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        this->setHasVertexColor();
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                          kVec2s_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
#ifdef SK_GAMMA_APPLY_TO_A8
    this->addTextureAccess(&fGammaTextureAccess);
#endif
}

bool GrDistanceFieldTextureEffect::onIsEqual(const GrGeometryProcessor& other) const {
    const GrDistanceFieldTextureEffect& cte = other.cast<GrDistanceFieldTextureEffect>();
    return
#ifdef SK_GAMMA_APPLY_TO_A8
           fLuminance == cte.fLuminance &&
#endif
           fFlags == cte.fFlags;
}

void GrDistanceFieldTextureEffect::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setUnknownSingleComponent();
}

void GrDistanceFieldTextureEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                                     const GrGLCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldTextureEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrDistanceFieldTextureEffect::createGLInstance(const GrBatchTracker& bt,
                                               const GrGLCaps&) const {
    return SkNEW_ARGS(GrGLDistanceFieldTextureEffect, (*this, bt));
}

void GrDistanceFieldTextureEffect::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    DistanceFieldBatchTracker* local = bt->cast<DistanceFieldBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                               SkToBool(fInColor));
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrDistanceFieldTextureEffect::onCanMakeEqual(const GrBatchTracker& m,
                                                  const GrGeometryProcessor& that,
                                                  const GrBatchTracker& t) const {
    const DistanceFieldBatchTracker& mine = m.cast<DistanceFieldBatchTracker>();
    const DistanceFieldBatchTracker& theirs = t.cast<DistanceFieldBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldTextureEffect);

GrGeometryProcessor* GrDistanceFieldTextureEffect::TestCreate(SkRandom* random,
                                                              GrContext*,
                                                              const GrDrawTargetCaps&,
                                                              GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
#ifdef SK_GAMMA_APPLY_TO_A8
    int texIdx2 = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                       GrProcessorUnitTest::kAlphaTextureIdx;
#endif
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
#ifdef SK_GAMMA_APPLY_TO_A8
    GrTextureParams params2(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                            GrTextureParams::kNone_FilterMode);
#endif

    return GrDistanceFieldTextureEffect::Create(GrRandomColor(random),
                                                GrProcessorUnitTest::TestMatrix(random),
                                                textures[texIdx], params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                textures[texIdx2], params2,
                                                random->nextF(),
#endif
                                                random->nextBool() ?
                                                    kSimilarity_DistanceFieldEffectFlag : 0,
                                                random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

struct DistanceFieldNoGammaBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLDistanceFieldNoGammaTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldNoGammaTextureEffect(const GrGeometryProcessor&,
                                          const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL), fTextureSize(SkISize::Make(-1, -1)) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
        const GrDistanceFieldNoGammaTextureEffect& dfTexEffect =
                args.fGP.cast<GrDistanceFieldNoGammaTextureEffect>();

        const DistanceFieldNoGammaBatchTracker& local =
                args.fBT.cast<DistanceFieldNoGammaBatchTracker>();
        GrGLGPBuilder* pb = args.fPB;
        GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
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
                             dfTexEffect.localMatrix(), args.fTransformsIn, args.fTransformsOut);

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

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 st = uv*%s;", textureSizeUniName);
        fsBuilder->codeAppend("float afwidth;");
        if (dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag) {
            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = abs(" SK_DistanceFieldAAFactor "*dFdx(st.x));");
        } else {
            fsBuilder->codeAppend("vec2 Jdx = dFdx(st);");
            fsBuilder->codeAppend("vec2 Jdy = dFdy(st);");

            fsBuilder->codeAppend("vec2 uv_grad;");
            if (args.fPB->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                fsBuilder->codeAppend("float uv_len2 = dot(uv, uv);");
                fsBuilder->codeAppend("if (uv_len2 < 0.0001) {");
                fsBuilder->codeAppend("uv_grad = vec2(0.7071, 0.7071);");
                fsBuilder->codeAppend("} else {");
                fsBuilder->codeAppend("uv_grad = uv*inversesqrt(uv_len2);");
                fsBuilder->codeAppend("}");
            } else {
                fsBuilder->codeAppend("uv_grad = normalize(uv);");
            }
            fsBuilder->codeAppend("vec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,");
            fsBuilder->codeAppend("                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("afwidth = " SK_DistanceFieldAAFactor "*length(grad);");
        }
        fsBuilder->codeAppend("float val = smoothstep(-afwidth, afwidth, distance);");

        fsBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& proc,
                         const GrBatchTracker& bt) SK_OVERRIDE {
        SkASSERT(fTextureSizeUni.isValid());

        GrTexture* texture = proc.texture(0);
        if (texture->width() != fTextureSize.width() || 
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            pdman.set2f(fTextureSizeUni,
                        SkIntToScalar(fTextureSize.width()),
                        SkIntToScalar(fTextureSize.height()));
        }

        this->setUniformViewMatrix(pdman, proc.viewMatrix());

        const DistanceFieldNoGammaBatchTracker& local = bt.cast<DistanceFieldNoGammaBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrBatchTracker& bt,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldNoGammaTextureEffect& dfTexEffect =
            gp.cast<GrDistanceFieldNoGammaTextureEffect>();

        const DistanceFieldNoGammaBatchTracker& local = bt.cast<DistanceFieldNoGammaBatchTracker>();
        uint32_t key = dfTexEffect.getFlags();
        key |= local.fInputColorType << 16;
        key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 << 24: 0x0;
        key |= ComputePosKey(gp.viewMatrix()) << 25;
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

GrDistanceFieldNoGammaTextureEffect::GrDistanceFieldNoGammaTextureEffect(
        GrColor color,
        const SkMatrix& viewMatrix,
        GrTexture* texture,
        const GrTextureParams& params,
        uint32_t flags,
        bool opaqueVertexColors)
    : INHERITED(color, viewMatrix, SkMatrix::I(), opaqueVertexColors)
    , fTextureAccess(texture, params)
    , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
    , fInColor(NULL) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->initClassID<GrDistanceFieldNoGammaTextureEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    if (flags & kColorAttr_DistanceFieldEffectFlag) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        this->setHasVertexColor();
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                          kVec2f_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

bool GrDistanceFieldNoGammaTextureEffect::onIsEqual(const GrGeometryProcessor& other) const {
    const GrDistanceFieldNoGammaTextureEffect& cte = 
                                                 other.cast<GrDistanceFieldNoGammaTextureEffect>();
    return fFlags == cte.fFlags;
}

void GrDistanceFieldNoGammaTextureEffect::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const{
    out->setUnknownSingleComponent();
}

void GrDistanceFieldNoGammaTextureEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                                            const GrGLCaps& caps,
                                                            GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldNoGammaTextureEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrDistanceFieldNoGammaTextureEffect::createGLInstance(const GrBatchTracker& bt,
                                                      const GrGLCaps&) const {
    return SkNEW_ARGS(GrGLDistanceFieldNoGammaTextureEffect, (*this, bt));
}

void GrDistanceFieldNoGammaTextureEffect::initBatchTracker(GrBatchTracker* bt,
                                                           const GrPipelineInfo& init) const {
    DistanceFieldNoGammaBatchTracker* local = bt->cast<DistanceFieldNoGammaBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                               SkToBool(fInColor));
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrDistanceFieldNoGammaTextureEffect::onCanMakeEqual(const GrBatchTracker& m,
                                                         const GrGeometryProcessor& that,
                                                         const GrBatchTracker& t) const {
    const DistanceFieldNoGammaBatchTracker& mine = m.cast<DistanceFieldNoGammaBatchTracker>();
    const DistanceFieldNoGammaBatchTracker& theirs = t.cast<DistanceFieldNoGammaBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldNoGammaTextureEffect);

GrGeometryProcessor* GrDistanceFieldNoGammaTextureEffect::TestCreate(SkRandom* random,
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

    return GrDistanceFieldNoGammaTextureEffect::Create(GrRandomColor(random),
                                                       GrProcessorUnitTest::TestMatrix(random),
                                                       textures[texIdx],
                                                       params,
        random->nextBool() ? kSimilarity_DistanceFieldEffectFlag : 0, random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

struct DistanceFieldLCDBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLDistanceFieldLCDTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldLCDTextureEffect(const GrGeometryProcessor&,
                                      const GrBatchTracker&)
    : fColor(GrColor_ILLEGAL)
    , fTextColor(GrColor_ILLEGAL) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                args.fGP.cast<GrDistanceFieldLCDTextureEffect>();
        const DistanceFieldLCDBatchTracker& local = args.fBT.cast<DistanceFieldLCDBatchTracker>();
        GrGLGPBuilder* pb = args.fPB;

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

        // emit attributes
        vsBuilder->emitAttributes(dfTexEffect);

        GrGLVertToFrag st(kVec2f_GrSLType);
        args.fPB->addVarying("IntTextureCoords", &st, kHigh_GrSLPrecision);
        vsBuilder->codeAppendf("%s = %s;", st.vsOut(), dfTexEffect.inTextureCoords()->fName);
        
        GrGLVertToFrag uv(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &uv, kHigh_GrSLPrecision);
        // this is only used with text, so our texture bounds always match the glyph atlas
        vsBuilder->codeAppendf("%s = vec2(" GR_FONT_ATLAS_A8_RECIP_WIDTH ", "
                               GR_FONT_ATLAS_RECIP_HEIGHT ")*%s;", uv.vsOut(),
                               dfTexEffect.inTextureCoords()->fName);
        
        // setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL,
                                    &fColorUniform);

        // Setup position
        this->setupPosition(pb, gpArgs, dfTexEffect.inPosition()->fName, dfTexEffect.viewMatrix());

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, dfTexEffect.inPosition()->fName,
                             dfTexEffect.localMatrix(), args.fTransformsIn, args.fTransformsOut);

        GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

        SkAssertResult(fsBuilder->enableFeature(
                GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        // create LCD offset adjusted by inverse of transform
        // Use highp to work around aliasing issues
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 uv = %s;\n", uv.fsIn());
        fsBuilder->codeAppend(GrGLShaderVar::PrecisionString(kHigh_GrSLPrecision,
                                                             pb->ctxInfo().standard()));
        fsBuilder->codeAppendf("vec2 st = %s;\n", st.fsIn());
        bool isUniformScale = !!(dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask);
        
        if (dfTexEffect.getFlags() & kBGR_DistanceFieldEffectFlag) {
            fsBuilder->codeAppend("float delta = -" GR_FONT_ATLAS_LCD_DELTA ";\n");
        } else {
            fsBuilder->codeAppend("float delta = " GR_FONT_ATLAS_LCD_DELTA ";\n");
        }
        if (isUniformScale) {
            fsBuilder->codeAppend("\tfloat dx = dFdx(st.x);\n");
            fsBuilder->codeAppend("\tvec2 offset = vec2(dx*delta, 0.0);\n");
        } else {
            fsBuilder->codeAppend("\tvec2 Jdx = dFdx(st);\n");
            fsBuilder->codeAppend("\tvec2 Jdy = dFdy(st);\n");
            fsBuilder->codeAppend("\tvec2 offset = delta*Jdx;\n");
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

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.

        // To be strictly correct, we should compute the anti-aliasing factor separately
        // for each color component. However, this is only important when using perspective
        // transformations, and even then using a single factor seems like a reasonable
        // trade-off between quality and speed.
        fsBuilder->codeAppend("\tfloat afwidth;\n");
        if (isUniformScale) {
            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = abs(" SK_DistanceFieldAAFactor "*dx);\n");
        } else {
            fsBuilder->codeAppend("\tvec2 uv_grad;\n");
            if (args.fPB->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                fsBuilder->codeAppend("\tfloat uv_len2 = dot(uv, uv);\n");
                fsBuilder->codeAppend("\tif (uv_len2 < 0.0001) {\n");
                fsBuilder->codeAppend("\t\tuv_grad = vec2(0.7071, 0.7071);\n");
                fsBuilder->codeAppend("\t} else {\n");
                fsBuilder->codeAppend("\t\tuv_grad = uv*inversesqrt(uv_len2);\n");
                fsBuilder->codeAppend("\t}\n");
            } else {
                fsBuilder->codeAppend("\tuv_grad = normalize(uv);\n");
            }
            fsBuilder->codeAppend("\tvec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,\n");
            fsBuilder->codeAppend("\t                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);\n");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*length(grad);\n");
        }

        fsBuilder->codeAppend("\tvec4 val = vec4(smoothstep(vec3(-afwidth), vec3(afwidth), distance), 1.0);\n");

        // adjust based on gamma
        const char* textColorUniName = NULL;
        fTextColorUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                             kVec3f_GrSLType, kDefault_GrSLPrecision,
                                             "TextColor", &textColorUniName);

        fsBuilder->codeAppendf("\tuv = vec2(val.x, %s.x);\n", textColorUniName);
        fsBuilder->codeAppend("float gammaColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(".r;\n");
        fsBuilder->codeAppend("\tval.x = gammaColor;\n");

        fsBuilder->codeAppendf("\tuv = vec2(val.y, %s.y);\n", textColorUniName);
        fsBuilder->codeAppend("\tgammaColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(".r;\n");
        fsBuilder->codeAppend("\tval.y = gammaColor;\n");

        fsBuilder->codeAppendf("\tuv = vec2(val.z, %s.z);\n", textColorUniName);
        fsBuilder->codeAppend("\tgammaColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(".r;\n");
        fsBuilder->codeAppend("\tval.z = gammaColor;\n");

        fsBuilder->codeAppendf("%s = vec4(val);", args.fOutputCoverage);
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& processor,
                         const GrBatchTracker& bt) SK_OVERRIDE {
        SkASSERT(fTextColorUni.isValid());

        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                processor.cast<GrDistanceFieldLCDTextureEffect>();
        GrColor textColor = dfTexEffect.getTextColor();
        if (textColor != fTextColor) {
            static const float ONE_OVER_255 = 1.f / 255.f;
            pdman.set3f(fTextColorUni,
                        GrColorUnpackR(textColor) * ONE_OVER_255,
                        GrColorUnpackG(textColor) * ONE_OVER_255,
                        GrColorUnpackB(textColor) * ONE_OVER_255);
            fTextColor = textColor;
        }

        this->setUniformViewMatrix(pdman, processor.viewMatrix());

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
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                gp.cast<GrDistanceFieldLCDTextureEffect>();

        const DistanceFieldLCDBatchTracker& local = bt.cast<DistanceFieldLCDBatchTracker>();
        uint32_t key = dfTexEffect.getFlags();
        key |= local.fInputColorType << 16;
        key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 << 24: 0x0;
        key |= ComputePosKey(gp.viewMatrix()) << 25;
        b->add32(key);
    }

private:
    GrColor       fColor;
    UniformHandle fColorUniform;
    UniformHandle fTextColorUni;
    SkColor       fTextColor;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldLCDTextureEffect::GrDistanceFieldLCDTextureEffect(
                                                  GrColor color, const SkMatrix& viewMatrix,
                                                  GrTexture* texture, const GrTextureParams& params,
                                                  GrTexture* gamma, const GrTextureParams& gParams,
                                                  SkColor textColor,
                                                  uint32_t flags)
    : INHERITED(color, viewMatrix, SkMatrix::I())
    , fTextureAccess(texture, params)
    , fGammaTextureAccess(gamma, gParams)
    , fTextColor(textColor)
    , fFlags(flags & kLCD_DistanceFieldEffectMask){
    SkASSERT(!(flags & ~kLCD_DistanceFieldEffectMask) && (flags & kUseLCD_DistanceFieldEffectFlag));
    this->initClassID<GrDistanceFieldLCDTextureEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                          kVec2s_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
    this->addTextureAccess(&fGammaTextureAccess);
}

bool GrDistanceFieldLCDTextureEffect::onIsEqual(const GrGeometryProcessor& other) const {
    const GrDistanceFieldLCDTextureEffect& cte = other.cast<GrDistanceFieldLCDTextureEffect>();
    return (fTextColor == cte.fTextColor &&
            fFlags == cte.fFlags);
}

void GrDistanceFieldLCDTextureEffect::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setUnknownFourComponents();
    out->setUsingLCDCoverage();
}

void GrDistanceFieldLCDTextureEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                                        const GrGLCaps& caps,
                                                        GrProcessorKeyBuilder* b) const {
    GrGLDistanceFieldLCDTextureEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrDistanceFieldLCDTextureEffect::createGLInstance(const GrBatchTracker& bt,
                                                  const GrGLCaps&) const {
    return SkNEW_ARGS(GrGLDistanceFieldLCDTextureEffect, (*this, bt));
}

void GrDistanceFieldLCDTextureEffect::initBatchTracker(GrBatchTracker* bt,
                                                       const GrPipelineInfo& init) const {
    DistanceFieldLCDBatchTracker* local = bt->cast<DistanceFieldLCDBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrDistanceFieldLCDTextureEffect::onCanMakeEqual(const GrBatchTracker& m,
                                                     const GrGeometryProcessor& that,
                                                     const GrBatchTracker& t) const {
    const DistanceFieldLCDBatchTracker& mine = m.cast<DistanceFieldLCDBatchTracker>();
    const DistanceFieldLCDBatchTracker& theirs = t.cast<DistanceFieldLCDBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldLCDTextureEffect);

GrGeometryProcessor* GrDistanceFieldLCDTextureEffect::TestCreate(SkRandom* random,
                                                                 GrContext*,
                                                                 const GrDrawTargetCaps&,
                                                                 GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    int texIdx2 = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
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
    GrTextureParams params2(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                           GrTextureParams::kNone_FilterMode);
    GrColor textColor = GrColorPackRGBA(random->nextULessThan(256),
                                        random->nextULessThan(256),
                                        random->nextULessThan(256),
                                        random->nextULessThan(256));
    uint32_t flags = kUseLCD_DistanceFieldEffectFlag;
    flags |= random->nextBool() ? kUniformScale_DistanceFieldEffectMask : 0;
    flags |= random->nextBool() ? kBGR_DistanceFieldEffectFlag : 0;
    return GrDistanceFieldLCDTextureEffect::Create(GrRandomColor(random),
                                                   GrProcessorUnitTest::TestMatrix(random),
                                                   textures[texIdx], params,
                                                   textures[texIdx2], params2,
                                                   textColor,
                                                   flags);
}
