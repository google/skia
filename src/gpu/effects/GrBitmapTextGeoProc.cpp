/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextGeoProc.h"

#include "GrTexture.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

class GrGLBitmapTextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc() : fColor(GrColor_ILLEGAL), fAtlasSize({0,0}) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrBitmapTextGeoProc& btgp = args.fGP.cast<GrBitmapTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(btgp);

        const char* atlasSizeInvName;
        fAtlasSizeInvUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                          kVec2f_GrSLType,
                                                          kHigh_GrSLPrecision,
                                                          "AtlasSizeInv",
                                                          &atlasSizeInvName);

        GrGLSLVertToFrag v(kVec2f_GrSLType);
        varyingHandler->addVarying("TextureCoords", &v, kHigh_GrSLPrecision);
        vertBuilder->codeAppendf("%s = float2(%s.x, %s.y) * %s;", v.vsOut(),
                                 btgp.inTextureCoords()->fName,
                                 btgp.inTextureCoords()->fName,
                                 atlasSizeInvName);

        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
        // Setup pass through color
        if (btgp.hasVertexColor()) {
            varyingHandler->addPassThroughAttribute(btgp.inColor(), args.fOutputColor);
        } else {
            this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                    &fColorUniform);
        }

        // Setup position
        this->writeOutputPosition(vertBuilder, gpArgs, btgp.inPosition()->fName);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             gpArgs->fPositionVar,
                             btgp.inPosition()->fName,
                             btgp.localMatrix(),
                             args.fFPCoordTransformHandler);

        if (btgp.maskFormat() == kARGB_GrMaskFormat) {
            fragBuilder->codeAppendf("%s = ", args.fOutputColor);
            fragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                        args.fTexSamplers[0],
                                                        v.fsIn(),
                                                        kVec2f_GrSLType);
            fragBuilder->codeAppend(";");
            fragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf("%s = ", args.fOutputCoverage);
            fragBuilder->appendTextureLookup(args.fTexSamplers[0], v.fsIn(), kVec2f_GrSLType);
            fragBuilder->codeAppend(";");
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& gp,
                 FPCoordTransformIter&& transformIter) override {
        const GrBitmapTextGeoProc& btgp = gp.cast<GrBitmapTextGeoProc>();
        if (btgp.color() != fColor && !btgp.hasVertexColor()) {
            float c[4];
            GrColorToRGBAFloat(btgp.color(), c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = btgp.color();
        }

        SkASSERT(btgp.numTextureSamplers() == 1);
        GrTexture* atlas = btgp.textureSampler(0).peekTexture();
        SkASSERT(atlas && SkIsPow2(atlas->width()) && SkIsPow2(atlas->height()));

        if (fAtlasSize.fWidth != atlas->width() || fAtlasSize.fHeight != atlas->height()) {
            pdman.set2f(fAtlasSizeInvUniform, 1.0f / atlas->width(), 1.0f / atlas->height());
            fAtlasSize.set(atlas->width(), atlas->height());
        }
        this->setTransformDataHelper(btgp.localMatrix(), pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBitmapTextGeoProc& btgp = proc.cast<GrBitmapTextGeoProc>();
        uint32_t key = 0;
        key |= (btgp.usesLocalCoords() && btgp.localMatrix().hasPerspective()) ? 0x1 : 0x0;
        key |= btgp.maskFormat() << 1;
        b->add32(key);
    }

private:
    GrColor       fColor;
    UniformHandle fColorUniform;

    SkISize       fAtlasSize;
    UniformHandle fAtlasSizeInvUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(GrColor color,
                                         sk_sp<GrTextureProxy> proxy,
                                         const GrSamplerParams& params, GrMaskFormat format,
                                         const SkMatrix& localMatrix, bool usesLocalCoords)
    : fColor(color)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fTextureSampler(std::move(proxy), params)
    , fInColor(nullptr)
    , fMaskFormat(format) {
    this->initClassID<GrBitmapTextGeoProc>();
    fInPosition =
            &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType, kHigh_GrSLPrecision);

    bool hasVertexColor = kA8_GrMaskFormat == fMaskFormat ||
                          kA565_GrMaskFormat == fMaskFormat;
    if (hasVertexColor) {
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
    }

    fInTextureCoords = &this->addVertexAttrib("inTextureCoords", kVec2us_uint_GrVertexAttribType,
                                              kHigh_GrSLPrecision);
    this->addTextureSampler(&fTextureSampler);
}

void GrBitmapTextGeoProc::getGLSLProcessorKey(const GrShaderCaps& caps,
                                              GrProcessorKeyBuilder* b) const {
    GrGLBitmapTextGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrBitmapTextGeoProc::createGLSLInstance(const GrShaderCaps& caps) const {
    return new GrGLBitmapTextGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrBitmapTextGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> GrBitmapTextGeoProc::TestCreate(GrProcessorTestData* d) {
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

    GrMaskFormat format = kARGB_GrMaskFormat; // init to avoid warning
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

    return GrBitmapTextGeoProc::Make(GrRandomColor(d->fRandom), std::move(proxy),
                                     params, format, GrTest::TestMatrix(d->fRandom),
                                     d->fRandom->nextBool());
}
#endif
