/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Include here to ensure SK_SUPPORT_GPU is set correctly before it is examined.
#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrGpu.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrGpuResourcePriv.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkMessageBus.h"
#include "SkMipMap.h"
#include "SkSurface.h"
#include "Test.h"

static const int gWidth = 640;
static const int gHeight = 480;

////////////////////////////////////////////////////////////////////////////////
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceCacheCache, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = gWidth;
    desc.fHeight = gHeight;
    SkImageInfo info = SkImageInfo::MakeN32Premul(gWidth, gHeight);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    const SkIRect size = SkIRect::MakeWH(gWidth, gHeight);

    SkBitmap src;
    src.allocN32Pixels(size.width(), size.height());
    src.eraseColor(SK_ColorBLACK);
    size_t srcSize = src.getSize();

    size_t initialCacheSize;
    context->getResourceCacheUsage(nullptr, &initialCacheSize);

    int oldMaxNum;
    size_t oldMaxBytes;
    context->getResourceCacheLimits(&oldMaxNum, &oldMaxBytes);

    // Set the cache limits so we can fit 10 "src" images and the
    // max number of textures doesn't matter
    size_t maxCacheSize = initialCacheSize + 10*srcSize;
    context->setResourceCacheLimits(1000, maxCacheSize);

    SkBitmap readback;
    readback.allocN32Pixels(size.width(), size.height());

    for (int i = 0; i < 100; ++i) {
        canvas->drawBitmap(src, 0, 0);
        canvas->readPixels(size, &readback);

        // "modify" the src texture
        src.notifyPixelsChanged();

        size_t curCacheSize;
        context->getResourceCacheUsage(nullptr, &curCacheSize);

        // we should never go over the size limit
        REPORTER_ASSERT(reporter, curCacheSize <= maxCacheSize);
    }

    context->setResourceCacheLimits(oldMaxNum, oldMaxBytes);
}

static bool is_rendering_and_not_angle_es3(sk_gpu_test::GrContextFactory::ContextType type) {
    if (type == sk_gpu_test::GrContextFactory::kANGLE_D3D11_ES3_ContextType ||
        type == sk_gpu_test::GrContextFactory::kANGLE_GL_ES3_ContextType) {
        return false;
    }
    return sk_gpu_test::GrContextFactory::IsRenderingContext(type);
}

