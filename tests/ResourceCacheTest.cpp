/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include <thread>

static const int gWidth = 640;
static const int gHeight = 480;

////////////////////////////////////////////////////////////////////////////////
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceCacheCache, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(gWidth, gHeight);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    const SkIRect size = SkIRect::MakeWH(gWidth, gHeight);

    SkBitmap src;
    src.allocN32Pixels(size.width(), size.height());
    src.eraseColor(SK_ColorBLACK);
    size_t srcSize = src.computeByteSize();

    size_t initialCacheSize;
    context->getResourceCacheUsage(nullptr, &initialCacheSize);

    size_t oldMaxBytes = context->getResourceCacheLimit();

    // Set the cache limits so we can fit 10 "src" images and the
    // max number of textures doesn't matter
    size_t maxCacheSize = initialCacheSize + 10*srcSize;
    context->setResourceCacheLimit(maxCacheSize);

    SkBitmap readback;
    readback.allocN32Pixels(size.width(), size.height());

    for (int i = 0; i < 100; ++i) {
        canvas->drawImage(src.asImage(), 0, 0);
        surface->readPixels(readback, 0, 0);

        // "modify" the src texture
        src.notifyPixelsChanged();

        size_t curCacheSize;
        context->getResourceCacheUsage(nullptr, &curCacheSize);

        // we should never go over the size limit
        REPORTER_ASSERT(reporter, curCacheSize <= maxCacheSize);
    }

    context->setResourceCacheLimit(oldMaxBytes);
}

static bool is_rendering_and_not_angle_es3(sk_gpu_test::GrContextFactory::ContextType type) {
    if (type == sk_gpu_test::GrContextFactory::kANGLE_D3D11_ES3_ContextType ||
        type == sk_gpu_test::GrContextFactory::kANGLE_GL_ES3_ContextType) {
        return false;
    }
    return sk_gpu_test::GrContextFactory::IsRenderingContext(type);
}

static GrAttachment* get_SB(GrRenderTarget* rt) { return rt->getStencilAttachment(); }

static sk_sp<GrRenderTarget> create_RT_with_SB(GrResourceProvider* provider,
                                               int size, int sampleCount, SkBudgeted budgeted) {
    auto format =
            provider->caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888, GrRenderable::kYes);
    sk_sp<GrTexture> tex(provider->createTexture({size, size}, format, GrTextureType::k2D,
                                                 GrRenderable::kYes, sampleCount, GrMipmapped::kNo,
                                                 budgeted, GrProtected::kNo));
    if (!tex || !tex->asRenderTarget()) {
        return nullptr;
    }

    if (!provider->attachStencilAttachment(tex->asRenderTarget(), sampleCount > 1)) {
        return nullptr;
    }
    SkASSERT(get_SB(tex->asRenderTarget()));

    return sk_ref_sp(tex->asRenderTarget());
}

// This currently fails on ES3 ANGLE contexts
DEF_GPUTEST_FOR_CONTEXTS(ResourceCacheStencilBuffers, &is_rendering_and_not_angle_es3, reporter,
                         ctxInfo, nullptr) {
    auto context = ctxInfo.directContext();
    const GrCaps* caps = context->priv().caps();

    if (caps->avoidStencilBuffers()) {
        return;
    }

    GrResourceProvider* resourceProvider = context->priv().resourceProvider();

    GrColorType grColorType = GrColorType::kRGBA_8888;
    GrBackendFormat format = caps->getDefaultBackendFormat(grColorType, GrRenderable::kYes);

    sk_sp<GrRenderTarget> smallRT0 = create_RT_with_SB(resourceProvider, 4, 1, SkBudgeted::kYes);
    REPORTER_ASSERT(reporter, smallRT0);

    {
       // Two budgeted RTs with the same desc should share a stencil buffer.
       sk_sp<GrRenderTarget> smallRT1 = create_RT_with_SB(resourceProvider, 4, 1, SkBudgeted::kYes);
       REPORTER_ASSERT(reporter, smallRT1);

       REPORTER_ASSERT(reporter, get_SB(smallRT0.get()) == get_SB(smallRT1.get()));
    }

    {
        // An unbudgeted RT with the same desc should also share.
        sk_sp<GrRenderTarget> smallRT2 = create_RT_with_SB(resourceProvider, 4, 1, SkBudgeted::kNo);
        REPORTER_ASSERT(reporter, smallRT2);

        REPORTER_ASSERT(reporter, get_SB(smallRT0.get()) == get_SB(smallRT2.get()));
    }

    {
        // An RT with a much larger size should not share.
        sk_sp<GrRenderTarget> bigRT = create_RT_with_SB(resourceProvider, 400, 1, SkBudgeted::kNo);
        REPORTER_ASSERT(reporter, bigRT);

        REPORTER_ASSERT(reporter, get_SB(smallRT0.get()) != get_SB(bigRT.get()));
    }

    int smallSampleCount =
            context->priv().caps()->getRenderTargetSampleCount(2, format);
    if (smallSampleCount > 1) {
        // An RT with a different sample count should not share.
        sk_sp<GrRenderTarget> smallMSAART0 = create_RT_with_SB(resourceProvider, 4,
                                                               smallSampleCount, SkBudgeted::kNo);
        REPORTER_ASSERT(reporter, smallMSAART0);

        REPORTER_ASSERT(reporter, get_SB(smallRT0.get()) != get_SB(smallMSAART0.get()));

        {
            // A second MSAA RT should share with the first MSAA RT.
            sk_sp<GrRenderTarget> smallMSAART1 = create_RT_with_SB(resourceProvider, 4,
                                                                   smallSampleCount,
                                                                   SkBudgeted::kNo);
            REPORTER_ASSERT(reporter, smallMSAART1);

            REPORTER_ASSERT(reporter, get_SB(smallMSAART0.get()) == get_SB(smallMSAART1.get()));
        }

        // But one with a larger sample count should not. (Also check that the two requests didn't
        // rounded up to the same actual sample count or else they could share.).
        int bigSampleCount = context->priv().caps()->getRenderTargetSampleCount(5, format);
        if (bigSampleCount > 0 && bigSampleCount != smallSampleCount) {
            sk_sp<GrRenderTarget> smallMSAART2 = create_RT_with_SB(resourceProvider, 4,
                                                                   bigSampleCount,
                                                                   SkBudgeted::kNo);
            REPORTER_ASSERT(reporter, smallMSAART2);

            REPORTER_ASSERT(reporter, get_SB(smallMSAART0.get()) != get_SB(smallMSAART2.get()));
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceCacheWrappedResources, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    GrGpu* gpu = context->priv().getGpu();
    // this test is only valid for GL
    if (!gpu || !gpu->glContextForTesting()) {
        return;
    }

    static const int kW = 100;
    static const int kH = 100;

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            context, kW, kH, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo);
    GrBackendTexture unmbet = context->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo);
    if (!mbet || !unmbet.isValid()) {
        ERRORF(reporter, "Could not create backend texture.");
        return;
    }

    context->resetContext();

    sk_sp<GrTexture> borrowed(resourceProvider->wrapBackendTexture(
            mbet->texture(), kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType));

    sk_sp<GrTexture> adopted(resourceProvider->wrapBackendTexture(
            unmbet, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType));

    REPORTER_ASSERT(reporter, borrowed != nullptr && adopted != nullptr);
    if (!borrowed || !adopted) {
        return;
    }

    borrowed.reset();
    adopted.reset();

    context->flushAndSubmit(/*sync*/ true);

    bool borrowedIsAlive = gpu->isTestingOnlyBackendTexture(mbet->texture());
    bool adoptedIsAlive = gpu->isTestingOnlyBackendTexture(unmbet);

    REPORTER_ASSERT(reporter, borrowedIsAlive);
    REPORTER_ASSERT(reporter, !adoptedIsAlive);

    if (adoptedIsAlive) {
        context->deleteBackendTexture(unmbet);
    }

    context->resetContext();
}

