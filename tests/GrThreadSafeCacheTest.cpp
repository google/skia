/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/private/SkMalloc.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkMessageBus.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/ProxyUtils.h"

#include <thread>

static constexpr int kImageWH = 32;
static constexpr auto kImageOrigin = kBottomLeft_GrSurfaceOrigin;
static constexpr int kNoID = -1;

static SkImageInfo default_ii(int wh) {
    return SkImageInfo::Make(wh, wh, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

static std::unique_ptr<skgpu::v1::SurfaceDrawContext> new_SDC(GrRecordingContext* rContext,
                                                              int wh) {
    return skgpu::v1::SurfaceDrawContext::Make(rContext,
                                               GrColorType::kRGBA_8888,
                                               nullptr,
                                               SkBackingFit::kExact,
                                               {wh, wh},
                                               SkSurfaceProps(),
                                               1,
                                               GrMipMapped::kNo,
                                               GrProtected::kNo,
                                               kImageOrigin,
                                               SkBudgeted::kYes);
}

static void create_view_key(GrUniqueKey* key, int wh, int id) {
    static const GrUniqueKey::Domain kViewDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kViewDomain, 1);
    builder[0] = wh;
    builder.finish();

    if (id != kNoID) {
        key->setCustomData(SkData::MakeWithCopy(&id, sizeof(id)));
    }
}

static void create_vert_key(GrUniqueKey* key, int wh, int id) {
    static const GrUniqueKey::Domain kVertDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kVertDomain, 1);
    builder[0] = wh;
    builder.finish();

    if (id != kNoID) {
        key->setCustomData(SkData::MakeWithCopy(&id, sizeof(id)));
    }
}

static bool default_is_newer_better(SkData* incumbent, SkData* challenger) {
    return false;
}

// When testing views we create a bitmap that covers the entire screen and has an inset blue rect
// atop a field of white.
// When testing verts we clear the background to white and simply draw an inset blur rect.
static SkBitmap create_bitmap(int wh) {
    SkBitmap bitmap;

    bitmap.allocPixels(default_ii(wh));

    SkCanvas tmp(bitmap);
    tmp.clear(SK_ColorWHITE);

    SkPaint blue;
    blue.setColor(SK_ColorBLUE);
    blue.setAntiAlias(false);

    tmp.drawRect({10, 10, wh-10.0f, wh-10.0f}, blue);

    bitmap.setImmutable();
    return bitmap;
}

class GrThreadSafeVertexTestOp;

class TestHelper {
public:
    struct Stats {
        int fCacheHits = 0;
        int fCacheMisses = 0;

        int fNumSWCreations = 0;
        int fNumLazyCreations = 0;
        int fNumHWCreations = 0;
    };

    TestHelper(GrDirectContext* dContext,
               GrThreadSafeCache::IsNewerBetter isNewerBetter = default_is_newer_better)
            : fDContext(dContext)
            , fIsNewerBetter(isNewerBetter) {

        fDst = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, default_ii(kImageWH));
        SkAssertResult(fDst);

        SkSurfaceCharacterization characterization;
        SkAssertResult(fDst->characterize(&characterization));

        fRecorder1 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        this->ddlCanvas1()->clear(SkColors::kWhite);

        fRecorder2 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        this->ddlCanvas2()->clear(SkColors::kWhite);

