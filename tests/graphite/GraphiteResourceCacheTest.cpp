/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/image/SkImage_Base.h"
#include "tools/Resources.h"
#include "tools/graphite/GraphiteTestContext.h"

namespace skgpu::graphite {

class TestResource : public Resource {
public:
    static sk_sp<TestResource> Make(const SharedContext* sharedContext,
                                    ResourceCache* resourceCache,
                                    Ownership owned,
                                    Budgeted budgeted,
                                    Shareable shareable,
                                    size_t gpuMemorySize = 1) {
        auto resource = sk_sp<TestResource>(new TestResource(sharedContext,
                                                             owned,
                                                             gpuMemorySize));
        if (!resource) {
            return nullptr;
        }

        GraphiteResourceKey key;
        CreateKey(&key);

        resourceCache->insertResource(resource.get(), key, budgeted, shareable);
        return resource;
    }

    const char* getResourceType() const override { return "Test Resource"; }

    static void CreateKey(GraphiteResourceKey* key) {
        // All unit tests that currently use TestResource are able to work with a single Resource,
        // so the key doesn't require any real state.
        static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
        GraphiteResourceKey::Builder(key, kType, 0);
    }

private:
    TestResource(const SharedContext* sharedContext,
                 Ownership owned,
                 size_t gpuMemorySize)
            : Resource(sharedContext, owned, gpuMemorySize) {}

    void freeGpuData() override {}
};

static sk_sp<SkData> create_image_data(const SkImageInfo& info) {
    const size_t rowBytes = info.minRowBytes();
    sk_sp<SkData> data(SkData::MakeUninitialized(rowBytes * info.height()));
    {
        SkBitmap bm;
        bm.installPixels(info, data->writable_data(), rowBytes);
        SkCanvas canvas(bm);
        canvas.clear(SK_ColorRED);
    }
    return data;
}

static skgpu::graphite::TextureProxy* top_device_graphite_target_proxy(SkCanvas* canvas) {
    if (auto gpuDevice = SkCanvasPriv::TopDevice(canvas)->asGraphiteDevice()) {
        return gpuDevice->target();
    }
    return nullptr;
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteBudgetedResourcesTest,
                                               reporter,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);

    // Test making a non budgeted, non shareable resource.
    sk_sp<Resource> resource = TestResource::Make(
            sharedContext, resourceCache, Ownership::kOwned, Budgeted::kNo, Shareable::kNo);
    if (!resource) {
        ERRORF(reporter, "Failed to make TestResource");
        return;
    }
    Resource* resourcePtr = resource.get();

