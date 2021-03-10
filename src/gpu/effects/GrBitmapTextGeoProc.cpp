/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrBitmapTextGeoProc.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/effects/GrAtlasedShaderHelpers.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrGLBitmapTextGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc()
            : fColor(SK_PMColor4fILLEGAL)
            , fAtlasDimensions{0,0}
            , fLocalMatrix(SkMatrix::InvalidMatrix()) {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrBitmapTextGeoProc& btgp = args.fGP.cast<GrBitmapTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(btgp);

        const char* atlasDimensionsInvName;
        fAtlasDimensionsInvUniform = uniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                kFloat2_GrSLType, "AtlasSizeInv", &atlasDimensionsInvName);

        GrGLSLVarying uv, texIdx;
        append_index_uv_varyings(args, btgp.numTextureSamplers(), btgp.inTextureCoords().name(),
                                 atlasDimensionsInvName, &uv, &texIdx, nullptr);

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        // Setup pass through color
        fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        if (btgp.hasVertexColor()) {
            varyingHandler->addPassThroughAttribute(btgp.inColor(), args.fOutputColor);
        } else {
            this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                    &fColorUniform);
        }

        // Setup position
        gpArgs->fPositionVar = btgp.inPosition().asShaderVar();
        this->writeLocalCoord(vertBuilder, uniformHandler, gpArgs, btgp.inPosition().asShaderVar(),
                              btgp.localMatrix(), &fLocalMatrixUniform);

        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, btgp.numTextureSamplers(),
                                   texIdx, uv.fsIn(), "texColor");

        if (btgp.maskFormat() == kARGB_GrMaskFormat) {
            // modulate by color
            fragBuilder->codeAppendf("%s = %s * texColor;", args.fOutputColor, args.fOutputColor);
            fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf("half4 %s = texColor;", args.fOutputCoverage);
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& gp) override {
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

        this->setTransform(pdman, fLocalMatrixUniform, btgp.localMatrix(), &fLocalMatrix);
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrBitmapTextGeoProc& btgp = proc.cast<GrBitmapTextGeoProc>();
        b->addBool(btgp.usesW(), "usesW");
        static_assert(kLast_GrMaskFormat < (1u << 2));
        b->addBits(2, btgp.maskFormat(), "maskFormat");
        b->addBits(kMatrixKeyBits, ComputeMatrixKey(btgp.localMatrix()), "localMatrixType");
        b->add32(btgp.numTextureSamplers(),"numTextures");
    }

private:
    SkPMColor4f   fColor;
    UniformHandle fColorUniform;

    SkISize       fAtlasDimensions;
    UniformHandle fAtlasDimensionsInvUniform;

    SkMatrix      fLocalMatrix;
    UniformHandle fLocalMatrixUniform;

    using INHERITED = GrGLSLGeometryProcessor;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(const GrShaderCaps& caps,
                                         const SkPMColor4f& color,
                                         bool wideColor,
                                         const GrSurfaceProxyView* views,
                                         int numActiveViews,
                                         GrSamplerState params,
                                         GrMaskFormat format,
                                         const SkMatrix& localMatrix,
                                         bool usesW)
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
                                      GrSamplerState params) {
    SkASSERT(numActiveViews <= kMaxTextures);
    // Just to make sure we don't try to add too many proxies
    numActiveViews = std::min(numActiveViews, kMaxTextures);

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
    auto [view, ct, at] = d->randomView();

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kLinear
                                                   : GrSamplerState::Filter::kNearest);

    GrMaskFormat format;
    switch (ct) {
        case GrColorType::kAlpha_8:
            format = kA8_GrMaskFormat;
            break;
        case GrColorType::kBGR_565:
            format = kA565_GrMaskFormat;
            break;
        case GrColorType::kRGBA_8888:
        default:  // It doesn't really matter that color type and mask format agree.
            format = kARGB_GrMaskFormat;
            break;
    }

    return GrBitmapTextGeoProc::Make(d->allocator(), *d->caps()->shaderCaps(),
                                     SkPMColor4f::FromBytes_RGBA(GrRandomColor(d->fRandom)),
                                     d->fRandom->nextBool(),
                                     &view, 1, samplerState, format,
                                     GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool());
}
#endif
