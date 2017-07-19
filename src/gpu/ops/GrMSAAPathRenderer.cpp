/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMSAAPathRenderer.h"
#include "GrAuditTrail.h"
#include "GrClip.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrFixedClip.h"
#include "GrMesh.h"
#include "GrOpFlushState.h"
#include "GrPathStencilSettings.h"
#include "GrPathUtils.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkAutoMalloc.h"
#include "SkGeometry.h"
#include "SkTraceEvent.h"
#include "gl/GrGLVaryingHandler.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUtil.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "ops/GrMeshDrawOp.h"
#include "ops/GrRectOpFactory.h"

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

namespace {

class MSAAQuadProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(const SkMatrix& viewMatrix) {
        return new MSAAQuadProcessor(viewMatrix);
    }

    ~MSAAQuadProcessor() override {}

    const char* name() const override { return "MSAAQuadProcessor"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inUV() const { return fInUV; }
    const Attribute* inColor() const { return fInColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }

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
                                 qp.inPosition()->fName, SkMatrix::I(),
                                 args.fFPCoordTransformHandler);

            GrGLSLPPFragmentBuilder* fsBuilder = args.fFragBuilder;
            fsBuilder->codeAppendf("if (%s.x * %s.x >= %s.y) discard;", uv.fsIn(), uv.fsIn(),
                                                                        uv.fsIn());
            fsBuilder->codeAppendf("%s = vec4(1.0);", args.fOutputCoverage);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
            const MSAAQuadProcessor& qp = gp.cast<MSAAQuadProcessor>();
            uint32_t key = 0;
            key |= qp.viewMatrix().hasPerspective() ? 0x1 : 0x0;
            key |= qp.viewMatrix().isIdentity() ? 0x2: 0x0;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& gp,
                     FPCoordTransformIter&& transformIter) override {
            const MSAAQuadProcessor& qp = gp.cast<MSAAQuadProcessor>();
            if (!qp.viewMatrix().isIdentity()) {
                float viewMatrix[3 * 3];
                GrGLSLGetMatrix<3>(viewMatrix, qp.viewMatrix());
                pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
            }
            this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;

        UniformHandle fViewMatrixUniform;
    };

    virtual void getGLSLProcessorKey(const GrShaderCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor(*this);
    }

private:
    MSAAQuadProcessor(const SkMatrix& viewMatrix)
        : fViewMatrix(viewMatrix) {
        this->initClassID<MSAAQuadProcessor>();
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                             kHigh_GrSLPrecision);
        fInUV = &this->addVertexAttrib("inUV", kVec2f_GrVertexAttribType, kHigh_GrSLPrecision);
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        this->setSampleShading(1.0f);
    }

    const Attribute* fInPosition;
    const Attribute* fInUV;
    const Attribute* fInColor;
    SkMatrix         fViewMatrix;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

class MSAAPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkPath& path, GrAAType aaType,
                                          const SkMatrix& viewMatrix, const SkRect& devBounds,
                                          const GrUserStencilSettings* stencilSettings) {
        int contourCount;
        int maxLineVertices;
        int maxQuadVertices;
        ComputeWorstCasePointCount(path, viewMatrix, &contourCount, &maxLineVertices,
                                   &maxQuadVertices);
        bool isIndexed = contourCount > 1;
        if (isIndexed &&
            (maxLineVertices > kMaxIndexedVertexCnt || maxQuadVertices > kMaxIndexedVertexCnt)) {
            return nullptr;
        }

        return Helper::FactoryHelper<MSAAPathOp>(std::move(paint), path, aaType, viewMatrix,
                                                 devBounds, maxLineVertices, maxQuadVertices,
                                                 isIndexed, stencilSettings);
    }

    const char* name() const override { return "MSAAPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Indexed: %d\n", fIsIndexed);
        for (const auto& path : fPaths) {
            string.appendf("Color: 0x%08x\n", path.fColor);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    MSAAPathOp(const Helper::MakeArgs& helperArgs, GrColor color, const SkPath& path,
               GrAAType aaType, const SkMatrix& viewMatrix, const SkRect& devBounds,
               int maxLineVertices, int maxQuadVertices, bool isIndexed,
               const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(helperArgs, aaType, stencilSettings)
            , fViewMatrix(viewMatrix)
            , fMaxLineVertices(maxLineVertices)
            , fMaxQuadVertices(maxQuadVertices)
            , fIsIndexed(isIndexed) {
        fPaths.emplace_back(PathInfo{color, path});
        this->setBounds(devBounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone,
                                            &fPaths.front().fColor);
    }

private:
    static void ComputeWorstCasePointCount(const SkPath& path, const SkMatrix& m, int* subpaths,
                                           int* outLinePointCount, int* outQuadPointCount) {
        SkScalar tolerance = GrPathUtils::scaleToleranceToSrc(kTolerance, m, path.getBounds());
        int linePointCount = 0;
        int quadPointCount = 0;
        *subpaths = 1;

        bool first = true;

        SkPath::Iter iter(path, true);
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
                    converter.computeQuads(pts, weight, tolerance);
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
                    GrPathUtils::convertCubicToQuads(pts, tolerance, &quadPts);
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
        if (fMaxLineVertices == 0) {
            SkASSERT(fMaxQuadVertices == 0);
            return;
        }

        GrPrimitiveType primitiveType = fIsIndexed ? GrPrimitiveType::kTriangles
                                                   : GrPrimitiveType::kTriangleFan;

        // allocate vertex / index buffers
        const GrBuffer* lineVertexBuffer;
        int firstLineVertex;
        MSAALineVertices lines;
        int lineVertexStride = sizeof(MSAALineVertices::Vertex);
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
        int quadVertexStride = sizeof(MSAAQuadVertices::Vertex);
        SkAutoMalloc quadVertexPtr(fMaxQuadVertices * quadVertexStride);
        quads.vertices = (MSAAQuadVertices::Vertex*) quadVertexPtr.get();
        quads.nextVertex = quads.vertices;
        SkDEBUGCODE(quads.verticesEnd = quads.vertices + fMaxQuadVertices;)

        const GrBuffer* lineIndexBuffer = nullptr;
        int firstLineIndex;
        if (fIsIndexed) {
            lines.indices =
                    target->makeIndexSpace(3 * fMaxLineVertices, &lineIndexBuffer, &firstLineIndex);
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
            quads.indices = (uint16_t*)sk_malloc_throw(3 * fMaxQuadVertices * sizeof(uint16_t));
            quadIndexPtr.reset(quads.indices);
            quads.nextIndex = quads.indices;
        } else {
            quads.indices = nullptr;
            quads.nextIndex = nullptr;
        }
        // fill buffers
        for (int i = 0; i < fPaths.count(); i++) {
            const PathInfo& pathInfo = fPaths[i];
            if (!this->createGeom(lines,
                                  quads,
                                  pathInfo.fPath,
                                  fViewMatrix,
                                  pathInfo.fColor,
                                  fIsIndexed)) {
                return;
            }
        }
        int lineVertexOffset = (int) (lines.nextVertex - lines.vertices);
        int lineIndexOffset = (int) (lines.nextIndex - lines.indices);
        SkASSERT(lineVertexOffset <= fMaxLineVertices && lineIndexOffset <= 3 * fMaxLineVertices);
        int quadVertexOffset = (int) (quads.nextVertex - quads.vertices);
        int quadIndexOffset = (int) (quads.nextIndex - quads.indices);
        SkASSERT(quadVertexOffset <= fMaxQuadVertices && quadIndexOffset <= 3 * fMaxQuadVertices);

        const GrPipeline* pipeline = fHelper.makePipeline(target);

        if (lineVertexOffset) {
            sk_sp<GrGeometryProcessor> lineGP;
            {
                using namespace GrDefaultGeoProcFactory;
                lineGP = GrDefaultGeoProcFactory::Make(Color(Color::kPremulGrColorAttribute_Type),
                                                       Coverage::kSolid_Type,
                                                       LocalCoords(LocalCoords::kUnused_Type),
                                                       fViewMatrix);
            }
            SkASSERT(lineVertexStride == lineGP->getVertexStride());

            GrMesh lineMeshes(primitiveType);
            if (!fIsIndexed) {
                lineMeshes.setNonIndexedNonInstanced(lineVertexOffset);
            } else {
                lineMeshes.setIndexed(lineIndexBuffer, lineIndexOffset, firstLineIndex,
                                      0, lineVertexOffset - 1);
            }
            lineMeshes.setVertexData(lineVertexBuffer, firstLineVertex);

            // We can get line vertices from path moveTos with no actual segments and thus no index
            // count. We assert that indexed draws contain a positive index count, so bail here in
            // that case.
            if (!fIsIndexed || lineIndexOffset) {
                target->draw(lineGP.get(), pipeline, lineMeshes);
            }
        }

        if (quadVertexOffset) {
            sk_sp<const GrGeometryProcessor> quadGP(MSAAQuadProcessor::Create(fViewMatrix));
            SkASSERT(quadVertexStride == quadGP->getVertexStride());

            const GrBuffer* quadVertexBuffer;
            int firstQuadVertex;
            MSAAQuadVertices::Vertex* quadVertices = (MSAAQuadVertices::Vertex*)
                    target->makeVertexSpace(quadVertexStride, quadVertexOffset, &quadVertexBuffer,
                                            &firstQuadVertex);
            memcpy(quadVertices, quads.vertices, quadVertexStride * quadVertexOffset);
            GrMesh quadMeshes(GrPrimitiveType::kTriangles);
            if (!fIsIndexed) {
                quadMeshes.setNonIndexedNonInstanced(quadVertexOffset);
            } else {
                const GrBuffer* quadIndexBuffer;
                int firstQuadIndex;
                uint16_t* quadIndices = (uint16_t*) target->makeIndexSpace(quadIndexOffset,
                                                                           &quadIndexBuffer,
                                                                           &firstQuadIndex);
                memcpy(quadIndices, quads.indices, sizeof(uint16_t) * quadIndexOffset);
                quadMeshes.setIndexed(quadIndexBuffer, quadIndexOffset, firstQuadIndex,
                                      0, quadVertexOffset - 1);
            }
            quadMeshes.setVertexData(quadVertexBuffer, firstQuadVertex);
            target->draw(quadGP.get(), pipeline, quadMeshes);
        }
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        MSAAPathOp* that = t->cast<MSAAPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (this->bounds().intersects(that->bounds())) {
            return false;
        }

        if (!fViewMatrix.cheapEqualTo(that->fViewMatrix)) {
            return false;
        }

        // If we grow to include 2+ paths we will be indexed.
        if (((fMaxLineVertices + that->fMaxLineVertices) > kMaxIndexedVertexCnt) ||
            ((fMaxQuadVertices + that->fMaxQuadVertices) > kMaxIndexedVertexCnt)) {
            return false;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        this->joinBounds(*that);
        fIsIndexed = true;
        fMaxLineVertices += that->fMaxLineVertices;
        fMaxQuadVertices += that->fMaxQuadVertices;
        return true;
    }

    bool createGeom(MSAALineVertices& lines,
                    MSAAQuadVertices& quads,
                    const SkPath& path,
                    const SkMatrix& m,
                    SkColor color,
                    bool isIndexed) const {
        {
            const SkScalar tolerance = GrPathUtils::scaleToleranceToSrc(kTolerance, m,
                                                                        path.getBounds());
            uint16_t subpathIdxStart = (uint16_t) (lines.nextVertex - lines.vertices);

            SkPoint pts[4];

            bool first = true;
            SkPath::Iter iter(path, true);

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
                        const SkPoint* quadPts = converter.computeQuads(pts, weight, tolerance);
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
                        GrPathUtils::convertCubicToQuads(pts, tolerance, &quadPts);
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

    // Lines and quads may render with an index buffer. However, we don't have any support for
    // overflowing the max index.
    static constexpr int kMaxIndexedVertexCnt = SK_MaxU16 / 3;
    struct PathInfo {
        GrColor  fColor;
        SkPath   fPath;
    };

    Helper fHelper;
    SkSTArray<1, PathInfo, true> fPaths;
    SkMatrix fViewMatrix;
    int fMaxLineVertices;
    int fMaxQuadVertices;
    bool fIsIndexed;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

bool GrMSAAPathRenderer::internalDrawPath(GrRenderTargetContext* renderTargetContext,
                                          GrPaint&& paint,
                                          GrAAType aaType,
                                          const GrUserStencilSettings& userStencilSettings,
                                          const GrClip& clip,
                                          const SkMatrix& viewMatrix,
                                          const GrShape& shape,
                                          bool stencilOnly) {
    SkASSERT(shape.style().isSimpleFill());
    SkPath path;
    shape.asPath(&path);

    const GrUserStencilSettings* passes[2] = {nullptr, nullptr};
    bool                         reverse = false;

    if (single_pass_shape(shape)) {
        if (stencilOnly) {
            passes[0] = &gDirectToStencil;
        } else {
            passes[0] = &userStencilSettings;
        }
    } else {
        switch (path.getFillType()) {
            case SkPath::kInverseEvenOdd_FillType:
                reverse = true;
                // fallthrough
            case SkPath::kEvenOdd_FillType:
                passes[0] = &gEOStencilPass;
                if (!stencilOnly) {
                    passes[1] = reverse ? &gInvEOColorPass : &gEOColorPass;
                }
                break;

            case SkPath::kInverseWinding_FillType:
                reverse = true;
                // fallthrough
            case SkPath::kWinding_FillType:
                passes[0] = &gWindStencilPass;
                if (!stencilOnly) {
                    passes[1] = reverse ? &gInvWindColorPass : &gWindColorPass;
                }
                break;
            default:
                SkDEBUGFAIL("Unknown path fFill!");
                return false;
        }
    }

    SkRect devBounds;
    GetPathDevBounds(path, renderTargetContext->width(), renderTargetContext->height(), viewMatrix,
                     &devBounds);

    SkASSERT(passes[0]);
    {  // First pass
        bool firstPassIsStencil = stencilOnly || passes[1];
        // If we have a cover pass then we ignore the paint in the first pass and apply it in the
        // second.
        GrPaint::MoveOrNew firstPassPaint(paint, firstPassIsStencil);
        if (firstPassIsStencil) {
            firstPassPaint.paint().setXPFactory(GrDisableColorXPFactory::Get());
        }
        std::unique_ptr<GrDrawOp> op = MSAAPathOp::Make(std::move(firstPassPaint), path, aaType,
                                                        viewMatrix, devBounds, passes[0]);
        if (!op) {
            return false;
        }
        renderTargetContext->addDrawOp(clip, std::move(op));
    }

    if (passes[1]) {
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
        const SkMatrix& viewM =
                (reverse && viewMatrix.hasPerspective()) ? SkMatrix::I() : viewMatrix;
        renderTargetContext->addDrawOp(
                clip,
                GrRectOpFactory::MakeNonAAFillWithLocalMatrix(std::move(paint), viewM, localMatrix,
                                                              bounds, aaType, passes[1]));
    }
    return true;
}

bool GrMSAAPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // If we aren't a single_pass_shape, we require stencil buffers.
    if (!single_pass_shape(*args.fShape) && args.fCaps->avoidStencilBuffers()) {
        return false;
    }
    // This path renderer only fills and relies on MSAA for antialiasing. Stroked shapes are
    // handled by passing on the original shape and letting the caller compute the stroked shape
    // which will have a fill style.
    return args.fShape->style().isSimpleFill() && (GrAAType::kCoverage != args.fAAType);
}

bool GrMSAAPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrMSAAPathRenderer::onDrawPath");
    SkTLazy<GrShape> tmpShape;
    const GrShape* shape = args.fShape;
    if (shape->style().applies()) {
        SkScalar styleScale = GrStyle::MatrixToScaleFactor(*args.fViewMatrix);
        tmpShape.init(args.fShape->applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale));
        shape = tmpShape.get();
    }
    return this->internalDrawPath(args.fRenderTargetContext,
                                  std::move(args.fPaint),
                                  args.fAAType,
                                  *args.fUserStencilSettings,
                                  *args.fClip,
                                  *args.fViewMatrix,
                                  *shape,
                                  false);
}

void GrMSAAPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrMSAAPathRenderer::onStencilPath");
    SkASSERT(args.fShape->style().isSimpleFill());
    SkASSERT(!args.fShape->mayBeInverseFilledAfterStyling());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());

    this->internalDrawPath(args.fRenderTargetContext, std::move(paint), args.fAAType,
                           GrUserStencilSettings::kUnused, *args.fClip, *args.fViewMatrix,
                           *args.fShape, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
