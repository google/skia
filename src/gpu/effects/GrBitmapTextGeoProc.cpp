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

class GrGLBitmapTextGeoProc : public GrGLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc(const GrGeometryProcessor&, const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
        const GrBitmapTextGeoProc& cte = args.fGP.cast<GrBitmapTextGeoProc>();

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
        if (!cte.colorIgnored()) {
            if (cte.hasVertexColor()) {
                pb->addPassThroughAttribute(cte.inColor(), args.fOutputColor);
            } else {
                this->setupUniformColor(pb, args.fOutputColor, &fColorUniform);
            }
        }

        // Setup position
        this->setupPosition(pb, gpArgs, cte.inPosition()->fName);

        // emit transforms
        this->emitTransforms(args.fPB, gpArgs->fPositionVar, cte.inPosition()->fName,
                             cte.localMatrix(), args.fTransformsIn, args.fTransformsOut);

        GrGLFragmentBuilder* fsBuilder = pb->getFragmentShaderBuilder();
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
                         const GrBatchTracker& bt) override {
        const GrBitmapTextGeoProc& btgp = gp.cast<GrBitmapTextGeoProc>();
        if (btgp.color() != fColor && !btgp.hasVertexColor()) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(btgp.color(), c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = btgp.color();
        }
    }

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<GrBitmapTextGeoProc>(primProc, pdman, index, transforms);
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrBatchTracker& bt,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBitmapTextGeoProc& gp = proc.cast<GrBitmapTextGeoProc>();
        uint32_t key = 0;
        key |= gp.usesLocalCoords() && gp.localMatrix().hasPerspective() ? 0x1 : 0x0;
        key |= gp.colorIgnored() ? 0x2 : 0x0;
        key |= gp.maskFormat() << 3;
        b->add32(key);
    }

private:
    GrColor fColor;
    UniformHandle fColorUniform;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(GrColor color, GrTexture* texture,
                                         const GrTextureParams& params, GrMaskFormat format,
                                         const SkMatrix& localMatrix, bool usesLocalCoords)
    : fColor(color)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fTextureAccess(texture, params)
    , fInColor(NULL)
    , fMaskFormat(format) {
    this->initClassID<GrBitmapTextGeoProc>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));

    // TODO we could think about removing this attribute if color is ignored, but unfortunately
    // we don't do text positioning in batch, so we can't quite do that yet.
    bool hasVertexColor = kA8_GrMaskFormat == fMaskFormat;
    if (hasVertexColor) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                        kVec2s_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

void GrBitmapTextGeoProc::getGLProcessorKey(const GrBatchTracker& bt,
                                            const GrGLSLCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GrGLBitmapTextGeoProc::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor*
GrBitmapTextGeoProc::createGLInstance(const GrBatchTracker& bt,
                                      const GrGLSLCaps& caps) const {
    return SkNEW_ARGS(GrGLBitmapTextGeoProc, (*this, bt));
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
                                       format, GrTest::TestMatrix(random), random->nextBool());
}
