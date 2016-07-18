/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMSAAPathRenderer.h"

#include "GrAuditTrail.h"
#include "GrBatchFlushState.h"
#include "GrClip.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPathStencilSettings.h"
#include "GrPathUtils.h"
#include "GrPipelineBuilder.h"
#include "GrMesh.h"
#include "SkGeometry.h"
#include "SkTraceEvent.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUtil.h"
#include "gl/GrGLVaryingHandler.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrVertexBatch.h"

static const float kTolerance = 0.5f;

////////////////////////////////////////////////////////////////////////////////
// Helpers for drawPath

static inline bool single_pass_shape(const GrShape& shape) {
    if (!shape.inverseFilled()) {
        return shape.knownToBeConvex();
    }
    return false;
}

GrPathRenderer::StencilSupport GrMSAAPathRenderer::onGetStencilSupport(const GrShape& shape) const {
    if (single_pass_shape(shape)) {
        return GrPathRenderer::kNoRestriction_StencilSupport;
    } else {
        return GrPathRenderer::kStencilOnly_StencilSupport;
    }
}

struct MSAALineVertices {
    struct Vertex {
        SkPoint fPosition;
        SkColor fColor;
    };
    Vertex* vertices;
    Vertex* nextVertex;
#ifdef SK_DEBUG
    Vertex* verticesEnd;
#endif
    uint16_t* indices;
    uint16_t* nextIndex;
};

struct MSAAQuadVertices {
    struct Vertex {
        SkPoint fPosition;
        SkPoint fUV;
        SkColor fColor;
    };
    Vertex* vertices;
    Vertex* nextVertex;
#ifdef SK_DEBUG
    Vertex* verticesEnd;
#endif
    uint16_t* indices;
    uint16_t* nextIndex;
};

static inline void append_contour_edge_indices(uint16_t fanCenterIdx,
                                               uint16_t edgeV0Idx,
                                               MSAALineVertices& lines) {
    *(lines.nextIndex++) = fanCenterIdx;
    *(lines.nextIndex++) = edgeV0Idx;
    *(lines.nextIndex++) = edgeV0Idx + 1;
}

static inline void add_quad(MSAALineVertices& lines, MSAAQuadVertices& quads, const SkPoint pts[],
                            SkColor color, bool indexed, uint16_t subpathLineIdxStart) {
    SkASSERT(lines.nextVertex < lines.verticesEnd);
    *lines.nextVertex = { pts[2], color };
    if (indexed) {
        int prevIdx = (uint16_t) (lines.nextVertex - lines.vertices - 1);
        if (prevIdx > subpathLineIdxStart) {
            append_contour_edge_indices(subpathLineIdxStart, prevIdx, lines);
        }
    }
    lines.nextVertex++;

    SkASSERT(quads.nextVertex + 2 < quads.verticesEnd);
    // the texture coordinates are drawn from the Loop-Blinn rendering algorithm
    *(quads.nextVertex++) = { pts[0], SkPoint::Make(0.0, 0.0), color };
    *(quads.nextVertex++) = { pts[1], SkPoint::Make(0.5, 0.0), color };
    *(quads.nextVertex++) = { pts[2], SkPoint::Make(1.0, 1.0), color };
    if (indexed) {
        uint16_t offset = (uint16_t) (quads.nextVertex - quads.vertices) - 3;
        *(quads.nextIndex++) = offset++;
        *(quads.nextIndex++) = offset++;
        *(quads.nextIndex++) = offset++;
    }
}

class MSAAQuadProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(const SkMatrix& viewMatrix) {
        return new MSAAQuadProcessor(viewMatrix);
    }

    virtual ~MSAAQuadProcessor() {}

    const char* name() const override { return "MSAAQuadProcessor"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inUV() const { return fInUV; }
    const Attribute* inColor() const { return fInColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return SkMatrix::I(); }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor(const GrGeometryProcessor& qpr) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const MSAAQuadProcessor& qp = args.fGP.cast<MSAAQuadProcessor>();
            GrGLSLVertexBuilder* vsBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(qp);
            varyingHandler->addPassThroughAttribute(qp.inColor(), args.fOutputColor);

            GrGLSLVertToFrag uv(kVec2f_GrSLType);
            varyingHandler->addVarying("uv", &uv, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = %s;", uv.vsOut(), qp.inUV()->fName);

            // Setup position
            this->setupPosition(vsBuilder, uniformHandler, gpArgs, qp.inPosition()->fName,
                                qp.viewMatrix(), &fViewMatrixUniform);

            // emit transforms
            this->emitTransforms(vsBuilder, varyingHandler, uniformHandler, gpArgs->fPositionVar,
                                 qp.inPosition()->fName, SkMatrix::I(), args.fTransformsIn,
                                 args.fTransformsOut);

            GrGLSLPPFragmentBuilder* fsBuilder = args.fFragBuilder;
            fsBuilder->codeAppendf("if (%s.x * %s.x >= %s.y) discard;", uv.fsIn(), uv.fsIn(),
                                                                        uv.fsIn());
            fsBuilder->codeAppendf("%s = vec4(1.0);", args.fOutputCoverage);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrGLSLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const MSAAQuadProcessor& qp = gp.cast<MSAAQuadProcessor>();
            uint32_t key = 0;
            key |= qp.viewMatrix().hasPerspective() ? 0x1 : 0x0;
            key |= qp.viewMatrix().isIdentity() ? 0x2: 0x0;
            b->add32(key);
        }

        virtual void setData(const GrGLSLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp) override {
            const MSAAQuadProcessor& qp = gp.cast<MSAAQuadProcessor>();
            if (!qp.viewMatrix().isIdentity()) {
                float viewMatrix[3 * 3];
                GrGLSLGetMatrix<3>(viewMatrix, qp.viewMatrix());
                pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
            }
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLSLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<MSAAQuadProcessor>(primProc, pdman, index, transforms);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;

        UniformHandle fViewMatrixUniform;
    };

    virtual void getGLSLProcessorKey(const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor(*this);
    }

private:
    MSAAQuadProcessor(const SkMatrix& viewMatrix)
        : fViewMatrix(viewMatrix) {
        this->initClassID<MSAAQuadProcessor>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
        fInUV = &this->addVertexAttrib(Attribute("inUV", kVec2f_GrVertexAttribType,
                                                 kHigh_GrSLPrecision));
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        this->setSampleShading(1.0f);
    }

    const Attribute* fInPosition;
    const Attribute* fInUV;
    const Attribute* fInColor;
    SkMatrix         fViewMatrix;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

class MSAAPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        GrColor fColor;
        SkPath fPath;
        SkScalar fTolerance;
    };

    static MSAAPathBatch* Create(const Geometry& geometry, const SkMatrix& viewMatrix,
                               const SkRect& devBounds) {
        return new MSAAPathBatch(geometry, viewMatrix, devBounds);
    }

    const char* name() const override { return "MSAAPathBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setKnownSingleComponent(0xff);
    }

    bool isValid() const {
        return !fIsIndexed || fMaxLineIndices <= SK_MaxU16;
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
    }

    void computeWorstCasePointCount(const SkPath& path, int* subpaths, SkScalar tol,
                                    int* outLinePointCount, int* outQuadPointCount) const {
        int linePointCount = 0;
        int quadPointCount = 0;
        *subpaths = 1;

        bool first = true;

        SkPath::Iter iter(path, false);
        SkPath::Verb verb;

        SkPoint pts[4];
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    linePointCount += 1;
                    break;
                case SkPath::kConic_Verb: {
                    SkScalar weight = iter.conicWeight();
                    SkAutoConicToQuads converter;
                    converter.computeQuads(pts, weight, kTolerance);
                    int quadPts = converter.countQuads();
                    linePointCount += quadPts;
                    quadPointCount += 3 * quadPts;
                }
                case SkPath::kQuad_Verb:
                    linePointCount += 1;
                    quadPointCount += 3;
                    break;
                case SkPath::kCubic_Verb: {
                    SkSTArray<15, SkPoint, true> quadPts;
                    GrPathUtils::convertCubicToQuads(pts, kTolerance, &quadPts);
                    int count = quadPts.count();
                    linePointCount += count / 3;
                    quadPointCount += count;
                    break;
                }
                case SkPath::kMove_Verb:
                    linePointCount += 1;
                    if (!first) {
                        ++(*subpaths);
                    }
                    break;
                default:
                    break;
            }
            first = false;
        }
        *outLinePointCount = linePointCount;
        *outQuadPointCount = quadPointCount;
    }

    void onPrepareDraws(Target* target) const override {
        SkASSERT(this->isValid());
        if (fMaxLineVertices == 0) {
            SkASSERT(fMaxQuadVertices == 0);
            return;
        }

        GrPrimitiveType primitiveType = fIsIndexed ? kTriangles_GrPrimitiveType
                                                   : kTriangleFan_GrPrimitiveType;

        // allocate vertex / index buffers
        const GrBuffer* lineVertexBuffer;
        int firstLineVertex;
        MSAALineVertices lines;
        size_t lineVertexStride = sizeof(MSAALineVertices::Vertex);
        lines.vertices = (MSAALineVertices::Vertex*) target->makeVertexSpace(lineVertexStride,
                                                                             fMaxLineVertices,
                                                                             &lineVertexBuffer,
                                                                             &firstLineVertex);
        if (!lines.vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        lines.nextVertex = lines.vertices;
        SkDEBUGCODE(lines.verticesEnd = lines.vertices + fMaxLineVertices;)

        MSAAQuadVertices quads;
        size_t quadVertexStride = sizeof(MSAAQuadVertices::Vertex);
        SkAutoFree quadVertexPtr(sk_malloc_throw(fMaxQuadVertices * quadVertexStride));
        quads.vertices = (MSAAQuadVertices::Vertex*) quadVertexPtr.get();
        quads.nextVertex = quads.vertices;
        SkDEBUGCODE(quads.verticesEnd = quads.vertices + fMaxQuadVertices;)

        const GrBuffer* lineIndexBuffer = nullptr;
        int firstLineIndex;
        if (fIsIndexed) {
            lines.indices = target->makeIndexSpace(fMaxLineIndices, &lineIndexBuffer,
                                                   &firstLineIndex);
            if (!lines.indices) {
                SkDebugf("Could not allocate indices\n");
                return;
            }
            lines.nextIndex = lines.indices;
        } else {
            lines.indices = nullptr;
            lines.nextIndex = nullptr;
        }

        SkAutoFree quadIndexPtr;
        if (fIsIndexed) {
            quads.indices = (uint16_t*) sk_malloc_throw(fMaxQuadIndices * sizeof(uint16_t));
            quadIndexPtr.set(quads.indices);
            quads.nextIndex = quads.indices;
        } else {
            quads.indices = nullptr;
            quads.nextIndex = nullptr;
        }

        // fill buffers
        for (int i = 0; i < fGeoData.count(); i++) {
            const Geometry& args = fGeoData[i];

            if (!this->createGeom(lines,
                                  quads,
                                  args.fPath,
                                  args.fTolerance,
                                  fViewMatrix,
                                  args.fColor,
                                  fIsIndexed)) {
                return;
            }
        }
        int lineVertexOffset = (int) (lines.nextVertex - lines.vertices);
        int lineIndexOffset = (int) (lines.nextIndex - lines.indices);
        SkASSERT(lineVertexOffset <= fMaxLineVertices && lineIndexOffset <= fMaxLineIndices);
        int quadVertexOffset = (int) (quads.nextVertex - quads.vertices);
        int quadIndexOffset = (int) (quads.nextIndex - quads.indices);
        SkASSERT(quadVertexOffset <= fMaxQuadVertices && quadIndexOffset <= fMaxQuadIndices);

        if (lineVertexOffset) {
            sk_sp<GrGeometryProcessor> lineGP;
            {
                using namespace GrDefaultGeoProcFactory;
                lineGP = GrDefaultGeoProcFactory::Make(Color(Color::kAttribute_Type),
                                                       Coverage(255),
                                                       LocalCoords(LocalCoords::kUnused_Type),
                                                       fViewMatrix);
            }
            SkASSERT(lineVertexStride == lineGP->getVertexStride());

            GrMesh lineMeshes;
            if (fIsIndexed) {
                lineMeshes.initIndexed(primitiveType, lineVertexBuffer, lineIndexBuffer,
                                         firstLineVertex, firstLineIndex, lineVertexOffset,
                                         lineIndexOffset);
            } else {
                lineMeshes.init(primitiveType, lineVertexBuffer, firstLineVertex,
                                  lineVertexOffset);
            }
            target->draw(lineGP.get(), lineMeshes);
        }

        if (quadVertexOffset) {
            SkAutoTUnref<const GrGeometryProcessor> quadGP(MSAAQuadProcessor::Create(fViewMatrix));
            SkASSERT(quadVertexStride == quadGP->getVertexStride());

            const GrBuffer* quadVertexBuffer;
            int firstQuadVertex;
            MSAAQuadVertices::Vertex* quadVertices = (MSAAQuadVertices::Vertex*)
                    target->makeVertexSpace(quadVertexStride, quadVertexOffset, &quadVertexBuffer,
                                            &firstQuadVertex);
            memcpy(quadVertices, quads.vertices, quadVertexStride * quadVertexOffset);
            GrMesh quadMeshes;
            if (fIsIndexed) {
                const GrBuffer* quadIndexBuffer;
                int firstQuadIndex;
                uint16_t* quadIndices = (uint16_t*) target->makeIndexSpace(quadIndexOffset,
                                                                           &quadIndexBuffer,
                                                                           &firstQuadIndex);
                memcpy(quadIndices, quads.indices, sizeof(uint16_t) * quadIndexOffset);
                quadMeshes.initIndexed(kTriangles_GrPrimitiveType, quadVertexBuffer,
                                       quadIndexBuffer, firstQuadVertex, firstQuadIndex,
                                       quadVertexOffset, quadIndexOffset);
            } else {
                quadMeshes.init(kTriangles_GrPrimitiveType, quadVertexBuffer, firstQuadVertex,
                                quadVertexOffset);
            }
            target->draw(quadGP, quadMeshes);
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    MSAAPathBatch(const Geometry& geometry, const SkMatrix& viewMatrix, const SkRect& devBounds)
        : INHERITED(ClassID())
        , fViewMatrix(viewMatrix) {
        fGeoData.push_back(geometry);
        this->setBounds(devBounds);
        int contourCount;
        this->computeWorstCasePointCount(geometry.fPath, &contourCount, kTolerance,
                                         &fMaxLineVertices, &fMaxQuadVertices);
        fMaxLineIndices = fMaxLineVertices * 3;
        fMaxQuadIndices = fMaxQuadVertices * 3;
        fIsIndexed = contourCount > 1;
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        MSAAPathBatch* that = t->cast<MSAAPathBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                     that->bounds(), caps)) {
            return false;
        }

        if (!fViewMatrix.cheapEqualTo(that->fViewMatrix)) {
            return false;
        }

        if ((fMaxLineIndices + that->fMaxLineIndices > SK_MaxU16) ||
            (fMaxQuadIndices + that->fMaxQuadIndices > SK_MaxU16)) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        fIsIndexed = true;
        fMaxLineVertices += that->fMaxLineVertices;
        fMaxQuadVertices += that->fMaxQuadVertices;
        fMaxLineIndices += that->fMaxLineIndices;
        fMaxQuadIndices += that->fMaxQuadIndices;
        return true;
    }

    bool createGeom(MSAALineVertices& lines,
                    MSAAQuadVertices& quads,
                    const SkPath& path,
                    SkScalar srcSpaceTol,
                    const SkMatrix& m,
                    SkColor color,
                    bool isIndexed) const {
        {
            uint16_t subpathIdxStart = (uint16_t) (lines.nextVertex - lines.vertices);

            SkPoint pts[4];

            bool first = true;
            SkPath::Iter iter(path, false);

            bool done = false;
            while (!done) {
                SkPath::Verb verb = iter.next(pts);
                switch (verb) {
                    case SkPath::kMove_Verb:
                        if (!first) {
                            uint16_t currIdx = (uint16_t) (lines.nextVertex - lines.vertices);
                            subpathIdxStart = currIdx;
                        }
                        SkASSERT(lines.nextVertex < lines.verticesEnd);
                        *(lines.nextVertex++) = { pts[0], color };
                        break;
                    case SkPath::kLine_Verb:
                        if (isIndexed) {
                            uint16_t prevIdx = (uint16_t) (lines.nextVertex - lines.vertices - 1);
                            if (prevIdx > subpathIdxStart) {
                                append_contour_edge_indices(subpathIdxStart, prevIdx, lines);
                            }
                        }
                        SkASSERT(lines.nextVertex < lines.verticesEnd);
                        *(lines.nextVertex++) = { pts[1], color };
                        break;
                    case SkPath::kConic_Verb: {
                        SkScalar weight = iter.conicWeight();
                        SkAutoConicToQuads converter;
                        const SkPoint* quadPts = converter.computeQuads(pts, weight,
                                                                        kTolerance);
                        for (int i = 0; i < converter.countQuads(); ++i) {
                            add_quad(lines, quads, quadPts + i * 2, color, isIndexed,
                                     subpathIdxStart);
                        }
                        break;
                    }
                    case SkPath::kQuad_Verb: {
                        add_quad(lines, quads, pts, color, isIndexed, subpathIdxStart);
                        break;
                    }
                    case SkPath::kCubic_Verb: {
                        SkSTArray<15, SkPoint, true> quadPts;
                        GrPathUtils::convertCubicToQuads(pts, kTolerance, &quadPts);
                        int count = quadPts.count();
                        for (int i = 0; i < count; i += 3) {
                            add_quad(lines, quads, &quadPts[i], color, isIndexed, subpathIdxStart);
                        }
                        break;
                    }
                    case SkPath::kClose_Verb:
                        break;
                    case SkPath::kDone_Verb:
                        done = true;
                }
                first = false;
            }
        }
        return true;
    }

    SkSTArray<1, Geometry, true> fGeoData;

    SkMatrix fViewMatrix;
    int fMaxLineVertices;
    int fMaxQuadVertices;
    int fMaxLineIndices;
    int fMaxQuadIndices;
    bool fIsIndexed;

    typedef GrVertexBatch INHERITED;
};