class TestResource : public GrGpuResource {
    enum ScratchConstructor { kScratchConstructor };
public:
    static const size_t kDefaultSize = 100;

    /** Property that distinctly categorizes the resource.
     * For example, textures have width, height, ... */
    enum SimulatedProperty { kA_SimulatedProperty, kB_SimulatedProperty };

    TestResource(GrGpu* gpu, SkBudgeted budgeted = SkBudgeted::kYes, size_t size = kDefaultSize)
        : INHERITED(gpu)
        , fToDelete(nullptr)
        , fSize(size)
        , fProperty(kA_SimulatedProperty)
        , fIsScratch(false) {
        ++fNumAlive;
        this->registerWithCache(budgeted);
    }

    static TestResource* CreateScratch(GrGpu* gpu, SkBudgeted budgeted,
                                       SimulatedProperty property, size_t size = kDefaultSize) {
        return new TestResource(gpu, budgeted, property, kScratchConstructor, size);
    }
    static TestResource* CreateWrapped(GrGpu* gpu, GrWrapCacheable cacheable,
                                       size_t size = kDefaultSize) {
        return new TestResource(gpu, cacheable, size);
    }

    ~TestResource() override {
        --fNumAlive;
    }

    static int NumAlive() { return fNumAlive; }

    void setUnrefWhenDestroyed(sk_sp<TestResource> resource) {
        fToDelete = std::move(resource);
    }

    static void ComputeScratchKey(SimulatedProperty property, skgpu::ScratchKey* key) {
        static skgpu::ScratchKey::ResourceType t = skgpu::ScratchKey::GenerateResourceType();
        skgpu::ScratchKey::Builder builder(key, t, kScratchKeyFieldCnt);
        for (int i = 0; i < kScratchKeyFieldCnt; ++i) {
            builder[i] = static_cast<uint32_t>(i + property);
        }
    }

    static size_t ExpectedScratchKeySize() {
        return sizeof(uint32_t) * (kScratchKeyFieldCnt + skgpu::ScratchKey::kMetaDataCnt);
    }
private:
    static const int kScratchKeyFieldCnt = 6;

    TestResource(GrGpu* gpu, SkBudgeted budgeted, SimulatedProperty property, ScratchConstructor,
                 size_t size = kDefaultSize)
        : INHERITED(gpu)
        , fToDelete(nullptr)
        , fSize(size)
        , fProperty(property)
        , fIsScratch(true) {
        ++fNumAlive;
        this->registerWithCache(budgeted);
    }

    // Constructor for simulating resources that wrap backend objects.
    TestResource(GrGpu* gpu, GrWrapCacheable cacheable, size_t size)
            : INHERITED(gpu)
            , fToDelete(nullptr)
            , fSize(size)
            , fProperty(kA_SimulatedProperty)
            , fIsScratch(false) {
        ++fNumAlive;
        this->registerWithCacheWrapped(cacheable);
    }

    void computeScratchKey(skgpu::ScratchKey* key) const override {
        if (fIsScratch) {
            ComputeScratchKey(fProperty, key);
        }
    }

    size_t onGpuMemorySize() const override { return fSize; }
    const char* getResourceType() const override { return "Test"; }

    sk_sp<TestResource> fToDelete;
    size_t fSize;
    static int fNumAlive;
    SimulatedProperty fProperty;
    bool fIsScratch;
    using INHERITED = GrGpuResource;
};
int TestResource::fNumAlive = 0;

class Mock {
public:
    Mock(size_t maxBytes) {
        fDContext = GrDirectContext::MakeMock(nullptr);
        SkASSERT(fDContext);
        fDContext->setResourceCacheLimit(maxBytes);
        GrResourceCache* cache = fDContext->priv().getResourceCache();
        cache->purgeUnlockedResources();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());
    }

    GrResourceCache* cache() { return fDContext->priv().getResourceCache(); }
    GrGpu* gpu() { return fDContext->priv().getGpu(); }
    GrDirectContext* dContext() { return fDContext.get(); }

    void reset() {
        fDContext.reset();
    }

private:
    sk_sp<GrDirectContext> fDContext;
};

static void test_no_key(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // Create a bunch of resources with no keys
    TestResource* a = new TestResource(gpu, SkBudgeted::kYes, 11);
    TestResource* b = new TestResource(gpu, SkBudgeted::kYes, 12);
    TestResource* c = new TestResource(gpu, SkBudgeted::kYes, 13 );
    TestResource* d = new TestResource(gpu, SkBudgeted::kYes, 14 );

    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() + c->gpuMemorySize() +
                              d->gpuMemorySize() == cache->getResourceBytes());

    // Should be safe to purge without deleting the resources since we still have refs.
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());

    // Since the resources have neither unique nor scratch keys, delete immediately upon unref.

    a->unref();
    REPORTER_ASSERT(reporter, 3 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() + c->gpuMemorySize() + d->gpuMemorySize() ==
                              cache->getResourceBytes());

    c->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() + d->gpuMemorySize() ==
                              cache->getResourceBytes());

    d->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() == cache->getResourceBytes());

    b->unref();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
}

// Each integer passed as a template param creates a new domain.
template <int>
static void make_unique_key(skgpu::UniqueKey* key, int data, const char* tag = nullptr) {
    static skgpu::UniqueKey::Domain d = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey::Builder builder(key, d, 1, tag);
    builder[0] = data;
}

static void test_purge_unlocked(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // Create two resource w/ a unique key and two w/o but all of which have scratch keys.
    TestResource* a = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty, 11);

    skgpu::UniqueKey uniqueKey;
    make_unique_key<0>(&uniqueKey, 0);

    TestResource* b = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty, 12);
    b->resourcePriv().setUniqueKey(uniqueKey);

    TestResource* c = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty, 13);

    skgpu::UniqueKey uniqueKey2;
    make_unique_key<0>(&uniqueKey2, 1);

    TestResource* d = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty, 14);
    d->resourcePriv().setUniqueKey(uniqueKey2);


    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() + c->gpuMemorySize() +
                              d->gpuMemorySize() == cache->getResourceBytes());

    // Should be safe to purge without deleting the resources since we still have refs.
    cache->purgeUnlockedResources(false);
    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());

    // Unref them all. Since they all have keys they should remain in the cache.

    a->unref();
    b->unref();
    c->unref();
    d->unref();
    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() + c->gpuMemorySize() +
                              d->gpuMemorySize() == cache->getResourceBytes());

    // Purge only the two scratch resources
    cache->purgeUnlockedResources(true);

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() + d->gpuMemorySize() ==
                              cache->getResourceBytes());

    // Purge the uniquely keyed resources
    cache->purgeUnlockedResources(false);

    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
}

static void test_purge_command_buffer_usage(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // Create two resource w/ scratch keys.
    TestResource* a = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty, 11);

    TestResource* b = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty, 12);

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() == cache->getResourceBytes());

    // Should be safe to purge without deleting the resources since we still have refs.
    cache->purgeUnlockedResources(true);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    // Add command buffer usages to all resources
    a->addCommandBufferUsage();
    b->addCommandBufferUsage();

    // Should be safe to purge without deleting the resources since we still have refs and command
    // buffer usages.
    cache->purgeUnlockedResources(true);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    // Unref the first resource
    a->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() == cache->getResourceBytes());

    // Should be safe to purge without deleting the resources since we still have command buffer
    // usages and the second still has a ref.
    cache->purgeUnlockedResources(true);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    // Remove command buffer usages
    a->removeCommandBufferUsage();
    b->removeCommandBufferUsage();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() == cache->getResourceBytes());

    // Purge this time should remove the first resources since it no longer has any refs or command
    // buffer usages.
    cache->purgeUnlockedResources(true);
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() == cache->getResourceBytes());

    // Unref the second resource
    b->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() == cache->getResourceBytes());

    // Purge the last resource
    cache->purgeUnlockedResources(false);

    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
}