    REPORTER_ASSERT(reporter, resource->budgeted() == Budgeted::kNo);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    // Resource is not shareable and we have a ref on it. Thus it shouldn't be findable in the cache
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);

    // When we reset our TestResource it should go back into the cache since it can be used as a
    // scratch resource (since it is not shareable). At that point the budget should be changed to
    // Budgeted::kYes.
    resource.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    // Even though we reset our ref on the resource we still have the ptr to it and should be the
    // resource in the cache. So in general this is dangerous it should be safe for this test to
    // directly access the resource.
    REPORTER_ASSERT(reporter, resourcePtr->budgeted() == Budgeted::kYes);

    // Test that the scratch resource can fulfill a new non-budgeted, non-shareable request
    GraphiteResourceKey key;
    TestResource::CreateKey(&key);
    Resource* resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kNo, Shareable::kNo);
    REPORTER_ASSERT(reporter, resourcePtr == resourcePtr2);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);
    REPORTER_ASSERT(reporter, resourcePtr2->budgeted() == Budgeted::kNo);
    resourcePtr2->unref();
    resourceCache->forceProcessReturnedResources();

    // Test making a budgeted, shareable resource. Since we returned all refs to the prior non
    // shareable resource, it should be able to be switched to a shareable resource.
    resource = sk_sp(resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kYes));
    REPORTER_ASSERT(reporter, resource.get() == resourcePtr);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1); // still findable
    REPORTER_ASSERT(reporter, resource->budgeted() == Budgeted::kYes);
    REPORTER_ASSERT(reporter, resource->shareable() == Shareable::kYes);

    // While the shareable resource is held, make a second shareable request which should still
    // find the existing resource in the cache.
    resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kYes);
    REPORTER_ASSERT(reporter, resourcePtr2 == resource.get());
    resourcePtr2->unref();

    // Now make a non-shareable request with the same key. This should fail to find a valid resource
    // since the one in the cache still has outstanding usage refs requiring it to be shareable.
    resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kNo);
    REPORTER_ASSERT(reporter, !resourcePtr2);

    // Return the shareable resource and then re-request the non-shareable key. Without any more
    // usage refs, the shareable resource can be restricted back to non-shareable usage.
    resource.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);

    resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kNo);
    REPORTER_ASSERT(reporter, resourcePtr2 == resourcePtr);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0); // not findable again
    REPORTER_ASSERT(reporter, resourcePtr2->budgeted() == Budgeted::kYes);
    REPORTER_ASSERT(reporter, resourcePtr2->shareable() == Shareable::kNo);
    resourcePtr2->unref();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Test that SkImage's and SkSurface's underlying Resource's follow the expected budgeted
    // system.
    auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // First test SkImages. Since we can't directly create a Graphite SkImage we first have to make
    // a raster SkImage than convert that to a Graphite SkImage via makeTextureImage.
    sk_sp<SkData> data(create_image_data(info));
    sk_sp<SkImage> image = SkImages::RasterFromData(info, std::move(data), info.minRowBytes());
    REPORTER_ASSERT(reporter, image);

    sk_sp<SkImage> imageGpu = SkImages::TextureFromImage(recorder.get(), image, {});
    REPORTER_ASSERT(reporter, imageGpu);

    TextureProxy* imageProxy = nullptr;
    {
        // We don't want the view holding a ref to the Proxy or else we can't send things back to
        // the cache.
        auto view = skgpu::graphite::AsView(imageGpu.get());
        REPORTER_ASSERT(reporter, view);
        imageProxy = view.proxy();
    }
    // Make sure the proxy is instantiated
    if (!imageProxy->instantiate(resourceProvider)) {
        ERRORF(reporter, "Failed to instantiate Proxy");
        return;
    }
    const Resource* imageResourcePtr = imageProxy->texture();
    REPORTER_ASSERT(reporter, imageResourcePtr);
    // There is an extra resource for the buffer that is uploading the data to the texture
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 3);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    REPORTER_ASSERT(reporter, imageResourcePtr->budgeted() == Budgeted::kNo);

    // Submit all upload work so we can drop refs to the image and get it returned to the cache.
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }
    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);
    recording.reset();
    imageGpu.reset();
    resourceCache->forceProcessReturnedResources();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 3);
    // Remapping async buffers before returning them to the cache can extend buffer lifetime.
    if (!context->priv().caps()->bufferMapsAreAsync()) {
        REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 3);
    }
    REPORTER_ASSERT(reporter, imageResourcePtr->budgeted() == Budgeted::kYes);

    // Now try an SkSurface. This is simpler since we can directly create Graphite SkSurface's.
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    if (!surface) {
        ERRORF(reporter, "Failed to make surface");
        return;
    }

    TextureProxy* surfaceProxy = top_device_graphite_target_proxy(surface->getCanvas());
    if (!surfaceProxy) {
        ERRORF(reporter, "Failed to get surface proxy");
        return;
    }

    // Make sure the proxy is instantiated
    if (!surfaceProxy->instantiate(resourceProvider)) {
        ERRORF(reporter, "Failed to instantiate surface proxy");
        return;
    }
    const Resource* surfaceResourcePtr = surfaceProxy->texture();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 4);
    // Remapping async buffers before returning them to the cache can extend buffer lifetime.
    if (!context->priv().caps()->bufferMapsAreAsync()) {
        REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 3);
    }
    REPORTER_ASSERT(reporter, surfaceResourcePtr->budgeted() == Budgeted::kNo);

    // The creation of the surface may have added an initial clear to it. Thus if we just reset the
    // surface it will flush the clean on the device and we don't be dropping all our refs to the
    // surface. So we force all the work to happen first.
    recording = recorder->snap();
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);
    recording.reset();

    surface.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, surfaceResourcePtr->budgeted() == Budgeted::kYes);
}