        fDst->getCanvas()->clear(SkColors::kWhite);
    }

    ~TestHelper() {
        fDContext->flush();
        fDContext->submit(true);
    }

    Stats* stats() { return &fStats; }

    int numCacheEntries() const { return this->threadSafeCache()->numEntries(); }

    GrDirectContext* dContext() { return fDContext; }

    SkCanvas* liveCanvas() { return fDst ? fDst->getCanvas() : nullptr; }
    SkCanvas* ddlCanvas1() { return fRecorder1 ? fRecorder1->getCanvas() : nullptr; }
    sk_sp<SkDeferredDisplayList> snap1() {
        if (fRecorder1) {
            sk_sp<SkDeferredDisplayList> tmp = fRecorder1->detach();
            fRecorder1 = nullptr;
            return tmp;
        }

        return nullptr;
    }
    SkCanvas* ddlCanvas2() { return fRecorder2 ? fRecorder2->getCanvas() : nullptr; }
    sk_sp<SkDeferredDisplayList> snap2() {
        if (fRecorder2) {
            sk_sp<SkDeferredDisplayList> tmp = fRecorder2->detach();
            fRecorder2 = nullptr;
            return tmp;
        }

        return nullptr;
    }

    GrThreadSafeCache* threadSafeCache() { return fDContext->priv().threadSafeCache(); }
    const GrThreadSafeCache* threadSafeCache() const { return fDContext->priv().threadSafeCache(); }

    typedef void (TestHelper::*addAccessFP)(SkCanvas*, int wh, int id,
                                            bool failLookUp, bool failFillingIn);
    typedef bool (TestHelper::*checkFP)(SkCanvas*, int wh,
                                        int expectedHits, int expectedMisses,
                                        int expectedNumRefs, int expectedID);

    // Add a draw on 'canvas' that will introduce a ref on the 'wh' view
    void addViewAccess(SkCanvas* canvas,
                       int wh,
                       int id = kNoID,
                       bool failLookup = false,
                       bool failFillingIn = false) {
        auto rContext = canvas->recordingContext();

        auto view = AccessCachedView(rContext, this->threadSafeCache(),
                                     wh, failLookup, failFillingIn, id, &fStats);
        SkASSERT(view);

        auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);

        sdc->drawTexture(nullptr,
                         view,
                         kPremul_SkAlphaType,
                         GrSamplerState::Filter::kNearest,
                         GrSamplerState::MipmapMode::kNone,
                         SkBlendMode::kSrcOver,
                         {1.0f, 1.0f, 1.0f, 1.0f},
                         SkRect::MakeWH(wh, wh),
                         SkRect::MakeWH(wh, wh),
                         GrAA::kNo,
                         GrQuadAAFlags::kNone,
                         SkCanvas::kFast_SrcRectConstraint,
                         SkMatrix::I(),
                         nullptr);
    }

    // Besides checking that the number of refs and cache hits and misses are as expected, this
    // method also validates that the unique key doesn't appear in any of the other caches.
    bool checkView(SkCanvas* canvas, int wh,
                   int expectedHits, int expectedMisses, int expectedNumRefs, int expectedID) {
        if (fStats.fCacheHits != expectedHits || fStats.fCacheMisses != expectedMisses) {
            SkDebugf("Hits E: %d A: %d --- Misses E: %d A: %d\n",
                     expectedHits, fStats.fCacheHits, expectedMisses, fStats.fCacheMisses);
            return false;
        }

        GrUniqueKey key;
        create_view_key(&key, wh, kNoID);

        auto threadSafeCache = this->threadSafeCache();

        auto [view, xtraData] = threadSafeCache->findWithData(key);
        if (!view.proxy()) {
            return false;
        }

        if (expectedID < 0) {
            if (xtraData) {
                return false;
            }
        } else {
            if (!xtraData) {
                return false;
            }

            const int* cachedID = static_cast<const int*>(xtraData->data());
            if (*cachedID != expectedID) {
                return false;
            }
        }

        if (!view.proxy()->refCntGreaterThan(expectedNumRefs+1) ||  // +1 for 'view's ref
            view.proxy()->refCntGreaterThan(expectedNumRefs+2)) {
            return false;
        }

        if (canvas) {
            GrRecordingContext* rContext = canvas->recordingContext();
            GrProxyProvider* recordingProxyProvider = rContext->priv().proxyProvider();
            sk_sp<GrTextureProxy> result = recordingProxyProvider->findProxyByUniqueKey(key);
            if (result) {
                // views in this cache should never appear in the recorder's cache
                return false;
            }
        }

        {
            GrProxyProvider* directProxyProvider = fDContext->priv().proxyProvider();
            sk_sp<GrTextureProxy> result = directProxyProvider->findProxyByUniqueKey(key);
            if (result) {
                // views in this cache should never appear in the main proxy cache
                return false;
            }
        }

        {
            auto resourceProvider = fDContext->priv().resourceProvider();
            sk_sp<GrSurface> surf = resourceProvider->findByUniqueKey<GrSurface>(key);
            if (surf) {
                // the textures backing the views in this cache should never be discoverable in the
                // resource cache
                return false;
            }
        }

        return true;
    }

    void addVertAccess(SkCanvas* canvas,
                       int wh,
                       int id,
                       bool failLookup,
                       bool failFillingIn,
                       GrThreadSafeVertexTestOp** createdOp);

    // Add a draw on 'canvas' that will introduce a ref on a 'wh' vertex data
    void addVertAccess(SkCanvas* canvas,
                       int wh,
                       int id = kNoID,
                       bool failLookup = false,
                       bool failFillingIn = false) {
        this->addVertAccess(canvas, wh, id, failLookup, failFillingIn, nullptr);
    }

    bool checkVert(SkCanvas* canvas, int wh,
                   int expectedHits, int expectedMisses, int expectedNumRefs, int expectedID) {
        if (fStats.fCacheHits != expectedHits || fStats.fCacheMisses != expectedMisses) {
            SkDebugf("Hits E: %d A: %d --- Misses E: %d A: %d\n",
                     expectedHits, fStats.fCacheHits, expectedMisses, fStats.fCacheMisses);
            return false;
        }

        GrUniqueKey key;
        create_vert_key(&key, wh, kNoID);

        auto threadSafeCache = this->threadSafeCache();

        auto [vertData, xtraData] = threadSafeCache->findVertsWithData(key);
        if (!vertData) {
            return false;
        }

        if (expectedID < 0) {
            if (xtraData) {
                return false;
            }
        } else {
            if (!xtraData) {
                return false;
            }

            const int* cachedID = static_cast<const int*>(xtraData->data());
            if (*cachedID != expectedID) {
                return false;
            }
        }

        if (!vertData->refCntGreaterThan(expectedNumRefs+1) ||  // +1 for 'vertData's ref
            vertData->refCntGreaterThan(expectedNumRefs+2)) {
            return false;
        }

        {
            auto resourceProvider = fDContext->priv().resourceProvider();
            sk_sp<GrGpuBuffer> buffer = resourceProvider->findByUniqueKey<GrGpuBuffer>(key);
            if (buffer) {
                // the buffer holding the vertex data in this cache should never be discoverable
                // in the resource cache
                return false;
            }
        }

        return true;
    }

    bool checkImage(skiatest::Reporter* reporter, sk_sp<SkSurface> s) {
        SkBitmap actual;

        actual.allocPixels(default_ii(kImageWH));

        if (!s->readPixels(actual, 0, 0)) {
            return false;
        }

        SkBitmap expected = create_bitmap(kImageWH);

        const float tols[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter](int x, int y, const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter, "mismatch at %d, %d (%f, %f, %f %f)",
                       x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
            });

        return ComparePixels(expected.pixmap(), actual.pixmap(), tols, error);
    }

    bool checkImage(skiatest::Reporter* reporter) {
        return this->checkImage(reporter, fDst);
    }

    bool checkImage(skiatest::Reporter* reporter, sk_sp<SkDeferredDisplayList> ddl) {
        sk_sp<SkSurface> tmp = SkSurface::MakeRenderTarget(fDContext,
                                                           SkBudgeted::kNo,
                                                           default_ii(kImageWH));
        if (!tmp) {
            return false;
        }

        if (!tmp->draw(std::move(ddl))) {
            return false;
        }

        return this->checkImage(reporter, std::move(tmp));
    }

    size_t gpuSize(int wh) const {
        GrBackendFormat format = fDContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                 GrRenderable::kNo);

        return GrSurface::ComputeSize(format, {wh, wh}, /*colorSamplesPerPixel=*/1,
                                      GrMipMapped::kNo, /*binSize=*/false);
    }

