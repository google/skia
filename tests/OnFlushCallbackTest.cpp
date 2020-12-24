/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/effects/GrTextureEffect.h"
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
    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkRect& r) {
        return Helper::FactoryHelper<NonAARectOp>(context, std::move(paint), r, nullptr, ClassID());
    }

    // This creates an instance of a simple non-AA textured rect-drawing Op
    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkRect& r,
                            const SkRect& local) {
        return Helper::FactoryHelper<NonAARectOp>(context, std::move(paint), r, &local, ClassID());
    }

    const SkPMColor4f& color() const { return fColor; }

    NonAARectOp(GrProcessorSet* processorSet, const SkPMColor4f& color, const SkRect& r,
                const SkRect* localRect, int32_t classID)
            : INHERITED(classID)
            , fColor(color)
            , fHasLocalRect(SkToBool(localRect))
            , fRect(r)
            , fHelper(processorSet, GrAAType::kNone) {
        if (fHasLocalRect) {
            fLocalQuad = GrQuad(*localRect);
        }
        // Choose some conservative values for aa bloat and zero area.
        this->setBounds(r, HasAABloat::kYes, IsHairline::kYes);
    }

    const char* name() const override { return "NonAARectOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip*, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        // Set the color to unknown because the subclass may change the color later.
        GrProcessorAnalysisColor gpColor;
        gpColor.setToUnknown();
        // We ignore the clip so pass this rather than the GrAppliedClip param.
        static GrAppliedClip kNoClip = GrAppliedClip::Disabled();
        return fHelper.finalizeProcessors(caps, &kNoClip, hasMixedSampledCoverage, clampType,
                                          GrProcessorAnalysisCoverage::kNone, &gpColor);
    }