static void test_budgeting(skiatest::Reporter* reporter) {
    Mock mock(300);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    skgpu::UniqueKey uniqueKey;
    make_unique_key<0>(&uniqueKey, 0);

    // Create a scratch, a unique, and a wrapped resource
    TestResource* scratch =
            TestResource::CreateScratch(gpu, SkBudgeted::kYes, TestResource::kB_SimulatedProperty,
                                        10);
    TestResource* unique = new TestResource(gpu, SkBudgeted::kYes, 11);
    unique->resourcePriv().setUniqueKey(uniqueKey);
    TestResource* wrappedCacheable = TestResource::CreateWrapped(gpu, GrWrapCacheable::kYes, 12);
    TestResource* wrappedUncacheable = TestResource::CreateWrapped(gpu, GrWrapCacheable::kNo, 13);
    TestResource* unbudgeted = new TestResource(gpu, SkBudgeted::kNo, 14);

    // Make sure we can add a unique key to the wrapped resources
    skgpu::UniqueKey uniqueKey2;
    make_unique_key<0>(&uniqueKey2, 1);
    skgpu::UniqueKey uniqueKey3;
    make_unique_key<0>(&uniqueKey3, 2);
    wrappedCacheable->resourcePriv().setUniqueKey(uniqueKey2);
    wrappedUncacheable->resourcePriv().setUniqueKey(uniqueKey3);
    GrGpuResource* wrappedCacheableViaKey = cache->findAndRefUniqueResource(uniqueKey2);
    REPORTER_ASSERT(reporter, wrappedCacheableViaKey);
    GrGpuResource* wrappedUncacheableViaKey = cache->findAndRefUniqueResource(uniqueKey3);
    REPORTER_ASSERT(reporter, wrappedUncacheableViaKey);

    // Remove the extra refs we just added.
    SkSafeUnref(wrappedCacheableViaKey);
    SkSafeUnref(wrappedUncacheableViaKey);

    // Make sure sizes are as we expect
    REPORTER_ASSERT(reporter, 5 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() +
                                              wrappedCacheable->gpuMemorySize() +
                                              wrappedUncacheable->gpuMemorySize() +
                                              unbudgeted->gpuMemorySize() ==
                                      cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() ==
                              cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    // Our refs mean that the resources are non purgeable.
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 5 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() +
                                              wrappedCacheable->gpuMemorySize() +
                                              wrappedUncacheable->gpuMemorySize() +
                                              unbudgeted->gpuMemorySize() ==
                                      cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() ==
                              cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    // Unreffing the cacheable wrapped resource with a unique key shouldn't free it right away.
    // However, unreffing the uncacheable wrapped resource should free it.
    wrappedCacheable->unref();
    wrappedUncacheable->unref();
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() +
                                              wrappedCacheable->gpuMemorySize() +
                                              unbudgeted->gpuMemorySize() ==
                                      cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    // Now try freeing the budgeted resources first
    wrappedUncacheable = TestResource::CreateWrapped(gpu, GrWrapCacheable::kNo);
    unique->unref();
    REPORTER_ASSERT(reporter, 11 == cache->getPurgeableBytes());
    // This will free 'unique' but not wrappedCacheable which has a key. That requires the key to be
    // removed to be freed.
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());

    wrappedCacheableViaKey = cache->findAndRefUniqueResource(uniqueKey2);
    REPORTER_ASSERT(reporter, wrappedCacheableViaKey);
    if (wrappedCacheableViaKey) {
        wrappedCacheableViaKey->resourcePriv().removeUniqueKey();
        wrappedCacheable->unref();
    }
    // We shouldn't have to call purgeAllUnlocked as removing the key on a wrapped cacheable
    // resource should immediately delete it.
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());

    wrappedCacheable = TestResource::CreateWrapped(gpu, GrWrapCacheable::kYes);
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + wrappedCacheable->gpuMemorySize() +
                                              wrappedUncacheable->gpuMemorySize() +
                                              unbudgeted->gpuMemorySize() ==
                                      cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    scratch->unref();
    REPORTER_ASSERT(reporter, 10 == cache->getPurgeableBytes());
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, unbudgeted->gpuMemorySize() + wrappedCacheable->gpuMemorySize() +
                                              wrappedUncacheable->gpuMemorySize() ==
                                      cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    // Unreffing the wrapped resources (with no unique key) should free them right away.
    wrappedUncacheable->unref();
    wrappedCacheable->unref();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, unbudgeted->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    unbudgeted->unref();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());
}

static void test_unbudgeted(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    skgpu::UniqueKey uniqueKey;
    make_unique_key<0>(&uniqueKey, 0);

    TestResource* scratch;
    TestResource* unique;
    TestResource* wrapped;
    TestResource* unbudgeted;

    // A large uncached or wrapped resource shouldn't evict anything.
    scratch = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                          TestResource::kB_SimulatedProperty, 10);

    scratch->unref();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 10 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 10 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 10 == cache->getPurgeableBytes());

    unique = new TestResource(gpu, SkBudgeted::kYes, 11);
    unique->resourcePriv().setUniqueKey(uniqueKey);
    unique->unref();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 21 == cache->getPurgeableBytes());

    size_t large = 2 * cache->getResourceBytes();
    unbudgeted = new TestResource(gpu, SkBudgeted::kNo, large);
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 + large == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 21 == cache->getPurgeableBytes());

    unbudgeted->unref();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 21 == cache->getPurgeableBytes());

    wrapped = TestResource::CreateWrapped(gpu, GrWrapCacheable::kYes, large);
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 + large == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 21 == cache->getPurgeableBytes());

    wrapped->unref();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 21 == cache->getPurgeableBytes());

    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());
}

// This method can't be static because it needs to friended in GrGpuResource::CacheAccess.
void test_unbudgeted_to_scratch(skiatest::Reporter* reporter);
/*static*/ void test_unbudgeted_to_scratch(skiatest::Reporter* reporter) {
    Mock mock(300);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    TestResource* resource =
        TestResource::CreateScratch(gpu, SkBudgeted::kNo, TestResource::kA_SimulatedProperty);
    skgpu::ScratchKey key;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &key);

    size_t size = resource->gpuMemorySize();
    for (int i = 0; i < 2; ++i) {
        // Since this resource is unbudgeted, it should not be reachable as scratch.
        REPORTER_ASSERT(reporter, resource->resourcePriv().getScratchKey() == key);
        REPORTER_ASSERT(reporter, !resource->cacheAccess().isScratch());
        REPORTER_ASSERT(reporter, GrBudgetedType::kUnbudgetedUncacheable ==
                                          resource->resourcePriv().budgetedType());
        REPORTER_ASSERT(reporter, !cache->findAndRefScratchResource(key));
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
        REPORTER_ASSERT(reporter, size == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
        REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

        // Once it is unrefed, it should become available as scratch.
        resource->unref();
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
        REPORTER_ASSERT(reporter, size == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, size == cache->getBudgetedResourceBytes());
        REPORTER_ASSERT(reporter, size == cache->getPurgeableBytes());
        resource = static_cast<TestResource*>(cache->findAndRefScratchResource(key));
        REPORTER_ASSERT(reporter, resource);
        REPORTER_ASSERT(reporter, resource->resourcePriv().getScratchKey() == key);
        REPORTER_ASSERT(reporter, resource->cacheAccess().isScratch());
        REPORTER_ASSERT(reporter,
                        GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType());

        if (0 == i) {
            // If made unbudgeted, it should return to original state: ref'ed and unbudgeted. Try
            // the above tests again.
            resource->resourcePriv().makeUnbudgeted();
        } else {
            // After the second time around, try removing the scratch key
            resource->resourcePriv().removeScratchKey();
            REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
            REPORTER_ASSERT(reporter, size == cache->getResourceBytes());
            REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
            REPORTER_ASSERT(reporter, size == cache->getBudgetedResourceBytes());
            REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());
            REPORTER_ASSERT(reporter, !resource->resourcePriv().getScratchKey().isValid());
            REPORTER_ASSERT(reporter, !resource->cacheAccess().isScratch());
            REPORTER_ASSERT(reporter,
                            GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType());

            // now when it is unrefed it should die since it has no key.
            resource->unref();
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
            REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
            REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
            REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
            REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());
        }
    }
}

