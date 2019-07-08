/*
 * Copyright 2014 Google Inc.
 * Copyright 2017 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrSmallPathRenderer.h"

#include "include/core/SkPaint.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkDraw.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDistanceFieldGenFromVector.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/effects/GrBitmapTextGeoProc.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

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
static const SkScalar kIdealMinMIP = 12;
static const SkScalar kMaxMIP = 162;

static const SkScalar kMaxDim = 73;
static const SkScalar kMinSize = SK_ScalarHalf;
static const SkScalar kMaxSize = 2*kMaxMIP;

class ShapeDataKey {
public:
    ShapeDataKey() {}
    ShapeDataKey(const ShapeDataKey& that) { *this = that; }
    ShapeDataKey(const GrShape& shape, uint32_t dim) { this->set(shape, dim); }
    ShapeDataKey(const GrShape& shape, const SkMatrix& ctm) { this->set(shape, ctm); }

    ShapeDataKey& operator=(const ShapeDataKey& that) {
        fKey.reset(that.fKey.count());
        memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
        return *this;
    }

    // for SDF paths
    void set(const GrShape& shape, uint32_t dim) {
        // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
        // relevant styling information.
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(shape.hasUnstyledKey());
        int shapeKeySize = shape.unstyledKeySize();
        fKey.reset(1 + shapeKeySize);
        fKey[0] = dim;
        shape.writeUnstyledKey(&fKey[1]);
    }

    // for bitmap paths
    void set(const GrShape& shape, const SkMatrix& ctm) {
        // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
        // relevant styling information.
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(shape.hasUnstyledKey());
        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = ctm.get(SkMatrix::kMScaleX);
        SkScalar sy = ctm.get(SkMatrix::kMScaleY);
        SkScalar kx = ctm.get(SkMatrix::kMSkewX);
        SkScalar ky = ctm.get(SkMatrix::kMSkewY);
        SkScalar tx = ctm.get(SkMatrix::kMTransX);
        SkScalar ty = ctm.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        tx -= SkScalarFloorToScalar(tx);
        ty -= SkScalarFloorToScalar(ty);
        SkFixed fracX = SkScalarToFixed(tx) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(ty) & 0x0000FF00;
        int shapeKeySize = shape.unstyledKeySize();
        fKey.reset(5 + shapeKeySize);
        fKey[0] = SkFloat2Bits(sx);
        fKey[1] = SkFloat2Bits(sy);
        fKey[2] = SkFloat2Bits(kx);
        fKey[3] = SkFloat2Bits(ky);
        fKey[4] = fracX | (fracY >> 8);
        shape.writeUnstyledKey(&fKey[5]);
    }

    bool operator==(const ShapeDataKey& that) const {
        return fKey.count() == that.fKey.count() &&
                0 == memcmp(fKey.get(), that.fKey.get(), sizeof(uint32_t) * fKey.count());
    }

    int count32() const { return fKey.count(); }
    const uint32_t* data() const { return fKey.get(); }

private:
    // The key is composed of the GrShape's key, and either the dimensions of the DF
    // generated for the path (32x32 max, 64x64 max, 128x128 max) if an SDF image or
    // the matrix for the path with only fractional translation.
    SkAutoSTArray<24, uint32_t> fKey;
};

class ShapeData {
public:
    ShapeDataKey           fKey;
    GrDrawOpAtlas::AtlasID fID;
    SkRect                 fBounds;
    GrIRect16              fTextureCoords;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeData);

    static inline const ShapeDataKey& GetKey(const ShapeData& data) {
        return data.fKey;
    }

    static inline uint32_t Hash(const ShapeDataKey& key) {
        return SkOpts::hash(key.data(), sizeof(uint32_t) * key.count32());
    }
};



// Callback to clear out internal path cache when eviction occurs
void GrSmallPathRenderer::HandleEviction(GrDrawOpAtlas::AtlasID id, void* pr) {
    GrSmallPathRenderer* dfpr = (GrSmallPathRenderer*)pr;
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
GrSmallPathRenderer::GrSmallPathRenderer() : fAtlas(nullptr) {}

GrSmallPathRenderer::~GrSmallPathRenderer() {
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
GrPathRenderer::CanDrawPath GrSmallPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (!args.fCaps->shaderCaps()->shaderDerivativeSupport()) {
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

    // Only support paths with bounds within kMaxDim by kMaxDim,
    // scaled to have bounds within kMaxSize by kMaxSize.
    // The goal is to accelerate rendering of lots of small paths that may be scaling.
    SkScalar scaleFactors[2] = { 1, 1 };
    if (!args.fViewMatrix->hasPerspective() && !args.fViewMatrix->getMinMaxScales(scaleFactors)) {
        return CanDrawPath::kNo;
    }
    SkRect bounds = args.fShape->styledBounds();
    SkScalar minDim = SkMinScalar(bounds.width(), bounds.height());
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
    SkScalar minSize = minDim * SkScalarAbs(scaleFactors[0]);
    SkScalar maxSize = maxDim * SkScalarAbs(scaleFactors[1]);
    if (maxDim > kMaxDim || kMinSize > minSize || maxSize > kMaxSize) {
        return CanDrawPath::kNo;
    }

    return CanDrawPath::kYes;
}

////////////////////////////////////////////////////////////////////////////////

// padding around path bounds to allow for antialiased pixels
static const SkScalar kAntiAliasPad = 1.0f;

class GrSmallPathRenderer::SmallPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    using ShapeCache = SkTDynamicHash<ShapeData, ShapeDataKey>;
    using ShapeDataList = GrSmallPathRenderer::ShapeDataList;

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const GrShape& shape,
                                          const SkMatrix& viewMatrix,
                                          GrDrawOpAtlas* atlas,
                                          ShapeCache* shapeCache,
                                          ShapeDataList* shapeList,
                                          bool gammaCorrect,
                                          const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<SmallPathOp>(context, std::move(paint), shape, viewMatrix,
                                                  atlas, shapeCache, shapeList, gammaCorrect,
                                                  stencilSettings);
    }

    SmallPathOp(Helper::MakeArgs helperArgs, const SkPMColor4f& color, const GrShape& shape,
                const SkMatrix& viewMatrix, GrDrawOpAtlas* atlas, ShapeCache* shapeCache,
                ShapeDataList* shapeList, bool gammaCorrect,
                const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage, stencilSettings) {
        SkASSERT(shape.hasUnstyledKey());
        // Compute bounds
        this->setTransformedBounds(shape.bounds(), viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);

#if defined(SK_BUILD_FOR_ANDROID) && !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
        fUsesDistanceField = true;
#else
        // only use distance fields on desktop and Android framework to save space in the atlas
        fUsesDistanceField = this->bounds().width() > kMaxMIP || this->bounds().height() > kMaxMIP;
#endif
        // always use distance fields if in perspective
        fUsesDistanceField = fUsesDistanceField || viewMatrix.hasPerspective();

        fShapes.emplace_back(Entry{color, shape, viewMatrix});

        fAtlas = atlas;
        fShapeCache = shapeCache;
        fShapeList = shapeList;
        fGammaCorrect = gammaCorrect;
    }

    const char* name() const override { return "SmallPathOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);

        const sk_sp<GrTextureProxy>* proxies = fAtlas->getProxies();
        for (uint32_t i = 0; i < fAtlas->numActivePages(); ++i) {
            SkASSERT(proxies[i]);
            func(proxies[i].get(), GrMipMapped::kNo);
        }
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        for (const auto& geo : fShapes) {
            string.appendf("Color: 0x%08x\n", geo.fColor.toBytes_RGBA());
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }
#endif

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        return fHelper.finalizeProcessors(
                caps, clip, hasMixedSampledCoverage, clampType,
                GrProcessorAnalysisCoverage::kSingleChannel, &fShapes.front().fColor, &fWideColor);
    }

private:
    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<GrGeometryProcessor>   fGeometryProcessor;
        GrPipeline::FixedDynamicState* fFixedDynamicState;
        int fVertexOffset;
        int fInstancesToFlush;
    };

    void onPrepareDraws(Target* target) override {
        int instanceCount = fShapes.count();

        static constexpr int kMaxTextures = GrDistanceFieldPathGeoProc::kMaxTextures;
        GR_STATIC_ASSERT(GrBitmapTextGeoProc::kMaxTextures == kMaxTextures);

        FlushInfo flushInfo;
        flushInfo.fFixedDynamicState = target->makeFixedDynamicState(kMaxTextures);
        int numActiveProxies = fAtlas->numActivePages();
        const auto proxies = fAtlas->getProxies();
        for (int i = 0; i < numActiveProxies; ++i) {
            flushInfo.fFixedDynamicState->fPrimitiveProcessorTextures[i] = proxies[i].get();
        }

        // Setup GrGeometryProcessor
        const SkMatrix& ctm = fShapes[0].fViewMatrix;
        if (fUsesDistanceField) {
            uint32_t flags = 0;
            // Still need to key off of ctm to pick the right shader for the transformed quad
            flags |= ctm.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
            flags |= ctm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
            flags |= fGammaCorrect ? kGammaCorrect_DistanceFieldEffectFlag : 0;

            const SkMatrix* matrix;
            SkMatrix invert;
            if (ctm.hasPerspective()) {
                matrix = &ctm;
            } else if (fHelper.usesLocalCoords()) {
                if (!ctm.invert(&invert)) {
                    return;
                }
                matrix = &invert;
            } else {
                matrix = &SkMatrix::I();
            }
            flushInfo.fGeometryProcessor = GrDistanceFieldPathGeoProc::Make(
                    *target->caps().shaderCaps(), *matrix, fWideColor, fAtlas->getProxies(),
                    fAtlas->numActivePages(), GrSamplerState::ClampBilerp(), flags);
        } else {
            SkMatrix invert;
            if (fHelper.usesLocalCoords()) {
                if (!ctm.invert(&invert)) {
                    return;
                }
            }

            flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
                    *target->caps().shaderCaps(), this->color(), fWideColor, fAtlas->getProxies(),
                    fAtlas->numActivePages(), GrSamplerState::ClampNearest(), kA8_GrMaskFormat,
                    invert, false);
        }

        // allocate vertices
        const size_t kVertexStride = flushInfo.fGeometryProcessor->vertexStride();

        // We need to make sure we don't overflow a 32 bit int when we request space in the
        // makeVertexSpace call below.
        if (instanceCount > SK_MaxS32 / kVerticesPerQuad) {
            return;
        }
        GrVertexWriter vertices{target->makeVertexSpace(kVertexStride,
                                                        kVerticesPerQuad * instanceCount,
                                                        &flushInfo.fVertexBuffer,
                                                        &flushInfo.fVertexOffset)};
        flushInfo.fIndexBuffer = target->resourceProvider()->refQuadIndexBuffer();
        if (!vertices.fPtr || !flushInfo.fIndexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        flushInfo.fInstancesToFlush = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Entry& args = fShapes[i];

            ShapeData* shapeData;
            if (fUsesDistanceField) {
                // get mip level
                SkScalar maxScale;
                const SkRect& bounds = args.fShape.bounds();
                if (args.fViewMatrix.hasPerspective()) {
                    // approximate the scale since we can't get it from the matrix
                    SkRect xformedBounds;
                    args.fViewMatrix.mapRect(&xformedBounds, bounds);
                    maxScale = SkScalarAbs(SkTMax(xformedBounds.width() / bounds.width(),
                                                  xformedBounds.height() / bounds.height()));
                } else {
                    maxScale = SkScalarAbs(args.fViewMatrix.getMaxScale());
                }
                SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
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
                SkASSERT(maxScale <= mipScale);

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
                SkScalar desiredDimension = SkTMin(mipSize, kMaxMIP);

                // check to see if df path is cached
                ShapeDataKey key(args.fShape, SkScalarCeilToInt(desiredDimension));
                shapeData = fShapeCache->find(key);
                if (nullptr == shapeData || !fAtlas->hasID(shapeData->fID)) {
                    // Remove the stale cache entry
                    if (shapeData) {
                        fShapeCache->remove(shapeData->fKey);
                        fShapeList->remove(shapeData);
                        delete shapeData;
                    }
                    SkScalar scale = desiredDimension / maxDim;

                    shapeData = new ShapeData;
                    if (!this->addDFPathToAtlas(target,
                                                &flushInfo,
                                                fAtlas,
                                                shapeData,
                                                args.fShape,
                                                SkScalarCeilToInt(desiredDimension),
                                                scale)) {
                        delete shapeData;
                        continue;
                    }
                }
            } else {
                // check to see if bitmap path is cached
                ShapeDataKey key(args.fShape, args.fViewMatrix);
                shapeData = fShapeCache->find(key);
                if (nullptr == shapeData || !fAtlas->hasID(shapeData->fID)) {
                    // Remove the stale cache entry
                    if (shapeData) {
                        fShapeCache->remove(shapeData->fKey);
                        fShapeList->remove(shapeData);
                        delete shapeData;
                    }

                    shapeData = new ShapeData;
                    if (!this->addBMPathToAtlas(target,
                                                &flushInfo,
                                                fAtlas,
                                                shapeData,
                                                args.fShape,
                                                args.fViewMatrix)) {
                        delete shapeData;
                        continue;
                    }
                }
            }

            auto uploadTarget = target->deferredUploadTarget();
            fAtlas->setLastUseToken(shapeData->fID, uploadTarget->tokenTracker()->nextDrawToken());

            this->writePathVertices(fAtlas, vertices, GrVertexColor(args.fColor, fWideColor),
                                    args.fViewMatrix, shapeData);
            flushInfo.fInstancesToFlush++;
        }

        this->flush(target, &flushInfo);
    }

    bool addToAtlas(GrMeshDrawOp::Target* target, FlushInfo* flushInfo, GrDrawOpAtlas* atlas,
                    int width, int height, const void* image,
                    GrDrawOpAtlas::AtlasID* id, SkIPoint16* atlasLocation) const {
        auto resourceProvider = target->resourceProvider();
        auto uploadTarget = target->deferredUploadTarget();

        GrDrawOpAtlas::ErrorCode code = atlas->addToAtlas(resourceProvider, id,
                                                          uploadTarget, width, height,
                                                          image, atlasLocation);
        if (GrDrawOpAtlas::ErrorCode::kError == code) {
            return false;
        }

        if (GrDrawOpAtlas::ErrorCode::kTryAgain == code) {
            this->flush(target, flushInfo);

            code = atlas->addToAtlas(resourceProvider, id, uploadTarget, width, height,
                                     image, atlasLocation);
        }

        return GrDrawOpAtlas::ErrorCode::kSucceeded == code;
    }

    bool addDFPathToAtlas(GrMeshDrawOp::Target* target, FlushInfo* flushInfo,
                          GrDrawOpAtlas* atlas, ShapeData* shapeData, const GrShape& shape,
                          uint32_t dimension, SkScalar scale) const {

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
        // pad to allow room for antialiasing
        const int intPad = SkScalarCeilToInt(kAntiAliasPad);
        // place devBounds at origin
        int width = devPathBounds.width() + 2*intPad;
        int height = devPathBounds.height() + 2*intPad;
        devPathBounds = SkIRect::MakeWH(width, height);
        SkScalar translateX = intPad - dx;
        SkScalar translateY = intPad - dy;

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
#ifndef SK_USE_LEGACY_DISTANCE_FIELDS
        // Generate signed distance field directly from SkPath
        bool succeed = GrGenerateDistanceFieldFromPath((unsigned char*)dfStorage.get(),
                                        path, drawMatrix,
                                        width, height, width * sizeof(unsigned char));
        if (!succeed) {
#endif
            // setup bitmap backing
            SkAutoPixmapStorage dst;
            if (!dst.tryAlloc(SkImageInfo::MakeA8(devPathBounds.width(),
                                                  devPathBounds.height()))) {
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
            draw.fMatrix = &drawMatrix;
            draw.fDst = dst;

            draw.drawPathCoverage(path, paint);

            // Generate signed distance field
            SkGenerateDistanceFieldFromA8Image((unsigned char*)dfStorage.get(),
                                               (const unsigned char*)dst.addr(),
                                               dst.width(), dst.height(), dst.rowBytes());
#ifndef SK_USE_LEGACY_DISTANCE_FIELDS
        }
#endif

        // add to atlas
        SkIPoint16 atlasLocation;
        GrDrawOpAtlas::AtlasID id;

        if (!this->addToAtlas(target, flushInfo, atlas,
                              width, height, dfStorage.get(), &id, &atlasLocation)) {
            return false;
        }

        // add to cache
        shapeData->fKey.set(shape, dimension);
        shapeData->fID = id;

        shapeData->fBounds = SkRect::Make(devPathBounds);
        shapeData->fBounds.offset(-translateX, -translateY);
        shapeData->fBounds.fLeft /= scale;
        shapeData->fBounds.fTop /= scale;
        shapeData->fBounds.fRight /= scale;
        shapeData->fBounds.fBottom /= scale;

        // We pack the 2bit page index in the low bit of the u and v texture coords
        uint16_t pageIndex = GrDrawOpAtlas::GetPageIndexFromID(id);
        SkASSERT(pageIndex < 4);
        uint16_t uBit = (pageIndex >> 1) & 0x1;
        uint16_t vBit = pageIndex & 0x1;
        shapeData->fTextureCoords.set((atlasLocation.fX+SK_DistanceFieldPad) << 1 | uBit,
                                      (atlasLocation.fY+SK_DistanceFieldPad) << 1 | vBit,
                                      (atlasLocation.fX+SK_DistanceFieldPad+
                                       devPathBounds.width()) << 1 | uBit,
                                      (atlasLocation.fY+SK_DistanceFieldPad+
                                       devPathBounds.height()) << 1 | vBit);

        fShapeCache->add(shapeData);
        fShapeList->addToTail(shapeData);
#ifdef DF_PATH_TRACKING
        ++g_NumCachedPaths;
#endif
        return true;
    }

    bool addBMPathToAtlas(GrMeshDrawOp::Target* target, FlushInfo* flushInfo,
                          GrDrawOpAtlas* atlas, ShapeData* shapeData, const GrShape& shape,
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
        // pad to allow room for antialiasing
        const int intPad = SkScalarCeilToInt(kAntiAliasPad);
        // place devBounds at origin
        int width = devPathBounds.width() + 2 * intPad;
        int height = devPathBounds.height() + 2 * intPad;
        devPathBounds = SkIRect::MakeWH(width, height);
        SkScalar translateX = intPad - dx;
        SkScalar translateY = intPad - dy;

        SkASSERT(devPathBounds.fLeft == 0);
        SkASSERT(devPathBounds.fTop == 0);
        SkASSERT(devPathBounds.width() > 0);
        SkASSERT(devPathBounds.height() > 0);

        SkPath path;
        shape.asPath(&path);
        // setup bitmap backing
        SkAutoPixmapStorage dst;
        if (!dst.tryAlloc(SkImageInfo::MakeA8(devPathBounds.width(),
                                              devPathBounds.height()))) {
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
        drawMatrix.postTranslate(translateX, translateY);
        draw.fMatrix = &drawMatrix;
        draw.fDst = dst;

        draw.drawPathCoverage(path, paint);

        // add to atlas
        SkIPoint16 atlasLocation;
        GrDrawOpAtlas::AtlasID id;

        if (!this->addToAtlas(target, flushInfo, atlas,
                              dst.width(), dst.height(), dst.addr(), &id, &atlasLocation)) {
            return false;
        }

        // add to cache
        shapeData->fKey.set(shape, ctm);
        shapeData->fID = id;

        shapeData->fBounds = SkRect::Make(devPathBounds);
        shapeData->fBounds.offset(-translateX, -translateY);

        // We pack the 2bit page index in the low bit of the u and v texture coords
        uint16_t pageIndex = GrDrawOpAtlas::GetPageIndexFromID(id);
        SkASSERT(pageIndex < 4);
        uint16_t uBit = (pageIndex >> 1) & 0x1;
        uint16_t vBit = pageIndex & 0x1;
        shapeData->fTextureCoords.set(atlasLocation.fX << 1 | uBit, atlasLocation.fY << 1 | vBit,
                                      (atlasLocation.fX+width) << 1 | uBit,
                                      (atlasLocation.fY+height) << 1 | vBit);

        fShapeCache->add(shapeData);
        fShapeList->addToTail(shapeData);
#ifdef DF_PATH_TRACKING
        ++g_NumCachedPaths;
#endif
        return true;
    }

    void writePathVertices(GrDrawOpAtlas* atlas,
                           GrVertexWriter& vertices,
                           const GrVertexColor& color,
                           const SkMatrix& ctm,
                           const ShapeData* shapeData) const {
        SkRect translatedBounds(shapeData->fBounds);
        if (!fUsesDistanceField) {
            translatedBounds.offset(SkScalarFloorToScalar(ctm.get(SkMatrix::kMTransX)),
                                    SkScalarFloorToScalar(ctm.get(SkMatrix::kMTransY)));
        }

        // set up texture coordinates
        GrVertexWriter::TriStrip<uint16_t> texCoords{
            (uint16_t)shapeData->fTextureCoords.fLeft,
            (uint16_t)shapeData->fTextureCoords.fTop,
            (uint16_t)shapeData->fTextureCoords.fRight,
            (uint16_t)shapeData->fTextureCoords.fBottom
        };

        if (fUsesDistanceField && !ctm.hasPerspective()) {
            vertices.writeQuad(GrQuad::MakeFromRect(translatedBounds, ctm),
                               color,
                               texCoords);
        } else {
            vertices.writeQuad(GrVertexWriter::TriStripFromRect(translatedBounds),
                               color,
                               texCoords);
        }
    }

    void flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
        GrGeometryProcessor* gp = flushInfo->fGeometryProcessor.get();
        int numAtlasTextures = SkToInt(fAtlas->numActivePages());
        auto proxies = fAtlas->getProxies();
        if (gp->numTextureSamplers() != numAtlasTextures) {
            for (int i = gp->numTextureSamplers(); i < numAtlasTextures; ++i) {
                flushInfo->fFixedDynamicState->fPrimitiveProcessorTextures[i] = proxies[i].get();
            }
            // During preparation the number of atlas pages has increased.
            // Update the proxies used in the GP to match.
            if (fUsesDistanceField) {
                reinterpret_cast<GrDistanceFieldPathGeoProc*>(gp)->addNewProxies(
                    fAtlas->getProxies(), fAtlas->numActivePages(), GrSamplerState::ClampBilerp());
            } else {
                reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewProxies(
                    fAtlas->getProxies(), fAtlas->numActivePages(), GrSamplerState::ClampNearest());
            }
        }

        if (flushInfo->fInstancesToFlush) {
            GrMesh* mesh = target->allocMesh(GrPrimitiveType::kTriangles);
            int maxInstancesPerDraw =
                    static_cast<int>(flushInfo->fIndexBuffer->size() / sizeof(uint16_t) / 6);
            mesh->setIndexedPatterned(flushInfo->fIndexBuffer, kIndicesPerQuad, kVerticesPerQuad,
                                      flushInfo->fInstancesToFlush, maxInstancesPerDraw);
            mesh->setVertexData(flushInfo->fVertexBuffer, flushInfo->fVertexOffset);
            target->recordDraw(
                    flushInfo->fGeometryProcessor, mesh, 1, flushInfo->fFixedDynamicState, nullptr);
            flushInfo->fVertexOffset += kVerticesPerQuad * flushInfo->fInstancesToFlush;
            flushInfo->fInstancesToFlush = 0;
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    const SkPMColor4f& color() const { return fShapes[0].fColor; }
    bool usesDistanceField() const { return fUsesDistanceField; }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        SmallPathOp* that = t->cast<SmallPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        if (this->usesDistanceField() != that->usesDistanceField()) {
            return CombineResult::kCannotCombine;
        }

        const SkMatrix& thisCtm = this->fShapes[0].fViewMatrix;
        const SkMatrix& thatCtm = that->fShapes[0].fViewMatrix;

        if (thisCtm.hasPerspective() != thatCtm.hasPerspective()) {
            return CombineResult::kCannotCombine;
        }

        // We can position on the cpu unless we're in perspective,
        // but also need to make sure local matrices are identical
        if ((thisCtm.hasPerspective() || fHelper.usesLocalCoords()) &&
            !thisCtm.cheapEqualTo(thatCtm)) {
            return CombineResult::kCannotCombine;
        }

        // Depending on the ctm we may have a different shader for SDF paths
        if (this->usesDistanceField()) {
            if (thisCtm.isScaleTranslate() != thatCtm.isScaleTranslate() ||
                thisCtm.isSimilarity() != thatCtm.isSimilarity()) {
                return CombineResult::kCannotCombine;
            }
        }

        fShapes.push_back_n(that->fShapes.count(), that->fShapes.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

    bool fUsesDistanceField;

    struct Entry {
        SkPMColor4f fColor;
        GrShape     fShape;
        SkMatrix    fViewMatrix;
    };

    SkSTArray<1, Entry> fShapes;
    Helper fHelper;
    GrDrawOpAtlas* fAtlas;
    ShapeCache* fShapeCache;
    ShapeDataList* fShapeList;
    bool fGammaCorrect;
    bool fWideColor;

    typedef GrMeshDrawOp INHERITED;
};

bool GrSmallPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrSmallPathRenderer::onDrawPath");

    // we've already bailed on inverse filled paths, so this is safe
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(args.fShape->hasUnstyledKey());
    if (!fAtlas) {
        const GrBackendFormat format =
                args.fContext->priv().caps()->getBackendFormatFromColorType(
                        GrColorType::kAlpha_8);
        fAtlas = GrDrawOpAtlas::Make(args.fContext->priv().proxyProvider(),
                                     format,
                                     GrColorType::kAlpha_8,
                                     ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                     PLOT_WIDTH, PLOT_HEIGHT,
                                     GrDrawOpAtlas::AllowMultitexturing::kYes,
                                     &GrSmallPathRenderer::HandleEviction,
                                     (void*)this);
        if (!fAtlas) {
            return false;
        }
    }

    std::unique_ptr<GrDrawOp> op = SmallPathOp::Make(
            args.fContext, std::move(args.fPaint), *args.fShape, *args.fViewMatrix, fAtlas.get(),
            &fShapeCache, &fShapeList, args.fGammaCorrect, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

struct GrSmallPathRenderer::PathTestStruct {
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

    static void HandleEviction(GrDrawOpAtlas::AtlasID id, void* pr) {
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
    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;
};

std::unique_ptr<GrDrawOp> GrSmallPathRenderer::createOp_TestingOnly(
                                                        GrRecordingContext* context,
                                                        GrPaint&& paint,
                                                        const GrShape& shape,
                                                        const SkMatrix& viewMatrix,
                                                        GrDrawOpAtlas* atlas,
                                                        ShapeCache* shapeCache,
                                                        ShapeDataList* shapeList,
                                                        bool gammaCorrect,
                                                        const GrUserStencilSettings* stencil) {

    return GrSmallPathRenderer::SmallPathOp::Make(context, std::move(paint), shape, viewMatrix,
                                                  atlas, shapeCache, shapeList, gammaCorrect,
                                                  stencil);

}

GR_DRAW_OP_TEST_DEFINE(SmallPathOp) {
    using PathTestStruct = GrSmallPathRenderer::PathTestStruct;
    static PathTestStruct gTestStruct;

    if (context->priv().contextID() != gTestStruct.fContextID) {
        gTestStruct.fContextID = context->priv().contextID();
        gTestStruct.reset();
        const GrBackendFormat format =
                context->priv().caps()->getBackendFormatFromColorType(GrColorType::kAlpha_8);
        gTestStruct.fAtlas = GrDrawOpAtlas::Make(context->priv().proxyProvider(),
                                                 format, GrColorType::kAlpha_8,
                                                 ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                 PLOT_WIDTH, PLOT_HEIGHT,
                                                 GrDrawOpAtlas::AllowMultitexturing::kYes,
                                                 &PathTestStruct::HandleEviction,
                                                 (void*)&gTestStruct);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    bool gammaCorrect = random->nextBool();

    // This path renderer only allows fill styles.
    GrShape shape(GrTest::TestPath(random), GrStyle::SimpleFill());
    return GrSmallPathRenderer::createOp_TestingOnly(
                                         context,
                                         std::move(paint), shape, viewMatrix,
                                         gTestStruct.fAtlas.get(),
                                         &gTestStruct.fShapeCache,
                                         &gTestStruct.fShapeList,
                                         gammaCorrect,
                                         GrGetRandomStencil(random, context));
}

#endif
