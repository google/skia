/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrBitmapTextGeoProc.h"

#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/effects/GrAtlasedShaderHelpers.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"

using MaskFormat = skgpu::MaskFormat;

class GrBitmapTextGeoProc::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps& shaderCaps,
                 const GrGeometryProcessor& geomProc) override {
        const GrBitmapTextGeoProc& btgp = geomProc.cast<GrBitmapTextGeoProc>();
        if (btgp.fColor != fColor && !btgp.hasVertexColor()) {
            pdman.set4fv(fColorUniform, 1, btgp.fColor.vec());
            fColor = btgp.fColor;
        }

        const SkISize& atlasDimensions = btgp.fAtlasDimensions;
        SkASSERT(SkIsPow2(atlasDimensions.fWidth) && SkIsPow2(atlasDimensions.fHeight));

        if (fAtlasDimensions != atlasDimensions) {
            pdman.set2f(fAtlasDimensionsInvUniform,
                        1.0f / atlasDimensions.fWidth,
                        1.0f / atlasDimensions.fHeight);
            fAtlasDimensions = atlasDimensions;
        }

        SetTransform(pdman, shaderCaps, fLocalMatrixUniform, btgp.fLocalMatrix, &fLocalMatrix);
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrBitmapTextGeoProc& btgp = args.fGeomProc.cast<GrBitmapTextGeoProc>();

        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        // emit attributes
        varyingHandler->emitAttributes(btgp);

        const char* atlasDimensionsInvName;
        fAtlasDimensionsInvUniform = uniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                SkSLType::kFloat2, "AtlasSizeInv", &atlasDimensionsInvName);

        GrGLSLVarying uv, texIdx;
        append_index_uv_varyings(args,
                                 btgp.numTextureSamplers(),
                                 btgp.fInTextureCoords.name(),
                                 atlasDimensionsInvName,
                                 &uv,
                                 &texIdx,
                                 nullptr);

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        // Setup pass through color
        fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        if (btgp.hasVertexColor()) {
            varyingHandler->addPassThroughAttribute(btgp.fInColor.asShaderVar(), args.fOutputColor);
        } else {
            this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                    &fColorUniform);
        }

        // Setup position
        gpArgs->fPositionVar = btgp.fInPosition.asShaderVar();
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        btgp.fInPosition.asShaderVar(),
                        btgp.fLocalMatrix,
                        &fLocalMatrixUniform);

        fragBuilder->codeAppend("half4 texColor;");
        append_multitexture_lookup(args, btgp.numTextureSamplers(),
                                   texIdx, uv.fsIn(), "texColor");

        if (btgp.fMaskFormat == MaskFormat::kARGB) {
            // modulate by color
            fragBuilder->codeAppendf("%s = %s * texColor;", args.fOutputColor, args.fOutputColor);
            fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
        } else {
            fragBuilder->codeAppendf("half4 %s = texColor;", args.fOutputCoverage);
        }
    }

private:
    SkPMColor4f fColor           = SK_PMColor4fILLEGAL;
    SkISize     fAtlasDimensions = {-1, -1};
    SkMatrix    fLocalMatrix     = SkMatrix::InvalidMatrix();

    UniformHandle fColorUniform;
    UniformHandle fAtlasDimensionsInvUniform;
    UniformHandle fLocalMatrixUniform;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(const GrShaderCaps& caps,
                                         const SkPMColor4f& color,
                                         bool wideColor,
                                         const GrSurfaceProxyView* views,
                                         int numActiveViews,
                                         GrSamplerState params,
                                         MaskFormat format,
                                         const SkMatrix& localMatrix,
                                         bool usesW)
        : INHERITED(kGrBitmapTextGeoProc_ClassID)
        , fColor(color)
        , fLocalMatrix(localMatrix)
        , fUsesW(usesW)
        , fMaskFormat(format) {
    SkASSERT(numActiveViews <= kMaxTextures);

    if (usesW) {
        fInPosition = {"inPosition", kFloat3_GrVertexAttribType, SkSLType::kFloat3};
    } else {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    }

    bool hasVertexColor = MaskFormat::kA8 == fMaskFormat || MaskFormat::kA565 == fMaskFormat;
    if (hasVertexColor) {
        fInColor = MakeColorAttribute("inColor", wideColor);
    }

    fInTextureCoords = {"inTextureCoords", kUShort2_GrVertexAttribType,
                        caps.fIntegerSupport ? SkSLType::kUShort2 : SkSLType::kFloat2};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);

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

void GrBitmapTextGeoProc::addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    b->addBool(fUsesW, "usesW");
    static_assert(static_cast<int>(MaskFormat::kLast) < (1u << 2));
    b->addBits(2, static_cast<int>(fMaskFormat), "maskFormat");
    b->addBits(ProgramImpl::kMatrixKeyBits,
               ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix),
               "localMatrixType");
    b->add32(this->numTextureSamplers(), "numTextures");
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrBitmapTextGeoProc::makeProgramImpl(
        const GrShaderCaps& caps) const {
    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrBitmapTextGeoProc)

#if GR_TEST_UTILS

GrGeometryProcessor* GrBitmapTextGeoProc::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();

    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(d->fRandom, wrapModes);
    GrSamplerState samplerState(wrapModes, d->fRandom->nextBool()
                                                   ? GrSamplerState::Filter::kLinear
                                                   : GrSamplerState::Filter::kNearest);

    MaskFormat format;
    switch (ct) {
        case GrColorType::kAlpha_8:
            format = MaskFormat::kA8;
            break;
        case GrColorType::kBGR_565:
            format = MaskFormat::kA565;
            break;
        case GrColorType::kRGBA_8888:
        default:  // It doesn't really matter that color type and mask format agree.
            format = MaskFormat::kARGB;
            break;
    }

    GrColor color = GrTest::RandomColor(d->fRandom);
    bool wideColor = d->fRandom->nextBool();
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
    bool usesW = d->fRandom->nextBool();
    return GrBitmapTextGeoProc::Make(d->allocator(), *d->caps()->shaderCaps(),
                                     SkPMColor4f::FromBytes_RGBA(color),
                                     wideColor,
                                     &view, 1, samplerState, format,
                                     localMatrix, usesW);
}
#endif