// This currently fails on ES3 ANGLE contexts
DEF_GPUTEST_FOR_CONTEXTS(ResourceCacheStencilBuffers, &is_rendering_and_not_angle_es3, reporter,
                         ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrSurfaceDesc smallDesc;
    smallDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    smallDesc.fConfig = kRGBA_8888_GrPixelConfig;
    smallDesc.fWidth = 4;
    smallDesc.fHeight = 4;
    smallDesc.fSampleCnt = 0;

    GrResourceProvider* resourceProvider = context->resourceProvider();
    // Test that two budgeted RTs with the same desc share a stencil buffer.
    sk_sp<GrTexture> smallRT0(resourceProvider->createTexture(smallDesc, SkBudgeted::kYes));
    if (smallRT0 && smallRT0->asRenderTarget()) {
        resourceProvider->attachStencilAttachment(smallRT0->asRenderTarget());
    }

    sk_sp<GrTexture> smallRT1(resourceProvider->createTexture(smallDesc, SkBudgeted::kYes));
    if (smallRT1 && smallRT1->asRenderTarget()) {
        resourceProvider->attachStencilAttachment(smallRT1->asRenderTarget());
    }

    REPORTER_ASSERT(reporter,
                    smallRT0 && smallRT1 &&
                    smallRT0->asRenderTarget() && smallRT1->asRenderTarget() &&
                    resourceProvider->attachStencilAttachment(smallRT0->asRenderTarget()) ==
                    resourceProvider->attachStencilAttachment(smallRT1->asRenderTarget()));

    // An unbudgeted RT with the same desc should also share.
    sk_sp<GrTexture> smallRT2(resourceProvider->createTexture(smallDesc, SkBudgeted::kNo));
    if (smallRT2 && smallRT2->asRenderTarget()) {
        resourceProvider->attachStencilAttachment(smallRT2->asRenderTarget());
    }
    REPORTER_ASSERT(reporter,
                    smallRT0 && smallRT2 &&
                    smallRT0->asRenderTarget() && smallRT2->asRenderTarget() &&
                    resourceProvider->attachStencilAttachment(smallRT0->asRenderTarget()) ==
                    resourceProvider->attachStencilAttachment(smallRT2->asRenderTarget()));

    // An RT with a much larger size should not share.
    GrSurfaceDesc bigDesc;
    bigDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    bigDesc.fConfig = kRGBA_8888_GrPixelConfig;
    bigDesc.fWidth = 400;
    bigDesc.fHeight = 200;
    bigDesc.fSampleCnt = 0;
    sk_sp<GrTexture> bigRT(resourceProvider->createTexture(bigDesc, SkBudgeted::kNo));
    if (bigRT && bigRT->asRenderTarget()) {
        resourceProvider->attachStencilAttachment(bigRT->asRenderTarget());
    }
    REPORTER_ASSERT(reporter,
                    smallRT0 && bigRT &&
                    smallRT0->asRenderTarget() && bigRT->asRenderTarget() &&
                    resourceProvider->attachStencilAttachment(smallRT0->asRenderTarget()) !=
                    resourceProvider->attachStencilAttachment(bigRT->asRenderTarget()));

    if (context->caps()->maxSampleCount() >= 4) {
        // An RT with a different sample count should not share.
        GrSurfaceDesc smallMSAADesc = smallDesc;
        smallMSAADesc.fSampleCnt = 4;
        sk_sp<GrTexture> smallMSAART0(resourceProvider->createTexture(smallMSAADesc,
                                                                      SkBudgeted::kNo));
        if (smallMSAART0 && smallMSAART0->asRenderTarget()) {
            resourceProvider->attachStencilAttachment(smallMSAART0->asRenderTarget());
        }
#ifdef SK_BUILD_FOR_ANDROID
        if (!smallMSAART0) {
            // The nexus player seems to fail to create MSAA textures.
            return;
        }
#endif
        REPORTER_ASSERT(reporter,
                        smallRT0 && smallMSAART0 &&
                        smallRT0->asRenderTarget() && smallMSAART0->asRenderTarget() &&
                        resourceProvider->attachStencilAttachment(smallRT0->asRenderTarget()) !=
                        resourceProvider->attachStencilAttachment(smallMSAART0->asRenderTarget()));
        // A second MSAA RT should share with the first MSAA RT.
        sk_sp<GrTexture> smallMSAART1(resourceProvider->createTexture(smallMSAADesc,
                                                                      SkBudgeted::kNo));
        if (smallMSAART1 && smallMSAART1->asRenderTarget()) {
            resourceProvider->attachStencilAttachment(smallMSAART1->asRenderTarget());
        }
        REPORTER_ASSERT(reporter,
                        smallMSAART0 && smallMSAART1 &&
                        smallMSAART0->asRenderTarget() &&
                        smallMSAART1->asRenderTarget() &&
                        resourceProvider->attachStencilAttachment(smallMSAART0->asRenderTarget()) ==
                        resourceProvider->attachStencilAttachment(smallMSAART1->asRenderTarget()));
        // But not one with a larger sample count should not. (Also check that the request for 4
        // samples didn't get rounded up to >= 8 or else they could share.).
        if (context->caps()->maxSampleCount() >= 8 &&
            smallMSAART0 && smallMSAART0->asRenderTarget() &&
            smallMSAART0->asRenderTarget()->numColorSamples() < 8) {
            smallMSAADesc.fSampleCnt = 8;
            smallMSAART1.reset(resourceProvider->createTexture(smallMSAADesc, SkBudgeted::kNo));
            sk_sp<GrTexture> smallMSAART1(
                resourceProvider->createTexture(smallMSAADesc, SkBudgeted::kNo));
            if (smallMSAART1 && smallMSAART1->asRenderTarget()) {
                resourceProvider->attachStencilAttachment(smallMSAART1->asRenderTarget());
            }
            REPORTER_ASSERT(reporter,
                        smallMSAART0 && smallMSAART1 &&
                        smallMSAART0->asRenderTarget() &&
                        smallMSAART1->asRenderTarget() &&
                        resourceProvider->attachStencilAttachment(smallMSAART0->asRenderTarget()) !=
                        resourceProvider->attachStencilAttachment(smallMSAART1->asRenderTarget()));
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceCacheWrappedResources, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGpu* gpu = context->getGpu();
    // this test is only valid for GL
    if (!gpu || !gpu->glContextForTesting()) {
        return;
    }

    GrBackendObject texHandles[2];
    static const int kW = 100;
    static const int kH = 100;

    texHandles[0] = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kRGBA_8888_GrPixelConfig);
    texHandles[1] = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kRGBA_8888_GrPixelConfig);

    context->resetContext();

    GrBackendTextureDesc desc;
    desc.fConfig = kBGRA_8888_GrPixelConfig;
    desc.fWidth = kW;
    desc.fHeight = kH;

    desc.fTextureHandle = texHandles[0];
    sk_sp<GrTexture> borrowed(context->resourceProvider()->wrapBackendTexture(
                              desc, kBorrow_GrWrapOwnership));

    desc.fTextureHandle = texHandles[1];
    sk_sp<GrTexture> adopted(context->resourceProvider()->wrapBackendTexture(
                             desc, kAdopt_GrWrapOwnership));

    REPORTER_ASSERT(reporter, borrowed != nullptr && adopted != nullptr);
    if (!borrowed || !adopted) {
        return;
    }

    borrowed.reset(nullptr);
    adopted.reset(nullptr);

    context->flush();

    bool borrowedIsAlive = gpu->isTestingOnlyBackendTexture(texHandles[0]);
    bool adoptedIsAlive = gpu->isTestingOnlyBackendTexture(texHandles[1]);

    REPORTER_ASSERT(reporter, borrowedIsAlive);
    REPORTER_ASSERT(reporter, !adoptedIsAlive);

    gpu->deleteTestingOnlyBackendTexture(texHandles[0], !borrowedIsAlive);
    gpu->deleteTestingOnlyBackendTexture(texHandles[1], !adoptedIsAlive);

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
                                       SimulatedProperty property) {
        return new TestResource(gpu, budgeted, property, kScratchConstructor);
    }
    static TestResource* CreateWrapped(GrGpu* gpu, size_t size = kDefaultSize) {
        return new TestResource(gpu, size);
    }

    ~TestResource() {
        --fNumAlive;
        SkSafeUnref(fToDelete);
    }

    void setSize(size_t size) {
        fSize = size;
        this->didChangeGpuMemorySize();
    }

    static int NumAlive() { return fNumAlive; }

    void setUnrefWhenDestroyed(TestResource* resource) {
        SkRefCnt_SafeAssign(fToDelete, resource);
    }

    static void ComputeScratchKey(SimulatedProperty property, GrScratchKey* key) {
        static GrScratchKey::ResourceType t = GrScratchKey::GenerateResourceType();
        GrScratchKey::Builder builder(key, t, kScratchKeyFieldCnt);
        for (int i = 0; i < kScratchKeyFieldCnt; ++i) {
            builder[i] = static_cast<uint32_t>(i + property);
        }
    }

    static size_t ExpectedScratchKeySize() {
        return sizeof(uint32_t) * (kScratchKeyFieldCnt + GrScratchKey::kMetaDataCnt);
    }
