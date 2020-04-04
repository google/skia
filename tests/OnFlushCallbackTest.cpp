/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrTexture.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/effects/generated/GrSimpleTextureEffect.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "tests/TestUtils.h"

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
        this->setBounds(r, HasAABloat::kYes, IsHairline::kYes);
    }

    const char* name() const override { return "NonAARectOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip*, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        // Set the color to unknown because the subclass may change the color later.
        GrProcessorAnalysisColor gpColor;
        gpColor.setToUnknown();
        // We ignore the clip so pass this rather than the GrAppliedClip param.
        static GrAppliedClip kNoClip;
        return fHelper.finalizeProcessors(caps, &kNoClip, hasMixedSampledCoverage, clampType,
                                          GrProcessorAnalysisCoverage::kNone, &gpColor);
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

        GrGeometryProcessor* gp = GrDefaultGeoProcFactory::Make(
                                              target->allocator(),
                                              target->caps().shaderCaps(),
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

        target->recordDraw(gp, mesh, 1, GrPrimitiveType::kTriangles);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                                 fHelper.detachProcessorSet(),
                                                                 fHelper.pipelineFlags());

        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline);
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
    // The Atlased ops have an internal singly-linked list of ops that land in the same opsTask
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
    AtlasObject(skiatest::Reporter* reporter) : fDone(false), fReporter(reporter) {}

    ~AtlasObject() override {
        SkASSERT(fDone);
    }

    void markAsDone() {
        fDone = true;
    }

    // Insert the new op in an internal singly-linked list for 'opsTaskID'
    void addOp(uint32_t opsTaskID, AtlasedRectOp* op) {
        LinkedListHeader* header = nullptr;
        for (int i = 0; i < fOps.count(); ++i) {
            if (opsTaskID == fOps[i].fID) {
                header = &(fOps[i]);
            }
        }

        if (!header) {
            fOps.push_back({opsTaskID, nullptr});
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

        const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                     GrRenderable::kYes);

        fAtlasProxy = GrProxyProvider::MakeFullyLazyProxy(
                [format](GrResourceProvider* resourceProvider)
                        -> GrSurfaceProxy::LazyCallbackResult {
                    GrSurfaceDesc desc;
                    // TODO: until partial flushes in MDB lands we're stuck having
                    // all 9 atlas draws occur
                    desc.fWidth = 9 /*this->numOps()*/ * kAtlasTileSize;
                    desc.fHeight = kAtlasTileSize;
                    desc.fConfig = kRGBA_8888_GrPixelConfig;

                    return resourceProvider->createTexture(desc, format, GrRenderable::kYes, 1,
                                                           GrMipMapped::kNo, SkBudgeted::kYes,
                                                           GrProtected::kNo);
                },
                format,
                GrRenderable::kYes,
                1,
                GrProtected::kNo,
                kBottomLeft_GrSurfaceOrigin,
                kRGBA_8888_GrPixelConfig,
                *proxyProvider->caps(),
                GrSurfaceProxy::UseAllocator::kNo);

        return fAtlasProxy;
    }

    /*
     * This callback creates the atlas and updates the AtlasedRectOps to read from it
     */
    void preFlush(GrOnFlushResourceProvider* resourceProvider,
                  const uint32_t* opsTaskIDs,
                  int numOpsTaskIDs) override {
        // Until MDB is landed we will most-likely only have one opsTask.
        SkTDArray<LinkedListHeader*> lists;
        for (int i = 0; i < numOpsTaskIDs; ++i) {
            if (LinkedListHeader* list = this->getList(opsTaskIDs[i])) {
                lists.push_back(list);
            }
        }

        if (!lists.count()) {
            return; // nothing to atlas
        }

        if (!resourceProvider->instatiateProxy(fAtlasProxy.get())) {
            return;
        }

        // At this point 'fAtlasProxy' should be instantiated and have:
        //    1 ref from the 'fAtlasProxy' sk_sp
        //    9 refs from the 9 AtlasedRectOps
        // The backing GrSurface should have only 1 though bc there is only one proxy
        CheckSingleThreadedProxyRefs(fReporter, fAtlasProxy.get(), 10, 1);
        auto rtc = resourceProvider->makeRenderTargetContext(fAtlasProxy, GrColorType::kRGBA_8888,
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
    }

private:
    typedef struct {
        uint32_t       fID;
        AtlasedRectOp* fHead;
    } LinkedListHeader;

    LinkedListHeader* getList(uint32_t opsTaskID) {
        for (int i = 0; i < fOps.count(); ++i) {
            if (opsTaskID == fOps[i].fID) {
                return &(fOps[i]);
            }
        }
        return nullptr;
    }

    void clearOpsFor(LinkedListHeader* header) {
        // The AtlasedRectOps have yet to execute (and this class doesn't own them) so just
        // forget about them in the laziest way possible.
        header->fHead = nullptr;
        header->fID = 0;            // invalid opsTask ID
    }

    // Each opsTask containing AtlasedRectOps gets its own internal singly-linked list
    SkTDArray<LinkedListHeader>  fOps;

    // The fully lazy proxy for the atlas
    sk_sp<GrTextureProxy>        fAtlasProxy;

    // Set to true when the testing harness expects this object to be no longer used
    bool                         fDone;

    skiatest::Reporter*           fReporter;
};

