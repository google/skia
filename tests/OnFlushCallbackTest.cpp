/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "GrBackendSemaphore.h"
#include "GrClip.h"
#include "GrContextPriv.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrOnFlushResourceProvider.h"
#include "GrProxyProvider.h"
#include "GrQuad.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"

#include "SkBitmap.h"
#include "SkPointPriv.h"
#include "effects/GrSimpleTextureEffect.h"
#include "ops/GrSimpleMeshDrawOpHelper.h"

namespace {
// This is a simplified mesh drawing op that can be used in the atlas generation test.
// Please see AtlasedRectOp below.
class NonAARectOp : public GrMeshDrawOp {
protected:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    // This creates an instance of a simple non-AA solid color rect-drawing Op
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkRect& r) {
        return Helper::FactoryHelper<NonAARectOp>(context, std::move(paint), r, nullptr, ClassID());
    }

    // This creates an instance of a simple non-AA textured rect-drawing Op
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkRect& r,
                                          const SkRect& local) {
        return Helper::FactoryHelper<NonAARectOp>(context, std::move(paint), r, &local, ClassID());
    }

    const SkPMColor4f& color() const { return fColor; }

    NonAARectOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color, const SkRect& r,
                const SkRect* localRect, int32_t classID)
            : INHERITED(classID)
            , fColor(color)
            , fHasLocalRect(SkToBool(localRect))
            , fRect(r)
            , fHelper(helperArgs, GrAAType::kNone) {
        if (fHasLocalRect) {
            fLocalQuad = GrQuad(*localRect);
        }
        // Choose some conservative values for aa bloat and zero area.
        this->setBounds(r, HasAABloat::kYes, IsZeroArea::kYes);
    }

    const char* name() const override { return "NonAARectOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        fHelper.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip*,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        // Set the color to unknown because the subclass may change the color later.
        GrProcessorAnalysisColor gpColor;
        gpColor.setToUnknown();
        // We ignore the clip so pass this rather than the GrAppliedClip param.
        static GrAppliedClip kNoClip;
        return fHelper.finalizeProcessors(
                caps, &kNoClip, fsaaType, clampType, GrProcessorAnalysisCoverage::kNone, &gpColor);
    }

protected:
    SkPMColor4f fColor;
    bool        fHasLocalRect;
    GrQuad      fLocalQuad;
    SkRect      fRect;

private:
    void onPrepareDraws(Target* target) override {
        using namespace GrDefaultGeoProcFactory;

        // The vertex attrib order is always pos, color, local coords.
        static const int kColorOffset = sizeof(SkPoint);
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);

        sk_sp<GrGeometryProcessor> gp =
                GrDefaultGeoProcFactory::Make(target->caps().shaderCaps(),
                                              Color::kPremulGrColorAttribute_Type,
                                              Coverage::kSolid_Type,
                                              fHasLocalRect ? LocalCoords::kHasExplicit_Type
                                                            : LocalCoords::kUnused_Type,
                                              SkMatrix::I());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor for GrAtlasedOp\n");
            return;
        }

        size_t vertexStride = gp->vertexStride();

        sk_sp<const GrBuffer> indexBuffer;
        int firstIndex;
        uint16_t* indices = target->makeIndexSpace(6, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Indices could not be allocated for GrAtlasedOp.\n");
            return;
        }

        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;
        void* vertices = target->makeVertexSpace(vertexStride, 4, &vertexBuffer, &firstVertex);
        if (!vertices) {
            SkDebugf("Vertices could not be allocated for GrAtlasedOp.\n");
            return;
        }

        // Setup indices
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 2;
        indices[4] = 1;
        indices[5] = 3;

        // Setup positions
        SkPoint* position = (SkPoint*) vertices;
        SkPointPriv::SetRectTriStrip(position, fRect, vertexStride);

        // Setup vertex colors
        GrColor* color = (GrColor*)((intptr_t)vertices + kColorOffset);
        for (int i = 0; i < 4; ++i) {
            *color = fColor.toBytes_RGBA();
            color = (GrColor*)((intptr_t)color + vertexStride);
        }

        // Setup local coords
        if (fHasLocalRect) {
            SkPoint* coords = (SkPoint*)((intptr_t) vertices + kLocalOffset);
            for (int i = 0; i < 4; i++) {
                *coords = fLocalQuad.point(i);
                coords = (SkPoint*)((intptr_t) coords + vertexStride);
            }
        }

        GrMesh* mesh = target->allocMesh(GrPrimitiveType::kTriangles);
        mesh->setIndexed(indexBuffer, 6, firstIndex, 0, 3, GrPrimitiveRestart::kNo);
        mesh->setVertexData(vertexBuffer, firstVertex);

        target->recordDraw(std::move(gp), mesh);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    Helper fHelper;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