static void test_duplicate_scratch_key(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // Create two resources that have the same scratch key.
    TestResource* a = TestResource::CreateScratch(gpu,
                                                  SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty, 11);
    TestResource* b = TestResource::CreateScratch(gpu,
                                                  SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty, 12);
    skgpu::ScratchKey scratchKey1;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey1);
    // Check for negative case consistency. (leaks upon test failure.)
    REPORTER_ASSERT(reporter, !cache->findAndRefScratchResource(scratchKey1));

    skgpu::ScratchKey scratchKey;
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey);

    // Scratch resources are registered with GrResourceCache just by existing. There are 2.
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    // As long as there are outstanding refs on the resources they will not be in the scratch map
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() ==
                              cache->getResourceBytes());

    // Our refs mean that the resources are non purgeable.
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());

    // Unref but don't purge
    a->unref();
    b->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    // Since we removed the refs to the resources they will now be in the scratch map
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->countScratchEntriesForKey(scratchKey));)

    // Purge again. This time resources should be purgeable.
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->countScratchEntriesForKey(scratchKey));)
}

static void test_remove_scratch_key(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // Create two resources that have the same scratch key.
    TestResource* a = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    TestResource* b = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    a->unref();
    b->unref();

    skgpu::ScratchKey scratchKey;
    // Ensure that scratch key lookup is correct for negative case.
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey);
    // (following leaks upon test failure).
    REPORTER_ASSERT(reporter, !cache->findAndRefScratchResource(scratchKey));

    // Scratch resources are registered with GrResourceCache just by existing. There are 2.
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());

    // Find the first resource and remove its scratch key
    GrGpuResource* find = cache->findAndRefScratchResource(scratchKey);
    find->resourcePriv().removeScratchKey();
    // It's still alive, but not cached by scratch key anymore
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 1 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());

    // The cache should immediately delete it when it's unrefed since it isn't accessible.
    find->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 1 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    // Repeat for the second resource.
    find = cache->findAndRefScratchResource(scratchKey);
    find->resourcePriv().removeScratchKey();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    // Should be able to call this multiple times with no problem.
    find->resourcePriv().removeScratchKey();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    find->unref();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

static void test_scratch_key_consistency(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // Create two resources that have the same scratch key.
    TestResource* a = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    TestResource* b = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    a->unref();
    b->unref();

    skgpu::ScratchKey scratchKey;
    // Ensure that scratch key comparison and assignment is consistent.
    skgpu::ScratchKey scratchKey1;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey1);
    skgpu::ScratchKey scratchKey2;
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey2);
    REPORTER_ASSERT(reporter, scratchKey1.size() == TestResource::ExpectedScratchKeySize());
    REPORTER_ASSERT(reporter, scratchKey1 != scratchKey2);
    REPORTER_ASSERT(reporter, scratchKey2 != scratchKey1);
    scratchKey = scratchKey1;
    REPORTER_ASSERT(reporter, scratchKey.size() == TestResource::ExpectedScratchKeySize());
    REPORTER_ASSERT(reporter, scratchKey1 == scratchKey);
    REPORTER_ASSERT(reporter, scratchKey == scratchKey1);
    REPORTER_ASSERT(reporter, scratchKey2 != scratchKey);
    REPORTER_ASSERT(reporter, scratchKey != scratchKey2);
    scratchKey = scratchKey2;
    REPORTER_ASSERT(reporter, scratchKey.size() == TestResource::ExpectedScratchKeySize());
    REPORTER_ASSERT(reporter, scratchKey1 != scratchKey);
    REPORTER_ASSERT(reporter, scratchKey != scratchKey1);
    REPORTER_ASSERT(reporter, scratchKey2 == scratchKey);
    REPORTER_ASSERT(reporter, scratchKey == scratchKey2);

    // Ensure that scratch key lookup is correct for negative case.
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey);
    // (following leaks upon test failure).
    REPORTER_ASSERT(reporter, !cache->findAndRefScratchResource(scratchKey));

    // Find the first resource with a scratch key and a copy of a scratch key.
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey);
    GrGpuResource* find = cache->findAndRefScratchResource(scratchKey);
    REPORTER_ASSERT(reporter, find != nullptr);
    find->unref();

    scratchKey2 = scratchKey;
    find = cache->findAndRefScratchResource(scratchKey2);
    REPORTER_ASSERT(reporter, find != nullptr);
    REPORTER_ASSERT(reporter, find == a || find == b);

    GrGpuResource* find2 = cache->findAndRefScratchResource(scratchKey2);
    REPORTER_ASSERT(reporter, find2 != nullptr);
    REPORTER_ASSERT(reporter, find2 == a || find2 == b);
    REPORTER_ASSERT(reporter, find2 != find);
    find2->unref();
    find->unref();
}

static void test_duplicate_unique_key(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    skgpu::UniqueKey key;
    make_unique_key<0>(&key, 0);

    // Create two resources that we will attempt to register with the same unique key.
    TestResource* a = new TestResource(gpu, SkBudgeted::kYes, 11);

    // Set key on resource a.
    a->resourcePriv().setUniqueKey(key);
    REPORTER_ASSERT(reporter, a == cache->findAndRefUniqueResource(key));
    a->unref();

    // Make sure that redundantly setting a's key works.
    a->resourcePriv().setUniqueKey(key);
    REPORTER_ASSERT(reporter, a == cache->findAndRefUniqueResource(key));
    a->unref();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Create resource b and set the same key. It should replace a's unique key cache entry.
    TestResource* b = new TestResource(gpu, SkBudgeted::kYes, 12);
    b->resourcePriv().setUniqueKey(key);
    REPORTER_ASSERT(reporter, b == cache->findAndRefUniqueResource(key));
    b->unref();

    // Still have two resources because a is still reffed.
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    a->unref();
    // Now a should be gone.
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Now replace b with c, but make sure c can start with one unique key and change it to b's key.
    // Also make b be unreffed when replacement occurs.
    b->unref();
    TestResource* c = new TestResource(gpu, SkBudgeted::kYes, 13);
    skgpu::UniqueKey differentKey;
    make_unique_key<0>(&differentKey, 1);
    c->resourcePriv().setUniqueKey(differentKey);
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() + c->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    // c replaces b and b should be immediately purged.
    c->resourcePriv().setUniqueKey(key);
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, c->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // c shouldn't be purged because it is ref'ed.
    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, c->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Drop the ref on c, it should be kept alive because it has a unique key.
    c->unref();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, c->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Verify that we can find c, then remove its unique key. It should get purged immediately.
    REPORTER_ASSERT(reporter, c == cache->findAndRefUniqueResource(key));
    c->resourcePriv().removeUniqueKey();
    c->unref();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());

    {
        skgpu::UniqueKey key2;
        make_unique_key<0>(&key2, 0);
        sk_sp<TestResource> d(new TestResource(gpu));
        int foo = 4132;
        key2.setCustomData(SkData::MakeWithCopy(&foo, sizeof(foo)));
        d->resourcePriv().setUniqueKey(key2);
    }

    skgpu::UniqueKey key3;
    make_unique_key<0>(&key3, 0);
    sk_sp<GrGpuResource> d2(cache->findAndRefUniqueResource(key3));
    REPORTER_ASSERT(reporter, *(int*) d2->getUniqueKey().getCustomData()->data() == 4132);
}