private:
    static GrSurfaceProxyView AccessCachedView(GrRecordingContext*,
                                               GrThreadSafeCache*,
                                               int wh,
                                               bool failLookup, bool failFillingIn, int id,
                                               Stats*);
    static GrSurfaceProxyView CreateViewOnCpu(GrRecordingContext*, int wh, Stats*);
    static bool FillInViewOnGpu(GrDirectContext*, int wh, Stats*,
                                const GrSurfaceProxyView& lazyView,
                                sk_sp<GrThreadSafeCache::Trampoline>);

    Stats fStats;
    GrDirectContext* fDContext = nullptr;
    GrThreadSafeCache::IsNewerBetter fIsNewerBetter;

    sk_sp<SkSurface> fDst;
    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder1;
    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder2;
};

class GrThreadSafeVertexTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* rContext, TestHelper::Stats* stats,
                            int wh, int id, bool failLookup, bool failFillingIn,
                            GrThreadSafeCache::IsNewerBetter isNewerBetter) {

        return GrOp::Make<GrThreadSafeVertexTestOp>(
                rContext, rContext, stats, wh, id, failLookup, failFillingIn, isNewerBetter);
    }

    const GrThreadSafeCache::VertexData* vertexData() const { return fVertexData.get(); }

private:
    friend class GrOp; // for ctor

    GrThreadSafeVertexTestOp(GrRecordingContext* rContext, TestHelper::Stats* stats, int wh, int id,
                             bool failLookup, bool failFillingIn,
                             GrThreadSafeCache::IsNewerBetter isNewerBetter)
            : INHERITED(ClassID())
            , fStats(stats)
            , fWH(wh)
            , fID(id)
            , fFailFillingIn(failFillingIn)
            , fIsNewerBetter(isNewerBetter) {
        this->setBounds(SkRect::MakeIWH(fWH, fWH), HasAABloat::kNo, IsHairline::kNo);

        // Normally we wouldn't add a ref to the vertex data at this point. However, it is
        // needed in this unit test to get the ref counts on the uniquely keyed resources
        // to be as expected.
        this->findOrCreateVertices(rContext, failLookup, fFailFillingIn);
    }

    const char* name() const override { return "GrThreadSafeVertexTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

    GrProgramInfo* createProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& writeView,
                                     GrAppliedClip&& appliedClip,
                                     const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) const {
        using namespace GrDefaultGeoProcFactory;

        Color color({ 0.0f, 0.0f, 1.0f, 1.0f });

        auto gp = MakeForDeviceSpace(arena, color,
                                     Coverage::kSolid_Type,
                                     LocalCoords::kUnused_Type,
                                     SkMatrix::I());

        return sk_gpu_test::CreateProgramInfo(caps, arena, writeView,
                                              std::move(appliedClip), dstProxyView,
                                              gp, SkBlendMode::kSrcOver,
                                              GrPrimitiveType::kTriangleStrip,
                                              renderPassXferBarriers, colorLoadOp);
    }

    GrProgramInfo* createProgramInfo(GrOpFlushState* flushState) const {
        return this->createProgramInfo(&flushState->caps(),
                                       flushState->allocator(),
                                       flushState->writeView(),
                                       flushState->detachAppliedClip(),
                                       flushState->dstProxyView(),
                                       flushState->renderPassBarriers(),
                                       flushState->colorLoadOp());
    }

    void findOrCreateVertices(GrRecordingContext* rContext, bool failLookup, bool failFillingIn) {

        if (!fVertexData) {
            auto threadSafeViewCache = rContext->priv().threadSafeCache();

            if (rContext->asDirectContext()) {
                // The vertex variant doesn't have a correlate to lazyProxies but increment this
                // here to make the unit tests happy.
                ++fStats->fNumLazyCreations;
            }

            GrUniqueKey key;
            create_vert_key(&key, fWH, fID);

            // We can "fail the lookup" to simulate a threaded race condition
            auto [cachedVerts, data] = threadSafeViewCache->findVertsWithData(key);
            if (cachedVerts && !failLookup) {
                fVertexData = cachedVerts;
                ++fStats->fCacheHits;
                return;
            }

            ++fStats->fCacheMisses;
            if (!rContext->asDirectContext()) {
                ++fStats->fNumSWCreations;
            }

            constexpr size_t kVertSize = sizeof(SkPoint);
            SkPoint* verts = static_cast<SkPoint*>(sk_malloc_throw(4 * kVertSize));

            verts[0].set(10.0f, 10.0f);
            verts[1].set(fWH-10.0f, 10.0f);
            verts[2].set(10.0f, fWH-10.0f);
            verts[3].set(fWH-10.0f, fWH-10.0f);

            fVertexData = GrThreadSafeCache::MakeVertexData(verts, 4, kVertSize);

            auto [tmpV, tmpD] = threadSafeViewCache->addVertsWithData(key, fVertexData,
                                                                      fIsNewerBetter);
            if (tmpV != fVertexData) {
                // Someone beat us to creating the vertex data. Use that version.
                fVertexData = tmpV;
            }
        }

        if (auto dContext = rContext->asDirectContext(); dContext && !fVertexData->gpuBuffer()) {
            auto rp = dContext->priv().resourceProvider();

            if (!failFillingIn) {
                ++fStats->fNumHWCreations;

                sk_sp<GrGpuBuffer> tmp = rp->createBuffer(fVertexData->size(),
                                                          GrGpuBufferType::kVertex,
                                                          kStatic_GrAccessPattern,
                                                          fVertexData->vertices());
                fVertexData->setGpuBuffer(std::move(tmp));
            }
        }
    }

    void onPrePrepare(GrRecordingContext* rContext,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip* clip,
                      const GrDstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {
        SkArenaAlloc* arena = rContext->priv().recordTimeAllocator();

        // This is equivalent to a GrOpFlushState::detachAppliedClip
        GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip::Disabled();

        fProgramInfo = this->createProgramInfo(rContext->priv().caps(), arena, writeView,
                                               std::move(appliedClip), dstProxyView,
                                               renderPassXferBarriers, colorLoadOp);

        rContext->priv().recordProgramInfo(fProgramInfo);

        // This is now a noop (bc it is always called in the ctor) but is where we would normally
        // create the vertices.
        this->findOrCreateVertices(rContext, false, fFailFillingIn);
    }

    void onPrepare(GrOpFlushState* flushState) override {
        auto dContext = flushState->gpu()->getContext();

        // This call site is not a noop bc this op could've been created on with DDL context
        // and, therefore, could be lacking a gpu-side buffer
        this->findOrCreateVertices(dContext, false, fFailFillingIn);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fVertexData || !fVertexData->gpuBuffer()) {
            return;
        }

        if (!fProgramInfo) {
            fProgramInfo = this->createProgramInfo(flushState);
        }

        flushState->bindPipeline(*fProgramInfo, SkRect::MakeIWH(fWH, fWH));
        flushState->bindBuffers(nullptr, nullptr, fVertexData->refGpuBuffer());
        flushState->draw(4, 0);
    }

    TestHelper::Stats*               fStats;
    int                              fWH;
    int                              fID;
    bool                             fFailFillingIn;
    GrThreadSafeCache::IsNewerBetter fIsNewerBetter;

    sk_sp<GrThreadSafeCache::VertexData> fVertexData;
    GrProgramInfo*                   fProgramInfo = nullptr;

    using INHERITED = GrDrawOp;
};

