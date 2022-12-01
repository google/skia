/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteTextureProxyTest, reporter, context) {
    const Caps* caps = context->priv().caps();
    constexpr SkISize kValidSize = SkISize::Make(1, 1);
    constexpr SkISize kInvalidSize = SkISize::MakeEmpty();
    constexpr SkColorType kValidColorType = kRGBA_8888_SkColorType;
    constexpr SkColorType kInvalidColorType = kUnknown_SkColorType;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    const TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(
            kValidColorType, Mipmapped::kNo, Protected::kNo, Renderable::kNo);
    const BackendTexture backendTexture = recorder->createBackendTexture(kValidSize, textureInfo);
    sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);

    auto nullCallback = [](ResourceProvider*) -> sk_sp<Texture> { return nullptr; };
    auto callback = [texture](ResourceProvider*) -> sk_sp<Texture> { return texture; };

    // Assign to assignableTexture before instantiating with this callback.
    sk_sp<Texture> assignableTexture;
    auto assignableCallback = [&assignableTexture](ResourceProvider*) -> sk_sp<Texture> {
        return assignableTexture;
    };

    // Invalid parameters.
    sk_sp<TextureProxy> textureProxy;
    textureProxy = TextureProxy::Make(caps,
                                      kInvalidSize,
                                      kValidColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, textureProxy == nullptr);
    textureProxy = TextureProxy::Make(caps,
                                      kValidSize,
                                      kInvalidColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, textureProxy == nullptr);

    // Non-lazy TextureProxy, successful instantiation.
    textureProxy = TextureProxy::Make(caps,
                                      kValidSize,
                                      kValidColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, !textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isVolatile());
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == kValidSize);

    bool instantiateSuccess = textureProxy->instantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == kValidSize);
    const Texture* createdTexture = textureProxy->texture();

    instantiateSuccess = textureProxy->instantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == createdTexture);

    // Lazy, non-volatile TextureProxy, unsuccessful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            kValidSize, textureInfo, SkBudgeted::kNo, Volatile::kNo, nullCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isVolatile());

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, !instantiateSuccess);
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Lazy, non-volatile TextureProxy, successful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            kValidSize, textureInfo, SkBudgeted::kNo, Volatile::kNo, callback);

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == texture.get());

    // Lazy, volatile TextureProxy, unsuccessful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            kValidSize, textureInfo, SkBudgeted::kNo, Volatile::kYes, nullCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, textureProxy->isVolatile());

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, !instantiateSuccess);
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Lazy, volatile TextureProxy, successful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            kValidSize, textureInfo, SkBudgeted::kNo, Volatile::kYes, callback);

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == texture.get());

    textureProxy->deinstantiate();
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Fully-lazy TextureProxy.
    textureProxy = TextureProxy::MakeFullyLazy(
            textureInfo, SkBudgeted::kNo, Volatile::kYes, assignableCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, textureProxy->isVolatile());

    assignableTexture = texture;
    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == kValidSize);

    textureProxy->deinstantiate();
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->isFullyLazy());

    constexpr SkISize kLargerSize = SkISize::Make(2, 2);
    const BackendTexture largerBackendTexture =
            recorder->createBackendTexture(kLargerSize, textureInfo);
    assignableTexture = resourceProvider->createWrappedTexture(largerBackendTexture);
    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == kLargerSize);

    // InstantiateIfNotLazy tests.
    textureProxy = TextureProxy::Make(caps,
                                      kValidSize,
                                      kValidColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    instantiateSuccess = TextureProxy::InstantiateIfNotLazy(resourceProvider, textureProxy.get());
    REPORTER_ASSERT(reporter, instantiateSuccess);

    textureProxy = TextureProxy::MakeLazy(
            kValidSize, textureInfo, SkBudgeted::kNo, Volatile::kNo, nullCallback);
    instantiateSuccess = TextureProxy::InstantiateIfNotLazy(resourceProvider, textureProxy.get());
    REPORTER_ASSERT(reporter, instantiateSuccess);
}

}  // namespace skgpu::graphite
