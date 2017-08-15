/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAppliedClip.h"
#include "GrColor.h"
#include "GrGeometryProcessor.h"
#include "GrDrawOpTest.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrQuad.h"
#include "GrTextureProxy.h"
#include "GrResourceProvider.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

class ImageGeometryProcessor : public GrGeometryProcessor {
public:
    struct Vertex {
        SkPoint fPosition;
        SkPoint fTextureCoords;
        GrColor fColor;
    };
    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> csxf,
                                           GrSamplerParams::FilterMode filter) {
        return sk_sp<ImageGeometryProcessor>(
                new ImageGeometryProcessor(std::move(proxy), std::move(csxf), filter));
    }

    const char* name() const override { return "ImageGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& imageGP = proc.cast<ImageGeometryProcessor>();
                this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                if (fColorSpaceXformHelper.isValid()) {
                    fColorSpaceXformHelper.setData(pdman, imageGP.fColorSpaceXform.get());
                }
            }
        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& igp = args.fGP.cast<ImageGeometryProcessor>();
                args.fVaryingHandler->setNoPerspective();
                args.fVaryingHandler->emitAttributes(igp);
                this->writeOutputPosition(args.fVertBuilder, gpArgs, igp.fPositions.fName);
                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     gpArgs->fPositionVar,
                                     igp.fTextureCoords.fName,
                                     args.fFPCoordTransformHandler);
                args.fVaryingHandler->addFlatPassThroughAttribute(&igp.fColors, args.fOutputColor);
                args.fFragBuilder->codeAppend("highp float2 texCoord;");
                args.fVaryingHandler->addPassThroughAttribute(&igp.fTextureCoords, "texCoord");
                args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                args.fFragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                                  args.fTexSamplers[0],
                                                                  "texCoord",
                                                                  kVec2f_GrSLType,
                                                                  &fColorSpaceXformHelper);
                args.fFragBuilder->codeAppend(";");
                args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
            }
            GrGLSLColorSpaceXformHelper fColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    ImageGeometryProcessor(sk_sp<GrTextureProxy> proxy, sk_sp<GrColorSpaceXform> csxf,
                           GrSamplerParams::FilterMode filter)
            : fSampler(std::move(proxy), filter), fColorSpaceXform(std::move(csxf)) {
        this->initClassID<ImageGeometryProcessor>();
        fPositions = this->addVertexAttrib("position", kVec2f_GrVertexAttribType,
                                           kHigh_GrSLPrecision);
        fTextureCoords = this->addVertexAttrib("textureCoords", kVec2f_GrVertexAttribType,
                                               kHigh_GrSLPrecision);
        fColors = this->addVertexAttrib("color", kVec4ub_GrVertexAttribType);
        this->addTextureSampler(&fSampler);
    }

    Attribute fPositions;
    Attribute fTextureCoords;
    Attribute fColors;
    TextureSampler fSampler;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
};

class ImageOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy,
                                          GrSamplerParams::FilterMode filter, GrColor color,
                                          const SkRect srcRect, const SkRect dstRect,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> csxf) {
        return std::unique_ptr<GrDrawOp>(new ImageOp(std::move(proxy), filter, color, srcRect,
                                                     dstRect, viewMatrix, std::move(csxf)));
    }

    ~ImageOp() override { fFinalized ? fProxy->completedRead() : fProxy->unref();  }

    const char* name() const override { return "ImageOp"; }

    SkString dumpInfo() const override {return SkString("");/*
        SkString str;return fHelper.fixedFunctionFlags();
        str.append(GrMeshDrawOp::dumpInfo());
        str.appendf("# combined: %d\n", fRects.count());
        for (int i = 0; i < fRects.count(); ++i) {
            const RectInfo& info = fRects[i];
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        info.fColor, info.fRect.fLeft, info.fRect.fTop, info.fRect.fRight,
                        info.fRect.fBottom);
        }
        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;*/
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        SkASSERT(!fFinalized);
        fFinalized = true;
        fProxy->addPendingRead();
        fProxy->unref();
        return RequiresDstTexture::kNo;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    DEFINE_OP_CLASS_ID

private:
    ImageOp(sk_sp<GrTextureProxy> proxy, GrSamplerParams::FilterMode filter, GrColor color,
            const SkRect& srcRect, const SkRect& dstRect, const SkMatrix& viewMatrix,
            sk_sp<GrColorSpaceXform> csxf)
            : INHERITED(ClassID())
            , fProxy(proxy.release())
            , fFilter(filter)
            , fSrcRect(srcRect)
            , fColorSpaceXform(std::move(csxf))
            , fColor(color)
            , fFinalized(false) {
        fQuad.setFromMappedRect(dstRect, viewMatrix);
        SkRect bounds;
        bounds.setBounds(fQuad.points(), 4);
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override {
        sk_sp<GrGeometryProcessor> gp =
                ImageGeometryProcessor::Make(sk_ref_sp(fProxy), std::move(fColorSpaceXform),
                                             fFilter);
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        const GrPipeline* pipeline = target->allocPipeline(args, GrProcessorSet::MakeEmptySet(),
                                                           target->detachAppliedClip());

        using Vertex = ImageGeometryProcessor::Vertex;
        SkASSERT(gp->getVertexStride() == sizeof(Vertex));

        int vstart;
        const GrBuffer* buffer;
        auto vertices = (Vertex*)target->makeVertexSpace(sizeof(Vertex), 4, &buffer, &vstart);
        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        float iw = 1.f / fProxy->width();
        float ih = 1.f / fProxy->height();
        float tl = iw * fSrcRect.fLeft;
        float tr = iw * fSrcRect.fRight;
        float tt = ih * fSrcRect.fTop;
        float tb = ih * fSrcRect.fBottom;
        if (fProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
            tt = 1.f - tt;
            tb = 1.f - tb;
        }
        vertices[0].fPosition = fQuad.points()[0];
        vertices[0].fTextureCoords = {tl, tt};
        vertices[0].fColor = fColor;
        vertices[1].fPosition = fQuad.points()[3];
        vertices[1].fTextureCoords = {tr, tt};
        vertices[1].fColor = fColor;
        vertices[2].fPosition = fQuad.points()[1];
        vertices[2].fTextureCoords = {tl, tb};
        vertices[2].fColor = fColor;
        vertices[3].fPosition = fQuad.points()[2];
        vertices[3].fTextureCoords = {tr, tb};
        vertices[3].fColor = fColor;

        GrMesh mesh(GrPrimitiveType::kTriangleStrip);
        mesh.setNonIndexedNonInstanced(4);
        mesh.setVertexData(buffer, vstart);
        target->draw(gp.get(), pipeline, mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    GrTextureProxy* fProxy;
    GrSamplerParams::FilterMode fFilter;
    SkRect fSrcRect;
    GrQuad fQuad;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    GrColor fColor;
    bool fFinalized;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrImageOp {

std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy, GrSamplerParams::FilterMode filter,
                               GrColor color, const SkRect& srcRect, const SkRect& dstRect,
                               const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> csxf) {
    SkASSERT(!viewMatrix.hasPerspective());
    return ImageOp::Make(std::move(proxy), filter, color, srcRect, dstRect, viewMatrix,
                         std::move(csxf));
}

}  // namespace GrImageOp