void TestHelper::addVertAccess(SkCanvas* canvas,
                               int wh, int id,
                               bool failLookup, bool failFillingIn,
                               GrThreadSafeVertexTestOp** createdOp) {
    auto rContext = canvas->recordingContext();
    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);

    GrOp::Owner op = GrThreadSafeVertexTestOp::Make(rContext, &fStats,
                                                    wh, id,
                                                    failLookup, failFillingIn,
                                                    fIsNewerBetter);
    if (createdOp) {
        *createdOp = (GrThreadSafeVertexTestOp*) op.get();
    }

    sdc->addDrawOp(std::move(op));
}

GrSurfaceProxyView TestHelper::CreateViewOnCpu(GrRecordingContext* rContext,
                                               int wh,
                                               Stats* stats) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxyFromBitmap(create_bitmap(wh),
                                                                       GrMipmapped::kNo,
                                                                       SkBackingFit::kExact,
                                                                       SkBudgeted::kYes);
    if (!proxy) {
        return {};
    }

    GrSwizzle swizzle = rContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                GrColorType::kRGBA_8888);
    ++stats->fNumSWCreations;
    return {std::move(proxy), kImageOrigin, swizzle};
}

bool TestHelper::FillInViewOnGpu(GrDirectContext* dContext, int wh, Stats* stats,
                                 const GrSurfaceProxyView& lazyView,
                                 sk_sp<GrThreadSafeCache::Trampoline> trampoline) {

    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc = new_SDC(dContext, wh);

    GrPaint paint;
    paint.setColor4f({0.0f, 0.0f, 1.0f, 1.0f});

    sdc->clear(SkPMColor4f{1.0f, 1.0f, 1.0f, 1.0f});
    sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                  { 10, 10, wh-10.0f, wh-10.0f }, &GrStyle::SimpleFill());

    ++stats->fNumHWCreations;
    auto view = sdc->readSurfaceView();

    SkASSERT(view.swizzle() == lazyView.swizzle());
    SkASSERT(view.origin() == lazyView.origin());
    trampoline->fProxy = view.asTextureProxyRef();

    return true;
}

