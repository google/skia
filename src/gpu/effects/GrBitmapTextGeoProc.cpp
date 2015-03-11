/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextGeoProc.h"
#include "GrFontAtlasSizes.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

struct BitmapTextBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLBitmapTextGeoProc : public GrGLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc(const GrGeometryProcessor&, const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
        const GrBitmapTextGeoProc& cte = args.fGP.cast<GrBitmapTextGeoProc>();
        const BitmapTextBatchTracker& local = args.fBT.cast<BitmapTextBatchTracker>();

        GrGLGPBuilder* pb = args.fPB;
        GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();

        // emit attributes
        vsBuilder->emitAttributes(cte);

        GrGLVertToFrag v(kVec2f_GrSLType);
        pb->addVarying("TextureCoords", &v);
        // this is only used with text, so our texture bounds always match the glyph atlas
        if (cte.maskFormat() == kA8_GrMaskFormat) {
            vsBuilder->codeAppendf("%s = vec2(" GR_FONT_ATLAS_A8_RECIP_WIDTH ", "
                                   GR_FONT_ATLAS_RECIP_HEIGHT ")*%s;", v.vsOut(),
                                   cte.inTextureCoords()->fName);
        } else {
            vsBuilder->codeAppendf("%s = vec2(" GR_FONT_ATLAS_RECIP_WIDTH ", "
                                   GR_FONT_ATLAS_RECIP_HEIGHT ")*%s;", v.vsOut(),
                                   cte.inTextureCoords()->fName);
        }

        // Setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, cte.inColor(),
                                    &fColorUniform);

        // Setup position
        this->setupPosition(pb, gpArgs, cte.inPosition()->fName, cte.viewMatrix());

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, cte.inPosition()->fName,
                             cte.localMatrix(), args.fTransformsIn, args.fTransformsOut);

        GrGLGPFragmentBuilder* fsBuilder = pb->getFragmentShaderBuilder();
        if (cte.maskFormat() == kARGB_GrMaskFormat) {
            fsBuilder->codeAppendf("%s = ", args.fOutputColor);
            fsBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                      args.fSamplers[0],
                                                      v.fsIn(),
                                                      kVec2f_GrSLType);
            fsBuilder->codeAppend(";");
            fsBuilder->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
        } else {
            fsBuilder->codeAppendf("%s = ", args.fOutputCoverage);
            fsBuilder->appendTextureLookup(args.fSamplers[0], v.fsIn(), kVec2f_GrSLType);
            fsBuilder->codeAppend(";");
        }
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& gp,
                         const GrBatchTracker& bt) SK_OVERRIDE {
        this->setUniformViewMatrix(pdman, gp.viewMatrix());

        const BitmapTextBatchTracker& local = bt.cast<BitmapTextBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrBatchTracker& bt,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const BitmapTextBatchTracker& local = bt.cast<BitmapTextBatchTracker>();
        // We have to put the optional vertex attribute as part of the key.  See the comment
        // on addVertexAttrib.
        // TODO When we have deferred geometry we can fix this
        const GrBitmapTextGeoProc& gp = proc.cast<GrBitmapTextGeoProc>();
        uint32_t key = 0;
        key |= SkToBool(gp.inColor()) ? 0x1 : 0x0;
        key |= local.fUsesLocalCoords && proc.localMatrix().hasPerspective() ? 0x2 : 0x0;
        key |= gp.maskFormat() == kARGB_GrMaskFormat ? 0x4 : 0x0;
        key |= ComputePosKey(gp.viewMatrix()) << 3;
        b->add32(local.fInputColorType << 16 | key);
    }

private:
    GrColor fColor;
    UniformHandle fColorUniform;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(GrColor color, GrTexture* texture,
                                         const GrTextureParams& params, GrMaskFormat format,
                                         bool opaqueVertexColors, const SkMatrix& localMatrix)
    : INHERITED(color, SkMatrix::I(), localMatrix, opaqueVertexColors)
    , fTextureAccess(texture, params)
    , fInColor(NULL)
    , fMaskFormat(format) {
    this->initClassID<GrBitmapTextGeoProc>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));

    bool hasVertexColor = kA8_GrMaskFormat == fMaskFormat;
    if (hasVertexColor) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        this->setHasVertexColor();
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                        kVec2s_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

bool GrBitmapTextGeoProc::onIsEqual(const GrGeometryProcessor& other) const {
    const GrBitmapTextGeoProc& gp = other.cast<GrBitmapTextGeoProc>();
    return SkToBool(this->inColor()) == SkToBool(gp.inColor());
}

void GrBitmapTextGeoProc::onGetInvariantOutputColor(GrInitInvariantOutput* out) const {
    if (kARGB_GrMaskFormat == fMaskFormat) {
        out->setUnknownFourComponents();
    }
}

void GrBitmapTextGeoProc::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    if (kARGB_GrMaskFormat != fMaskFormat) {
        if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
            out->setUnknownSingleComponent();
        } else if (GrPixelConfigIsOpaque(this->texture(0)->config())) {
            out->setUnknownOpaqueFourComponents();
            out->setUsingLCDCoverage();
        } else {
            out->setUnknownFourComponents();
            out->setUsingLCDCoverage();
        }
    } else {
        out->setKnownSingleComponent(0xff);
    }
}

void GrBitmapTextGeoProc::getGLProcessorKey(const GrBatchTracker& bt,
                                            const GrGLCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GrGLBitmapTextGeoProc::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrBitmapTextGeoProc::createGLInstance(const GrBatchTracker& bt,
                                      const GrGLCaps& caps) const {
    return SkNEW_ARGS(GrGLBitmapTextGeoProc, (*this, bt));
}

void GrBitmapTextGeoProc::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    BitmapTextBatchTracker* local = bt->cast<BitmapTextBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                               SkToBool(fInColor));
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrBitmapTextGeoProc::onCanMakeEqual(const GrBatchTracker& m,
                                         const GrGeometryProcessor& that,
                                         const GrBatchTracker& t) const {
    const BitmapTextBatchTracker& mine = m.cast<BitmapTextBatchTracker>();
    const BitmapTextBatchTracker& theirs = t.cast<BitmapTextBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrBitmapTextGeoProc);

GrGeometryProcessor* GrBitmapTextGeoProc::TestCreate(SkRandom* random,
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

    GrMaskFormat format;
    switch (random->nextULessThan(3)) {
        default:
            SkFAIL("Incomplete enum\n");
        case 0:
            format = kA8_GrMaskFormat;
            break;
        case 1:
            format = kA565_GrMaskFormat;
            break;
        case 2:
            format = kARGB_GrMaskFormat;
            break;
    }

    return GrBitmapTextGeoProc::Create(GrRandomColor(random), textures[texIdx], params,
                                       format, random->nextBool(),
                                       GrProcessorUnitTest::TestMatrix(random));
}