namespace {
sk_sp<Resource> add_new_resource(skiatest::Reporter* reporter,
                                 const SharedContext* sharedContext,
                                 ResourceCache* resourceCache,
                                 size_t gpuMemorySize,
                                 Budgeted budgeted = Budgeted::kYes) {
    auto resource = TestResource::Make(sharedContext,
                              resourceCache,
                              Ownership::kOwned,
                              budgeted,
                              Shareable::kNo,
                              gpuMemorySize);
    if (!resource) {
        ERRORF(reporter, "Failed to make TestResource");
        return nullptr;
    }
    return resource;
}

Resource* add_new_purgeable_resource(skiatest::Reporter* reporter,
                                     const SharedContext* sharedContext,
                                     ResourceCache* resourceCache,
                                     size_t gpuMemorySize) {
    auto resource = add_new_resource(reporter, sharedContext, resourceCache, gpuMemorySize);
    if (!resource) {
        return nullptr;
    }

    Resource* ptr = resource.get();
    resource.reset();
    resourceCache->forceProcessReturnedResources();
    return ptr;
}
} // namespace

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphitePurgeAsNeededResourcesTest, reporter, context,
                                   CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    resourceCache->setMaxBudget(10);

    auto resourceSize10 = add_new_resource(reporter,
                                           sharedContext,
                                           resourceCache,
                                           /*gpuMemorySize=*/10);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 10);

    auto resourceSize1 = add_new_resource(reporter,
                                          sharedContext,
                                          resourceCache,
                                          /*gpuMemorySize=*/1);

    // We should now be over budget, but nothing should be purged since neither resource is
    // purgeable.
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 11);

    // Dropping the ref to the size 1 resource should cause it to get purged when we add a new
    // resource to the cache.
    resourceSize1.reset();

    auto resourceSize2 = add_new_resource(reporter,
                                          sharedContext,
                                          resourceCache,
                                          /*gpuMemorySize=*/2);

    // The purging should have happened when we return the resource above so we also shouldn't
    // see anything in the purgeable queue.
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 12);

    // Reset the cache back to no resources by setting budget to 0.
    resourceSize10.reset();
    resourceSize2.reset();
    resourceCache->forceProcessReturnedResources();
    resourceCache->setMaxBudget(0);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 0);

    // Add a bunch of purgeable resources that keeps us under budget. Nothing should ever get purged.
    resourceCache->setMaxBudget(10);
    auto resourceSize1Ptr = add_new_purgeable_resource(reporter,
                                                       sharedContext,
                                                       resourceCache,
                                                       /*gpuMemorySize=*/1);
    /*auto resourceSize2Ptr=*/ add_new_purgeable_resource(reporter,
                                                          sharedContext,
                                                          resourceCache,
                                                          /*gpuMemorySize=*/2);
    auto resourceSize3Ptr = add_new_purgeable_resource(reporter,
                                                       sharedContext,
                                                       resourceCache,
                                                       /*gpuMemorySize=*/3);
    auto resourceSize4Ptr = add_new_purgeable_resource(reporter,
                                                       sharedContext,
                                                       resourceCache,
                                                       /*gpuMemorySize=*/4);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 4);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == resourceSize1Ptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 10);

    // Now add some resources that should cause things to get purged.
    // Add a size 2 resource should purge the original size 1 and size 2
    add_new_purgeable_resource(reporter,
                               sharedContext,
                               resourceCache,
                               /*gpuMemorySize=*/2);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 3);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == resourceSize3Ptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 9);

    // Adding a non-purgeable resource should also trigger resources to be purged from purgeable
    // queue.
    resourceSize10 = add_new_resource(reporter,
                                      sharedContext,
                                      resourceCache,
                                      /*gpuMemorySize=*/10);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 10);

    // Adding a resources that is purgeable back to the cache shouldn't trigger the previous
    // non-purgeable resource or itself to be purged yet (since processing our return mailbox
    // doesn't trigger the purgeAsNeeded call)
    resourceSize4Ptr = add_new_purgeable_resource(reporter,
                                                  sharedContext,
                                                  resourceCache,
                                                  /*gpuMemorySize=*/4);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == resourceSize4Ptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 14);

    // Resetting the budget to 0 should trigger purging the size 4 purgeable resource but should
    // leave the non purgeable size 10 alone.
    resourceCache->setMaxBudget(0);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 10);

    resourceSize10.reset();
    resourceCache->forceProcessReturnedResources();
    resourceCache->forcePurgeAsNeeded();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteZeroSizedResourcesTest, reporter, context,
                                   CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    // First make a normal resource that has a non zero size
    Resource* resourcePtr = add_new_purgeable_resource(reporter,
                                                       sharedContext,
                                                       resourceCache,
                                                       /*gpuMemorySize=*/1);
    if (!resourcePtr) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == resourcePtr);

    // First confirm if we set the max budget to zero, this sized resource is removed.
    resourceCache->setMaxBudget(0);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == nullptr);

    // Set the budget back to something higher
    resourceCache->setMaxBudget(100);

    // Now create a zero sized resource and add it to the cache.
    resourcePtr = add_new_purgeable_resource(reporter,
                                             sharedContext,
                                             resourceCache,
                                             /*gpuMemorySize=*/0);
    if (!resourcePtr) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == resourcePtr);

    // Setting the budget down to 0 should not cause the zero sized resource to be purged
    resourceCache->setMaxBudget(0);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == resourcePtr);

    // Now add a sized resource to cache. Set budget higher again so that it fits
    resourceCache->setMaxBudget(100);

    Resource* sizedResourcePtr = add_new_purgeable_resource(reporter,
                                                            sharedContext,
                                                            resourceCache,
                                                            /*gpuMemorySize=*/1);
    if (!resourcePtr) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 2);
    // Even though the zero sized resource was added to the cache first, the top of the purgeable
    // stack should be the sized resource.
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == sizedResourcePtr);

    // Add another zero sized resource
    resourcePtr = add_new_purgeable_resource(reporter,
                                             sharedContext,
                                             resourceCache,
                                             /*gpuMemorySize=*/0);
    if (!resourcePtr) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 3);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 3);
    // Again the sized resource should still be the top of the purgeable queue
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue() == sizedResourcePtr);

    // If we set the cache budget to 0, it should clear out the sized resource but leave the two
    // zero-sized resources.
    resourceCache->setMaxBudget(0);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 2);
    REPORTER_ASSERT(reporter, resourceCache->topOfPurgeableQueue()->gpuMemorySize() == 0);

    // However, purging all resources should clear the zero-sized resources.
    resourceCache->purgeResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);
}