static constexpr SkRect kEmptyRect = SkRect::MakeEmpty();

namespace {

/*
 * Atlased ops just draw themselves as textured rects with the texture pixels being
 * pulled out of the atlas. Their color is based on their ID.
 */
class AtlasedRectOp final : public NonAARectOp {
public:
    DEFINE_OP_CLASS_ID

    ~AtlasedRectOp() override {
        fID = -1;
    }

    const char* name() const override { return "AtlasedRectOp"; }

    int id() const { return fID; }

    static std::unique_ptr<AtlasedRectOp> Make(GrContext* context,
                                               GrPaint&& paint,
                                               const SkRect& r,
                                               int id) {
        GrDrawOp* op = Helper::FactoryHelper<AtlasedRectOp>(context, std::move(paint),
                                                            r, id).release();
        return std::unique_ptr<AtlasedRectOp>(static_cast<AtlasedRectOp*>(op));
    }

    // We set the initial color of the NonAARectOp based on the ID.
    // Note that we force creation of a NonAARectOp that has local coords in anticipation of
    // pulling from the atlas.
    AtlasedRectOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color, const SkRect& r,
                  int id)
            : INHERITED(helperArgs, SkPMColor4f::FromBytes_RGBA(kColors[id]), r, &kEmptyRect,
                        ClassID())
            , fID(id)
            , fNext(nullptr) {
        SkASSERT(fID < kMaxIDs);
    }

    void setColor(const SkPMColor4f& color) { fColor = color; }
    void setLocalRect(const SkRect& localRect) {
        SkASSERT(fHasLocalRect);    // This should've been created to anticipate this
        fLocalQuad = GrQuad(localRect);
    }

    AtlasedRectOp* next() const { return fNext; }
    void setNext(AtlasedRectOp* next) {
        fNext = next;
    }

private:

    static const int kMaxIDs = 9;
    static const GrColor kColors[kMaxIDs];

    int            fID;
    // The Atlased ops have an internal singly-linked list of ops that land in the same opList
    AtlasedRectOp* fNext;

    typedef NonAARectOp INHERITED;
};

}  // anonymous namespace

const GrColor AtlasedRectOp::kColors[kMaxIDs] = {
    GrColorPackRGBA(255, 0, 0, 255),
    GrColorPackRGBA(0, 255, 0, 255),
    GrColorPackRGBA(0, 0, 255, 255),
    GrColorPackRGBA(0, 255, 255, 255),
    GrColorPackRGBA(255, 0, 255, 255),
    GrColorPackRGBA(255, 255, 0, 255),
    GrColorPackRGBA(0, 0, 0, 255),
    GrColorPackRGBA(128, 128, 128, 255),
    GrColorPackRGBA(255, 255, 255, 255)
};

static const int kDrawnTileSize = 16;

/*
 * Rather than performing any rect packing, this atlaser just lays out constant-sized
 * tiles in an Nx1 row
 */
static const int kAtlasTileSize = 2;

/*
 * This class aggregates the op information required for atlasing
 */
class AtlasObject final : public GrOnFlushCallbackObject {
public:
    AtlasObject() : fDone(false) { }

    ~AtlasObject() override {
        SkASSERT(fDone);
    }

    void markAsDone() {
        fDone = true;
    }

    // Insert the new op in an internal singly-linked list for 'opListID'
    void addOp(uint32_t opListID, AtlasedRectOp* op) {
        LinkedListHeader* header = nullptr;
        for (int i = 0; i < fOps.count(); ++i) {
            if (opListID == fOps[i].fID) {
                header = &(fOps[i]);
            }
        }

        if (!header) {
            fOps.push_back({opListID, nullptr});
            header = &(fOps[fOps.count()-1]);
        }

        op->setNext(header->fHead);
        header->fHead = op;
    }

