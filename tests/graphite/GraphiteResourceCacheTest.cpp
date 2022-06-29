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
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/SkStuff.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/image/SkImage_Base.h"

namespace skgpu::graphite {

class TestResource : public Resource {
public:
    static sk_sp<TestResource> Make(const Gpu* gpu,
                                    Ownership owned,
                                    SkBudgeted budgeted,
                                    Shareable shareable) {
        auto resource = sk_sp<TestResource>(new TestResource(gpu, owned, budgeted));
        if (!resource) {
            return nullptr;
        }

        GraphiteResourceKey key;
        CreateKey(&key, shareable);

        resource->setKey(key);
        return resource;
    }

    static void CreateKey(GraphiteResourceKey* key, Shareable shareable) {
        // Internally we assert that we don't make the same key twice where the only difference is
        // shareable vs non-shareable. That allows us to now have Shareable be part of the Key's
        // key. So here we make two different resource types so the keys will be different.
        static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
        static const ResourceType kShareableType = GraphiteResourceKey::GenerateResourceType();
        ResourceType type = shareable == Shareable::kNo ? kType : kShareableType;
        GraphiteResourceKey::Builder(key, type, 0, shareable);
    }

private:
    TestResource(const Gpu* gpu, Ownership owned, SkBudgeted budgeted)
            : Resource(gpu, owned, budgeted) {}

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

DEF_GRAPHITE_TEST_FOR_CONTEXTS(GraphiteBudgetedResourcesTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const Gpu* gpu = resourceProvider->gpu();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);

    // Test making a non budgeted, non shareable resource.
    auto resource = TestResource::Make(gpu, Ownership::kOwned, SkBudgeted::kNo, Shareable::kNo);
    if (!resource) {
        ERRORF(reporter, "Failed to make TestResource");
        return;
    }
    Resource* resourcePtr = resource.get();

    REPORTER_ASSERT(reporter, resource->budgeted() == SkBudgeted::kNo);
    resourceCache->insertResource(resourcePtr);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    // Resource is not shareable and we have a ref on it. Thus it shouldn't ben findable in the
    // cache.
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);

    // When we reset our TestResource it should go back into the cache since it can be used as a
    // scratch texture (since it is not shareable). At that point the budget should be changed to
    // SkBudgeted::kYes.
    resource.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 1);
    // Even though we reset our ref on the resource we still have the ptr to it and should be the
    // resource in the cache. So in general this is dangerous it should be safe for this test to
    // directly access the texture.
    REPORTER_ASSERT(reporter, resourcePtr->budgeted() == SkBudgeted::kYes);

    GraphiteResourceKey key;
    TestResource::CreateKey(&key, Shareable::kNo);
    Resource* resourcePtr2 = resourceCache->findAndRefResource(key, SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, resourcePtr == resourcePtr2);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 1);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 0);
    REPORTER_ASSERT(reporter, resourcePtr2->budgeted() == SkBudgeted::kNo);
    resourcePtr2->unref();
    resourceCache->forceProcessReturnedResources();

    // Test making a non budgeted, non shareable resource.
    resource = TestResource::Make(gpu, Ownership::kOwned, SkBudgeted::kYes, Shareable::kYes);
    if (!resource) {
        ERRORF(reporter, "Failed to make TestResource");
        return;
    }
    resourcePtr = resource.get();
    REPORTER_ASSERT(reporter, resource->budgeted() == SkBudgeted::kYes);
    resourceCache->insertResource(resourcePtr);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 2);

    resource.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 2);
    REPORTER_ASSERT(reporter, resourcePtr->budgeted() == SkBudgeted::kYes);

    TestResource::CreateKey(&key, Shareable::kYes);
    resourcePtr2 = resourceCache->findAndRefResource(key, SkBudgeted::kYes);
    REPORTER_ASSERT(reporter, resourcePtr == resourcePtr2);
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 2);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 2);
    REPORTER_ASSERT(reporter, resourcePtr2->budgeted() == SkBudgeted::kYes);
    resourcePtr2->unref();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Test that SkImage's and SkSurface's underlying Resource's follow the expected budgeted
    // system.
    auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // First test SkImages. Since we can't directly create a Graphite SkImage we first have to make
    // a raster SkImage than convert that to a Graphite SkImage via makeTextureImage.
    sk_sp<SkData> data(create_image_data(info));
    sk_sp<SkImage> image = SkImage::MakeRasterData(info, std::move(data), info.minRowBytes());
    REPORTER_ASSERT(reporter, image);

    sk_sp<SkImage> imageGpu = image->makeTextureImage(recorder.get());
    REPORTER_ASSERT(reporter, imageGpu);

    TextureProxy* imageProxy = nullptr;
    {
        // We don't want the view holding a ref to the Proxy or else we can't send things back to
        // the cache.
        auto [view, colorType] = as_IB(imageGpu.get())->asView(recorder.get(), Mipmapped::kNo);
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
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 4);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 2);
    REPORTER_ASSERT(reporter, imageResourcePtr->budgeted() == SkBudgeted::kNo);

    // Submit all upload work so we can drop refs to the image and get it returned to the cache.
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }
    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    context->submit(SyncToCpu::kYes);
    recording.reset();
    imageGpu.reset();
    resourceCache->forceProcessReturnedResources();

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 4);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 4);
    REPORTER_ASSERT(reporter, imageResourcePtr->budgeted() == SkBudgeted::kYes);

    // Now try an SkSurface. This is simpler since we can directly create Graphite SkSurface's.
    sk_sp<SkSurface> surface = MakeGraphite(recorder.get(), info);
    if (!surface) {
        ERRORF(reporter, "Failed to make surface");
        return;
    }

    TextureProxy* surfaceProxy = SkCanvasPriv::TopDeviceGraphiteTargetProxy(surface->getCanvas());
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

    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 5);
    REPORTER_ASSERT(reporter, resourceCache->numFindableResources() == 4);
    REPORTER_ASSERT(reporter, surfaceResourcePtr->budgeted() == SkBudgeted::kNo);

    surface.reset();
    resourceCache->forceProcessReturnedResources();
    REPORTER_ASSERT(reporter, surfaceResourcePtr->budgeted() == SkBudgeted::kYes);
}

}  // namespace skgpu::graphite