GrSurfaceProxyView TestHelper::AccessCachedView(GrRecordingContext* rContext,
                                                GrThreadSafeCache* threadSafeCache,
                                                int wh,
                                                bool failLookup, bool failFillingIn, int id,
                                                Stats* stats) {
    GrUniqueKey key;
    create_view_key(&key, wh, id);

    if (GrDirectContext* dContext = rContext->asDirectContext()) {
        // The gpu thread gets priority over the recording threads. If the gpu thread is first,
        // it crams a lazy proxy into the cache and then fills it in later.
        auto [lazyView, trampoline] = GrThreadSafeCache::CreateLazyView(
            dContext, GrColorType::kRGBA_8888, {wh, wh}, kImageOrigin, SkBackingFit::kExact);
        ++stats->fNumLazyCreations;

        auto [view, data] = threadSafeCache->findOrAddWithData(key, lazyView);
        if (view != lazyView) {
            ++stats->fCacheHits;
            return view;
        } else if (id != kNoID) {
            // Make sure, in this case, that the customData stuck
            SkASSERT(data);
            SkDEBUGCODE(const int* cachedID = static_cast<const int*>(data->data());)
            SkASSERT(*cachedID == id);
        }

        ++stats->fCacheMisses;

        if (failFillingIn) {
            // Simulate something going horribly wrong at flush-time so no GrTexture is
            // available to fulfill the lazy proxy.
            return view;
        }

        if (!FillInViewOnGpu(dContext, wh, stats, lazyView, std::move(trampoline))) {
            // In this case something has gone disastrously wrong so set up to drop the draw
            // that needed this resource and reduce future pollution of the cache.
            threadSafeCache->remove(key);
            return {};
        }

        return view;
    } else {
        GrSurfaceProxyView view;

        // We can "fail the lookup" to simulate a threaded race condition
        if (view = threadSafeCache->find(key); !failLookup && view) {
            ++stats->fCacheHits;
            return view;
        }

        ++stats->fCacheMisses;

        view = CreateViewOnCpu(rContext, wh, stats);
        SkASSERT(view);

        auto [newView, data] = threadSafeCache->addWithData(key, view);
        if (view == newView && id != kNoID) {
            // Make sure, in this case, that the customData stuck
            SkASSERT(data);
            SkDEBUGCODE(const int* cachedID = static_cast<const int*>(data->data());)
            SkASSERT(*cachedID == id);
        }
        return newView;
    }
}