// Depending on the granularity of the clock for a given device, in the
// GraphitePurgeNotUsedSinceResourcesTest we may end up with times that are all equal which messes
// up the expected behavior of the purge calls. So this helper forces us to return a new time that
// is different from a previous one.
skgpu::StdSteadyClock::time_point force_newer_timepoint(
        const skgpu::StdSteadyClock::time_point& prevTime) {
    auto time = skgpu::StdSteadyClock::now();
    while (time <= prevTime) {
        time = skgpu::StdSteadyClock::now();
    }
    return time;
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphitePurgeNotUsedSinceResourcesTest, reporter, context,
                                   CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    // Basic test where we purge 1 resource
    auto beforeTime = skgpu::StdSteadyClock::now();

    auto resourcePtr = add_new_purgeable_resource(reporter,
                                                  sharedContext,
                                                  resourceCache,
                                                  /*gpuMemorySize=*/1);
    if (!resourcePtr) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);

    auto afterTime = force_newer_timepoint(skgpu::StdSteadyClock::now());

    // purging beforeTime should not get rid of the resource
    resourceCache->purgeResourcesNotUsedSince(beforeTime);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);

    // purging at afterTime which is after resource became purgeable should purge it.
    resourceCache->purgeResourcesNotUsedSince(afterTime);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);

    // Test making 2 purgeable resources, but asking to purge on a time between the two.
    Resource* resourcePtr1 = add_new_purgeable_resource(reporter,
                                                       sharedContext,
                                                       resourceCache,
                                                       /*gpuMemorySize=*/1);

    auto betweenTime = force_newer_timepoint(skgpu::StdSteadyClock::now());

    Resource* resourcePtr2 = add_new_purgeable_resource(reporter,
                                                        sharedContext,
                                                        resourceCache,
                                                        /*gpuMemorySize=*/1);

    afterTime = force_newer_timepoint(skgpu::StdSteadyClock::now());

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr1));
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr2));

    resourceCache->purgeResourcesNotUsedSince(betweenTime);

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr2));

    resourceCache->purgeResourcesNotUsedSince(afterTime);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);

    // purgeResourcesNotUsedSince should have no impact on non-purgeable resources
    auto resource = add_new_resource(reporter,
                                     sharedContext,
                                     resourceCache,
                                     /*gpuMemorySize=*/1);
    if (!resource) {
        return;
    }
    resourcePtr = resource.get();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);

    afterTime = force_newer_timepoint(skgpu::StdSteadyClock::now());
    resourceCache->purgeResourcesNotUsedSince(afterTime);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, !resourceCache->testingInPurgeableQueue(resourcePtr));

    resource.reset();
    // purgeResourcesNotUsedSince should check the mailbox for the returned resource. Though the
    // time is set before that happens so nothing should purge.
    resourceCache->purgeResourcesNotUsedSince(skgpu::StdSteadyClock::now());
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr));

    // Now it should be purged since it is already purgeable
    resourceCache->purgeResourcesNotUsedSince(force_newer_timepoint(skgpu::StdSteadyClock::now()));
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
}

