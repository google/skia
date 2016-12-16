/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAADistanceFieldPathRenderer.h"

#include "GrBatchTest.h"
#include "GrBuffer.h"
#include "GrContext.h"
#include "GrOpFlushState.h"
#include "GrPipelineBuilder.h"
#include "GrResourceProvider.h"
#include "GrSWMaskHelper.h"
#include "GrSurfacePriv.h"
#include "GrTexturePriv.h"
#include "effects/GrDistanceFieldGeoProc.h"
#include "ops/GrMeshDrawOp.h"

#include "SkDistanceFieldGen.h"

#define ATLAS_TEXTURE_WIDTH 2048
#define ATLAS_TEXTURE_HEIGHT 2048
#define PLOT_WIDTH  512
#define PLOT_HEIGHT 256

#define NUM_PLOTS_X   (ATLAS_TEXTURE_WIDTH / PLOT_WIDTH)
#define NUM_PLOTS_Y   (ATLAS_TEXTURE_HEIGHT / PLOT_HEIGHT)

#ifdef DF_PATH_TRACKING
static int g_NumCachedShapes = 0;
static int g_NumFreedShapes = 0;
#endif

// mip levels
static const int kSmallMIP = 32;
static const int kMediumMIP = 73;
static const int kLargeMIP = 162;

