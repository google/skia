/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ganesh/ProxyUtils.h"

#include <thread>

namespace {

skgpu::UniqueKey create_key(int keyValue) {
    static skgpu::UniqueKey::Domain d = skgpu::UniqueKey::GenerateDomain();

    skgpu::UniqueKey key;

    skgpu::UniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = keyValue;
    builder.finish();

    return key;
}

sk_sp<SkImage> create_keyed_image(GrRecordingContext* rContext, int keyValue) {
    SkImageInfo ii = SkImageInfo::MakeN32Premul(16, 16);
    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kYes, ii);

    surf->getCanvas()->clear(SK_ColorWHITE);
    surf->getCanvas()->drawCircle(8, 8, 8, SkPaint());

    sk_sp<SkImage> img = surf->makeImageSnapshot();

    GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(img.get(),
                                                              surf->recordingContext());

    skgpu::UniqueKey key = create_key(keyValue);

    rContext->priv().proxyProvider()->assignUniqueKeyToProxy(key, proxy);

    return img;
}

void ddl_deleter(skiatest::Reporter* reporter,
                 sk_sp<GrUniquelyKeyedProxyRegistry> registry,
                 sk_sp<GrDeferredDisplayList> ddl,
                 int keyValue) {
    const auto kSleepDuration = std::chrono::milliseconds(1);

    std::this_thread::sleep_for(kSleepDuration);

    skgpu::UniqueKey key = create_key(keyValue);

    REPORTER_ASSERT(reporter, registry->find(key));
    ddl.reset();
    REPORTER_ASSERT(reporter, !registry->find(key));
}

} // anonymous namespace

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrUniqueProxyTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();

    GrSurfaceCharacterization characterization;

    // Grab a characterization
    {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(16, 16);
        sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, ii);
        SkAssertResult(surf);

        SkAssertResult(surf->characterize(&characterization));
    }

    // Create a recorder
    std::unique_ptr<GrDeferredDisplayListRecorder> recorder;
    recorder = std::make_unique<GrDeferredDisplayListRecorder>(characterization);

    GrProxyProvider* proxyProvider = recorder->getCanvas()->recordingContext()->priv().proxyProvider();
    sk_sp<GrUniquelyKeyedProxyRegistry> registry = proxyProvider->uniquelyKeyedProxyRegistry();

    std::vector<std::thread> threads;

    // Create DDLs and immediately hand them off to a thread for deletion. Each DDL will have
    // a single uniquely keyed proxy.
    for (int i = 0; i < 10; i++) {
        SkCanvas* canvas = recorder->getCanvas();
        canvas->clear(SkColors::kRed);

        sk_sp<SkImage> img = create_keyed_image(canvas->recordingContext(), i);

        canvas->drawImage(img.get(), 0, 0);

        sk_sp<GrDeferredDisplayList> ddl = recorder->detach();

        threads.push_back(std::thread(ddl_deleter, reporter, registry, std::move(ddl), i));
    }

    // Delete the recorder - hopefully before all the DDLs are deleted
    recorder.reset();

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // All the texture proxies should be deleted and have deregistered their keys
    REPORTER_ASSERT(reporter, registry->count() == 0);
    REPORTER_ASSERT(reporter, registry->unique());
}