// Case 1: ensure two DDL recorders share the view/vertexData
static void test_1(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, 1, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, /*id*/ 1));

    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, 2, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, /*id*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);

    helper.checkImage(reporter, helper.snap1());
    helper.checkImage(reporter, helper.snap2());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache1View, reporter, ctxInfo) {
    test_1(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache1Verts, reporter, ctxInfo) {
    test_1(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 2: ensure that, if the direct context version wins, its result is reused by the
//         DDL recorders
static void test_2(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, 1, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, /*id*/ 1));

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, 2, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, /*id*/ 1));

    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, 3, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), kImageWH,
                                              /*hits*/ 2, /*misses*/ 1, /*refs*/ 3, /*id*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    helper.checkImage(reporter);
    helper.checkImage(reporter, helper.snap1());
    helper.checkImage(reporter, helper.snap2());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache2View, reporter, ctxInfo) {
    test_2(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache2Verts, reporter, ctxInfo) {
    test_2(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 3: ensure that, if the cpu-version wins, its result is reused by the direct context
static void test_3(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, 1, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, /*id*/ 1));

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, 2, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, /*id*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);

    helper.checkImage(reporter);
    helper.checkImage(reporter, helper.snap1());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache3View, reporter, ctxInfo) {
    test_3(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache3Verts, reporter, ctxInfo) {
    test_3(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 4: ensure that, if two DDL recorders get in a race, they still end up sharing a single
//         view/vertexData
static void test_4(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, 1, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, /*id*/ 1));

    static const bool kFailLookup = true;
    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, 2, kFailLookup, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 2, /*id*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 2);

    helper.checkImage(reporter, helper.snap1());
    helper.checkImage(reporter, helper.snap2());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache4View, reporter, ctxInfo) {
    test_4(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache4Verts, reporter, ctxInfo) {
    test_4(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 4.5: check that, if a live rendering and a DDL recording get into a race, the live
//           rendering takes precedence.
static void test_4_5(GrDirectContext* dContext, skiatest::Reporter* reporter,
                     TestHelper::addAccessFP addAccess,
                     TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, 1, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, /*id*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    static const bool kFailLookup = true;
    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, 2, kFailLookup, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 2, /*id*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);

    helper.checkImage(reporter);
    helper.checkImage(reporter, helper.snap1());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache4_5View, reporter, ctxInfo) {
    test_4_5(ctxInfo.directContext(), reporter,
             &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache4_5Verts, reporter, ctxInfo) {
    test_4_5(ctxInfo.directContext(), reporter,
             &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 4.75: check that, if a live rendering fails to generate the content needed to instantiate
//            its lazy proxy, life goes on
static void test_4_75(GrDirectContext* dContext, skiatest::Reporter* reporter,
                      TestHelper::addAccessFP addAccess,
                      TestHelper::checkFP check) {

    TestHelper helper(dContext);

    static const bool kFailFillingIn = true;
    (helper.*addAccess)(helper.liveCanvas(), kImageWH, kNoID, false, kFailFillingIn);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    dContext->flush();
    dContext->submit(true);

    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 0, kNoID));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache4_75View, reporter, ctxInfo) {
    test_4_75(ctxInfo.directContext(), reporter,
              &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache4_75Verts, reporter, ctxInfo) {
    test_4_75(ctxInfo.directContext(), reporter,
              &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 5: ensure that expanding the map works (esp. wrt custom data)
static void test_5(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    auto threadSafeCache = helper.threadSafeCache();

    int size = 16;
    (helper.*addAccess)(helper.ddlCanvas1(), size, /*id*/ size, false, false);

    size_t initialSize = threadSafeCache->approxBytesUsedForHash();

    while (initialSize == threadSafeCache->approxBytesUsedForHash()) {
        size *= 2;
        (helper.*addAccess)(helper.ddlCanvas1(), size, /*id*/ size, false, false);
    }

    for (int i = 16; i <= size; i *= 2) {
        REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(),
                                                  /*wh*/ i,
                                                  /*hits*/ 0,
                                                  /*misses*/ threadSafeCache->numEntries(),
                                                  /*refs*/ 1,
                                                  /*id*/ i));
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache5View, reporter, ctxInfo) {
    test_5(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache5Verts, reporter, ctxInfo) {
    test_5(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 6: Check on dropping refs. In particular, that the cache has its own ref to keep
//         the backing resource alive and locked.
static void test_6(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, kNoID));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    ddl1 = nullptr;
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 1, kNoID));

    ddl2 = nullptr;
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 0, kNoID));

    // The cache still has its ref
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 0, kNoID));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache6View, reporter, ctxInfo) {
    test_6(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache6Verts, reporter, ctxInfo) {
    test_6(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 7: Check that invoking dropAllRefs and dropUniqueRefs directly works as expected; i.e.,
//         dropAllRefs removes everything while dropUniqueRefs is more measured.
static void test_7(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.ddlCanvas2(), 2*kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, 2*kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 1, kNoID));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    helper.threadSafeCache()->dropUniqueRefs(nullptr);
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    ddl1 = nullptr;

    helper.threadSafeCache()->dropUniqueRefs(nullptr);
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, (helper.*check)(nullptr, 2*kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 1, kNoID));

    helper.threadSafeCache()->dropAllRefs();
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    ddl2 = nullptr;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache7View, reporter, ctxInfo) {
    test_7(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache7Verts, reporter, ctxInfo) {
    test_7(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 8: This checks that GrContext::abandonContext works as expected wrt the thread
//         safe cache. This simulates the case where we have one DDL that has finished
//         recording but one still recording when the abandonContext fires.
static void test_8(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, kNoID));

    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), kImageWH,
                                              /*hits*/ 2, /*misses*/ 1, /*refs*/ 3, kNoID));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    dContext->abandonContext(); // This should exercise dropAllRefs

    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    ddl1 = nullptr;
    ddl2 = nullptr;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache8View, reporter, ctxInfo) {
    test_8(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache8Verts, reporter, ctxInfo) {
    test_8(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 9: This checks that GrContext::releaseResourcesAndAbandonContext works as expected wrt
//         the thread safe cache. This simulates the case where we have one DDL that has finished
//         recording but one still recording when the releaseResourcesAndAbandonContext fires.
static void test_9(GrDirectContext* dContext, skiatest::Reporter* reporter,
                   TestHelper::addAccessFP addAccess,
                   TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, kNoID));

    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), kImageWH,
                                              /*hits*/ 2, /*misses*/ 1, /*refs*/ 3, kNoID));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumLazyCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    dContext->releaseResourcesAndAbandonContext(); // This should hit dropAllRefs

    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    ddl1 = nullptr;
    ddl2 = nullptr;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache9View, reporter, ctxInfo) {
    test_9(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache9Verts, reporter, ctxInfo) {
    test_9(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 10: This checks that the GrContext::purgeUnlockedResources(size_t) variant works as
//          expected wrt the thread safe cache. It, in particular, tests out the MRU behavior
//          of the shared cache.
static void test_10(GrDirectContext* dContext, skiatest::Reporter* reporter,
                    TestHelper::addAccessFP addAccess,
                    TestHelper::checkFP check) {

    if (GrBackendApi::kOpenGL != dContext->backend()) {
        // The lower-level backends have too much going on for the following simple purging
        // test to work
        return;
    }

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, kNoID));

    (helper.*addAccess)(helper.liveCanvas(), 2*kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 1, /*misses*/ 2, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.ddlCanvas2(), 2*kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), 2*kImageWH,
                                              /*hits*/ 2, /*misses*/ 2, /*refs*/ 2, kNoID));

    dContext->flush();
    dContext->submit(true);

    // This should clear out everything but the textures locked in the thread-safe cache
    dContext->purgeUnlockedResources(false);

    ddl1 = nullptr;
    ddl2 = nullptr;

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 2, /*misses*/ 2, /*refs*/ 0, kNoID));
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 2, /*misses*/ 2, /*refs*/ 0, kNoID));

    // Regardless of which image is MRU, this should force the other out
    size_t desiredBytes = helper.gpuSize(2*kImageWH) + helper.gpuSize(kImageWH)/2;

    auto cache = dContext->priv().getResourceCache();
    size_t currentBytes = cache->getResourceBytes();

    SkASSERT(currentBytes >= desiredBytes);
    size_t amountToPurge = currentBytes - desiredBytes;

    // The 2*kImageWH texture should be MRU.
    dContext->purgeUnlockedResources(amountToPurge, true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 2, /*misses*/ 2, /*refs*/ 0, kNoID));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache10View, reporter, ctxInfo) {
    test_10(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

// To enable test_10 with verts would require a bit more work, namely:
//    have a different # of verts based on size
//    also pass in a gpuSize function to 'test_10'
//DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache10Verts, reporter, ctxInfo) {
//    test_10(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
//}

// Case 11: This checks that scratch-only variant of GrContext::purgeUnlockedResources works as
//          expected wrt the thread safe cache. In particular, that when 'scratchResourcesOnly'
//          is true, the call has no effect on the cache.
static void test_11(GrDirectContext* dContext, skiatest::Reporter* reporter,
                    TestHelper::addAccessFP addAccess,
                    TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));

    (helper.*addAccess)(helper.liveCanvas(), 2*kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 1, kNoID));

    dContext->flush();
    dContext->submit(true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 0, kNoID));
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 0, kNoID));

    // This shouldn't remove anything from the cache
    dContext->purgeUnlockedResources(/* scratchResourcesOnly */ true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    dContext->purgeUnlockedResources(/* scratchResourcesOnly */ false);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache11View, reporter, ctxInfo) {
    test_11(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache11Verts, reporter, ctxInfo) {
    test_11(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 12: Test out purges caused by resetting the cache budget to 0. Note that, due to
//          the how the cache operates (i.e., not directly driven by ref/unrefs) there
//          needs to be an explicit kick to purge the cache.
static void test_12(GrDirectContext* dContext, skiatest::Reporter* reporter,
                    TestHelper::addAccessFP addAccess,
                    TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.liveCanvas(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));
    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 1, /*refs*/ 2, kNoID));

    (helper.*addAccess)(helper.liveCanvas(), 2*kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 1, /*misses*/ 2, /*refs*/ 1, kNoID));

    dContext->flush();
    dContext->submit(true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), kImageWH,
                                              /*hits*/ 1, /*misses*/ 2, /*refs*/ 1, kNoID));
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 1, /*misses*/ 2, /*refs*/ 0, kNoID));

    dContext->setResourceCacheLimit(0);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    ddl1 = nullptr;

    // Explicitly kick off the purge - it won't happen automatically on unref
    dContext->performDeferredCleanup(std::chrono::milliseconds(0));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache12View, reporter, ctxInfo) {
    test_12(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache12Verts, reporter, ctxInfo) {
    test_12(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 13: Test out the 'msNotUsed' parameter to GrContext::performDeferredCleanup.
static void test_13(GrDirectContext* dContext, skiatest::Reporter* reporter,
                    TestHelper::addAccessFP addAccess,
                    TestHelper::checkFP check) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto firstTime = GrStdSteadyClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    (helper.*addAccess)(helper.ddlCanvas2(), 2*kImageWH, kNoID, false, false);

    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), 2*kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 1, kNoID));
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    ddl1 = nullptr;
    ddl2 = nullptr;

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    auto secondTime = GrStdSteadyClock::now();

    auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(secondTime - firstTime);
    dContext->performDeferredCleanup(msecs);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.liveCanvas(), 2*kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 0, kNoID));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache13View, reporter, ctxInfo) {
    test_13(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache13Verts, reporter, ctxInfo) {
    test_13(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert);
}

// Case 14: Test out mixing & matching view & vertex data w/ recycling of the cache entries to
//          wring out the anonymous union code. This is mainly for the MSAN bot's consumption.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache14, reporter, ctxInfo) {
    constexpr int kBestPrimeNumber = 73; // palindromic in binary
    SkRandom rand(kBestPrimeNumber);

    TestHelper helper(ctxInfo.directContext());

    for (int i = 0; i < 2; ++i) {
        SkCanvas* ddlCanvas = (!i) ? helper.ddlCanvas1() : helper.ddlCanvas2();

        for (int j = 0; j < 10; ++j) {
            int numResources = 10*i + j + 1;
            int wh = numResources;

            if (rand.nextBool()) {
                helper.addViewAccess(ddlCanvas, wh, kNoID, false, false);
                REPORTER_ASSERT(reporter, helper.checkView(ddlCanvas, wh,
                                                           /*hits*/ 0, /*misses*/ numResources,
                                                           /*refs*/ 1, kNoID));
            } else {
                helper.addVertAccess(ddlCanvas, wh, kNoID, false, false);
                REPORTER_ASSERT(reporter, helper.checkVert(ddlCanvas, wh,
                                                           /*hits*/ 0, /*misses*/ numResources,
                                                           /*refs*/ 1, kNoID));
            }
        }

        if (!i) {
            // Drop all the accumulated resources from the thread-safe cache
            helper.snap1();
            ctxInfo.directContext()->purgeUnlockedResources(/* scratchResourcesOnly */ false);
        }
    }
}