// This test is used to check the case where we call purgeNotUsedSince, which triggers us to return
// resources from mailbox. Even though the returned resources aren't purged by the last used, we
// still end up purging things to get under budget.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphitePurgeNotUsedOverBudgetTest, reporter, context,
                                   CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    // set resourceCache budget to 10 for testing.
    resourceCache->setMaxBudget(10);

    // First make a purgeable resources
    auto resourcePtr = add_new_purgeable_resource(reporter,
                                                  sharedContext,
                                                  resourceCache,
                                                  /*gpuMemorySize=*/1);
    if (!resourcePtr) {
        return;
    }

    // Now create a bunch of non purgeable (yet) resources that are not budgeted (i.e. in real world
    // they would be wrapped in an SkSurface or SkImage), but will cause us to go over our budget
    // limit when they do return to cache.

    auto resource1 = add_new_resource(reporter,
                                      sharedContext,
                                      resourceCache,
                                      /*gpuMemorySize=*/15,
                                      Budgeted::kNo);

    auto resource2 = add_new_resource(reporter,
                                      sharedContext,
                                      resourceCache,
                                      /*gpuMemorySize=*/16,
                                      Budgeted::kNo);

    auto resource3 = add_new_resource(reporter,
                                      sharedContext,
                                      resourceCache,
                                      /*gpuMemorySize=*/3,
                                      Budgeted::kNo);

    auto resource1Ptr = resource1.get();
    auto resource2Ptr = resource2.get();
    auto resource3Ptr = resource3.get();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 4);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 1);

    auto timeBeforeReturningToCache = skgpu::StdSteadyClock::now();

    // Now reset all the non budgeted resources so they return to the cache and become budgeted.
    // Returning to the cache will not immedidately trigger a purgeAsNeededCall.
    resource1.reset();
    resource2.reset();
    resource3.reset();

    resourceCache->forceProcessReturnedResources();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 4);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 35);
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr));
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resource1Ptr));
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resource2Ptr));
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resource3Ptr));

    // Now we call purgeNotUsedSince with timeBeforeReturnToCache. The original resource should get
    // purged because it is older than this time. The three originally non budgeted resources are
    // newer than this time so they won't be purged by the time on this call. However, since we are
    // overbudget it should trigger us to purge the first two of these resources to get us back
    // under.
    resourceCache->purgeResourcesNotUsedSince(timeBeforeReturningToCache);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->currentBudgetedBytes() == 3);
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resource3Ptr));
}