    int numOps() const { return fOps.count(); }

    // Get the fully lazy proxy that is backing the atlas. Its actual width isn't
    // known until flush time.
    sk_sp<GrTextureProxy> getAtlasProxy(GrProxyProvider* proxyProvider, const GrCaps* caps) {
        if (fAtlasProxy) {
            return fAtlasProxy;
        }

        const GrBackendFormat format = caps->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

        fAtlasProxy = GrProxyProvider::MakeFullyLazyProxy(
                [](GrResourceProvider* resourceProvider) {
                    if (!resourceProvider) {
                        return sk_sp<GrTexture>();
                    }

                    GrSurfaceDesc desc;
                    desc.fFlags = kRenderTarget_GrSurfaceFlag;
                    // TODO: until partial flushes in MDB lands we're stuck having
                    // all 9 atlas draws occur
                    desc.fWidth = 9 /*this->numOps()*/ * kAtlasTileSize;
                    desc.fHeight = kAtlasTileSize;
                    desc.fConfig = kRGBA_8888_GrPixelConfig;

                    return resourceProvider->createTexture(desc, SkBudgeted::kYes,
                                                           GrResourceProvider::Flags::kNoPendingIO);
                },
                format,
                GrProxyProvider::Renderable::kYes,
                kBottomLeft_GrSurfaceOrigin,
                kRGBA_8888_GrPixelConfig,
                *proxyProvider->caps());
        return fAtlasProxy;
    }

    /*
     * This callback creates the atlas and updates the AtlasedRectOps to read from it
     */
    void preFlush(GrOnFlushResourceProvider* resourceProvider,
                  const uint32_t* opListIDs, int numOpListIDs,
                  SkTArray<sk_sp<GrRenderTargetContext>>* results) override {
        SkASSERT(!results->count());

        // Until MDB is landed we will most-likely only have one opList.
        SkTDArray<LinkedListHeader*> lists;
        for (int i = 0; i < numOpListIDs; ++i) {
            if (LinkedListHeader* list = this->getList(opListIDs[i])) {
                lists.push_back(list);
            }
        }

        if (!lists.count()) {
            return; // nothing to atlas
        }

        if (!resourceProvider->instatiateProxy(fAtlasProxy.get())) {
            return;
        }

        // At this point all the GrAtlasedOp's should have lined up to read from 'atlasDest' and
        // there should either be two writes to clear it or no writes.
        SkASSERT(9 == fAtlasProxy->getPendingReadCnt_TestOnly());
        SkASSERT(2 == fAtlasProxy->getPendingWriteCnt_TestOnly() ||
                 0 == fAtlasProxy->getPendingWriteCnt_TestOnly());
        sk_sp<GrRenderTargetContext> rtc = resourceProvider->makeRenderTargetContext(
                                                                           fAtlasProxy,
                                                                           nullptr, nullptr);

        // clear the atlas
        rtc->clear(nullptr, SK_PMColor4fTRANSPARENT,
                   GrRenderTargetContext::CanClearFullscreen::kYes);

        int blocksInAtlas = 0;
        for (int i = 0; i < lists.count(); ++i) {
            for (AtlasedRectOp* op = lists[i]->fHead; op; op = op->next()) {
                SkIRect r = SkIRect::MakeXYWH(blocksInAtlas*kAtlasTileSize, 0,
                                              kAtlasTileSize, kAtlasTileSize);

                // For now, we avoid the resource buffer issues and just use clears
#if 1
                rtc->clear(&r, op->color(), GrRenderTargetContext::CanClearFullscreen::kNo);
#else
                GrPaint paint;
                paint.setColor4f(op->color());
                std::unique_ptr<GrDrawOp> drawOp(NonAARectOp::Make(std::move(paint),
                                                                   SkRect::Make(r)));
                rtc->priv().testingOnly_addDrawOp(std::move(drawOp));
#endif
                blocksInAtlas++;

                // Set the atlased Op's color to white (so we know we're not using it for
                // the final draw).
                op->setColor(SK_PMColor4fWHITE);

                // Set the atlased Op's localRect to point to where it landed in the atlas
                op->setLocalRect(SkRect::Make(r));
            }

            // We've updated all these ops and we certainly don't want to process them again
            this->clearOpsFor(lists[i]);
        }

        results->push_back(std::move(rtc));
    }

private:
    typedef struct {
        uint32_t       fID;
        AtlasedRectOp* fHead;
    } LinkedListHeader;