private:
    static const int kScratchKeyFieldCnt = 6;

    TestResource(GrGpu* gpu, SkBudgeted budgeted, SimulatedProperty property, ScratchConstructor)
        : INHERITED(gpu)
        , fToDelete(nullptr)
        , fSize(kDefaultSize)
        , fProperty(property)
        , fIsScratch(true) {
        ++fNumAlive;
        this->registerWithCache(budgeted);
    }

    // Constructor for simulating resources that wrap backend objects.
    TestResource(GrGpu* gpu, size_t size)
        : INHERITED(gpu)
        , fToDelete(nullptr)
        , fSize(size)
        , fProperty(kA_SimulatedProperty)
        , fIsScratch(false) {
        ++fNumAlive;
        this->registerWithCacheWrapped();
    }

    void computeScratchKey(GrScratchKey* key) const override {
        if (fIsScratch) {
            ComputeScratchKey(fProperty, key);
        }
    }

    size_t onGpuMemorySize() const override { return fSize; }

    TestResource* fToDelete;
    size_t fSize;
    static int fNumAlive;
    SimulatedProperty fProperty;
    bool fIsScratch;
    typedef GrGpuResource INHERITED;
};
int TestResource::fNumAlive = 0;

class Mock {
public:
    Mock(int maxCnt, size_t maxBytes) {
        fContext.reset(GrContext::CreateMockContext());
        SkASSERT(fContext);
        fContext->setResourceCacheLimits(maxCnt, maxBytes);
        GrResourceCache* cache = fContext->getResourceCache();
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());
    }

    GrResourceCache* cache() { return fContext->getResourceCache(); }

    GrContext* context() { return fContext.get(); }

private:
    sk_sp<GrContext> fContext;
};

static void test_no_key(skiatest::Reporter* reporter) {
    Mock mock(10, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    // Create a bunch of resources with no keys
    TestResource* a = new TestResource(context->getGpu());
    TestResource* b = new TestResource(context->getGpu());
    TestResource* c = new TestResource(context->getGpu());
    TestResource* d = new TestResource(context->getGpu());
    a->setSize(11);
    b->setSize(12);
    c->setSize(13);
    d->setSize(14);

    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() + c->gpuMemorySize() +
                              d->gpuMemorySize() == cache->getResourceBytes());

    // Should be safe to purge without deleting the resources since we still have refs.
    cache->purgeAllUnlocked();
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
static void make_unique_key(GrUniqueKey* key, int data, const char* tag = nullptr) {
    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, d, 1, tag);
    builder[0] = data;
}

static void test_budgeting(skiatest::Reporter* reporter) {
    Mock mock(10, 300);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    GrUniqueKey uniqueKey;
    make_unique_key<0>(&uniqueKey, 0);

    // Create a scratch, a unique, and a wrapped resource
    TestResource* scratch =
            TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes, TestResource::kB_SimulatedProperty);
    scratch->setSize(10);
    TestResource* unique = new TestResource(context->getGpu());
    unique->setSize(11);
    unique->resourcePriv().setUniqueKey(uniqueKey);
    TestResource* wrapped = TestResource::CreateWrapped(context->getGpu());
    wrapped->setSize(12);
    TestResource* unbudgeted =
            new TestResource(context->getGpu(), SkBudgeted::kNo);
    unbudgeted->setSize(13);

    // Make sure we can't add a unique key to the wrapped resource
    GrUniqueKey uniqueKey2;
    make_unique_key<0>(&uniqueKey2, 1);
    wrapped->resourcePriv().setUniqueKey(uniqueKey2);
    REPORTER_ASSERT(reporter, nullptr == cache->findAndRefUniqueResource(uniqueKey2));

    // Make sure sizes are as we expect
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() +
                              wrapped->gpuMemorySize() + unbudgeted->gpuMemorySize() ==
                              cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() ==
                              cache->getBudgetedResourceBytes());

    // Our refs mean that the resources are non purgeable.
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 4 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() +
                              wrapped->gpuMemorySize() + unbudgeted->gpuMemorySize() ==
                              cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() ==
                              cache->getBudgetedResourceBytes());

    // Unreffing the wrapped resource should free it right away.
    wrapped->unref();
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + unique->gpuMemorySize() +
                              unbudgeted->gpuMemorySize() == cache->getResourceBytes());

    // Now try freeing the budgeted resources first
    wrapped = TestResource::CreateWrapped(context->getGpu());
    scratch->setSize(12);
    unique->unref();
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + wrapped->gpuMemorySize() +
                              unbudgeted->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() == cache->getBudgetedResourceBytes());

    scratch->unref();
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, unbudgeted->gpuMemorySize() + wrapped->gpuMemorySize() ==
                              cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());

    wrapped->unref();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, unbudgeted->gpuMemorySize() == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());

    unbudgeted->unref();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
}