bool GrMSAAPathRenderer::internalDrawPath(GrDrawContext* drawContext,
                                          const GrPaint& paint,
                                          const GrUserStencilSettings* userStencilSettings,
                                          const GrClip& clip,
                                          GrColor color,
                                          const SkMatrix& viewMatrix,
                                          const GrShape& shape,
                                          bool stencilOnly) {
    SkASSERT(shape.style().isSimpleFill());
    SkPath path;
    shape.asPath(&path);

    int                          passCount = 0;
    const GrUserStencilSettings* passes[3];
    GrPipelineBuilder::DrawFace  drawFace[3];
    bool                         reverse = false;
    bool                         lastPassIsBounds;

    if (single_pass_shape(shape)) {
        passCount = 1;
        if (stencilOnly) {
            passes[0] = &gDirectToStencil;
        } else {
            passes[0] = nullptr;
        }
        drawFace[0] = GrPipelineBuilder::kBoth_DrawFace;
        lastPassIsBounds = false;
    } else {
        switch (path.getFillType()) {
            case SkPath::kInverseEvenOdd_FillType:
                reverse = true;
                // fallthrough
            case SkPath::kEvenOdd_FillType:
                passes[0] = &gEOStencilPass;
                if (stencilOnly) {
                    passCount = 1;
                    lastPassIsBounds = false;
                } else {
                    passCount = 2;
                    lastPassIsBounds = true;
                    if (reverse) {
                        passes[1] = &gInvEOColorPass;
                    } else {
                        passes[1] = &gEOColorPass;
                    }
                }
                drawFace[0] = drawFace[1] = GrPipelineBuilder::kBoth_DrawFace;
                break;

            case SkPath::kInverseWinding_FillType:
                reverse = true;
                // fallthrough
            case SkPath::kWinding_FillType:
                passes[0] = &gWindStencilSeparateWithWrap;
                passCount = 2;
                drawFace[0] = GrPipelineBuilder::kBoth_DrawFace;
                if (stencilOnly) {
                    lastPassIsBounds = false;
                    --passCount;
                } else {
                    lastPassIsBounds = true;
                    drawFace[passCount-1] = GrPipelineBuilder::kBoth_DrawFace;
                    if (reverse) {
                        passes[passCount-1] = &gInvWindColorPass;
                    } else {
                        passes[passCount-1] = &gWindColorPass;
                    }
                }
                break;
            default:
                SkDEBUGFAIL("Unknown path fFill!");
                return false;
        }
    }

    SkRect devBounds;
    GetPathDevBounds(path, drawContext->width(), drawContext->height(), viewMatrix, &devBounds);

    for (int p = 0; p < passCount; ++p) {
        if (lastPassIsBounds && (p == passCount-1)) {
            SkRect bounds;
            SkMatrix localMatrix = SkMatrix::I();
            if (reverse) {
                // draw over the dev bounds (which will be the whole dst surface for inv fill).
                bounds = devBounds;
                SkMatrix vmi;
                // mapRect through persp matrix may not be correct
                if (!viewMatrix.hasPerspective() && viewMatrix.invert(&vmi)) {
                    vmi.mapRect(&bounds);
                } else {
                    if (!viewMatrix.invert(&localMatrix)) {
                        return false;
                    }
                }
            } else {
                bounds = path.getBounds();
            }
            const SkMatrix& viewM = (reverse && viewMatrix.hasPerspective()) ? SkMatrix::I() :
                                                                               viewMatrix;
            SkAutoTUnref<GrDrawBatch> batch(
                    GrRectBatchFactory::CreateNonAAFill(color, viewM, bounds, nullptr,
                                                        &localMatrix));

            GrPipelineBuilder pipelineBuilder(paint, drawContext->mustUseHWAA(paint));
            pipelineBuilder.setDrawFace(drawFace[p]);
            if (passes[p]) {
                pipelineBuilder.setUserStencil(passes[p]);
            } else {
                pipelineBuilder.setUserStencil(userStencilSettings);
            }

            drawContext->drawBatch(pipelineBuilder, clip, batch);
        } else {
            MSAAPathBatch::Geometry geometry;
            geometry.fColor = color;
            geometry.fPath = path;
            geometry.fTolerance = kTolerance;

            SkAutoTUnref<MSAAPathBatch> batch(MSAAPathBatch::Create(geometry, viewMatrix,
                                                                    devBounds));
            if (!batch->isValid()) {
                return false;
            }

            GrPipelineBuilder pipelineBuilder(paint, drawContext->mustUseHWAA(paint));
            pipelineBuilder.setDrawFace(drawFace[p]);
            if (passes[p]) {
                pipelineBuilder.setUserStencil(passes[p]);
            } else {
                pipelineBuilder.setUserStencil(userStencilSettings);
            }
            if (passCount > 1) {
                pipelineBuilder.setDisableColorXPFactory();
            }

            drawContext->drawBatch(pipelineBuilder, clip, batch);
        }
    }
    return true;
}