static void test_purge_invalidated(skiatest::Reporter* reporter) {
    Mock mock(30000);
    auto dContext = mock.dContext();
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    skgpu::UniqueKey key1, key2, key3;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);
    make_unique_key<0>(&key3, 3);

    // Add three resources to the cache. Only c is usable as scratch.
    TestResource* a = new TestResource(gpu);
    TestResource* b = new TestResource(gpu);
    TestResource* c = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                  TestResource::kA_SimulatedProperty);
    a->resourcePriv().setUniqueKey(key1);
    b->resourcePriv().setUniqueKey(key2);
    c->resourcePriv().setUniqueKey(key3);
    a->unref();
    // hold b until *after* the message is sent.
    c->unref();

    REPORTER_ASSERT(reporter, cache->hasUniqueKey(key1));
    REPORTER_ASSERT(reporter, cache->hasUniqueKey(key2));
    REPORTER_ASSERT(reporter, cache->hasUniqueKey(key3));
    REPORTER_ASSERT(reporter, 3 == TestResource::NumAlive());

    typedef skgpu::UniqueKeyInvalidatedMessage Msg;
    typedef SkMessageBus<Msg, uint32_t> Bus;

    // Invalidate two of the three, they should be purged and no longer accessible via their keys.
    Bus::Post(Msg(key1, dContext->priv().contextID()));
    Bus::Post(Msg(key2, dContext->priv().contextID()));
    cache->purgeAsNeeded();
    // a should be deleted now, but we still have a ref on b.
    REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key1));
    REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key2));
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, cache->hasUniqueKey(key3));

    // Invalidate the third.
    Bus::Post(Msg(key3, dContext->priv().contextID()));
    cache->purgeAsNeeded();
    // we still have a ref on b, c should be recycled as scratch.
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key3));

    // make b purgeable. It should be immediately deleted since it has no key.
    b->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Make sure we actually get to c via it's scratch key, before we say goodbye.
    skgpu::ScratchKey scratchKey;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey);
    GrGpuResource* scratch = cache->findAndRefScratchResource(scratchKey);
    REPORTER_ASSERT(reporter, scratch == c);
    SkSafeUnref(scratch);

    // Get rid of c.
    cache->purgeUnlockedResources();
    scratch = cache->findAndRefScratchResource(scratchKey);
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, !scratch);
    SkSafeUnref(scratch);
}

static void test_cache_chained_purge(skiatest::Reporter* reporter) {
    Mock mock(30000);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    skgpu::UniqueKey key1, key2;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);

    sk_sp<TestResource> a(new TestResource(gpu));
    sk_sp<TestResource> b(new TestResource(gpu));
    a->resourcePriv().setUniqueKey(key1);
    b->resourcePriv().setUniqueKey(key2);

    // Make a cycle
    a->setUnrefWhenDestroyed(b);
    b->setUnrefWhenDestroyed(a);

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    TestResource* unownedA = a.release();
    unownedA->unref();
    b.reset();

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    // Break the cycle
    unownedA->setUnrefWhenDestroyed(nullptr);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    cache->purgeUnlockedResources();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
}

static void test_timestamp_wrap(skiatest::Reporter* reporter) {
    static const int kCount = 50;
    static const int kLockedFreq = 8;
    static const int kBudgetSize = 0; // always over budget

    SkRandom random;

    // Run the test 2*kCount times;
    for (int i = 0; i < 2 * kCount; ++i ) {
        Mock mock(kBudgetSize);
        GrResourceCache* cache = mock.cache();
        GrGpu* gpu = mock.gpu();

        // Pick a random number of resources to add before the timestamp will wrap.
        cache->changeTimestamp(UINT32_MAX - random.nextULessThan(kCount + 1));

        static const int kNumToPurge = kCount;

        SkTDArray<int> shouldPurgeIdxs;
        int purgeableCnt = 0;
        SkTDArray<GrGpuResource*> resourcesToUnref;

        // Add kCount resources, holding onto resources at random so we have a mix of purgeable and
        // unpurgeable resources.
        for (int j = 0; j < kCount; ++j) {
            skgpu::UniqueKey key;
            make_unique_key<0>(&key, j);

            TestResource* r = new TestResource(gpu);
            r->resourcePriv().setUniqueKey(key);
            if (random.nextU() % kLockedFreq) {
                // Make this is purgeable.
                r->unref();
                ++purgeableCnt;
                if (purgeableCnt <= kNumToPurge) {
                    *shouldPurgeIdxs.append() = j;
                }
            } else {
                *resourcesToUnref.append() = r;
            }
        }

        // Verify that the correct resources were purged.
        int currShouldPurgeIdx = 0;
        for (int j = 0; j < kCount; ++j) {
            skgpu::UniqueKey key;
            make_unique_key<0>(&key, j);
            GrGpuResource* res = cache->findAndRefUniqueResource(key);
            if (currShouldPurgeIdx < shouldPurgeIdxs.count() &&
                shouldPurgeIdxs[currShouldPurgeIdx] == j) {
                ++currShouldPurgeIdx;
                REPORTER_ASSERT(reporter, nullptr == res);
            } else {
                REPORTER_ASSERT(reporter, nullptr != res);
            }
            SkSafeUnref(res);
        }

        for (int j = 0; j < resourcesToUnref.count(); ++j) {
            resourcesToUnref[j]->unref();
        }
    }
}