static void test_unbudgeted(skiatest::Reporter* reporter) {
    Mock mock(10, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    GrUniqueKey uniqueKey;
    make_unique_key<0>(&uniqueKey, 0);

    TestResource* scratch;
    TestResource* unique;
    TestResource* wrapped;
    TestResource* unbudgeted;

    // A large uncached or wrapped resource shouldn't evict anything.
    scratch = TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes,
                                          TestResource::kB_SimulatedProperty);

    scratch->setSize(10);
    scratch->unref();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 10 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 10 == cache->getBudgetedResourceBytes());

    unique = new TestResource(context->getGpu());
    unique->setSize(11);
    unique->resourcePriv().setUniqueKey(uniqueKey);
    unique->unref();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());

    size_t large = 2 * cache->getResourceBytes();
    unbudgeted = new TestResource(context->getGpu(), SkBudgeted::kNo, large);
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 + large == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());

    unbudgeted->unref();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());

    wrapped = TestResource::CreateWrapped(context->getGpu(), large);
    REPORTER_ASSERT(reporter, 3 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 + large == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());

    wrapped->unref();
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache->getBudgetedResourceBytes());

    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
}

// This method can't be static because it needs to friended in GrGpuResource::CacheAccess.
void test_unbudgeted_to_scratch(skiatest::Reporter* reporter);
/*static*/ void test_unbudgeted_to_scratch(skiatest::Reporter* reporter) {
    Mock mock(10, 300);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    TestResource* resource =
        TestResource::CreateScratch(context->getGpu(), SkBudgeted::kNo,
                                    TestResource::kA_SimulatedProperty);
    GrScratchKey key;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &key);

    size_t size = resource->gpuMemorySize();
    for (int i = 0; i < 2; ++i) {
        // Since this resource is unbudgeted, it should not be reachable as scratch.
        REPORTER_ASSERT(reporter, resource->resourcePriv().getScratchKey() == key);
        REPORTER_ASSERT(reporter, !resource->cacheAccess().isScratch());
        REPORTER_ASSERT(reporter, SkBudgeted::kNo == resource->resourcePriv().isBudgeted());
        REPORTER_ASSERT(reporter, nullptr == cache->findAndRefScratchResource(key, TestResource::kDefaultSize, 0));
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
        REPORTER_ASSERT(reporter, size == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());

        // Once it is unrefed, it should become available as scratch.
        resource->unref();
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
        REPORTER_ASSERT(reporter, size == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, size == cache->getBudgetedResourceBytes());
        resource = static_cast<TestResource*>(cache->findAndRefScratchResource(key, TestResource::kDefaultSize, 0));
        REPORTER_ASSERT(reporter, resource);
        REPORTER_ASSERT(reporter, resource->resourcePriv().getScratchKey() == key);
        REPORTER_ASSERT(reporter, resource->cacheAccess().isScratch());
        REPORTER_ASSERT(reporter, SkBudgeted::kYes == resource->resourcePriv().isBudgeted());

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
            REPORTER_ASSERT(reporter, !resource->resourcePriv().getScratchKey().isValid());
            REPORTER_ASSERT(reporter, !resource->cacheAccess().isScratch());
            REPORTER_ASSERT(reporter, SkBudgeted::kYes == resource->resourcePriv().isBudgeted());

            // now when it is unrefed it should die since it has no key.
            resource->unref();
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
            REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
            REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceCount());
            REPORTER_ASSERT(reporter, 0 == cache->getBudgetedResourceBytes());
        }
    }
}

static void test_duplicate_scratch_key(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    // Create two resources that have the same scratch key.
    TestResource* a = TestResource::CreateScratch(context->getGpu(),
                                                  SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    TestResource* b = TestResource::CreateScratch(context->getGpu(),
                                                  SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    a->setSize(11);
    b->setSize(12);
    GrScratchKey scratchKey1;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey1);
    // Check for negative case consistency. (leaks upon test failure.)
    REPORTER_ASSERT(reporter, nullptr == cache->findAndRefScratchResource(scratchKey1, TestResource::kDefaultSize, 0));

    GrScratchKey scratchKey;
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey);

    // Scratch resources are registered with GrResourceCache just by existing. There are 2.
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() ==
                              cache->getResourceBytes());

    // Our refs mean that the resources are non purgeable.
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());

    // Unref but don't purge
    a->unref();
    b->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->countScratchEntriesForKey(scratchKey));)

    // Purge again. This time resources should be purgeable.
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->countScratchEntriesForKey(scratchKey));)
}