// Callback to clear out internal path cache when eviction occurs
void GrAADistanceFieldPathRenderer::HandleEviction(GrBatchAtlas::AtlasID id, void* pr) {
    GrAADistanceFieldPathRenderer* dfpr = (GrAADistanceFieldPathRenderer*)pr;
    // remove any paths that use this plot
    ShapeDataList::Iter iter;
    iter.init(dfpr->fShapeList, ShapeDataList::Iter::kHead_IterStart);
    ShapeData* shapeData;
    while ((shapeData = iter.get())) {
        iter.next();
        if (id == shapeData->fID) {
            dfpr->fShapeCache.remove(shapeData->fKey);
            dfpr->fShapeList.remove(shapeData);
            delete shapeData;
#ifdef DF_PATH_TRACKING
            ++g_NumFreedPaths;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
GrAADistanceFieldPathRenderer::GrAADistanceFieldPathRenderer() : fAtlas(nullptr) {}

GrAADistanceFieldPathRenderer::~GrAADistanceFieldPathRenderer() {
    ShapeDataList::Iter iter;
    iter.init(fShapeList, ShapeDataList::Iter::kHead_IterStart);
    ShapeData* shapeData;
    while ((shapeData = iter.get())) {
        iter.next();
        delete shapeData;
    }

#ifdef DF_PATH_TRACKING
    SkDebugf("Cached shapes: %d, freed shapes: %d\n", g_NumCachedShapes, g_NumFreedShapes);
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool GrAADistanceFieldPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (!args.fShaderCaps->shaderDerivativeSupport()) {
        return false;
    }
    // If the shape has no key then we won't get any reuse.
    if (!args.fShape->hasUnstyledKey()) {
        return false;
    }
    // This only supports filled paths, however, the caller may apply the style to make a filled
    // path and try again.
    if (!args.fShape->style().isSimpleFill()) {
        return false;
    }
    // This does non-inverse coverage-based antialiased fills.
    if (GrAAType::kCoverage != args.fAAType) {
        return false;
    }
    // TODO: Support inverse fill
    if (args.fShape->inverseFilled()) {
        return false;
    }
    // currently don't support perspective
    if (args.fViewMatrix->hasPerspective()) {
        return false;
    }

    // only support paths with bounds within kMediumMIP by kMediumMIP,
    // scaled to have bounds within 2.0f*kLargeMIP by 2.0f*kLargeMIP
    // the goal is to accelerate rendering of lots of small paths that may be scaling
    SkScalar maxScale = args.fViewMatrix->getMaxScale();
    SkRect bounds = args.fShape->styledBounds();
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());

    return maxDim <= kMediumMIP && maxDim * maxScale <= 2.0f*kLargeMIP;
}

////////////////////////////////////////////////////////////////////////////////

// padding around path bounds to allow for antialiased pixels
static const SkScalar kAntiAliasPad = 1.0f;

class AADistanceFieldPathOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    using ShapeData = GrAADistanceFieldPathRenderer::ShapeData;
    using ShapeCache = SkTDynamicHash<ShapeData, ShapeData::Key>;
    using ShapeDataList = GrAADistanceFieldPathRenderer::ShapeDataList;

    static sk_sp<GrDrawOp> Make(GrColor color, const GrShape& shape, const SkMatrix& viewMatrix,
                                GrBatchAtlas* atlas, ShapeCache* shapeCache,
                                ShapeDataList* shapeList, bool gammaCorrect) {
        return sk_sp<GrDrawOp>(new AADistanceFieldPathOp(color, shape, viewMatrix, atlas,
                                                         shapeCache, shapeList, gammaCorrect));
    }

    const char* name() const override { return "AADistanceFieldPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (const auto& geo : fShapes) {
            string.appendf("Color: 0x%08x\n", geo.fColor);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fShapes[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    AADistanceFieldPathOp(GrColor color, const GrShape& shape, const SkMatrix& viewMatrix,
                          GrBatchAtlas* atlas, ShapeCache* shapeCache, ShapeDataList* shapeList,
                          bool gammaCorrect)
            : INHERITED(ClassID()) {
        SkASSERT(shape.hasUnstyledKey());
        fViewMatrix = viewMatrix;
        fShapes.emplace_back(Entry{color, shape});

        fAtlas = atlas;
        fShapeCache = shapeCache;
        fShapeList = shapeList;
        fGammaCorrect = gammaCorrect;

        // Compute bounds
        this->setTransformedBounds(shape.bounds(), viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fShapes[0].fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fShapes[0].fColor);

        fColorIgnored = !overrides.readsColor();
        fUsesLocalCoords = overrides.readsLocalCoords();
        fCoverageIgnored = !overrides.readsCoverage();
    }

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<GrGeometryProcessor>   fGeometryProcessor;
        int fVertexOffset;
        int fInstancesToFlush;
    };

    void onPrepareDraws(Target* target) const override {
        int instanceCount = fShapes.count();

        SkMatrix invert;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&invert)) {
            SkDebugf("Could not invert viewmatrix\n");
            return;
        }

        const SkMatrix& ctm = this->viewMatrix();
        uint32_t flags = 0;
        flags |= ctm.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
        flags |= ctm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
        flags |= fGammaCorrect ? kGammaCorrect_DistanceFieldEffectFlag : 0;

        GrSamplerParams params(SkShader::kRepeat_TileMode, GrSamplerParams::kBilerp_FilterMode);

        FlushInfo flushInfo;

        // Setup GrGeometryProcessor
        GrBatchAtlas* atlas = fAtlas;
        flushInfo.fGeometryProcessor = GrDistanceFieldPathGeoProc::Make(this->color(),
                                                                        this->viewMatrix(),
                                                                        atlas->getTexture(),
                                                                        params,
                                                                        flags,
                                                                        this->usesLocalCoords());

        // allocate vertices
        size_t vertexStride = flushInfo.fGeometryProcessor->getVertexStride();
        SkASSERT(vertexStride == 2 * sizeof(SkPoint) + sizeof(GrColor));

        const GrBuffer* vertexBuffer;
        void* vertices = target->makeVertexSpace(vertexStride,
                                                 kVerticesPerQuad * instanceCount,
                                                 &vertexBuffer,
                                                 &flushInfo.fVertexOffset);
        flushInfo.fVertexBuffer.reset(SkRef(vertexBuffer));
        flushInfo.fIndexBuffer.reset(target->resourceProvider()->refQuadIndexBuffer());
        if (!vertices || !flushInfo.fIndexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        flushInfo.fInstancesToFlush = 0;
        // Pointer to the next set of vertices to write.
        intptr_t offset = reinterpret_cast<intptr_t>(vertices);
        for (int i = 0; i < instanceCount; i++) {
            const Entry& args = fShapes[i];

            // get mip level
            SkScalar maxScale = this->viewMatrix().getMaxScale();
            const SkRect& bounds = args.fShape.bounds();
            SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
            SkScalar size = maxScale * maxDim;
            SkScalar desiredDimension;
            // For minimizing (or the common case of identity) transforms, we try to
            // create the DF at the appropriately sized native src-space path resolution.
            // In the majority of cases this will yield a crisper rendering.
            if (size <= maxDim && maxDim < kSmallMIP) {
                desiredDimension = maxDim;
            } else if (size <= kSmallMIP) {
                desiredDimension = kSmallMIP;
            } else if (size <= maxDim) {
                desiredDimension = maxDim;
            } else if (size <= kMediumMIP) {
                desiredDimension = kMediumMIP;
            } else {
                desiredDimension = kLargeMIP;
            }

            // check to see if path is cached
            ShapeData::Key key(args.fShape, SkScalarCeilToInt(desiredDimension));
            ShapeData* shapeData = fShapeCache->find(key);
            if (nullptr == shapeData || !atlas->hasID(shapeData->fID)) {
                // Remove the stale cache entry
                if (shapeData) {
                    fShapeCache->remove(shapeData->fKey);
                    fShapeList->remove(shapeData);
                    delete shapeData;
                }
                SkScalar scale = desiredDimension/maxDim;

                shapeData = new ShapeData;
                if (!this->addPathToAtlas(target,
                                          &flushInfo,
                                          atlas,
                                          shapeData,
                                          args.fShape,
                                          SkScalarCeilToInt(desiredDimension),
                                          scale)) {
                    delete shapeData;
                    SkDebugf("Can't rasterize path\n");
                    continue;
                }
            }

            atlas->setLastUseToken(shapeData->fID, target->nextDrawToken());

            this->writePathVertices(target,
                                    atlas,
                                    offset,
                                    args.fColor,
                                    vertexStride,
                                    maxScale,
                                    shapeData);
            offset += kVerticesPerQuad * vertexStride;
            flushInfo.fInstancesToFlush++;
        }

        this->flush(target, &flushInfo);
    }

    bool addPathToAtlas(GrMeshDrawOp::Target* target, FlushInfo* flushInfo, GrBatchAtlas* atlas,
                        ShapeData* shapeData, const GrShape& shape, uint32_t dimension,
                        SkScalar scale) const {
        const SkRect& bounds = shape.bounds();

        // generate bounding rect for bitmap draw
        SkRect scaledBounds = bounds;
        // scale to mip level size
        scaledBounds.fLeft *= scale;
        scaledBounds.fTop *= scale;
        scaledBounds.fRight *= scale;
        scaledBounds.fBottom *= scale;
        // subtract out integer portion of origin
        // (SDF created will be placed with fractional offset burnt in)
        SkScalar dx = SkScalarFloorToInt(scaledBounds.fLeft);
        SkScalar dy = SkScalarFloorToInt(scaledBounds.fTop);
        scaledBounds.offset(-dx, -dy);
        // get integer boundary
        SkIRect devPathBounds;
        scaledBounds.roundOut(&devPathBounds);
        // pad to allow room for antialiasing
        const int intPad = SkScalarCeilToInt(kAntiAliasPad);
        // place devBounds at origin
        int width = devPathBounds.width() + 2*intPad;
        int height = devPathBounds.height() + 2*intPad;
        devPathBounds = SkIRect::MakeWH(width, height);

        // draw path to bitmap
        SkMatrix drawMatrix;
        drawMatrix.setScale(scale, scale);
        drawMatrix.postTranslate(intPad - dx, intPad - dy);

        // setup bitmap backing
        SkASSERT(devPathBounds.fLeft == 0);
        SkASSERT(devPathBounds.fTop == 0);
        SkAutoPixmapStorage dst;
        if (!dst.tryAlloc(SkImageInfo::MakeA8(devPathBounds.width(),
                                              devPathBounds.height()))) {
            return false;
        }
        sk_bzero(dst.writable_addr(), dst.getSafeSize());

        // rasterize path
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);

        SkDraw draw;
        sk_bzero(&draw, sizeof(draw));

        SkRasterClip rasterClip;
        rasterClip.setRect(devPathBounds);
        draw.fRC = &rasterClip;
        draw.fMatrix = &drawMatrix;
        draw.fDst = dst;

        SkPath path;
        shape.asPath(&path);
        draw.drawPathCoverage(path, paint);

        // generate signed distance field
        devPathBounds.outset(SK_DistanceFieldPad, SK_DistanceFieldPad);
        width = devPathBounds.width();
        height = devPathBounds.height();
        // TODO We should really generate this directly into the plot somehow
        SkAutoSMalloc<1024> dfStorage(width * height * sizeof(unsigned char));

        // Generate signed distance field
        SkGenerateDistanceFieldFromA8Image((unsigned char*)dfStorage.get(),
                                           (const unsigned char*)dst.addr(),
                                           dst.width(), dst.height(), dst.rowBytes());

        // add to atlas
        SkIPoint16 atlasLocation;
        GrBatchAtlas::AtlasID id;
        if (!atlas->addToAtlas(&id, target, width, height, dfStorage.get(), &atlasLocation)) {
            this->flush(target, flushInfo);
            if (!atlas->addToAtlas(&id, target, width, height, dfStorage.get(), &atlasLocation)) {
                return false;
            }
        }

        // add to cache
        shapeData->fKey.set(shape, dimension);
        shapeData->fID = id;

        // set the bounds rect to the original bounds
        shapeData->fBounds = bounds;

        // set up texture coordinates
        SkScalar texLeft = bounds.fLeft;
        SkScalar texTop = bounds.fTop;
        SkScalar texRight = bounds.fRight;
        SkScalar texBottom = bounds.fBottom;

        // transform original path's bounds to texture space
        texLeft *= scale;
        texTop *= scale;
        texRight *= scale;
        texBottom *= scale;
        dx -= SK_DistanceFieldPad + kAntiAliasPad;
        dy -= SK_DistanceFieldPad + kAntiAliasPad;
        texLeft += atlasLocation.fX - dx;
        texTop += atlasLocation.fY - dy;
        texRight += atlasLocation.fX - dx;
        texBottom += atlasLocation.fY - dy;

        GrTexture* texture = atlas->getTexture();
        shapeData->fTexCoords.setLTRB(texLeft / texture->width(),
                                      texTop / texture->height(),
                                      texRight / texture->width(),
                                      texBottom / texture->height());

        fShapeCache->add(shapeData);
        fShapeList->addToTail(shapeData);
#ifdef DF_PATH_TRACKING
        ++g_NumCachedPaths;
#endif
        return true;
    }

    void writePathVertices(GrDrawOp::Target* target,
                           GrBatchAtlas* atlas,
                           intptr_t offset,
                           GrColor color,
                           size_t vertexStride,
                           SkScalar maxScale,
                           const ShapeData* shapeData) const {

        SkPoint* positions = reinterpret_cast<SkPoint*>(offset);

        // vertex positions
        // TODO make the vertex attributes a struct
        positions->setRectFan(shapeData->fBounds.left(), shapeData->fBounds.top(),
                              shapeData->fBounds.right(), shapeData->fBounds.bottom(), vertexStride);

        // colors
        for (int i = 0; i < kVerticesPerQuad; i++) {
            GrColor* colorPtr = (GrColor*)(offset + sizeof(SkPoint) + i * vertexStride);
            *colorPtr = color;
        }

        // vertex texture coords
        // TODO make these int16_t
        SkPoint* textureCoords = (SkPoint*)(offset + sizeof(SkPoint) + sizeof(GrColor));
        textureCoords->setRectFan(shapeData->fTexCoords.left(), shapeData->fTexCoords.top(),
                                  shapeData->fTexCoords.right(), shapeData->fTexCoords.bottom(),
                                  vertexStride);
    }

    void flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
        if (flushInfo->fInstancesToFlush) {
            GrMesh mesh;
            int maxInstancesPerDraw =
                static_cast<int>(flushInfo->fIndexBuffer->gpuMemorySize() / sizeof(uint16_t) / 6);
            mesh.initInstanced(kTriangles_GrPrimitiveType, flushInfo->fVertexBuffer.get(),
                flushInfo->fIndexBuffer.get(), flushInfo->fVertexOffset, kVerticesPerQuad,
                kIndicesPerQuad, flushInfo->fInstancesToFlush, maxInstancesPerDraw);
            target->draw(flushInfo->fGeometryProcessor.get(), mesh);
            flushInfo->fVertexOffset += kVerticesPerQuad * flushInfo->fInstancesToFlush;
            flushInfo->fInstancesToFlush = 0;
        }
    }

    GrColor color() const { return fShapes[0].fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AADistanceFieldPathOp* that = t->cast<AADistanceFieldPathOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        // TODO We can position on the cpu
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fShapes.push_back_n(that->fShapes.count(), that->fShapes.begin());
        this->joinBounds(*that);
        return true;
    }

    SkMatrix fViewMatrix;
    bool fUsesLocalCoords;
    bool fColorIgnored;
    bool fCoverageIgnored;

    struct Entry {
        GrColor fColor;
        GrShape fShape;
    };

    SkSTArray<1, Entry> fShapes;
    GrBatchAtlas* fAtlas;
    ShapeCache* fShapeCache;
    ShapeDataList* fShapeList;
    bool fGammaCorrect;

    typedef GrMeshDrawOp INHERITED;
};