static void test_time_purge(skiatest::Reporter* reporter) {
    Mock mock(1000000);
    auto dContext = mock.dContext();
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    static constexpr int kCnts[] = {1, 10, 1024};
    auto nowish = []() {
        // We sleep so that we ensure we get a value that is greater than the last call to
        // GrStdSteadyClock::now().
        std::this_thread::sleep_for(GrStdSteadyClock::duration(5));
        auto result = GrStdSteadyClock::now();
        // Also sleep afterwards so we don't get this value again.
        std::this_thread::sleep_for(GrStdSteadyClock::duration(5));
        return result;
    };

    for (int cnt : kCnts) {
        std::unique_ptr<GrStdSteadyClock::time_point[]> timeStamps(
                new GrStdSteadyClock::time_point[cnt]);
        {
            // Insert resources and get time points between each addition.
            for (int i = 0; i < cnt; ++i) {
                TestResource* r = new TestResource(gpu);
                skgpu::UniqueKey k;
                make_unique_key<1>(&k, i);
                r->resourcePriv().setUniqueKey(k);
                r->unref();
                timeStamps.get()[i] = nowish();
            }

            // Purge based on the time points between resource additions. Each purge should remove
            // the oldest resource.
            for (int i = 0; i < cnt; ++i) {
                cache->purgeResourcesNotUsedSince(timeStamps[i]);
                REPORTER_ASSERT(reporter, cnt - i - 1 == cache->getResourceCount());
                for (int j = 0; j < i; ++j) {
                    skgpu::UniqueKey k;
                    make_unique_key<1>(&k, j);
                    GrGpuResource* r = cache->findAndRefUniqueResource(k);
                    REPORTER_ASSERT(reporter, !SkToBool(r));
                    SkSafeUnref(r);
                }
            }

            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
            cache->purgeUnlockedResources();
        }

        // Do a similar test but where we leave refs on some resources to prevent them from being
        // purged.
        {
            std::unique_ptr<GrGpuResource* []> refedResources(new GrGpuResource*[cnt / 2]);
            for (int i = 0; i < cnt; ++i) {
                TestResource* r = new TestResource(gpu);
                skgpu::UniqueKey k;
                make_unique_key<1>(&k, i);
                r->resourcePriv().setUniqueKey(k);
                // Leave a ref on every other resource, beginning with the first.
                if (SkToBool(i & 0x1)) {
                    refedResources.get()[i / 2] = r;
                } else {
                    r->unref();
                }
                timeStamps.get()[i] = nowish();
            }

            for (int i = 0; i < cnt; ++i) {
                // Should get a resource purged every other frame.
                cache->purgeResourcesNotUsedSince(timeStamps[i]);
                REPORTER_ASSERT(reporter, cnt - i / 2 - 1 == cache->getResourceCount());
            }

            // Unref all the resources that we kept refs on in the first loop.
            for (int i = 0; i < (cnt / 2); ++i) {
                refedResources.get()[i]->unref();
                cache->purgeResourcesNotUsedSince(nowish());
                REPORTER_ASSERT(reporter, cnt / 2 - i - 1 == cache->getResourceCount());
            }

            cache->purgeUnlockedResources();
        }

        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

        // Do a similar test where we alternate adding scratch and uniquely keyed resources, but
        // then purge old scratch resources.
        {
            for (int i = 0; i < cnt; ++i) {
                const bool isScratch = (i % 2 == 0);
                const SkBudgeted budgeted = SkBudgeted::kYes;
                const TestResource::SimulatedProperty property = TestResource::kA_SimulatedProperty;
                TestResource* r = isScratch ? TestResource::CreateScratch(gpu, budgeted, property)
                                            : new TestResource(gpu, budgeted, property);
                if (!isScratch) {
                    skgpu::UniqueKey k;
                    make_unique_key<1>(&k, i);
                    r->resourcePriv().setUniqueKey(k);
                }
                r->unref();
                timeStamps.get()[i] = nowish();
            }

            for (int i = 0; i < cnt; ++i) {
                // Should get a resource purged every other frame, since the uniquely keyed
                // resources will not be considered.
                cache->purgeResourcesNotUsedSince(timeStamps[i], /*scratchResourcesOnly=*/true);
                REPORTER_ASSERT(reporter, cnt - i / 2 - 1 == cache->getResourceCount());
            }
            // Unref remaining resources
            cache->purgeResourcesNotUsedSince(nowish());
        }

        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

        // Verify that calling flush() on a context with nothing to do will not trigger resource
        // eviction
        dContext->flushAndSubmit();
        for (int i = 0; i < 10; ++i) {
            TestResource* r = new TestResource(gpu);
            skgpu::UniqueKey k;
            make_unique_key<1>(&k, i);
            r->resourcePriv().setUniqueKey(k);
            r->unref();
        }
        REPORTER_ASSERT(reporter, 10 == cache->getResourceCount());
        dContext->flushAndSubmit();
        REPORTER_ASSERT(reporter, 10 == cache->getResourceCount());
        cache->purgeResourcesNotUsedSince(nowish());
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    }
}

static void test_partial_purge(skiatest::Reporter* reporter) {
    Mock mock(100);
    auto dContext = mock.dContext();
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    enum TestsCase {
        kOnlyScratch_TestCase = 0,
        kPartialScratch_TestCase = 1,
        kAllScratch_TestCase = 2,
        kPartial_TestCase = 3,
        kAll_TestCase = 4,
        kNone_TestCase = 5,
        kEndTests_TestCase = kNone_TestCase + 1
    };

    for (int testCase = 0; testCase < kEndTests_TestCase; testCase++) {

        skgpu::UniqueKey key1, key2, key3;
        make_unique_key<0>(&key1, 1);
        make_unique_key<0>(&key2, 2);
        make_unique_key<0>(&key3, 3);

        // Add three unique resources to the cache.
        TestResource *unique1 = new TestResource(gpu, SkBudgeted::kYes, 10);
        TestResource *unique2 = new TestResource(gpu, SkBudgeted::kYes, 11);
        TestResource *unique3 = new TestResource(gpu, SkBudgeted::kYes, 12);

        unique1->resourcePriv().setUniqueKey(key1);
        unique2->resourcePriv().setUniqueKey(key2);
        unique3->resourcePriv().setUniqueKey(key3);

        // Add two scratch resources to the cache.
        TestResource *scratch1 = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                             TestResource::kA_SimulatedProperty,
                                                             13);
        TestResource *scratch2 = TestResource::CreateScratch(gpu, SkBudgeted::kYes,
                                                             TestResource::kB_SimulatedProperty,
                                                             14);

        REPORTER_ASSERT(reporter, 5 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, 60 == cache->getBudgetedResourceBytes());
        REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

        // Add resources to the purgeable queue
        unique1->unref();
        scratch1->unref();
        unique2->unref();
        scratch2->unref();
        unique3->unref();

        REPORTER_ASSERT(reporter, 5 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, 60 == cache->getBudgetedResourceBytes());
        REPORTER_ASSERT(reporter, 60 == cache->getPurgeableBytes());

        switch(testCase) {
            case kOnlyScratch_TestCase: {
                dContext->purgeUnlockedResources(14, true);
                REPORTER_ASSERT(reporter, 3 == cache->getBudgetedResourceCount());
                REPORTER_ASSERT(reporter, 33 == cache->getBudgetedResourceBytes());
                break;
            }
            case kPartialScratch_TestCase: {
                dContext->purgeUnlockedResources(3, true);
                REPORTER_ASSERT(reporter, 4 == cache->getBudgetedResourceCount());
                REPORTER_ASSERT(reporter, 47 == cache->getBudgetedResourceBytes());
                break;
            }
            case kAllScratch_TestCase: {
                dContext->purgeUnlockedResources(50, true);
                REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
                REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
                break;
            }
            case kPartial_TestCase: {
                dContext->purgeUnlockedResources(13, false);
                REPORTER_ASSERT(reporter, 3 == cache->getBudgetedResourceCount());
                REPORTER_ASSERT(reporter, 37 == cache->getBudgetedResourceBytes());
                break;
            }
            case kAll_TestCase: {
                dContext->purgeUnlockedResources(50, false);
                REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
                REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
                break;
            }
            case kNone_TestCase: {
                dContext->purgeUnlockedResources(0, true);
                dContext->purgeUnlockedResources(0, false);
                REPORTER_ASSERT(reporter, 5 == cache->getBudgetedResourceCount());
                REPORTER_ASSERT(reporter, 60 == cache->getBudgetedResourceBytes());
                REPORTER_ASSERT(reporter, 60 == cache->getPurgeableBytes());
                break;
            }
        }

        // ensure all are purged before the next
        dContext->priv().getResourceCache()->purgeUnlockedResources();
        REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, 0 == cache->getPurgeableBytes());

    }
}

static void test_custom_data(skiatest::Reporter* reporter) {
    skgpu::UniqueKey key1, key2;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);
    int foo = 4132;
    key1.setCustomData(SkData::MakeWithCopy(&foo, sizeof(foo)));
    REPORTER_ASSERT(reporter, *(int*) key1.getCustomData()->data() == 4132);
    REPORTER_ASSERT(reporter, key2.getCustomData() == nullptr);

    // Test that copying a key also takes a ref on its custom data.
    skgpu::UniqueKey key3 = key1;
    REPORTER_ASSERT(reporter, *(int*) key3.getCustomData()->data() == 4132);
}