// This creates an off-screen rendertarget whose ops which eventually pull from the atlas.
static sk_sp<GrTextureProxy> make_upstream_image(GrContext* context, AtlasObject* object, int start,
                                                 sk_sp<GrTextureProxy> atlasProxy,
                                                 SkAlphaType atlasAlphaType) {
    auto rtc = context->priv().makeDeferredRenderTargetContext(SkBackingFit::kApprox,
                                                               3* kDrawnTileSize,
                                                               kDrawnTileSize,
                                                               GrColorType::kRGBA_8888,
                                                               nullptr);

    rtc->clear(nullptr, { 1, 0, 0, 1 }, GrRenderTargetContext::CanClearFullscreen::kYes);

    for (int i = 0; i < 3; ++i) {
        SkRect r = SkRect::MakeXYWH(i*kDrawnTileSize, 0, kDrawnTileSize, kDrawnTileSize);

        auto fp = GrSimpleTextureEffect::Make(atlasProxy, atlasAlphaType, SkMatrix::I());
        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        std::unique_ptr<AtlasedRectOp> op(AtlasedRectOp::Make(context,
                                                              std::move(paint), r, start + i));

        AtlasedRectOp* sparePtr = op.get();

        uint32_t opsTaskID;
        rtc->priv().testingOnly_addDrawOp(GrNoClip(), std::move(op),
                                          [&opsTaskID](GrOp* op, uint32_t id) { opsTaskID = id; });
        SkASSERT(SK_InvalidUniqueID != opsTaskID);

        object->addOp(opsTaskID, sparePtr);
    }

    return rtc->asTextureProxyRef();
}

// Enable this if you want to debug the final draws w/o having the atlasCallback create the
// atlas
#if 0
#include "SkGrPriv.h"
#include "include/core/SkImageEncoder.h"
#include "tools/ToolUtils.h"

static void save_bm(const SkBitmap& bm, const char name[]) {
    bool result = ToolUtils::EncodeImageToFile(name, bm, SkEncodedImageFormat::kPNG, 100);
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

    AtlasObject object(reporter);

    context->priv().addOnFlushCallbackObject(&object);

    sk_sp<GrTextureProxy> proxies[kNumProxies];
    for (int i = 0; i < kNumProxies; ++i) {
        proxies[i] = make_upstream_image(
                context, &object, i*3,
                object.getAtlasProxy(proxyProvider, context->priv().caps()), kPremul_SkAlphaType);
    }

    static const int kFinalWidth = 6*kDrawnTileSize;
    static const int kFinalHeight = kDrawnTileSize;

    auto rtc = context->priv().makeDeferredRenderTargetContext(
            SkBackingFit::kApprox, kFinalWidth, kFinalHeight, GrColorType::kRGBA_8888, nullptr);

    rtc->clear(nullptr, SK_PMColor4fWHITE, GrRenderTargetContext::CanClearFullscreen::kYes);

    // Note that this doesn't include the third texture proxy
    for (int i = 0; i < kNumProxies-1; ++i) {
        SkRect r = SkRect::MakeXYWH(i*3*kDrawnTileSize, 0, 3*kDrawnTileSize, kDrawnTileSize);

        SkMatrix t = SkMatrix::MakeTrans(-i*3*kDrawnTileSize, 0);

        GrPaint paint;
        auto fp = GrSimpleTextureEffect::Make(std::move(proxies[i]), kPremul_SkAlphaType, t);
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));

        rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);
    }

    rtc->flush(SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo());

    SkBitmap readBack;
    readBack.allocN32Pixels(kFinalWidth, kFinalHeight);

    SkDEBUGCODE(bool result =) rtc->readPixels(readBack.info(), readBack.getPixels(),
                                               readBack.rowBytes(), {0, 0});
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
