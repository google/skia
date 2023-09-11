/*
 * Copyright 2014 Google Inc.
 * Copyright 2017 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/SmallPathRenderer.h"

#include "include/core/SkPaint.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDistanceFieldGenFromVector.h"
#include "src/gpu/ganesh/GrDrawOpTest.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrBitmapTextGeoProc.h"
#include "src/gpu/ganesh/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelperWithStencil.h"
#include "src/gpu/ganesh/ops/SmallPathAtlasMgr.h"
#include "src/gpu/ganesh/ops/SmallPathShapeData.h"

using namespace skia_private;

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

using MaskFormat = skgpu::MaskFormat;

namespace skgpu::ganesh {

namespace {

// mip levels
static constexpr SkScalar kIdealMinMIP = 12;
static constexpr SkScalar kMaxMIP = 162;

static constexpr SkScalar kMaxDim = 73;
static constexpr SkScalar kMinSize = SK_ScalarHalf;
static constexpr SkScalar kMaxSize = 2*kMaxMIP;

////////////////////////////////////////////////////////////////////////////////

// padding around path bounds to allow for antialiased pixels
static const int kAntiAliasPad = 1;

class SmallPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const GrStyledShape& shape,
                            const SkMatrix& viewMatrix,
                            bool gammaCorrect,
                            const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<SmallPathOp>(context, std::move(paint), shape, viewMatrix,
                                                  gammaCorrect, stencilSettings);
    }

    SmallPathOp(GrProcessorSet* processorSet, const SkPMColor4f& color, const GrStyledShape& shape,
                const SkMatrix& viewMatrix, bool gammaCorrect,
                const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(processorSet, GrAAType::kCoverage, stencilSettings) {
        SkASSERT(shape.hasUnstyledKey());
        // Compute bounds
        this->setTransformedBounds(shape.bounds(), viewMatrix, HasAABloat::kYes, IsHairline::kNo);

#if defined(SK_BUILD_FOR_ANDROID) && !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
        fUsesDistanceField = true;
#else
        // only use distance fields on desktop and Android framework to save space in the atlas
        fUsesDistanceField = this->bounds().width() > kMaxMIP || this->bounds().height() > kMaxMIP;
#endif
        // always use distance fields if in perspective
        fUsesDistanceField = fUsesDistanceField || viewMatrix.hasPerspective();

        fShapes.emplace_back(Entry{color, shape, viewMatrix});

        fGammaCorrect = gammaCorrect;
    }

    const char* name() const override { return "SmallPathOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        return fHelper.finalizeProcessors(caps, clip, clampType,
                                          GrProcessorAnalysisCoverage::kSingleChannel,
                                          &fShapes.front().fColor, &fWideColor);
    }

private:
    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        GrGeometryProcessor*  fGeometryProcessor;
        const GrSurfaceProxy** fPrimProcProxies;
        int fVertexOffset;
        int fInstancesToFlush;
    };

    GrProgramInfo* programInfo() override {
        // TODO [PI]: implement
        return nullptr;
    }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        // We cannot surface the SmallPathOp's programInfo at record time. As currently
        // implemented, the GP is modified at flush time based on the number of pages in the
        // atlas.
    }

    void onPrePrepareDraws(GrRecordingContext*,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip*,
                           const GrDstProxyView&,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {
        // TODO [PI]: implement
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        int instanceCount = fShapes.size();

        auto atlasMgr = target->smallPathAtlasManager();
        if (!atlasMgr) {
            return;
        }

        static constexpr int kMaxTextures = GrDistanceFieldPathGeoProc::kMaxTextures;
        static_assert(GrBitmapTextGeoProc::kMaxTextures == kMaxTextures);

        FlushInfo flushInfo;
        flushInfo.fPrimProcProxies = target->allocPrimProcProxyPtrs(kMaxTextures);

        int numActiveProxies;
        const GrSurfaceProxyView* views = atlasMgr->getViews(&numActiveProxies);
        for (int i = 0; i < numActiveProxies; ++i) {
            // This op does not know its atlas proxies when it is added to a OpsTasks, so the
            // proxies don't get added during the visitProxies call. Thus we add them here.
            flushInfo.fPrimProcProxies[i] = views[i].proxy();
            target->sampledProxyArray()->push_back(views[i].proxy());
        }

        // Setup GrGeometryProcessor
        const SkMatrix& ctm = fShapes[0].fViewMatrix;
        SkMatrix invert;
        if (fHelper.usesLocalCoords()) {
            if (!ctm.invert(&invert)) {
                return;
            }
        }
        if (fUsesDistanceField) {
            uint32_t flags = 0;
            // Still need to key off of ctm to pick the right shader for the transformed quad
            flags |= ctm.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
            flags |= ctm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
            flags |= fGammaCorrect ? kGammaCorrect_DistanceFieldEffectFlag : 0;
            flags |= fWideColor ? kWideColor_DistanceFieldEffectFlag : 0;
            // We always use Point3 for position
            flags |= kPerspective_DistanceFieldEffectFlag;

            flushInfo.fGeometryProcessor = GrDistanceFieldPathGeoProc::Make(
                    target->allocator(), *target->caps().shaderCaps(),
                    views, numActiveProxies, GrSamplerState::Filter::kLinear,
                    invert, flags);
        } else {
            flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
                    target->allocator(), *target->caps().shaderCaps(), this->color(), fWideColor,
                    views, numActiveProxies, GrSamplerState::Filter::kNearest,
                    MaskFormat::kA8, invert, false);
        }

        // allocate vertices
        const size_t kVertexStride = flushInfo.fGeometryProcessor->vertexStride();

        // We need to make sure we don't overflow a 32 bit int when we request space in the
        // makeVertexSpace call below.
        if (instanceCount > SK_MaxS32 / GrResourceProvider::NumVertsPerNonAAQuad()) {
            return;
        }
        VertexWriter vertices = target->makeVertexWriter(
            kVertexStride, GrResourceProvider::NumVertsPerNonAAQuad() * instanceCount,
            &flushInfo.fVertexBuffer, &flushInfo.fVertexOffset);

        flushInfo.fIndexBuffer = target->resourceProvider()->refNonAAQuadIndexBuffer();
        if (!vertices || !flushInfo.fIndexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        flushInfo.fInstancesToFlush = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Entry& args = fShapes[i];

            skgpu::ganesh::SmallPathShapeData* shapeData;
            if (fUsesDistanceField) {
                // get mip level
                SkScalar maxScale;
                const SkRect& bounds = args.fShape.bounds();
                if (args.fViewMatrix.hasPerspective()) {
                    // approximate the scale since we can't get it from the matrix
                    SkRect xformedBounds;
                    args.fViewMatrix.mapRect(&xformedBounds, bounds);
                    maxScale = SkScalarAbs(std::max(xformedBounds.width() / bounds.width(),
                                                  xformedBounds.height() / bounds.height()));
                } else {
                    maxScale = SkScalarAbs(args.fViewMatrix.getMaxScale());
                }
                SkScalar maxDim = std::max(bounds.width(), bounds.height());
                // We try to create the DF at a 2^n scaled path resolution (1/2, 1, 2, 4, etc.)
                // In the majority of cases this will yield a crisper rendering.
                SkScalar mipScale = 1.0f;
                // Our mipscale is the maxScale clamped to the next highest power of 2
                if (maxScale <= SK_ScalarHalf) {
                    SkScalar log = SkScalarFloorToScalar(SkScalarLog2(SkScalarInvert(maxScale)));
                    mipScale = SkScalarPow(2, -log);
                } else if (maxScale > SK_Scalar1) {
                    SkScalar log = SkScalarCeilToScalar(SkScalarLog2(maxScale));
                    mipScale = SkScalarPow(2, log);
                }
                // Log2 isn't very precise at values close to a power of 2,
                // so add a little tolerance here. A little bit of scaling up is fine.
                SkASSERT(maxScale <= mipScale + SK_ScalarNearlyZero);

                SkScalar mipSize = mipScale*SkScalarAbs(maxDim);
                // For sizes less than kIdealMinMIP we want to use as large a distance field as we can
                // so we can preserve as much detail as possible. However, we can't scale down more
                // than a 1/4 of the size without artifacts. So the idea is that we pick the mipsize
                // just bigger than the ideal, and then scale down until we are no more than 4x the
                // original mipsize.
                if (mipSize < kIdealMinMIP) {
                    SkScalar newMipSize = mipSize;
                    do {
                        newMipSize *= 2;
                    } while (newMipSize < kIdealMinMIP);
                    while (newMipSize > 4 * mipSize) {
                        newMipSize *= 0.25f;
                    }
                    mipSize = newMipSize;
                }

                SkScalar desiredDimension = std::min(mipSize, kMaxMIP);
                int ceilDesiredDimension = SkScalarCeilToInt(desiredDimension);

                // check to see if df path is cached
                shapeData = atlasMgr->findOrCreate(args.fShape, ceilDesiredDimension);
                if (!shapeData->fAtlasLocator.plotLocator().isValid()) {
                    SkScalar scale = desiredDimension / maxDim;

                    if (!this->addDFPathToAtlas(target,
                                                &flushInfo,
                                                atlasMgr,
                                                shapeData,
                                                args.fShape,
                                                ceilDesiredDimension,
                                                scale)) {
                        atlasMgr->deleteCacheEntry(shapeData);
                        continue;
                    }
                }
            } else {
                // check to see if bitmap path is cached
                shapeData = atlasMgr->findOrCreate(args.fShape, args.fViewMatrix);
                if (!shapeData->fAtlasLocator.plotLocator().isValid()) {
                    if (!this->addBMPathToAtlas(target,
                                                &flushInfo,
                                                atlasMgr,
                                                shapeData,
                                                args.fShape,
                                                args.fViewMatrix)) {
                        atlasMgr->deleteCacheEntry(shapeData);
                        continue;
                    }
                }
            }

            auto uploadTarget = target->deferredUploadTarget();
            atlasMgr->setUseToken(shapeData, uploadTarget->tokenTracker()->nextDrawToken());

            this->writePathVertices(vertices, VertexColor(args.fColor, fWideColor),
                                    args.fViewMatrix, shapeData);
            flushInfo.fInstancesToFlush++;
        }

        this->flush(target, &flushInfo);
    }

    bool addToAtlasWithRetry(GrMeshDrawTarget* target,
                             FlushInfo* flushInfo,
                             skgpu::ganesh::SmallPathAtlasMgr* atlasMgr,
                             int width,
                             int height,
                             const void* image,
                             const SkRect& bounds,
                             int srcInset,
                             skgpu::ganesh::SmallPathShapeData* shapeData) const {
        auto resourceProvider = target->resourceProvider();
        auto uploadTarget = target->deferredUploadTarget();

        auto code = atlasMgr->addToAtlas(resourceProvider, uploadTarget, width, height,
                                         image, &shapeData->fAtlasLocator);
        if (GrDrawOpAtlas::ErrorCode::kError == code) {
            return false;
        }

        if (GrDrawOpAtlas::ErrorCode::kTryAgain == code) {
            this->flush(target, flushInfo);

            code = atlasMgr->addToAtlas(resourceProvider, uploadTarget, width, height,
                                        image, &shapeData->fAtlasLocator);
        }

        shapeData->fAtlasLocator.insetSrc(srcInset);
        shapeData->fBounds = bounds;

        return GrDrawOpAtlas::ErrorCode::kSucceeded == code;
    }

    bool addDFPathToAtlas(GrMeshDrawTarget* target,
                          FlushInfo* flushInfo,
                          skgpu::ganesh::SmallPathAtlasMgr* atlasMgr,
                          skgpu::ganesh::SmallPathShapeData* shapeData,
                          const GrStyledShape& shape,
                          uint32_t dimension,
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
        SkScalar dx = SkScalarFloorToScalar(scaledBounds.fLeft);
        SkScalar dy = SkScalarFloorToScalar(scaledBounds.fTop);
        scaledBounds.offset(-dx, -dy);
        // get integer boundary
        SkIRect devPathBounds;
        scaledBounds.roundOut(&devPathBounds);
        // place devBounds at origin with padding to allow room for antialiasing
        int width = devPathBounds.width() + 2 * kAntiAliasPad;
        int height = devPathBounds.height() + 2 * kAntiAliasPad;
        devPathBounds = SkIRect::MakeWH(width, height);
        SkScalar translateX = kAntiAliasPad - dx;
        SkScalar translateY = kAntiAliasPad - dy;

        // draw path to bitmap
        SkMatrix drawMatrix;
        drawMatrix.setScale(scale, scale);
        drawMatrix.postTranslate(translateX, translateY);

        SkASSERT(devPathBounds.fLeft == 0);
        SkASSERT(devPathBounds.fTop == 0);
        SkASSERT(devPathBounds.width() > 0);
        SkASSERT(devPathBounds.height() > 0);

        // setup signed distance field storage
        SkIRect dfBounds = devPathBounds.makeOutset(SK_DistanceFieldPad, SK_DistanceFieldPad);
        width = dfBounds.width();
        height = dfBounds.height();
        // TODO We should really generate this directly into the plot somehow
        SkAutoSMalloc<1024> dfStorage(width * height * sizeof(unsigned char));

        SkPath path;
        shape.asPath(&path);
        // Generate signed distance field directly from SkPath
        bool succeed = GrGenerateDistanceFieldFromPath((unsigned char*)dfStorage.get(),
                                                       path, drawMatrix, width, height,
                                                       width * sizeof(unsigned char));
        if (!succeed) {
            // setup bitmap backing
            SkAutoPixmapStorage dst;
            if (!dst.tryAlloc(SkImageInfo::MakeA8(devPathBounds.width(), devPathBounds.height()))) {
                return false;
            }
            sk_bzero(dst.writable_addr(), dst.computeByteSize());

            // rasterize path
            SkPaint paint;
            paint.setStyle(SkPaint::kFill_Style);
            paint.setAntiAlias(true);

            SkDraw draw;

            SkRasterClip rasterClip;
            rasterClip.setRect(devPathBounds);
            draw.fRC = &rasterClip;
            draw.fCTM = &drawMatrix;
            draw.fDst = dst;

            draw.drawPathCoverage(path, paint);

            // Generate signed distance field
            SkGenerateDistanceFieldFromA8Image((unsigned char*)dfStorage.get(),
                                               (const unsigned char*)dst.addr(),
                                               dst.width(), dst.height(), dst.rowBytes());
        }

        SkRect drawBounds = SkRect::Make(devPathBounds).makeOffset(-translateX, -translateY);
        drawBounds.fLeft /= scale;
        drawBounds.fTop /= scale;
        drawBounds.fRight /= scale;
        drawBounds.fBottom /= scale;

        return this->addToAtlasWithRetry(target, flushInfo, atlasMgr,
                                         width, height, dfStorage.get(),
                                         drawBounds, SK_DistanceFieldPad, shapeData);
    }

    bool addBMPathToAtlas(GrMeshDrawTarget* target,
                          FlushInfo* flushInfo,
                          skgpu::ganesh::SmallPathAtlasMgr* atlasMgr,
                          skgpu::ganesh::SmallPathShapeData* shapeData,
                          const GrStyledShape& shape,
                          const SkMatrix& ctm) const {
        const SkRect& bounds = shape.bounds();
        if (bounds.isEmpty()) {
            return false;
        }
        SkMatrix drawMatrix(ctm);
        SkScalar tx = ctm.getTranslateX();
        SkScalar ty = ctm.getTranslateY();
        tx -= SkScalarFloorToScalar(tx);
        ty -= SkScalarFloorToScalar(ty);
        drawMatrix.set(SkMatrix::kMTransX, tx);
        drawMatrix.set(SkMatrix::kMTransY, ty);
        SkRect shapeDevBounds;
        drawMatrix.mapRect(&shapeDevBounds, bounds);
        SkScalar dx = SkScalarFloorToScalar(shapeDevBounds.fLeft);
        SkScalar dy = SkScalarFloorToScalar(shapeDevBounds.fTop);

        // get integer boundary
        SkIRect devPathBounds;
        shapeDevBounds.roundOut(&devPathBounds);
        // place devBounds at origin with padding to allow room for antialiasing
        int width = devPathBounds.width() + 2 * kAntiAliasPad;
        int height = devPathBounds.height() + 2 * kAntiAliasPad;
        devPathBounds = SkIRect::MakeWH(width, height);
        SkScalar translateX = kAntiAliasPad - dx;
        SkScalar translateY = kAntiAliasPad - dy;

        SkASSERT(devPathBounds.fLeft == 0);
        SkASSERT(devPathBounds.fTop == 0);
        SkASSERT(devPathBounds.width() > 0);
        SkASSERT(devPathBounds.height() > 0);

        SkPath path;
        shape.asPath(&path);
        // setup bitmap backing
        SkAutoPixmapStorage dst;
        if (!dst.tryAlloc(SkImageInfo::MakeA8(devPathBounds.width(), devPathBounds.height()))) {
            return false;
        }
        sk_bzero(dst.writable_addr(), dst.computeByteSize());

        // rasterize path
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);

        SkDraw draw;

        SkRasterClip rasterClip;
        rasterClip.setRect(devPathBounds);
        drawMatrix.postTranslate(translateX, translateY);
        draw.fRC = &rasterClip;
        draw.fCTM = &drawMatrix;
        draw.fDst = dst;

        draw.drawPathCoverage(path, paint);

        SkRect drawBounds = SkRect::Make(devPathBounds).makeOffset(-translateX, -translateY);

        return this->addToAtlasWithRetry(target, flushInfo, atlasMgr,
                                         dst.width(), dst.height(), dst.addr(),
                                         drawBounds, 0, shapeData);
    }

    void writePathVertices(VertexWriter& vertices,
                           const VertexColor& color,
                           const SkMatrix& ctm,
                           const skgpu::ganesh::SmallPathShapeData* shapeData) const {
        SkRect translatedBounds(shapeData->fBounds);
        if (!fUsesDistanceField) {
            translatedBounds.offset(SkScalarFloorToScalar(ctm.get(SkMatrix::kMTransX)),
                                    SkScalarFloorToScalar(ctm.get(SkMatrix::kMTransY)));
        }

        // set up texture coordinates
        auto texCoords = VertexWriter::TriStripFromUVs(shapeData->fAtlasLocator.getUVs());

        if (fUsesDistanceField) {
            SkPoint pts[4];
            SkPoint3 out[4];
            translatedBounds.toQuad(pts);
            ctm.mapHomogeneousPoints(out, pts, 4);

            vertices << out[0] << color << texCoords.l << texCoords.t;
            vertices << out[3] << color << texCoords.l << texCoords.b;
            vertices << out[1] << color << texCoords.r << texCoords.t;
            vertices << out[2] << color << texCoords.r << texCoords.b;
        } else {
            vertices.writeQuad(VertexWriter::TriStripFromRect(translatedBounds),
                               color,
                               texCoords);
        }
    }

    void flush(GrMeshDrawTarget* target, FlushInfo* flushInfo) const {
        auto atlasMgr = target->smallPathAtlasManager();
        if (!atlasMgr) {
            return;
        }

        int numActiveProxies;
        const GrSurfaceProxyView* views = atlasMgr->getViews(&numActiveProxies);

        GrGeometryProcessor* gp = flushInfo->fGeometryProcessor;
        if (gp->numTextureSamplers() != numActiveProxies) {
            for (int i = gp->numTextureSamplers(); i < numActiveProxies; ++i) {
                flushInfo->fPrimProcProxies[i] = views[i].proxy();
                // This op does not know its atlas proxies when it is added to a OpsTasks, so the
                // proxies don't get added during the visitProxies call. Thus we add them here.
                target->sampledProxyArray()->push_back(views[i].proxy());
            }
            // During preparation the number of atlas pages has increased.
            // Update the proxies used in the GP to match.
            if (fUsesDistanceField) {
                reinterpret_cast<GrDistanceFieldPathGeoProc*>(gp)->addNewViews(
                        views, numActiveProxies, GrSamplerState::Filter::kLinear);
            } else {
                reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewViews(
                        views, numActiveProxies, GrSamplerState::Filter::kNearest);
            }
        }

        if (flushInfo->fInstancesToFlush) {
            GrSimpleMesh* mesh = target->allocMesh();
            mesh->setIndexedPatterned(flushInfo->fIndexBuffer,
                                      GrResourceProvider::NumIndicesPerNonAAQuad(),
                                      flushInfo->fInstancesToFlush,
                                      GrResourceProvider::MaxNumNonAAQuads(),
                                      flushInfo->fVertexBuffer,
                                      GrResourceProvider::NumVertsPerNonAAQuad(),
                                      flushInfo->fVertexOffset);
            target->recordDraw(flushInfo->fGeometryProcessor, mesh, 1, flushInfo->fPrimProcProxies,
                               GrPrimitiveType::kTriangles);
            flushInfo->fVertexOffset += GrResourceProvider::NumVertsPerNonAAQuad() *
                                        flushInfo->fInstancesToFlush;
            flushInfo->fInstancesToFlush = 0;
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipeline = fHelper.createPipeline(flushState);

        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline,
                                                        fHelper.stencilSettings());
    }

    const SkPMColor4f& color() const { return fShapes[0].fColor; }
    bool usesDistanceField() const { return fUsesDistanceField; }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        SmallPathOp* that = t->cast<SmallPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        if (this->usesDistanceField() != that->usesDistanceField()) {
            return CombineResult::kCannotCombine;
        }

        const SkMatrix& thisCtm = this->fShapes[0].fViewMatrix;
        const SkMatrix& thatCtm = that->fShapes[0].fViewMatrix;

        if (this->usesDistanceField()) {
            // Need to make sure local matrices are identical
            if (fHelper.usesLocalCoords() && !SkMatrixPriv::CheapEqual(thisCtm, thatCtm)) {
                return CombineResult::kCannotCombine;
            }

            // Depending on the ctm we may have a different shader for SDF paths
            if (thisCtm.isScaleTranslate() != thatCtm.isScaleTranslate() ||
                thisCtm.isSimilarity() != thatCtm.isSimilarity()) {
                return CombineResult::kCannotCombine;
            }
        } else {
            if (thisCtm.hasPerspective() != thatCtm.hasPerspective()) {
                return CombineResult::kCannotCombine;
            }

            // We can position on the cpu unless we're in perspective,
            // but also need to make sure local matrices are identical
            if ((thisCtm.hasPerspective() || fHelper.usesLocalCoords()) &&
                !SkMatrixPriv::CheapEqual(thisCtm, thatCtm)) {
                return CombineResult::kCannotCombine;
            }
        }

        fShapes.push_back_n(that->fShapes.size(), that->fShapes.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

#if defined(GR_TEST_UTILS)
    SkString onDumpInfo() const override {
        SkString string;
        for (const auto& geo : fShapes) {
            string.appendf("Color: 0x%08x\n", geo.fColor.toBytes_RGBA());
        }
        string += fHelper.dumpInfo();
        return string;
    }
#endif

    bool fUsesDistanceField;

    struct Entry {
        SkPMColor4f   fColor;
        GrStyledShape fShape;
        SkMatrix      fViewMatrix;
    };

    STArray<1, Entry> fShapes;
    Helper fHelper;
    bool fGammaCorrect;
    bool fWideColor;

    using INHERITED = GrMeshDrawOp;
};

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

PathRenderer::CanDrawPath SmallPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (!args.fCaps->shaderCaps()->fShaderDerivativeSupport) {
        return CanDrawPath::kNo;
    }
    // If the shape has no key then we won't get any reuse.
    if (!args.fShape->hasUnstyledKey()) {
        return CanDrawPath::kNo;
    }
    // This only supports filled paths, however, the caller may apply the style to make a filled
    // path and try again.
    if (!args.fShape->style().isSimpleFill()) {
        return CanDrawPath::kNo;
    }
    // This does non-inverse coverage-based antialiased fills.
    if (GrAAType::kCoverage != args.fAAType) {
        return CanDrawPath::kNo;
    }
    // TODO: Support inverse fill
    if (args.fShape->inverseFilled()) {
        return CanDrawPath::kNo;
    }

    SkScalar scaleFactors[2] = { 1, 1 };
    // TODO: handle perspective distortion
    if (!args.fViewMatrix->hasPerspective() && !args.fViewMatrix->getMinMaxScales(scaleFactors)) {
        return CanDrawPath::kNo;
    }
    // For affine transformations, too much shear can produce artifacts.
    if (!scaleFactors[0] || scaleFactors[1]/scaleFactors[0] > 4) {
        return CanDrawPath::kNo;
    }
    // Only support paths with bounds within kMaxDim by kMaxDim,
    // scaled to have bounds within kMaxSize by kMaxSize.
    // The goal is to accelerate rendering of lots of small paths that may be scaling.
    SkRect bounds = args.fShape->styledBounds();
    SkScalar minDim = std::min(bounds.width(), bounds.height());
    SkScalar maxDim = std::max(bounds.width(), bounds.height());
    SkScalar minSize = minDim * SkScalarAbs(scaleFactors[0]);
    SkScalar maxSize = maxDim * SkScalarAbs(scaleFactors[1]);
    if (maxDim > kMaxDim || kMinSize > minSize || maxSize > kMaxSize) {
        return CanDrawPath::kNo;
    }

    return CanDrawPath::kYes;
}

bool SmallPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "SmallPathRenderer::onDrawPath");

    // we've already bailed on inverse filled paths, so this is safe
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(args.fShape->hasUnstyledKey());

    GrOp::Owner op = SmallPathOp::Make(
            args.fContext, std::move(args.fPaint), *args.fShape, *args.fViewMatrix,
            args.fGammaCorrect, args.fUserStencilSettings);
    args.fSurfaceDrawContext->addDrawOp(args.fClip, std::move(op));

    return true;
}

}  // namespace skgpu::ganesh

#if defined(GR_TEST_UTILS)

GR_DRAW_OP_TEST_DEFINE(SmallPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    bool gammaCorrect = random->nextBool();

    // This path renderer only allows fill styles.
    GrStyledShape shape(GrTest::TestPath(random), GrStyle::SimpleFill());
    return skgpu::ganesh::SmallPathOp::Make(context,
                                            std::move(paint),
                                            shape,
                                            viewMatrix,
                                            gammaCorrect,
                                            GrGetRandomStencil(random, context));
}

#endif // defined(GR_TEST_UTILS)

#endif // SK_ENABLE_OPTIMIZE_SIZE
