/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrBitmapTextGeoProc.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/effects/GrAtlasedShaderHelpers.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrGLBitmapTextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc() : fColor(SK_PMColor4fILLEGAL), fAtlasDimensions{0,0} {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrBitmapTextGeoProc& btgp = args.fGP.cast<GrBitmapTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(btgp);

        const char* atlasDimensionsInvName;
        fAtlasDimensionsInvUniform = uniformHandler->addUniform(
                kVertex_GrShaderFlag, kFloat2_GrSLType, "AtlasSizeInv", &atlasDimensionsInvName);

        GrGLSLVarying uv(kFloat2_GrSLType);
        GrSLType texIdxType = args.fShaderCaps->integerSupport() ? kInt_GrSLType : kFloat_GrSLType;
        GrGLSLVarying texIdx(texIdxType);
        append_index_uv_varyings(args, btgp.inTextureCoords().name(), atlasDimensionsInvName, &uv,
                                 &texIdx, nullptr);

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        // Setup pass through color
        if (btgp.hasVertexColor()) {
            varyingHandler->addPassThroughAttribute(btgp.inColor(), args.fOutputColor);
        } else {
            this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                    &fColorUniform);
        }

        // Setup position
        gpArgs->fPositionVar = btgp.inPosition().asShaderVar();

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             btgp.inPosition().asShaderVar(),
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
                 const CoordTransformRange& transformRange) override {
        const GrBitmapTextGeoProc& btgp = gp.cast<GrBitmapTextGeoProc>();
        if (btgp.color() != fColor && !btgp.hasVertexColor()) {
            pdman.set4fv(fColorUniform, 1, btgp.color().vec());
            fColor = btgp.color();
        }

        const SkISize& atlasDimensions = btgp.atlasDimensions();
        SkASSERT(SkIsPow2(atlasDimensions.fWidth) && SkIsPow2(atlasDimensions.fHeight));

        if (fAtlasDimensions != atlasDimensions) {
            pdman.set2f(fAtlasDimensionsInvUniform,
                        1.0f / atlasDimensions.fWidth,
                        1.0f / atlasDimensions.fHeight);
            fAtlasDimensions = atlasDimensions;
        }
        this->setTransformDataHelper(btgp.localMatrix(), pdman, transformRange);
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBitmapTextGeoProc& btgp = proc.cast<GrBitmapTextGeoProc>();
        uint32_t key = 0;
        key |= btgp.usesW() ? 0x1 : 0x0;
        key |= btgp.maskFormat() << 1;
        b->add32(key);
        b->add32(btgp.numTextureSamplers());
    }

private:
    SkPMColor4f   fColor;
    UniformHandle fColorUniform;

    SkISize       fAtlasDimensions;
    UniformHandle fAtlasDimensionsInvUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(const GrShaderCaps& caps,
                                         const SkPMColor4f& color,
                                         bool wideColor,
                                         const GrSurfaceProxyView* views,
                                         int numActiveViews,
                                         const GrSamplerState& params, GrMaskFormat format,
                                         const SkMatrix& localMatrix, bool usesW)
        : INHERITED(kGrBitmapTextGeoProc_ClassID)
        , fColor(color)
        , fLocalMatrix(localMatrix)
        , fUsesW(usesW)
        , fMaskFormat(format) {
    SkASSERT(numActiveViews <= kMaxTextures);

    if (usesW) {
        fInPosition = {"inPosition", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
    } else {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    }

    bool hasVertexColor = kA8_GrMaskFormat == fMaskFormat ||
                          kA565_GrMaskFormat == fMaskFormat;
    if (hasVertexColor) {
        fInColor = MakeColorAttribute("inColor", wideColor);
    }

    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.integerSupport() ? kUShort2_GrSLType : kFloat2_GrSLType};
    this->setVertexAttributes(&fInPosition, 3);

    if (numActiveViews) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }
    for (int i = 0; i < numActiveViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);
        fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
    }
    this->setTextureSamplerCnt(numActiveViews);
}

void GrBitmapTextGeoProc::addNewViews(const GrSurfaceProxyView* views,
                                      int numActiveViews,
                                      const GrSamplerState& params) {
    SkASSERT(numActiveViews <= kMaxTextures);
    // Just to make sure we don't try to add too many proxies
    numActiveViews = SkTMin(numActiveViews, kMaxTextures);

    if (!fTextureSamplers[0].isInitialized()) {
        fAtlasDimensions = views[0].proxy()->dimensions();
    }

    for (int i = 0; i < numActiveViews; ++i) {
        const GrSurfaceProxy* proxy = views[i].proxy();
        SkASSERT(proxy);
        SkASSERT(proxy->dimensions() == fAtlasDimensions);

        if (!fTextureSamplers[i].isInitialized()) {
            fTextureSamplers[i].reset(params, proxy->backendFormat(), views[i].swizzle());
        }
    }
    this->setTextureSamplerCnt(numActiveViews);
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

GrGeometryProcessor* GrBitmapTextGeoProc::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

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
    GrSurfaceOrigin origin = proxy->origin();
    const GrSwizzle& swizzle = proxy->textureSwizzle();
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);

    return GrBitmapTextGeoProc::Make(d->allocator(), *d->caps()->shaderCaps(),
                                     SkPMColor4f::FromBytes_RGBA(GrRandomColor(d->fRandom)),
                                     d->fRandom->nextBool(),
                                     &view, 1, samplerState, format,
                                     GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool());
}
#endif