bool GrAADistanceFieldPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrAADistanceFieldPathRenderer::onDrawPath");

    // we've already bailed on inverse filled paths, so this is safe
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(args.fShape->hasUnstyledKey());
    if (!fAtlas) {
        fAtlas = args.fResourceProvider->makeAtlas(kAlpha_8_GrPixelConfig,
                                                   ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                   NUM_PLOTS_X, NUM_PLOTS_Y,
                                                   &GrAADistanceFieldPathRenderer::HandleEviction,
                                                   (void*)this);
        if (!fAtlas) {
            return false;
        }
    }

    sk_sp<GrDrawOp> op = AADistanceFieldPathOp::Make(args.fPaint->getColor(), *args.fShape,
                                                     *args.fViewMatrix, fAtlas.get(), &fShapeCache,
                                                     &fShapeList, args.fGammaCorrect);
    GrPipelineBuilder pipelineBuilder(*args.fPaint, args.fAAType);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fRenderTargetContext->addDrawOp(pipelineBuilder, *args.fClip, std::move(op));

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

struct PathTestStruct {
    typedef GrAADistanceFieldPathRenderer::ShapeCache ShapeCache;
    typedef GrAADistanceFieldPathRenderer::ShapeData ShapeData;
    typedef GrAADistanceFieldPathRenderer::ShapeDataList ShapeDataList;
    PathTestStruct() : fContextID(SK_InvalidGenID), fAtlas(nullptr) {}
    ~PathTestStruct() { this->reset(); }