    LinkedListHeader* getList(uint32_t opListID) {
        for (int i = 0; i < fOps.count(); ++i) {
            if (opListID == fOps[i].fID) {
                return &(fOps[i]);
            }
        }
        return nullptr;
    }

    void clearOpsFor(LinkedListHeader* header) {
        // The AtlasedRectOps have yet to execute (and this class doesn't own them) so just
        // forget about them in the laziest way possible.
        header->fHead = nullptr;
        header->fID = 0;            // invalid opList ID
    }

    // Each opList containing AtlasedRectOps gets its own internal singly-linked list
    SkTDArray<LinkedListHeader>  fOps;

    // The fully lazy proxy for the atlas
    sk_sp<GrTextureProxy>        fAtlasProxy;

    // Set to true when the testing harness expects this object to be no longer used
    bool                         fDone;
};

// This creates an off-screen rendertarget whose ops which eventually pull from the atlas.
static sk_sp<GrTextureProxy> make_upstream_image(GrContext* context, AtlasObject* object, int start,
                                                 sk_sp<GrTextureProxy> atlasProxy) {
    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    sk_sp<GrRenderTargetContext> rtc(context->priv().makeDeferredRenderTargetContext(
                                                                      format,
                                                                      SkBackingFit::kApprox,
                                                                      3*kDrawnTileSize,
                                                                      kDrawnTileSize,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    rtc->clear(nullptr, { 1, 0, 0, 1 }, GrRenderTargetContext::CanClearFullscreen::kYes);

    for (int i = 0; i < 3; ++i) {
        SkRect r = SkRect::MakeXYWH(i*kDrawnTileSize, 0, kDrawnTileSize, kDrawnTileSize);

        auto fp = GrSimpleTextureEffect::Make(atlasProxy, SkMatrix::I());
        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        std::unique_ptr<AtlasedRectOp> op(AtlasedRectOp::Make(context,
                                                              std::move(paint), r, start + i));

        AtlasedRectOp* sparePtr = op.get();

        uint32_t opListID;
        rtc->priv().testingOnly_addDrawOp(GrNoClip(), std::move(op),
                                          [&opListID](GrOp* op, uint32_t id) { opListID = id; });
        SkASSERT(SK_InvalidUniqueID != opListID);

        object->addOp(opListID, sparePtr);
    }

    return rtc->asTextureProxyRef();
}

// Enable this if you want to debug the final draws w/o having the atlasCallback create the
// atlas
#if 0
#include "SkImageEncoder.h"
#include "SkGrPriv.h"
#include "sk_tool_utils.h"

static void save_bm(const SkBitmap& bm, const char name[]) {
    bool result = sk_tool_utils::EncodeImageToFile(name, bm, SkEncodedImageFormat::kPNG, 100);
    SkASSERT(result);
}

sk_sp<GrTextureProxy> pre_create_atlas(GrContext* context) {
    SkBitmap bm;
    bm.allocN32Pixels(18, 2, true);
    bm.erase(SK_ColorRED,     SkIRect::MakeXYWH(0, 0, 2, 2));
    bm.erase(SK_ColorGREEN,   SkIRect::MakeXYWH(2, 0, 2, 2));
    bm.erase(SK_ColorBLUE,    SkIRect::MakeXYWH(4, 0, 2, 2));
    bm.erase(SK_ColorCYAN,    SkIRect::MakeXYWH(6, 0, 2, 2));
    bm.erase(SK_ColorMAGENTA, SkIRect::MakeXYWH(8, 0, 2, 2));
    bm.erase(SK_ColorYELLOW,  SkIRect::MakeXYWH(10, 0, 2, 2));
    bm.erase(SK_ColorBLACK,   SkIRect::MakeXYWH(12, 0, 2, 2));
    bm.erase(SK_ColorGRAY,    SkIRect::MakeXYWH(14, 0, 2, 2));
    bm.erase(SK_ColorWHITE,   SkIRect::MakeXYWH(16, 0, 2, 2));

#if 1
    save_bm(bm, "atlas-fake.png");
#endif

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bm.info());
    desc.fFlags |= kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurfaceProxy> tmp = GrSurfaceProxy::MakeDeferred(*context->caps(),
                                                             context->textureProvider(),
                                                             desc, SkBudgeted::kYes,
                                                             bm.getPixels(), bm.rowBytes());

    return sk_ref_sp(tmp->asTextureProxy());
}
#endif


static void test_color(skiatest::Reporter* reporter, const SkBitmap& bm, int x, SkColor expected) {
    SkColor readback = bm.getColor(x, kDrawnTileSize/2);
    REPORTER_ASSERT(reporter, expected == readback);
    if (expected != readback) {
        SkDebugf("Color mismatch: %x %x\n", expected, readback);
    }
}

/*
 * For the atlasing test we make a DAG that looks like:
 *
 *    RT1 with ops: 0,1,2       RT2 with ops: 3,4,5       RT3 with ops: 6,7,8
 *                     \         /
 *                      \       /
 *                         RT4
 * We then flush RT4 and expect only ops 0-5 to be atlased together.
 * Each op is just a solid colored rect so both the atlas and the final image should appear as:
 *           R G B C M Y
 * with the atlas having width = 6*kAtlasTileSize and height = kAtlasTileSize.
 *
 * Note: until partial flushes in MDB lands, the atlas will actually have width= 9*kAtlasTileSize
 * and look like:
 *           R G B C M Y K Grey White
 */
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(OnFlushCallbackTest, reporter, ctxInfo) {
    static const int kNumProxies = 3;

    GrContext* context = ctxInfo.grContext();
    auto proxyProvider = context->priv().proxyProvider();

    AtlasObject object;

    context->priv().addOnFlushCallbackObject(&object);

    sk_sp<GrTextureProxy> proxies[kNumProxies];
    for (int i = 0; i < kNumProxies; ++i) {
        proxies[i] = make_upstream_image(context, &object, i*3,
                                         object.getAtlasProxy(proxyProvider,
                                                              context->priv().caps()));
    }

    static const int kFinalWidth = 6*kDrawnTileSize;
    static const int kFinalHeight = kDrawnTileSize;

    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    sk_sp<GrRenderTargetContext> rtc(context->priv().makeDeferredRenderTargetContext(
                                                                      format,
                                                                      SkBackingFit::kApprox,
                                                                      kFinalWidth,
                                                                      kFinalHeight,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    rtc->clear(nullptr, SK_PMColor4fWHITE, GrRenderTargetContext::CanClearFullscreen::kYes);

    // Note that this doesn't include the third texture proxy
    for (int i = 0; i < kNumProxies-1; ++i) {
        SkRect r = SkRect::MakeXYWH(i*3*kDrawnTileSize, 0, 3*kDrawnTileSize, kDrawnTileSize);

        SkMatrix t = SkMatrix::MakeTrans(-i*3*kDrawnTileSize, 0);

        GrPaint paint;
        auto fp = GrSimpleTextureEffect::Make(std::move(proxies[i]), t);
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));

        rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);
    }

    rtc->prepareForExternalIO(SkSurface::BackendSurfaceAccess::kNoAccess,
                              kNone_GrFlushFlags, 0, nullptr, nullptr, nullptr);

    SkBitmap readBack;
    readBack.allocN32Pixels(kFinalWidth, kFinalHeight);

    SkDEBUGCODE(bool result =) rtc->readPixels(readBack.info(), readBack.getPixels(),
                                               readBack.rowBytes(), 0, 0);
    SkASSERT(result);

    context->priv().testingOnly_flushAndRemoveOnFlushCallbackObject(&object);

    object.markAsDone();

    int x = kDrawnTileSize/2;
    test_color(reporter, readBack, x, SK_ColorRED);
    x += kDrawnTileSize;
    test_color(reporter, readBack, x, SK_ColorGREEN);
    x += kDrawnTileSize;
    test_color(reporter, readBack, x, SK_ColorBLUE);
    x += kDrawnTileSize;
    test_color(reporter, readBack, x, SK_ColorCYAN);
    x += kDrawnTileSize;
    test_color(reporter, readBack, x, SK_ColorMAGENTA);
    x += kDrawnTileSize;
    test_color(reporter, readBack, x, SK_ColorYELLOW);
}
