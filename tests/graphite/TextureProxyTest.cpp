/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteTextureProxyTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    const Caps* caps = context->priv().caps();
    constexpr SkISize kValidSize = SkISize::Make(1, 1);
    constexpr SkISize kInvalidSize = SkISize::MakeEmpty();
    constexpr SkColorType kValidColorType = kRGBA_8888_SkColorType;
    constexpr SkColorType kInvalidColorType = kUnknown_SkColorType;

    Protected isProtected = Protected(caps->protectedSupport());

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    const TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(
            kValidColorType, Mipmapped::kNo, isProtected, Renderable::kNo);
    BackendTexture backendTexture = recorder->createBackendTexture(kValidSize, textureInfo);
    sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);

    auto makeProxy = [&](SkISize dimensions, SkColorType colorType, Mipmapped mipmapped,
                         Protected isProtected, Renderable renderable, Budgeted budgeted) {
        auto textureInfo = caps->getDefaultSampledTextureInfo(colorType, mipmapped,
                                                              isProtected, renderable);
        return TextureProxy::Make(caps, recorder->priv().resourceProvider(),
                                  dimensions, textureInfo, budgeted);
    };

    auto nullCallback = [](ResourceProvider*) -> sk_sp<Texture> { return nullptr; };
    auto callback = [texture](ResourceProvider*) -> sk_sp<Texture> { return texture; };

    // Assign to assignableTexture before instantiating with this callback.
    sk_sp<Texture> assignableTexture;
    auto assignableCallback = [&assignableTexture](ResourceProvider*) -> sk_sp<Texture> {
        return assignableTexture;
    };

    // Invalid parameters.
    sk_sp<TextureProxy> textureProxy;
    textureProxy = makeProxy(kInvalidSize,
                             kValidColorType,
                             Mipmapped::kNo,
                             isProtected,
                             Renderable::kNo,
                             skgpu::Budgeted::kNo);
    REPORTER_ASSERT(reporter, textureProxy == nullptr);
    textureProxy = makeProxy(kValidSize,
                             kInvalidColorType,
                             Mipmapped::kNo,
                             isProtected,
                             Renderable::kNo,
                             skgpu::Budgeted::kNo);
    REPORTER_ASSERT(reporter, textureProxy == nullptr);

    // Non-budgeted, non-lazy TextureProxy is instantiated on return
    textureProxy = makeProxy(kValidSize,
                             kValidColorType,
                             Mipmapped::kNo,
                             isProtected,
                             Renderable::kNo,
                             skgpu::Budgeted::kNo);
    REPORTER_ASSERT(reporter, !textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isVolatile());
    REPORTER_ASSERT(reporter, textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == kValidSize);

    // Budgeted, non-lazy TextureProxy, successful instantiation later on
    textureProxy = makeProxy(kValidSize,
                             kValidColorType,
                             Mipmapped::kNo,
                             isProtected,
                             Renderable::kNo,
                             skgpu::Budgeted::kYes);
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
            caps, kValidSize, textureInfo, skgpu::Budgeted::kNo, Volatile::kNo, nullCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isVolatile());

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, !instantiateSuccess);
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Lazy, non-volatile TextureProxy, successful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            caps, kValidSize, textureInfo, skgpu::Budgeted::kNo, Volatile::kNo, callback);

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == texture.get());

    // Lazy, volatile TextureProxy, unsuccessful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            caps, kValidSize, textureInfo, skgpu::Budgeted::kNo, Volatile::kYes, nullCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isFullyLazy());
    REPORTER_ASSERT(reporter, textureProxy->isVolatile());

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, !instantiateSuccess);
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Lazy, volatile TextureProxy, successful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            caps, kValidSize, textureInfo, skgpu::Budgeted::kNo, Volatile::kYes, callback);

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == texture.get());

    textureProxy->deinstantiate();
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Fully-lazy TextureProxy.
    textureProxy = TextureProxy::MakeFullyLazy(
            textureInfo, skgpu::Budgeted::kNo, Volatile::kYes, assignableCallback);
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
    BackendTexture largerBackendTexture =
            recorder->createBackendTexture(kLargerSize, textureInfo);
    assignableTexture = resourceProvider->createWrappedTexture(largerBackendTexture);
    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == kLargerSize);

    // InstantiateIfNotLazy tests.
    textureProxy = makeProxy(kValidSize,
                             kValidColorType,
                             Mipmapped::kNo,
                             isProtected,
                             Renderable::kNo,
                             skgpu::Budgeted::kYes);
    instantiateSuccess = TextureProxy::InstantiateIfNotLazy(resourceProvider, textureProxy.get());
    REPORTER_ASSERT(reporter, instantiateSuccess);

    textureProxy = TextureProxy::MakeLazy(
            caps, kValidSize, textureInfo, skgpu::Budgeted::kNo, Volatile::kNo, nullCallback);
    instantiateSuccess = TextureProxy::InstantiateIfNotLazy(resourceProvider, textureProxy.get());
    REPORTER_ASSERT(reporter, instantiateSuccess);
    // Clean up the backend textures.
    recorder->deleteBackendTexture(backendTexture);
    recorder->deleteBackendTexture(largerBackendTexture);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteTextureTooLargeTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const Caps* caps = context->priv().caps();

    // Try to create a texture that is too large for the backend.
    SkBitmap bitmap;
    SkISize dimensions = SkISize::Make(caps->maxTextureSize() + 1, 1);
    bitmap.allocPixels(SkImageInfo::Make(
            dimensions, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType));
    sk_sp<SkImage> rasterImage = SkImages::RasterFromBitmap(bitmap);
    sk_sp<SkImage> graphiteImage =
            SkImages::TextureFromImage(recorder.get(), rasterImage.get(), /*requiredProps=*/{});

    // Image creation should have failed.
    REPORTER_ASSERT(reporter, !graphiteImage);

    // Snapping should still succeed, no texture upload should actually be attempted.
    REPORTER_ASSERT(reporter, recorder->snap());
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteLazyTextureInvalidDimensions, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    class FulfillContext {
    public:
        FulfillContext(BackendTexture backendTexture) : fBackendTexture(backendTexture) {}

        static std::tuple<BackendTexture, void*> Fulfill(void* ctx) {
            FulfillContext* self = reinterpret_cast<FulfillContext*>(ctx);
            return {self->fBackendTexture, nullptr};
        }

        BackendTexture fBackendTexture;
    };

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const Caps* caps = context->priv().caps();

    // Try to create textures with invalid dimensions.
    SkISize largeDimensions = SkISize::Make(caps->maxTextureSize() + 1, 1);
    SkISize negativeDimensions = SkISize::Make(-1, -1);

    for (const SkISize& dimensions : {largeDimensions, negativeDimensions}) {
        SkImageInfo imageInfo = SkImageInfo::Make(
                dimensions, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
        TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(
                imageInfo.colorInfo().colorType(), Mipmapped::kNo, Protected::kNo, Renderable::kNo);

        // The created BackendTexture should be invalid, so an invalid texture would be used to
        // fulfill the promise image created later, if we were to attempt to draw it.
        BackendTexture backendTexture =
                recorder->createBackendTexture(imageInfo.dimensions(), textureInfo);
        FulfillContext fulfillContext(backendTexture);
        REPORTER_ASSERT(reporter, !backendTexture.isValid());

        // Drawing should still succeed, as no image draw should actually be attempted with this
        // texture.
        SkImageInfo surfaceImageInfo = SkImageInfo::Make(
                1, 1, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), surfaceImageInfo);
        sk_sp<SkImage> promiseImage = SkImages::PromiseTextureFrom(recorder.get(),
                                                                   imageInfo.dimensions(),
                                                                   textureInfo,
                                                                   imageInfo.colorInfo(),
                                                                   Volatile::kNo,
                                                                   FulfillContext::Fulfill,
                                                                   nullptr,
                                                                   nullptr,
                                                                   &fulfillContext);

        surface->getCanvas()->drawImage(promiseImage, 0.0f, 0.0f);
        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, context->insertRecording({recording.get()}));
        // Clean up backend texture
        context->deleteBackendTexture(fulfillContext.fBackendTexture);
    }
}

}  // namespace skgpu::graphite