protected:
    SkPMColor4f fColor;
    bool        fHasLocalRect;
    GrQuad      fLocalQuad;
    SkRect      fRect;

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             GrAppliedClip&& appliedClip,
                             const GrXferProcessor::DstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        using namespace GrDefaultGeoProcFactory;

        GrGeometryProcessor* gp = GrDefaultGeoProcFactory::Make(
                                              arena,
                                              Color::kPremulGrColorAttribute_Type,
                                              Coverage::kSolid_Type,
                                              fHasLocalRect ? LocalCoords::kHasExplicit_Type
                                                            : LocalCoords::kUnused_Type,
                                              SkMatrix::I());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        fProgramInfo = fHelper.createProgramInfo(caps, arena, writeView, std::move(appliedClip),
                                                 dstProxyView, gp, GrPrimitiveType::kTriangles,
                                                 renderPassXferBarriers, colorLoadOp);
    }

    void onPrepareDraws(Target* target) override {

        // The vertex attrib order is always pos, color, local coords.
        static const int kColorOffset = sizeof(SkPoint);
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);

        if (!fProgramInfo) {
            this->createProgramInfo(target);
            if (!fProgramInfo) {
                return;
            }
        }

        size_t vertexStride =  fProgramInfo->primProc().vertexStride();

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

        fMesh = target->allocMesh();
        fMesh->setIndexed(indexBuffer, 6, firstIndex, 0, 3, GrPrimitiveRestart::kNo, vertexBuffer,
                          firstVertex);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->primProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    Helper         fHelper;
    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
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

    static GrOp::Owner Make(GrRecordingContext* rContext,
                            GrPaint&& paint,
                            const SkRect& r,
                            int id) {
        return Helper::FactoryHelper<AtlasedRectOp>(rContext, std::move(paint), r, id);
    }

    // We set the initial color of the NonAARectOp based on the ID.
    // Note that we force creation of a NonAARectOp that has local coords in anticipation of
    // pulling from the atlas.
    AtlasedRectOp(GrProcessorSet* processorSet, const SkPMColor4f& color, const SkRect& r,
                  int id)
            : INHERITED(processorSet, SkPMColor4f::FromBytes_RGBA(kColors[id]), r, &kEmptyRect,
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

    using INHERITED = NonAARectOp;
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
    GrSurfaceProxyView getAtlasView(GrProxyProvider* proxyProvider, const GrCaps* caps) {
        if (fAtlasView) {
            return fAtlasView;
        }

        const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                     GrRenderable::kYes);
        auto proxy = GrProxyProvider::MakeFullyLazyProxy(
                [](GrResourceProvider* resourceProvider,
                   const GrSurfaceProxy::LazySurfaceDesc& desc)
                        -> GrSurfaceProxy::LazyCallbackResult {
                    SkASSERT(desc.fDimensions.width() < 0 && desc.fDimensions.height() < 0);
                    SkISize dims;
                    // TODO: until partial flushes in MDB lands we're stuck having
                    // all 9 atlas draws occur
                    dims.fWidth = 9 /*this->numOps()*/ * kAtlasTileSize;
                    dims.fHeight = kAtlasTileSize;

                    return resourceProvider->createTexture(dims, desc.fFormat, desc.fRenderable,
                                                           desc.fSampleCnt, desc.fMipmapped,
                                                           desc.fBudgeted, desc.fProtected);
                },
                format,
                GrRenderable::kYes,
                1,
                GrProtected::kNo,
                *proxyProvider->caps(),
                GrSurfaceProxy::UseAllocator::kNo);

        GrSwizzle readSwizzle = caps->getReadSwizzle(format, GrColorType::kRGBA_8888);
        fAtlasView = {std::move(proxy), kBottomLeft_GrSurfaceOrigin, readSwizzle};
        return fAtlasView;
    }

    /*
     * This callback creates the atlas and updates the AtlasedRectOps to read from it
     */
    void preFlush(GrOnFlushResourceProvider* resourceProvider,
                  SkSpan<const uint32_t> renderTaskIDs) override {
        // Until MDB is landed we will most-likely only have one opsTask.
        SkTDArray<LinkedListHeader*> lists;
        for (uint32_t taskID : renderTaskIDs) {
            if (LinkedListHeader* list = this->getList(taskID)) {
                lists.push_back(list);
            }
        }

        if (!lists.count()) {
            return; // nothing to atlas
        }

        if (!resourceProvider->instatiateProxy(fAtlasView.proxy())) {
            return;
        }

        // At this point 'fAtlasView' proxy should be instantiated and have:
        //    1 ref from the 'fAtlasView' proxy sk_sp
        //    9 refs from the 9 AtlasedRectOps
        // The backing GrSurface should have only 1 though bc there is only one proxy
        CheckSingleThreadedProxyRefs(fReporter, fAtlasView.proxy(), 10, 1);
        auto rtc = resourceProvider->makeRenderTargetContext(
                fAtlasView.refProxy(), fAtlasView.origin(), GrColorType::kRGBA_8888, nullptr,
                nullptr);

        // clear the atlas
        rtc->clear(SK_PMColor4fTRANSPARENT);

        int blocksInAtlas = 0;
        for (int i = 0; i < lists.count(); ++i) {
            for (AtlasedRectOp* op = lists[i]->fHead; op; op = op->next()) {
                SkIRect r = SkIRect::MakeXYWH(blocksInAtlas*kAtlasTileSize, 0,
                                              kAtlasTileSize, kAtlasTileSize);

                // For now, we avoid the resource buffer issues and just use clears
#if 1
                rtc->clear(r, op->color());
#else
                GrPaint paint;
                paint.setColor4f(op->color());
                std::unique_ptr<GrDrawOp> drawOp(NonAARectOp::Make(std::move(paint),
                                                                   SkRect::Make(r)));
                rtc->addDrawOp(std::move(drawOp));
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
    SkTDArray<LinkedListHeader> fOps;

    // The fully lazy proxy for the atlas
    GrSurfaceProxyView fAtlasView;

    // Set to true when the testing harness expects this object to be no longer used
    bool fDone;

    skiatest::Reporter* fReporter;
};

// This creates an off-screen rendertarget whose ops which eventually pull from the atlas.
static GrSurfaceProxyView make_upstream_image(GrRecordingContext* rContext,
                                              AtlasObject* object,
                                              int start,
                                              GrSurfaceProxyView atlasView,
                                              SkAlphaType atlasAlphaType) {
    auto rtc = GrSurfaceDrawContext::Make(
            rContext, GrColorType::kRGBA_8888, nullptr,
            SkBackingFit::kApprox, {3 * kDrawnTileSize, kDrawnTileSize});

    rtc->clear(SkPMColor4f{1, 0, 0, 1});

    for (int i = 0; i < 3; ++i) {
        SkRect r = SkRect::MakeXYWH(i*kDrawnTileSize, 0, kDrawnTileSize, kDrawnTileSize);

        auto fp = GrTextureEffect::Make(atlasView, atlasAlphaType);
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        GrOp::Owner op = AtlasedRectOp::Make(rContext, std::move(paint), r, start + i);
        AtlasedRectOp* sparePtr = (AtlasedRectOp*)op.get();

        uint32_t opsTaskID;
        rtc->addDrawOp(nullptr, std::move(op),
                       [&opsTaskID](GrOp* op, uint32_t id) { opsTaskID = id; });
        SkASSERT(SK_InvalidUniqueID != opsTaskID);

        object->addOp(opsTaskID, sparePtr);
    }

    return rtc->readSurfaceView();
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

sk_sp<GrTextureProxy> pre_create_atlas(GrRecordingContext* rContext) {
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

    desc.fFlags |= kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurfaceProxy> tmp = GrSurfaceProxy::MakeDeferred(*rContext->caps(),
                                                             rContext->textureProvider(),
                                                             dm.dimensions(), SkBudgeted::kYes,
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
    static const int kNumViews = 3;

    auto dContext = ctxInfo.directContext();
    auto proxyProvider = dContext->priv().proxyProvider();

    AtlasObject object(reporter);

    dContext->priv().addOnFlushCallbackObject(&object);

    GrSurfaceProxyView views[kNumViews];
    for (int i = 0; i < kNumViews; ++i) {
        views[i] = make_upstream_image(dContext, &object, i * 3,
                                       object.getAtlasView(proxyProvider, dContext->priv().caps()),
                                       kPremul_SkAlphaType);
    }

    static const int kFinalWidth = 6*kDrawnTileSize;
    static const int kFinalHeight = kDrawnTileSize;

    auto rtc = GrSurfaceDrawContext::Make(
            dContext, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kApprox,
            {kFinalWidth, kFinalHeight});

    rtc->clear(SK_PMColor4fWHITE);

    // Note that this doesn't include the third texture proxy
    for (int i = 0; i < kNumViews - 1; ++i) {
        SkRect r = SkRect::MakeXYWH(i*3*kDrawnTileSize, 0, 3*kDrawnTileSize, kDrawnTileSize);

        SkMatrix t = SkMatrix::Translate(-i*3*kDrawnTileSize, 0);

        GrPaint paint;
        auto fp = GrTextureEffect::Make(std::move(views[i]), kPremul_SkAlphaType, t);
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.setColorFragmentProcessor(std::move(fp));

        rtc->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(), r);
    }

    dContext->priv().flushSurface(rtc->asSurfaceProxy());

    SkBitmap readBack;
    readBack.allocN32Pixels(kFinalWidth, kFinalHeight);

    SkAssertResult(rtc->readPixels(dContext, readBack.pixmap(), {0, 0}));

    dContext->priv().testingOnly_flushAndRemoveOnFlushCallbackObject(&object);

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