// Test call purgeResources on the ResourceCache and make sure all unlocked resources are getting
// purged regardless of when they were last used.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphitePurgeResourcesTest, reporter, context,
                                   CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    // set resourceCache budget to 10 for testing.
    resourceCache->setMaxBudget(10);

    // Basic test where we purge 1 resource
    auto resourcePtr = add_new_purgeable_resource(reporter,
                                                  sharedContext,
                                                  resourceCache,
                                                  /*gpuMemorySize=*/1);
    if (!resourcePtr) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);

    // purging should purge the one unlocked resource.
    resourceCache->purgeResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);

    // Test making 2 purgeable resources
    Resource* resourcePtr1 = add_new_purgeable_resource(reporter,
                                                        sharedContext,
                                                        resourceCache,
                                                        /*gpuMemorySize=*/1);

    Resource* resourcePtr2 = add_new_purgeable_resource(reporter,
                                                        sharedContext,
                                                        resourceCache,
                                                        /*gpuMemorySize=*/1);
    if (!resourcePtr1 || !resourcePtr2) {
        return;
    }

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr1));
    REPORTER_ASSERT(reporter, resourceCache->testingInPurgeableQueue(resourcePtr2));

    resourceCache->purgeResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);

    // purgeResources should have no impact on non-purgeable resources
    auto resource = add_new_resource(reporter,
                                     sharedContext,
                                     resourceCache,
                                     /*gpuMemorySize=*/1);
    if (!resource) {
        return;
    }
    resourcePtr = resource.get();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);

    resourceCache->purgeResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, !resourceCache->testingInPurgeableQueue(resourcePtr));

    resource.reset();
    resourceCache->purgeResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteScratchResourcesTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();


    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);

    // Test making a non budgeted, non shareable resource.
    sk_sp<Resource> resource = TestResource::Make(
            sharedContext, resourceCache, Ownership::kOwned, Budgeted::kNo, Shareable::kNo);
    if (!resource) {
        ERRORF(reporter, "Failed to make TestResource");
        return;
    }
    Resource* resourcePtr = resource.get();

    REPORTER_ASSERT(reporter, resource->budgeted() == Budgeted::kNo);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    // Resource is not shareable and we have a ref on it. Thus it shouldn't be findable in the cache
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);

    // Requesting a scratch shareable resouce will not return the non-shareable resource.
    GraphiteResourceKey key;
    TestResource::CreateKey(&key);

    ResourceCache::ScratchResourceSet unavailable;

    REPORTER_ASSERT(reporter, key == resource->key());
    Resource* resourcePtr2 = resourceCache->findAndRefResource(
            key, Budgeted::kYes, Shareable::kScratch, &unavailable);
    REPORTER_ASSERT(reporter, !resourcePtr2);

    // Return the non-shareable resource and verify that it can now be requested as scratch
    resource.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);

    resource = sk_sp(resourceCache->findAndRefResource(
            key, Budgeted::kYes, Shareable::kScratch, &unavailable));
    REPORTER_ASSERT(reporter, resource.get() == resourcePtr);
    REPORTER_ASSERT(reporter, resource->budgeted() == Budgeted::kYes);
    REPORTER_ASSERT(reporter, resource->shareable() == Shareable::kScratch);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1); // still findable

    // A request of the same key as non-shareable will not return the scratch resource
    resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kNo);
    REPORTER_ASSERT(reporter, !resourcePtr2);

    // Similarly, a request for a fully shareable resource cannot be satisfied by a scratch resource
    resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kYes);
    REPORTER_ASSERT(reporter, !resourcePtr2);

    // A request for another scratch resource can return the existing one if it hasn't been marked
    // unavailable in the set passed to the cache.
    resourcePtr2 = resourceCache->findAndRefResource(
            key, Budgeted::kYes, Shareable::kScratch, &unavailable);
    REPORTER_ASSERT(reporter, resourcePtr2 == resourcePtr);
    resourcePtr2->unref();

    // Mark the original resource as unvailable and now it shouldn't be seen by the request.
    unavailable.add(resourcePtr);
    resourcePtr2 = resourceCache->findAndRefResource(
            key, Budgeted::kYes, Shareable::kScratch, &unavailable);
    REPORTER_ASSERT(reporter, !resourcePtr2);

    // Return the scratch resource, and then simulate a threading race where there's a request for
    // the scratch resource that comes in before the return queue is processed (adding a usage ref),
    // and then the queue is processed as part of a non-shareable request (which should then fail).
    unavailable.reset();
    resource.reset();
    resource = sk_sp(resourceCache->findAndRefResource(
            key, Budgeted::kYes, Shareable::kScratch, &unavailable));
    REPORTER_ASSERT(reporter, resource.get() == resourcePtr);
    // At this point, resourcePtr has a usage ref and should be in the return queue
    REPORTER_ASSERT(reporter, resourceCache->testingInReturnQueue(resourcePtr));
    resourceCache->forceProcessReturnedResources();
    // Its shareable type should not have changed after being processed.
    REPORTER_ASSERT(reporter, !resourceCache->testingInReturnQueue(resourcePtr));
    REPORTER_ASSERT(reporter, resource->shareable() == Shareable::kScratch);

    // Now actually return the resource and confirm that it can be used for non-shareable requests
    // once all usage refs are dropped.
    resource.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    REPORTER_ASSERT(reporter, resourcePtr->shareable() == Shareable::kNo);

    // Returning the scratch resource allows it to be changed to a different shareable type
    resourcePtr2 = resourceCache->findAndRefResource(key, Budgeted::kYes, Shareable::kYes);
    REPORTER_ASSERT(reporter, resourcePtr2 == resourcePtr);
    REPORTER_ASSERT(reporter, resourcePtr2->shareable() == Shareable::kYes);
    resourcePtr2->unref();
}

}  // namespace skgpu::graphite
