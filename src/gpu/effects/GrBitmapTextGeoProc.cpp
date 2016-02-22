/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextGeoProc.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

class GrGLBitmapTextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc() : fColor(GrColor_ILLEGAL) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrBitmapTextGeoProc& cte = args.fGP.cast<GrBitmapTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(cte);

        // compute numbers to be hardcoded to convert texture coordinates from int to float
        SkASSERT(cte.numTextures() == 1);
        SkDEBUGCODE(GrTexture* atlas = cte.textureAccess(0).getTexture());
        SkASSERT(atlas && SkIsPow2(atlas->width()) && SkIsPow2(atlas->height()));

        GrGLSLVertToFrag v(kVec2f_GrSLType);
        varyingHandler->addVarying("TextureCoords", &v, kHigh_GrSLPrecision);
        vertBuilder->codeAppendf("%s = %s;", v.vsOut(),
                                 cte.inTextureCoords()->fName);

        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
        // Setup pass through color
        if (!cte.colorIgnored()) {
            if (cte.hasVertexColor()) {
                varyingHandler->addPassThroughAttribute(cte.inColor(), args.fOutputColor);
            } else {
                this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                        &fColorUniform);
            }
        }

        // Setup position
        this->setupPosition(vertBuilder, gpArgs, cte.inPosition()->fName);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             gpArgs->fPositionVar,
                             cte.inPosition()->fName,
                             cte.localMatrix(),
                             args.fTransformsIn,
                             args.fTransformsOut);

        if (cte.maskFormat() == kARGB_GrMaskFormat) {
            fragBuilder->codeAppendf("%s = ", args.fOutputColor);
            fragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                        args.fSamplers[0],
                                                        v.fsIn(),
                                                        kVec2f_GrSLType);
            fragBuilder->codeAppend(";");
            fragBuilder->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf("%s = ", args.fOutputCoverage);
            fragBuilder->appendTextureLookup(args.fSamplers[0], v.fsIn(), kVec2f_GrSLType);
            fragBuilder->codeAppend(";");
            if (cte.maskFormat() == kA565_GrMaskFormat) {
                // set alpha to be max of rgb coverage
                fragBuilder->codeAppendf("%s.a = max(max(%s.r, %s.g), %s.b);",
                                         args.fOutputCoverage, args.fOutputCoverage,
                                         args.fOutputCoverage, args.fOutputCoverage);
            }
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& gp) override {
        const GrBitmapTextGeoProc& btgp = gp.cast<GrBitmapTextGeoProc>();
        if (btgp.color() != fColor && !btgp.hasVertexColor()) {
            float c[4];
            GrColorToRGBAFloat(btgp.color(), c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = btgp.color();
        }
    }

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLSLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<GrBitmapTextGeoProc>(primProc, pdman, index, transforms);
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBitmapTextGeoProc& gp = proc.cast<GrBitmapTextGeoProc>();
        uint32_t key = 0;
        key |= gp.usesLocalCoords() && gp.localMatrix().hasPerspective() ? 0x1 : 0x0;
        key |= gp.colorIgnored() ? 0x2 : 0x0;
        key |= gp.maskFormat() << 3;
        b->add32(key);

        // Currently we hardcode numbers to convert atlas coordinates to normalized floating point
        SkASSERT(gp.numTextures() == 1);
        GrTexture* atlas = gp.textureAccess(0).getTexture();
        SkASSERT(atlas);
        b->add32(atlas->width());
        b->add32(atlas->height());
    }

private:
    GrColor fColor;
    UniformHandle fColorUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(GrColor color, GrTexture* texture,
                                         const GrTextureParams& params, GrMaskFormat format,
                                         const SkMatrix& localMatrix, bool usesLocalCoords)
    : fColor(color)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fTextureAccess(texture, params)
    , fInColor(nullptr)
    , fMaskFormat(format) {
    this->initClassID<GrBitmapTextGeoProc>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));

    bool hasVertexColor = kA8_GrMaskFormat == fMaskFormat ||
                          kA565_GrMaskFormat == fMaskFormat;
    if (hasVertexColor) {
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
    }
    fInTextureCoords = &this->addVertexAttrib(Attribute("inTextureCoords",
                                                        kVec2us_GrVertexAttribType,
                                                        kHigh_GrSLPrecision));
    this->addTextureAccess(&fTextureAccess);
}

void GrBitmapTextGeoProc::getGLSLProcessorKey(const GrGLSLCaps& caps,
                                              GrProcessorKeyBuilder* b) const {
    GrGLBitmapTextGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrBitmapTextGeoProc::createGLSLInstance(const GrGLSLCaps& caps) const {
    return new GrGLBitmapTextGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrBitmapTextGeoProc);

const GrGeometryProcessor* GrBitmapTextGeoProc::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, d->fRandom->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                           GrTextureParams::kNone_FilterMode);

    GrMaskFormat format;
    switch (d->fRandom->nextULessThan(3)) {
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

    return GrBitmapTextGeoProc::Create(GrRandomColor(d->fRandom), d->fTextures[texIdx], params,
                                       format, GrTest::TestMatrix(d->fRandom),
                                       d->fRandom->nextBool());
}
