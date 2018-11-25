/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextGeoProc.h"

#include "GrAtlasedShaderHelpers.h"
#include "GrTexture.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

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
                                                          kFloat2_GrSLType,
                                                          kHigh_GrSLPrecision,
                                                          "AtlasSizeInv",
                                                          &atlasSizeInvName);

        GrGLSLVarying uv(kFloat2_GrSLType);
        GrSLType texIdxType = args.fShaderCaps->integerSupport() ? kInt_GrSLType : kFloat_GrSLType;
        GrGLSLVarying texIdx(texIdxType);
        append_index_uv_varyings(args, btgp.inTextureCoords()->fName, atlasSizeInvName,
                                 &uv, &texIdx, nullptr);

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
                             btgp.inPosition()->asShaderVar(),
                             btgp.localMatrix(),
                             args.fFPCoordTransformHandler);

        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, btgp.numTextureSamplers(),
                                   texIdx, uv.fsIn(), "texColor");

        if (btgp.maskFormat() == kARGB_GrMaskFormat) {
            // modulate by color
            fragBuilder->codeAppendf("%s = %s * texColor;", args.fOutputColor, args.fOutputColor);
            fragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf("%s = texColor;", args.fOutputCoverage);
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

        SkASSERT(btgp.numTextureSamplers() >= 1);
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
        b->add32(btgp.numTextureSamplers());
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
                                         const sk_sp<GrTextureProxy> proxies[kMaxTextures],
                                         const GrSamplerState& params, GrMaskFormat format,
                                         const SkMatrix& localMatrix, bool usesLocalCoords)
        : INHERITED(kGrBitmapTextGeoProc_ClassID)
        , fColor(color)
        , fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords)
        , fInColor(nullptr)
        , fMaskFormat(format) {
    fInPosition = &this->addVertexAttrib("inPosition", kFloat2_GrVertexAttribType);

    bool hasVertexColor = kA8_GrMaskFormat == fMaskFormat ||
                          kA565_GrMaskFormat == fMaskFormat;
    if (hasVertexColor) {
        fInColor = &this->addVertexAttrib("inColor", kUByte4_norm_GrVertexAttribType);
    }

    fInTextureCoords = &this->addVertexAttrib("inTextureCoords", kUShort2_GrVertexAttribType);
    for (int i = 0; i < kMaxTextures; ++i) {
        if (proxies[i]) {
            fTextureSamplers[i].reset(std::move(proxies[i]), params);
            this->addTextureSampler(&fTextureSamplers[i]);
        }
    }
}

void GrBitmapTextGeoProc::addNewProxies(const sk_sp<GrTextureProxy> proxies[kMaxTextures],
                                       const GrSamplerState& params) {
    for (int i = 0; i < kMaxTextures; ++i) {
        if (proxies[i] && !fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(std::move(proxies[i]), params);
            this->addTextureSampler(&fTextureSamplers[i]);
        }
    }
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

    return GrBitmapTextGeoProc::Make(GrRandomColor(d->fRandom), proxies, samplerState,
                                     format, GrTest::TestMatrix(d->fRandom),
                                     d->fRandom->nextBool());
}
#endif