    void reset() {
        ShapeDataList::Iter iter;
        iter.init(fShapeList, ShapeDataList::Iter::kHead_IterStart);
        ShapeData* shapeData;
        while ((shapeData = iter.get())) {
            iter.next();
            fShapeList.remove(shapeData);
            delete shapeData;
        }
        fAtlas = nullptr;
        fShapeCache.reset();
    }

    static void HandleEviction(GrBatchAtlas::AtlasID id, void* pr) {
        PathTestStruct* dfpr = (PathTestStruct*)pr;
        // remove any paths that use this plot
        ShapeDataList::Iter iter;
        iter.init(dfpr->fShapeList, ShapeDataList::Iter::kHead_IterStart);
        ShapeData* shapeData;
        while ((shapeData = iter.get())) {
            iter.next();
            if (id == shapeData->fID) {
                dfpr->fShapeCache.remove(shapeData->fKey);
                dfpr->fShapeList.remove(shapeData);
                delete shapeData;
            }
        }
    }

    uint32_t fContextID;
    std::unique_ptr<GrBatchAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;
};

DRAW_BATCH_TEST_DEFINE(AADistanceFieldPathOp) {
    static PathTestStruct gTestStruct;

    if (context->uniqueID() != gTestStruct.fContextID) {
        gTestStruct.fContextID = context->uniqueID();
        gTestStruct.reset();
        gTestStruct.fAtlas =
                context->resourceProvider()->makeAtlas(kAlpha_8_GrPixelConfig,
                                                       ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                       NUM_PLOTS_X, NUM_PLOTS_Y,
                                                       &PathTestStruct::HandleEviction,
                                                       (void*)&gTestStruct);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    bool gammaCorrect = random->nextBool();

    // This path renderer only allows fill styles.
    GrShape shape(GrTest::TestPath(random), GrStyle::SimpleFill());

    return AADistanceFieldPathOp::Make(color,
                                       shape,
                                       viewMatrix,
                                       gTestStruct.fAtlas.get(),
                                       &gTestStruct.fShapeCache,
                                       &gTestStruct.fShapeList,
                                       gammaCorrect)
            .release();
}

#endif