bool GrMSAAPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This path renderer only fills and relies on MSAA for antialiasing. Stroked shapes are
    // handled by passing on the original shape and letting the caller compute the stroked shape
    // which will have a fill style.
    return args.fShape->style().isSimpleFill() && !args.fAntiAlias;
}

bool GrMSAAPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrMSAAPathRenderer::onDrawPath");
    SkTLazy<GrShape> tmpShape;
    const GrShape* shape = args.fShape;
    if (shape->style().applies()) {
        SkScalar styleScale = GrStyle::MatrixToScaleFactor(*args.fViewMatrix);
        tmpShape.init(args.fShape->applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale));
        shape = tmpShape.get();
    }
    return this->internalDrawPath(args.fDrawContext,
                                  *args.fPaint,
                                  args.fUserStencilSettings,
                                  *args.fClip,
                                  args.fColor,
                                  *args.fViewMatrix,
                                  *shape,
                                  false);
}

void GrMSAAPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrMSAAPathRenderer::onStencilPath");
    SkASSERT(args.fShape->style().isSimpleFill());
    SkASSERT(!args.fShape->mayBeInverseFilledAfterStyling());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Make());
    paint.setAntiAlias(args.fIsAA);

    this->internalDrawPath(args.fDrawContext, paint, &GrUserStencilSettings::kUnused, *args.fClip,
                           GrColor_WHITE, *args.fViewMatrix, *args.fShape, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