static void test_remove_scratch_key(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    // Create two resources that have the same scratch key.
    TestResource* a = TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    TestResource* b = TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    a->unref();
    b->unref();

    GrScratchKey scratchKey;
    // Ensure that scratch key lookup is correct for negative case.
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey);
    // (following leaks upon test failure).
    REPORTER_ASSERT(reporter, cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0) == nullptr);

    // Scratch resources are registered with GrResourceCache just by existing. There are 2.
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());

    // Find the first resource and remove its scratch key
    GrGpuResource* find;
    find = cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0);
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
    find = cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0);
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
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    // Create two resources that have the same scratch key.
    TestResource* a = TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    TestResource* b = TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes,
                                                  TestResource::kB_SimulatedProperty);
    a->unref();
    b->unref();

    GrScratchKey scratchKey;
    // Ensure that scratch key comparison and assignment is consistent.
    GrScratchKey scratchKey1;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey1);
    GrScratchKey scratchKey2;
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
    REPORTER_ASSERT(reporter, cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0) == nullptr);

    // Find the first resource with a scratch key and a copy of a scratch key.
    TestResource::ComputeScratchKey(TestResource::kB_SimulatedProperty, &scratchKey);
    GrGpuResource* find = cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0);
    REPORTER_ASSERT(reporter, find != nullptr);
    find->unref();

    scratchKey2 = scratchKey;
    find = cache->findAndRefScratchResource(scratchKey2, TestResource::kDefaultSize, 0);
    REPORTER_ASSERT(reporter, find != nullptr);
    REPORTER_ASSERT(reporter, find == a || find == b);

    GrGpuResource* find2 = cache->findAndRefScratchResource(scratchKey2, TestResource::kDefaultSize, 0);
    REPORTER_ASSERT(reporter, find2 != nullptr);
    REPORTER_ASSERT(reporter, find2 == a || find2 == b);
    REPORTER_ASSERT(reporter, find2 != find);
    find2->unref();
    find->unref();
}

static void test_duplicate_unique_key(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    GrUniqueKey key;
    make_unique_key<0>(&key, 0);

    // Create two resources that we will attempt to register with the same unique key.
    TestResource* a = new TestResource(context->getGpu());
    a->setSize(11);

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
    TestResource* b = new TestResource(context->getGpu());
    b->setSize(12);
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
    TestResource* c = new TestResource(context->getGpu());
    GrUniqueKey differentKey;
    make_unique_key<0>(&differentKey, 1);
    c->setSize(13);
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
    cache->purgeAllUnlocked();
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
        GrUniqueKey key2;
        make_unique_key<0>(&key2, 0);
        sk_sp<TestResource> d(new TestResource(context->getGpu()));
        int foo = 4132;
        key2.setCustomData(SkData::MakeWithCopy(&foo, sizeof(foo)));
        d->resourcePriv().setUniqueKey(key2);
    }

    GrUniqueKey key3;
    make_unique_key<0>(&key3, 0);
    sk_sp<GrGpuResource> d2(cache->findAndRefUniqueResource(key3));
    REPORTER_ASSERT(reporter, *(int*) d2->getUniqueKey().getCustomData()->data() == 4132);
}

static void test_purge_invalidated(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    GrUniqueKey key1, key2, key3;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);
    make_unique_key<0>(&key3, 3);

    // Add three resources to the cache. Only c is usable as scratch.
    TestResource* a = new TestResource(context->getGpu());
    TestResource* b = new TestResource(context->getGpu());
    TestResource* c = TestResource::CreateScratch(context->getGpu(), SkBudgeted::kYes,
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

    typedef GrUniqueKeyInvalidatedMessage Msg;
    typedef SkMessageBus<GrUniqueKeyInvalidatedMessage> Bus;

    // Invalidate two of the three, they should be purged and no longer accessible via their keys.
    Bus::Post(Msg(key1));
    Bus::Post(Msg(key2));
    cache->purgeAsNeeded();
    // a should be deleted now, but we still have a ref on b.
    REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key1));
    REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key2));
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, cache->hasUniqueKey(key3));

    // Invalidate the third.
    Bus::Post(Msg(key3));
    cache->purgeAsNeeded();
    // we still have a ref on b, c should be recycled as scratch.
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key3));

    // make b purgeable. It should be immediately deleted since it has no key.
    b->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Make sure we actually get to c via it's scratch key, before we say goodbye.
    GrScratchKey scratchKey;
    TestResource::ComputeScratchKey(TestResource::kA_SimulatedProperty, &scratchKey);
    GrGpuResource* scratch = cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0);
    REPORTER_ASSERT(reporter, scratch == c);
    SkSafeUnref(scratch);

    // Get rid of c.
    cache->purgeAllUnlocked();
    scratch = cache->findAndRefScratchResource(scratchKey, TestResource::kDefaultSize, 0);
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceBytes());
    REPORTER_ASSERT(reporter, !scratch);
    SkSafeUnref(scratch);
}

static void test_cache_chained_purge(skiatest::Reporter* reporter) {
    Mock mock(3, 30000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    GrUniqueKey key1, key2;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);

    TestResource* a = new TestResource(context->getGpu());
    TestResource* b = new TestResource(context->getGpu());
    a->resourcePriv().setUniqueKey(key1);
    b->resourcePriv().setUniqueKey(key2);

    // Make a cycle
    a->setUnrefWhenDestroyed(b);
    b->setUnrefWhenDestroyed(a);

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    a->unref();
    b->unref();

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    // Break the cycle
    a->setUnrefWhenDestroyed(nullptr);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
}