static void test_abandoned(skiatest::Reporter* reporter) {
    Mock mock(300);
    auto dContext = mock.dContext();
    GrGpu* gpu = mock.gpu();

    sk_sp<GrGpuResource> resource(new TestResource(gpu));
    dContext->abandonContext();

    REPORTER_ASSERT(reporter, resource->wasDestroyed());

    // Call all the public methods on resource in the abandoned state. They shouldn't crash.

    resource->uniqueID();
    resource->getUniqueKey();
    resource->wasDestroyed();
    resource->gpuMemorySize();
    resource->getContext();

    resource->resourcePriv().getScratchKey();
    resource->resourcePriv().budgetedType();
    resource->resourcePriv().makeBudgeted();
    resource->resourcePriv().makeUnbudgeted();
    resource->resourcePriv().removeScratchKey();
    skgpu::UniqueKey key;
    make_unique_key<0>(&key, 1);
    resource->resourcePriv().setUniqueKey(key);
    resource->resourcePriv().removeUniqueKey();
}

static void test_tags(skiatest::Reporter* reporter) {
#ifdef SK_DEBUG
    // We will insert 1 resource with tag "tag1", 2 with "tag2", and so on, up through kLastTagIdx.
    static constexpr int kLastTagIdx = 10;
    static constexpr int kNumResources = kLastTagIdx * (kLastTagIdx + 1) / 2;

    Mock mock(kNumResources * TestResource::kDefaultSize);
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    // tag strings are expected to be long lived
    std::vector<SkString> tagStrings;

    SkString tagStr;
    int tagIdx = 0;
    int currTagCnt = 0;

    for (int i = 0; i < kNumResources; ++i, ++currTagCnt) {

        sk_sp<GrGpuResource> resource(new TestResource(gpu));
        skgpu::UniqueKey key;
        if (currTagCnt == tagIdx) {
            tagIdx += 1;
            currTagCnt = 0;
            tagStr.printf("tag%d", tagIdx);
            tagStrings.emplace_back(tagStr);
        }
        make_unique_key<1>(&key, i, tagStrings.back().c_str());
        resource->resourcePriv().setUniqueKey(key);
    }
    SkASSERT(kLastTagIdx == tagIdx);
    SkASSERT(currTagCnt == kLastTagIdx);

    // Test i = 0 to exercise unused tag string.
    for (int i = 0; i <= kLastTagIdx; ++i) {
        tagStr.printf("tag%d", i);
        REPORTER_ASSERT(reporter, cache->countUniqueKeysWithTag(tagStr.c_str()) == i);
    }
#endif
}

static void test_free_texture_messages(skiatest::Reporter* reporter) {
    Mock mock(30000);
    auto dContext = mock.dContext();
    GrResourceCache* cache = mock.cache();
    GrGpu* gpu = mock.gpu();

    GrBackendTexture backends[3];
    GrTexture* wrapped[3];
    int freed[3] = { 0, 0, 0 };

    auto releaseProc = [](void* ctx) {
        int* index = (int*) ctx;
        *index = 1;
    };

    for (int i = 0; i < 3; ++i) {
        backends[i] = dContext->createBackendTexture(16, 16, SkColorType::kRGBA_8888_SkColorType,
                                                     GrMipmapped::kNo, GrRenderable::kNo);
        wrapped[i] = gpu->wrapBackendTexture(backends[i],
                                             GrWrapOwnership::kBorrow_GrWrapOwnership,
                                             (i < 2) ? GrWrapCacheable::kYes : GrWrapCacheable::kNo,
                                             GrIOType::kRead_GrIOType)
                             .release();
        wrapped[i]->setRelease(releaseProc, &freed[i]);
    }

    cache->insertDelayedTextureUnref(wrapped[0]);
    cache->insertDelayedTextureUnref(wrapped[1]);

    // An uncacheable cross-context should not be purged as soon as we drop our ref. This
    // is because inserting it as a cross-context resource actually holds a ref until the
    // message is received.
    cache->insertDelayedTextureUnref(wrapped[2]);

    REPORTER_ASSERT(reporter, 0 == (freed[0] + freed[1] + freed[2]));

    // Have only ref waiting on message.
    wrapped[0]->unref();
    wrapped[1]->unref();
    wrapped[2]->unref();

    REPORTER_ASSERT(reporter, 0 == (freed[0] + freed[1] + freed[2]));

    // This should free nothing since no messages were sent.
    cache->purgeAsNeeded();

    REPORTER_ASSERT(reporter, 0 == (freed[0] + freed[1] + freed[2]));

    // Send message to free the first resource
    GrTextureFreedMessage msg1{wrapped[0], dContext->directContextID()};
    SkMessageBus<GrTextureFreedMessage, GrDirectContext::DirectContextID>::Post(msg1);
    cache->purgeAsNeeded();

    REPORTER_ASSERT(reporter, 1 == (freed[0] + freed[1] + freed[2]));
    REPORTER_ASSERT(reporter, 1 == freed[0]);

    GrTextureFreedMessage msg2{wrapped[2], dContext->directContextID()};
    SkMessageBus<GrTextureFreedMessage, GrDirectContext::DirectContextID>::Post(msg2);
    cache->purgeAsNeeded();

    REPORTER_ASSERT(reporter, 2 == (freed[0] + freed[1] + freed[2]));
    REPORTER_ASSERT(reporter, 0 == freed[1]);

    mock.reset();

    REPORTER_ASSERT(reporter, 3 == (freed[0] + freed[1] + freed[2]));
}

DEF_GPUTEST(ResourceCacheMisc, reporter, /* options */) {
    // The below tests create their own mock contexts.
    test_no_key(reporter);
    test_purge_unlocked(reporter);
    test_purge_command_buffer_usage(reporter);
    test_budgeting(reporter);
    test_unbudgeted(reporter);
    test_unbudgeted_to_scratch(reporter);
    test_duplicate_unique_key(reporter);
    test_duplicate_scratch_key(reporter);
    test_remove_scratch_key(reporter);
    test_scratch_key_consistency(reporter);
    test_purge_invalidated(reporter);
    test_cache_chained_purge(reporter);
    test_timestamp_wrap(reporter);
    test_time_purge(reporter);
    test_partial_purge(reporter);
    test_custom_data(reporter);
    test_abandoned(reporter);
    test_tags(reporter);
    test_free_texture_messages(reporter);
}

// This simulates a portion of Chrome's context abandonment processing.
// Please see: crbug.com/1011368 and crbug.com/1014993
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceMessagesAfterAbandon, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrGpu* gpu = dContext->priv().getGpu();
    GrResourceCache* cache = dContext->priv().getResourceCache();

    GrBackendTexture backend = dContext->createBackendTexture(16, 16,
                                                              SkColorType::kRGBA_8888_SkColorType,
                                                              GrMipmapped::kNo, GrRenderable::kNo);
    GrTexture* tex = gpu->wrapBackendTexture(backend,
                                             GrWrapOwnership::kBorrow_GrWrapOwnership,
                                             GrWrapCacheable::kYes,
                                             GrIOType::kRead_GrIOType)
                             .release();

    auto releaseProc = [](void* ctx) {
        int* index = (int*) ctx;
        *index = 1;
    };

    int freed = 0;

    tex->setRelease(releaseProc, &freed);

    cache->insertDelayedTextureUnref(tex);

    // Now only the cache is holding a ref to this texture
    tex->unref();

    REPORTER_ASSERT(reporter, 0 == freed);

    // We must delete the backend texture before abandoning the context in vulkan. We just do it
    // for all the backends for consistency.
    dContext->deleteBackendTexture(backend);
    dContext->abandonContext();

    REPORTER_ASSERT(reporter, 1 == freed);

    // In the past, creating this message could cause an exception due to
    // an un-safe downcast from GrTexture to GrGpuResource
    GrTextureFreedMessage msg{tex, dContext->directContextID()};
    SkMessageBus<GrTextureFreedMessage, GrDirectContext::DirectContextID>::Post(msg);

    // This doesn't actually do anything but it does trigger us to read messages
    dContext->purgeUnlockedResources(false);
}