// Case 15: Test out posting invalidation messages that involve the thread safe cache
static void test_15(GrDirectContext* dContext, skiatest::Reporter* reporter,
                    TestHelper::addAccessFP addAccess,
                    TestHelper::checkFP check,
                    void (*create_key)(GrUniqueKey*, int wh, int id)) {

    TestHelper helper(dContext);

    (helper.*addAccess)(helper.ddlCanvas1(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas1(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    GrUniqueKey key;
    (*create_key)(&key, kImageWH, kNoID);

    GrUniqueKeyInvalidatedMessage msg(key, dContext->priv().contextID(),
                                      /* inThreadSafeCache */ true);

    SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Post(msg);

    // This purge call is needed to process the invalidation messages
    dContext->purgeUnlockedResources(/* scratchResourcesOnly */ true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    (helper.*addAccess)(helper.ddlCanvas2(), kImageWH, kNoID, false, false);
    REPORTER_ASSERT(reporter, (helper.*check)(helper.ddlCanvas2(), kImageWH,
                                              /*hits*/ 0, /*misses*/ 2, /*refs*/ 1, kNoID));
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    helper.checkImage(reporter, std::move(ddl1));
    helper.checkImage(reporter, std::move(ddl2));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache15View, reporter, ctxInfo) {
    test_15(ctxInfo.directContext(), reporter, &TestHelper::addViewAccess, &TestHelper::checkView,
            create_view_key);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache15Verts, reporter, ctxInfo) {
    test_15(ctxInfo.directContext(), reporter, &TestHelper::addVertAccess, &TestHelper::checkVert,
            create_vert_key);
}

// Case 16: Test out pre-emption of an existing vertex-data cache entry. This test simulates
//          the case where there is a race to create vertex data. However, the second one
//          to finish is better and usurps the first's position in the cache.
//
//          This capability isn't available for views.

static bool newer_is_always_better(SkData* /* incumbent */, SkData* /* challenger */) {
    return true;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeCache16Verts, reporter, ctxInfo) {
    GrUniqueKey key;
    create_vert_key(&key, kImageWH, kNoID);

    TestHelper helper(ctxInfo.directContext(), newer_is_always_better);

    GrThreadSafeVertexTestOp* op1 = nullptr, *op2 = nullptr;

    helper.addVertAccess(helper.ddlCanvas1(), kImageWH, kNoID, false, false, &op1);
    REPORTER_ASSERT(reporter, helper.checkVert(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1, kNoID));
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();

    {
        REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
        auto [vertexData, xtraData] = helper.threadSafeCache()->findVertsWithData(key);
        REPORTER_ASSERT(reporter, vertexData.get() == op1->vertexData());
    }

    helper.addVertAccess(helper.ddlCanvas2(), kImageWH, kNoID, /* failLookup */ true, false, &op2);
    REPORTER_ASSERT(reporter, helper.checkVert(helper.ddlCanvas2(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 1, kNoID));
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    REPORTER_ASSERT(reporter, op1->vertexData() != op2->vertexData());

    {
        REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
        auto [vertexData, xtraData] = helper.threadSafeCache()->findVertsWithData(key);
        REPORTER_ASSERT(reporter, vertexData.get() == op2->vertexData());
    }

    helper.checkImage(reporter, std::move(ddl1));
    helper.checkImage(reporter, std::move(ddl2));
}