static void test_resource_size_changed(skiatest::Reporter* reporter) {
    GrUniqueKey key1, key2;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);

    // Test changing resources sizes (both increase & decrease).
    {
        Mock mock(3, 30000);
        GrContext* context = mock.context();
        GrResourceCache* cache = mock.cache();

        TestResource* a = new TestResource(context->getGpu());
        a->resourcePriv().setUniqueKey(key1);
        a->unref();

        TestResource* b = new TestResource(context->getGpu());
        b->resourcePriv().setUniqueKey(key2);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
        {
            sk_sp<TestResource> find2(
                static_cast<TestResource*>(cache->findAndRefUniqueResource(key2)));
            find2->setSize(200);
            sk_sp<TestResource> find1(
                static_cast<TestResource*>(cache->findAndRefUniqueResource(key1)));
            find1->setSize(50);
        }

        REPORTER_ASSERT(reporter, 250 == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());
    }

    // Test increasing a resources size beyond the cache budget.
    {
        Mock mock(2, 300);
        GrContext* context = mock.context();
        GrResourceCache* cache = mock.cache();

        TestResource* a = new TestResource(context->getGpu());
        a->setSize(100);
        a->resourcePriv().setUniqueKey(key1);
        a->unref();

        TestResource* b = new TestResource(context->getGpu());
        b->setSize(100);
        b->resourcePriv().setUniqueKey(key2);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache->getResourceCount());

        {
            sk_sp<TestResource> find2(static_cast<TestResource*>(
                cache->findAndRefUniqueResource(key2)));
            find2->setSize(201);
        }
        REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key1));

        REPORTER_ASSERT(reporter, 201 == cache->getResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    }
}