////////////////////////////////////////////////////////////////////////////////
static sk_sp<GrTexture> make_normal_texture(GrResourceProvider* provider,
                                            GrRenderable renderable,
                                            SkISize dims,
                                            int sampleCnt) {
    auto format = provider->caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888, renderable);
    return provider->createTexture(dims, format, GrTextureType::k2D, renderable, sampleCnt,
                                   GrMipmapped::kNo, SkBudgeted::kYes, GrProtected::kNo);
}

static sk_sp<GrTextureProxy> make_mipmap_proxy(GrRecordingContext* rContext,
                                               GrRenderable renderable,
                                               SkISize dims,
                                               int sampleCnt) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    const GrCaps* caps = rContext->priv().caps();


    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                 GrRenderable::kNo);

    return proxyProvider->createProxy(format, dims, renderable, sampleCnt, GrMipmapped::kYes,
                                      SkBackingFit::kExact, SkBudgeted::kYes, GrProtected::kNo);
}

// Exercise GrSurface::gpuMemorySize for different combos of MSAA, RT-only,
// Texture-only, both-RT-and-Texture and MIPmapped
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GPUMemorySize, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    const GrCaps* caps = context->priv().caps();

    static constexpr SkISize kSize = {64, 64};
    static constexpr auto kArea = kSize.area();

    // Normal versions
    {
        sk_sp<GrTexture> tex;

        tex = make_normal_texture(resourceProvider, GrRenderable::kYes, kSize, 1);
        size_t size = tex->gpuMemorySize();
        REPORTER_ASSERT(reporter, kArea*4 == size);

        size_t sampleCount = (size_t)caps->getRenderTargetSampleCount(4, tex->backendFormat());
        if (sampleCount >= 4) {
            tex = make_normal_texture(resourceProvider, GrRenderable::kYes, kSize, sampleCount);
            size = tex->gpuMemorySize();
            REPORTER_ASSERT(reporter,
                            kArea*4 == size ||                  // msaa4 failed
                            kArea*4*sampleCount == size ||      // auto-resolving
                            kArea*4*(sampleCount+1) == size);   // explicit resolve buffer
        }

        tex = make_normal_texture(resourceProvider, GrRenderable::kNo, kSize, 1);
        size = tex->gpuMemorySize();
        REPORTER_ASSERT(reporter, kArea*4 == size);
    }

    // Mipmapped versions
    if (caps->mipmapSupport()) {
        sk_sp<GrTextureProxy> proxy;

        proxy = make_mipmap_proxy(context, GrRenderable::kYes, kSize, 1);
        size_t size = proxy->gpuMemorySize();
        REPORTER_ASSERT(reporter, kArea*4 + (kArea*4)/3 == size);

        size_t sampleCount = (size_t)caps->getRenderTargetSampleCount(4, proxy->backendFormat());
        if (sampleCount >= 4) {
            proxy = make_mipmap_proxy(context, GrRenderable::kYes, kSize, sampleCount);
            size = proxy->gpuMemorySize();
            REPORTER_ASSERT(reporter,
               kArea*4 + (kArea*4)/3 == size ||                 // msaa4 failed
               kArea*4*sampleCount + (kArea*4)/3 == size ||     // auto-resolving
               kArea*4*(sampleCount+1) + (kArea*4)/3 == size);  // explicit resolve buffer
        }

        proxy = make_mipmap_proxy(context, GrRenderable::kNo, kSize, 1);
        size = proxy->gpuMemorySize();
        REPORTER_ASSERT(reporter, kArea*4 + (kArea*4)/3 == size);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PurgeToMakeHeadroom, reporter, ctxInfo) {
    constexpr size_t kTexSize = 16 * 16 * 4;

    auto dContext = ctxInfo.directContext();
    dContext->setResourceCacheLimit(2 * kTexSize);
    auto resourceProvider = dContext->priv().resourceProvider();
    auto resourceCache = dContext->priv().getResourceCache();
    for (bool success : { true, false }) {
        reporter->push(SkString(success ? "success" : "failure"));

        resourceCache->releaseAll();
        REPORTER_ASSERT(reporter, resourceCache->getBudgetedResourceBytes() == 0);

        // Make one unpurgeable texture and one purgeable texture.
        auto lockedTex = make_normal_texture(resourceProvider, GrRenderable::kNo, {16, 16}, 1);
        REPORTER_ASSERT(reporter, lockedTex->gpuMemorySize() == kTexSize);

        // N.b. this surface is renderable so "reuseScratchTextures = false" won't mess us up.
        auto purgeableTex = make_normal_texture(resourceProvider, GrRenderable::kYes, {16, 16}, 1);
        if (success) {
            purgeableTex = nullptr;
        }

        size_t expectedPurgeable = success ? kTexSize : 0;
        REPORTER_ASSERT(reporter, expectedPurgeable == resourceCache->getPurgeableBytes(),
                        "%zu vs %zu", expectedPurgeable, resourceCache->getPurgeableBytes());
        REPORTER_ASSERT(reporter, success == resourceCache->purgeToMakeHeadroom(kTexSize));
        size_t expectedBudgeted = success ? kTexSize : (2 * kTexSize);
        REPORTER_ASSERT(reporter, expectedBudgeted == resourceCache->getBudgetedResourceBytes(),
                        "%zu vs %zu", expectedBudgeted, resourceCache->getBudgetedResourceBytes());
        reporter->pop();
    }
}

#if GR_GPU_STATS
DEF_GPUTEST_FOR_MOCK_CONTEXT(OverbudgetFlush, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    context->setResourceCacheLimit(1);

    // Helper that determines if cache is overbudget.
    auto overbudget = [context] {
         int uNum;
         size_t uSize;
         context->getResourceCacheUsage(&uNum, &uSize);
         size_t bSize = context->getResourceCacheLimit();
         return uSize > bSize;
    };

    // Helper that does a trivial draw to a surface.
    auto drawToSurf = [](SkSurface* surf) {
        surf->getCanvas()->drawRect(SkRect::MakeWH(1,1), SkPaint());
    };

    // Helper that checks whether a flush has occurred between calls.
    int baseFlushCount = 0;
    auto getFlushCountDelta = [context, &baseFlushCount]() {
        int cur = context->priv().getGpu()->stats()->numSubmitToGpus();
        int delta = cur - baseFlushCount;
        baseFlushCount = cur;
        return delta;
    };

    auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf1 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, 1, nullptr);
    auto surf2 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, 1, nullptr);

    drawToSurf(surf1.get());
    drawToSurf(surf2.get());

    // Flush each surface once to ensure that their backing stores are allocated.
    surf1->flushAndSubmit();
    surf2->flushAndSubmit();
    REPORTER_ASSERT(reporter, overbudget());
    getFlushCountDelta();

    // Nothing should be purgeable so drawing to either surface doesn't cause a flush.
    drawToSurf(surf1.get());
    REPORTER_ASSERT(reporter, !getFlushCountDelta());
    drawToSurf(surf2.get());
    REPORTER_ASSERT(reporter, !getFlushCountDelta());
    REPORTER_ASSERT(reporter, overbudget());

    // Make surf1 purgeable. Drawing to surf2 should flush.
    surf1->flushAndSubmit();
    surf1.reset();
    drawToSurf(surf2.get());
    REPORTER_ASSERT(reporter, getFlushCountDelta());
    REPORTER_ASSERT(reporter, overbudget());
}
#endif