static void test_timestamp_wrap(skiatest::Reporter* reporter) {
    static const int kCount = 50;
    static const int kBudgetCnt = kCount / 2;
    static const int kLockedFreq = 8;
    static const int kBudgetSize = 0x80000000;

    SkRandom random;

    // Run the test 2*kCount times;
    for (int i = 0; i < 2 * kCount; ++i ) {
        Mock mock(kBudgetCnt, kBudgetSize);
        GrContext* context = mock.context();
        GrResourceCache* cache = mock.cache();

        // Pick a random number of resources to add before the timestamp will wrap.
        cache->changeTimestamp(SK_MaxU32 - random.nextULessThan(kCount + 1));

        static const int kNumToPurge = kCount - kBudgetCnt;

        SkTDArray<int> shouldPurgeIdxs;
        int purgeableCnt = 0;
        SkTDArray<GrGpuResource*> resourcesToUnref;

        // Add kCount resources, holding onto resources at random so we have a mix of purgeable and
        // unpurgeable resources.
        for (int j = 0; j < kCount; ++j) {
            GrUniqueKey key;
            make_unique_key<0>(&key, j);

            TestResource* r = new TestResource(context->getGpu());
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
            GrUniqueKey key;
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

static void test_flush(skiatest::Reporter* reporter) {
    Mock mock(1000000, 1000000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    // The current cache impl will round the max flush count to the next power of 2. So we choose a
    // power of two here to keep things simpler.
    static const int kFlushCount = 16;
    cache->setLimits(1000000, 1000000, kFlushCount);

    {
        // Insert a resource and send a flush notification kFlushCount times.
        for (int i = 0; i < kFlushCount; ++i) {
            TestResource* r = new TestResource(context->getGpu());
            GrUniqueKey k;
            make_unique_key<1>(&k, i);
            r->resourcePriv().setUniqueKey(k);
            r->unref();
            cache->notifyFlushOccurred(GrResourceCache::kExternal);
        }

        // Send flush notifications to the cache. Each flush should purge the oldest resource.
        for (int i = 0; i < kFlushCount; ++i) {
            cache->notifyFlushOccurred(GrResourceCache::kExternal);
            REPORTER_ASSERT(reporter, kFlushCount - i - 1 == cache->getResourceCount());
            for (int j = 0; j < i; ++j) {
                GrUniqueKey k;
                make_unique_key<1>(&k, j);
                GrGpuResource* r = cache->findAndRefUniqueResource(k);
                REPORTER_ASSERT(reporter, !SkToBool(r));
                SkSafeUnref(r);
            }
        }

        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
        cache->purgeAllUnlocked();
    }

    // Do a similar test but where we leave refs on some resources to prevent them from being
    // purged.
    {
        GrGpuResource* refedResources[kFlushCount >> 1];
        for (int i = 0; i < kFlushCount; ++i) {
            TestResource* r = new TestResource(context->getGpu());
            GrUniqueKey k;
            make_unique_key<1>(&k, i);
            r->resourcePriv().setUniqueKey(k);
            // Leave a ref on every other resource, beginning with the first.
            if (SkToBool(i & 0x1)) {
                refedResources[i/2] = r;
            } else {
                r->unref();
            }
            cache->notifyFlushOccurred(GrResourceCache::kExternal);
        }

        for (int i = 0; i < kFlushCount; ++i) {
            // Should get a resource purged every other flush.
            cache->notifyFlushOccurred(GrResourceCache::kExternal);
            REPORTER_ASSERT(reporter, kFlushCount - i/2 - 1 == cache->getResourceCount());
        }

        // Unref all the resources that we kept refs on in the first loop.
        for (int i = 0; i < kFlushCount >> 1; ++i) {
            refedResources[i]->unref();
        }

        // After kFlushCount + 1 flushes they all will have sat in the purgeable queue for
        // kFlushCount full flushes.
        for (int i = 0; i < kFlushCount + 1; ++i) {
            REPORTER_ASSERT(reporter, kFlushCount >> 1 == cache->getResourceCount());
            cache->notifyFlushOccurred(GrResourceCache::kExternal);
        }
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

        cache->purgeAllUnlocked();
    }

    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    // Verify that calling flush() on a GrContext with nothing to do will not trigger resource
    // eviction.
    context->flush();
    for (int i = 0; i < 10; ++i) {
        TestResource* r = new TestResource(context->getGpu());
        GrUniqueKey k;
        make_unique_key<1>(&k, i);
        r->resourcePriv().setUniqueKey(k);
        r->unref();
    }
    REPORTER_ASSERT(reporter, 10 == cache->getResourceCount());
    for (int i = 0; i < 10 * kFlushCount; ++i) {
        context->flush();
    }
    REPORTER_ASSERT(reporter, 10 == cache->getResourceCount());
}

static void test_large_resource_count(skiatest::Reporter* reporter) {
    // Set the cache size to double the resource count because we're going to create 2x that number
    // resources, using two different key domains. Add a little slop to the bytes because we resize
    // down to 1 byte after creating the resource.
    static const int kResourceCnt = 2000;

    Mock mock(2 * kResourceCnt, 2 * kResourceCnt + 1000);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    for (int i = 0; i < kResourceCnt; ++i) {
        GrUniqueKey key1, key2;
        make_unique_key<1>(&key1, i);
        make_unique_key<2>(&key2, i);

        TestResource* resource;

        resource = new TestResource(context->getGpu());
        resource->resourcePriv().setUniqueKey(key1);
        resource->setSize(1);
        resource->unref();

        resource = new TestResource(context->getGpu());
        resource->resourcePriv().setUniqueKey(key2);
        resource->setSize(1);
        resource->unref();
    }

    REPORTER_ASSERT(reporter, TestResource::NumAlive() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache->getBudgetedResourceBytes() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache->getBudgetedResourceCount() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache->getResourceBytes() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache->getResourceCount() == 2 * kResourceCnt);
    for (int i = 0; i < kResourceCnt; ++i) {
        GrUniqueKey key1, key2;
        make_unique_key<1>(&key1, i);
        make_unique_key<2>(&key2, i);

        REPORTER_ASSERT(reporter, cache->hasUniqueKey(key1));
        REPORTER_ASSERT(reporter, cache->hasUniqueKey(key2));
    }

    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, TestResource::NumAlive() == 0);
    REPORTER_ASSERT(reporter, cache->getBudgetedResourceBytes() == 0);
    REPORTER_ASSERT(reporter, cache->getBudgetedResourceCount() == 0);
    REPORTER_ASSERT(reporter, cache->getResourceBytes() == 0);
    REPORTER_ASSERT(reporter, cache->getResourceCount() == 0);

    for (int i = 0; i < kResourceCnt; ++i) {
        GrUniqueKey key1, key2;
        make_unique_key<1>(&key1, i);
        make_unique_key<2>(&key2, i);

        REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key1));
        REPORTER_ASSERT(reporter, !cache->hasUniqueKey(key2));
    }
}

static void test_custom_data(skiatest::Reporter* reporter) {
    GrUniqueKey key1, key2;
    make_unique_key<0>(&key1, 1);
    make_unique_key<0>(&key2, 2);
    int foo = 4132;
    key1.setCustomData(SkData::MakeWithCopy(&foo, sizeof(foo)));
    REPORTER_ASSERT(reporter, *(int*) key1.getCustomData()->data() == 4132);
    REPORTER_ASSERT(reporter, key2.getCustomData() == nullptr);

    // Test that copying a key also takes a ref on its custom data.
    GrUniqueKey key3 = key1;
    REPORTER_ASSERT(reporter, *(int*) key3.getCustomData()->data() == 4132);
}

static void test_abandoned(skiatest::Reporter* reporter) {
    Mock mock(10, 300);
    GrContext* context = mock.context();
    sk_sp<GrGpuResource> resource(new TestResource(context->getGpu()));
    context->abandonContext();

    REPORTER_ASSERT(reporter, resource->wasDestroyed());

    // Call all the public methods on resource in the abandoned state. They shouldn't crash.

    resource->uniqueID();
    resource->getUniqueKey();
    resource->wasDestroyed();
    resource->gpuMemorySize();
    resource->getContext();

    resource->abandon();
    resource->resourcePriv().getScratchKey();
    resource->resourcePriv().isBudgeted();
    resource->resourcePriv().makeBudgeted();
    resource->resourcePriv().makeUnbudgeted();
    resource->resourcePriv().removeScratchKey();
    GrUniqueKey key;
    make_unique_key<0>(&key, 1);
    resource->resourcePriv().setUniqueKey(key);
    resource->resourcePriv().removeUniqueKey();
}

static void test_tags(skiatest::Reporter* reporter) {
#ifdef SK_DEBUG
    // We will insert 1 resource with tag "tag1", 2 with "tag2", and so on, up through kLastTagIdx.
    static constexpr int kLastTagIdx = 10;
    static constexpr int kNumResources = kLastTagIdx * (kLastTagIdx + 1) / 2;

    Mock mock(kNumResources, kNumResources * TestResource::kDefaultSize);
    GrContext* context = mock.context();
    GrResourceCache* cache = mock.cache();

    SkString tagStr;
    int tagIdx = 0;
    int currTagCnt = 0;

    for (int i = 0; i < kNumResources; ++i, ++currTagCnt) {
        sk_sp<GrGpuResource> resource(new TestResource(context->getGpu()));
        GrUniqueKey key;
        if (currTagCnt == tagIdx) {
            tagIdx += 1;
            currTagCnt = 0;
            tagStr.printf("tag%d", tagIdx);
        }
        make_unique_key<1>(&key, i, tagStr.c_str());
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

DEF_GPUTEST(ResourceCacheMisc, reporter, factory) {
    // The below tests create their own mock contexts.
    test_no_key(reporter);
    test_budgeting(reporter);
    test_unbudgeted(reporter);
    test_unbudgeted_to_scratch(reporter);
    test_duplicate_unique_key(reporter);
    test_duplicate_scratch_key(reporter);
    test_remove_scratch_key(reporter);
    test_scratch_key_consistency(reporter);
    test_purge_invalidated(reporter);
    test_cache_chained_purge(reporter);
    test_resource_size_changed(reporter);
    test_timestamp_wrap(reporter);
    test_flush(reporter);
    test_large_resource_count(reporter);
    test_custom_data(reporter);
    test_abandoned(reporter);
    test_tags(reporter);
}

////////////////////////////////////////////////////////////////////////////////
static sk_sp<GrTexture> make_normal_texture(GrResourceProvider* provider,
                                            GrSurfaceFlags flags,
                                            int width, int height,
                                            int sampleCnt) {
    GrSurfaceDesc desc;
    desc.fFlags = flags;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = sampleCnt;

    return sk_sp<GrTexture>(provider->createTexture(desc, SkBudgeted::kYes));
}

static sk_sp<GrTexture> make_mipmap_texture(GrResourceProvider* provider,
                                            GrSurfaceFlags flags,
                                            int width, int height,
                                            int sampleCnt) {
    SkBitmap bm;

    bm.allocN32Pixels(width, height, true);
    bm.eraseColor(SK_ColorBLUE);

    sk_sp<SkMipMap> mipmaps(SkMipMap::Build(bm, SkDestinationSurfaceColorMode::kLegacy, nullptr));
    SkASSERT(mipmaps);
    SkASSERT(mipmaps->countLevels() > 1);

    int mipLevelCount = mipmaps->countLevels() + 1;

    std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

    texels[0].fPixels = bm.getPixels();
    texels[0].fRowBytes = bm.rowBytes();

    for (int i = 1; i < mipLevelCount; ++i) {
        SkMipMap::Level generatedMipLevel;
        mipmaps->getLevel(i - 1, &generatedMipLevel);
        texels[i].fPixels = generatedMipLevel.fPixmap.addr();
        texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
    }

    GrSurfaceDesc desc;
    desc.fFlags = flags;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = sampleCnt;
    desc.fIsMipMapped = true;

    return sk_sp<GrTexture>(provider->createMipMappedTexture(desc, SkBudgeted::kYes,
                                                             texels.get(), mipLevelCount));
}

// Exercise GrSurface::gpuMemorySize for different combos of MSAA, RT-only,
// Texture-only, both-RT-and-Texture and MIPmapped
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GPUMemorySize, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrResourceProvider* provider = context->resourceProvider();

    static const int kSize = 64;

    sk_sp<GrTexture> tex;

    // Normal versions
    tex = make_normal_texture(provider, kRenderTarget_GrSurfaceFlag, kSize, kSize, 0);
    size_t size = tex->gpuMemorySize();
    REPORTER_ASSERT(reporter, kSize*kSize*4 == size);

    if (context->caps()->maxSampleCount() >= 4) {
        tex = make_normal_texture(provider, kRenderTarget_GrSurfaceFlag, kSize, kSize, 4);
        size = tex->gpuMemorySize();
        REPORTER_ASSERT(reporter, kSize*kSize*4 == size ||    // msaa4 failed
                                  kSize*kSize*4*4 == size ||  // auto-resolving
                                  kSize*kSize*4*5 == size);   // explicit resolve buffer
    }

    tex = make_normal_texture(provider, kNone_GrSurfaceFlags, kSize, kSize, 0);
    size = tex->gpuMemorySize();
    REPORTER_ASSERT(reporter, kSize*kSize*4 == size);

    // Mipmapped versions
    tex = make_mipmap_texture(provider, kRenderTarget_GrSurfaceFlag, kSize, kSize, 0);
    size = tex->gpuMemorySize();
    REPORTER_ASSERT(reporter, kSize*kSize*4+(kSize*kSize*4)/3 == size);

    if (context->caps()->maxSampleCount() >= 4) {
        tex = make_mipmap_texture(provider, kRenderTarget_GrSurfaceFlag, kSize, kSize, 4);
        size = tex->gpuMemorySize();
        REPORTER_ASSERT(reporter, 
                            kSize*kSize*4+(kSize*kSize*4)/3 == size ||   // msaa4 failed
                            kSize*kSize*4*4+(kSize*kSize*4)/3 == size || // auto-resolving
                            kSize*kSize*4*5+(kSize*kSize*4)/3 == size);  // explicit resolve buffer
    }

    tex = make_mipmap_texture(provider, kNone_GrSurfaceFlags, kSize, kSize, 0);
    size = tex->gpuMemorySize();
    REPORTER_ASSERT(reporter, kSize*kSize*4+(kSize*kSize*4)/3 == size);
}

#endif
