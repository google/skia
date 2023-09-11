/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/TestFormatColorTypeCombination.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

// Tests that GrSurface::asTexture(), GrSurface::asRenderTarget(), and static upcasting of texture
// and render targets to GrSurface all work as expected.
DEF_GANESH_TEST_FOR_MOCK_CONTEXT(GrSurface, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    auto resourceProvider = context->priv().resourceProvider();

    static constexpr SkISize kDesc = {256, 256};
    auto format = context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                  GrRenderable::kYes);
    sk_sp<GrSurface> texRT1 = resourceProvider->createTexture(kDesc,
                                                              format,
                                                              GrTextureType::k2D,
                                                              GrRenderable::kYes,
                                                              1,
                                                              skgpu::Mipmapped::kNo,
                                                              skgpu::Budgeted::kNo,
                                                              GrProtected::kNo,
                                                              /*label=*/{});

    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    texRT1->asTexture());
    REPORTER_ASSERT(reporter, texRT1->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT1->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT1->asTexture()));

    sk_sp<GrTexture> tex1 = resourceProvider->createTexture(kDesc,
                                                            format,
                                                            GrTextureType::k2D,
                                                            GrRenderable::kNo,
                                                            1,
                                                            skgpu::Mipmapped::kNo,
                                                            skgpu::Budgeted::kNo,
                                                            GrProtected::kNo,
                                                            /*label=*/{});
    REPORTER_ASSERT(reporter, nullptr == tex1->asRenderTarget());
    REPORTER_ASSERT(reporter, tex1.get() == tex1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(tex1.get()) == tex1->asTexture());

    GrBackendTexture backendTex = context->createBackendTexture(256,
                                                                256,
                                                                kRGBA_8888_SkColorType,
                                                                SkColors::kTransparent,
                                                                skgpu::Mipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                GrProtected::kNo);

    sk_sp<GrSurface> texRT2 = resourceProvider->wrapRenderableBackendTexture(
            backendTex, 1, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo);

    REPORTER_ASSERT(reporter, texRT2.get() == texRT2->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT2.get() == texRT2->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT2->asRenderTarget()) ==
                    texRT2->asTexture());
    REPORTER_ASSERT(reporter, texRT2->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT2->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT2->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT2->asTexture()));

    context->deleteBackendTexture(backendTex);
}

// This test checks that the isFormatTexturable and isFormatRenderable are
// consistent with createTexture's result.
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(GrSurfaceRenderability,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    const GrCaps* caps = context->priv().caps();

    // TODO: Should only need format here but need to determine compression type from format
    // without config.
    auto createTexture = [](SkISize dimensions, GrColorType colorType,
                            const GrBackendFormat& format, GrRenderable renderable,
                            GrResourceProvider* rp) -> sk_sp<GrTexture> {
        SkTextureCompressionType compression = GrBackendFormatToCompressionType(format);
        if (compression != SkTextureCompressionType::kNone) {
            if (renderable == GrRenderable::kYes) {
                return nullptr;
            }
            auto size = SkCompressedDataSize(compression, dimensions, nullptr, false);
            auto data = SkData::MakeUninitialized(size);
            SkColor4f color = {0, 0, 0, 0};
            GrFillInCompressedData(compression,
                                   dimensions,
                                   skgpu::Mipmapped::kNo,
                                   (char*)data->writable_data(),
                                   color);
            return rp->createCompressedTexture(dimensions,
                                               format,
                                               skgpu::Budgeted::kNo,
                                               skgpu::Mipmapped::kNo,
                                               GrProtected::kNo,
                                               data.get(),
                                               /*label=*/{});
        } else {
            return rp->createTexture(dimensions,
                                     format,
                                     GrTextureType::k2D,
                                     renderable,
                                     1,
                                     skgpu::Mipmapped::kNo,
                                     skgpu::Budgeted::kNo,
                                     GrProtected::kNo,
                                     /*label=*/{});
        }
    };

    static constexpr SkISize kDims = {64, 64};

    const std::vector<GrTest::TestFormatColorTypeCombination>& combos =
            caps->getTestingCombinations();

    for (const GrTest::TestFormatColorTypeCombination& combo : combos) {

        SkASSERT(combo.fColorType != GrColorType::kUnknown);
        SkASSERT(combo.fFormat.isValid());

        // Right now Vulkan has two backend formats that support ABGR_4444 (R4G4B4A4 and B4G4R4A4).
        // Until we can create textures directly from the backend format this yields some
        // ambiguity in what is actually supported and which textures can be created.
        if (ctxInfo.backend() == kVulkan_GrBackend && combo.fColorType == GrColorType::kABGR_4444) {
            continue;
        }

        // Check if 'isFormatTexturable' agrees with 'createTexture' and that the mipmap
        // support check is working
        {
            bool isCompressed = caps->isFormatCompressed(combo.fFormat);
            bool isTexturable = caps->isFormatTexturable(combo.fFormat, GrTextureType::k2D);

            sk_sp<GrSurface> tex = createTexture(kDims, combo.fColorType, combo.fFormat,
                                                 GrRenderable::kNo, resourceProvider);
            REPORTER_ASSERT(reporter, SkToBool(tex) == isTexturable,
                            "ct:%s format:%s, tex:%d, isTexturable:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(tex), isTexturable);

            // Check that the lack of mipmap support blocks the creation of mipmapped
            // proxies
            bool expectedMipMapability = isTexturable && caps->mipmapSupport() && !isCompressed;

            sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(combo.fFormat,
                                                                     kDims,
                                                                     GrRenderable::kNo,
                                                                     1,
                                                                     skgpu::Mipmapped::kYes,
                                                                     SkBackingFit::kExact,
                                                                     skgpu::Budgeted::kNo,
                                                                     GrProtected::kNo,
                                                                     /*label=*/{});
            REPORTER_ASSERT(reporter, SkToBool(proxy.get()) == expectedMipMapability,
                            "ct:%s format:%s, tex:%d, expectedMipMapability:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(proxy.get()), expectedMipMapability);
        }

        // Check if 'isFormatAsColorTypeRenderable' agrees with 'createTexture' (w/o MSAA)
        {
            bool isRenderable = caps->isFormatRenderable(combo.fFormat, 1);

            sk_sp<GrSurface> tex = resourceProvider->createTexture(kDims,
                                                                   combo.fFormat,
                                                                   GrTextureType::k2D,
                                                                   GrRenderable::kYes,
                                                                   1,
                                                                   skgpu::Mipmapped::kNo,
                                                                   skgpu::Budgeted::kNo,
                                                                   GrProtected::kNo,
                                                                   /*label=*/{});
            REPORTER_ASSERT(reporter, SkToBool(tex) == isRenderable,
                            "ct:%s format:%s, tex:%d, isRenderable:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(tex), isRenderable);
        }

        // Check if 'isFormatAsColorTypeRenderable' agrees with 'createTexture' w/ MSAA
        {
            bool isRenderable = caps->isFormatRenderable(combo.fFormat, 2);

            sk_sp<GrSurface> tex = resourceProvider->createTexture(kDims,
                                                                   combo.fFormat,
                                                                   GrTextureType::k2D,
                                                                   GrRenderable::kYes,
                                                                   2,
                                                                   skgpu::Mipmapped::kNo,
                                                                   skgpu::Budgeted::kNo,
                                                                   GrProtected::kNo,
                                                                   /*label=*/{});
            REPORTER_ASSERT(reporter, SkToBool(tex) == isRenderable,
                            "ct:%s format:%s, tex:%d, isRenderable:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(tex), isRenderable);
        }
    }
}

// For each context, set it to always clear the textures and then run through all the
// supported formats checking that the textures are actually cleared
DEF_GANESH_TEST(InitialTextureClear, reporter, baseOptions, CtsEnforcement::kApiLevel_T) {
    GrContextOptions options = baseOptions;
    options.fClearAllTextures = true;

    static constexpr int kSize = 100;
    static constexpr SkColor kClearColor = 0xABABABAB;

    const SkImageInfo imageInfo = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                    kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(imageInfo);

    SkISize desc;
    desc.fWidth = desc.fHeight = kSize;

    for (int ct = 0; ct < skgpu::kContextTypeCount; ++ct) {
        sk_gpu_test::GrContextFactory factory(options);
        auto contextType = static_cast<skgpu::ContextType>(ct);
        if (!skgpu::IsRenderingContext(contextType)) {
            continue;
        }
        auto dContext = factory.get(contextType);
        if (!dContext) {
            continue;
        }

        GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
        const GrCaps* caps = dContext->priv().caps();

        const std::vector<GrTest::TestFormatColorTypeCombination>& combos =
                caps->getTestingCombinations();

        for (const GrTest::TestFormatColorTypeCombination& combo : combos) {

            SkASSERT(combo.fColorType != GrColorType::kUnknown);
            SkASSERT(combo.fFormat.isValid());

            if (!caps->isFormatTexturable(combo.fFormat, GrTextureType::k2D)) {
                continue;
            }

            auto checkColor = [reporter](const GrTest::TestFormatColorTypeCombination& combo,
                                         uint32_t readColor) {
                // We expect that if there is no alpha in the src color type and we read it to a
                // color type with alpha that we will get one for alpha rather than zero. We used to
                // require this but the Intel Iris 6100 on Win 10 test bot doesn't put one in the
                // alpha channel when reading back from GL_RG16 or GL_RG16F. So now we allow either.
                uint32_t channels = GrColorTypeChannelFlags(combo.fColorType);
                bool allowAlphaOne = !(channels & kAlpha_SkColorChannelFlag);
                if (allowAlphaOne) {
                    if (readColor != 0x00000000 && readColor != 0xFF000000) {
                        ERRORF(reporter,
                               "Failed on ct %s format %s 0x%08x is not 0x00000000 or 0xFF000000",
                               GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                               readColor);
                        return false;
                    }
                } else {
                    if (readColor) {
                        ERRORF(reporter, "Failed on ct %s format %s 0x%08x != 0x00000000",
                               GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                               readColor);
                        return false;
                    }
                }
                return true;
            };

            for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
                if (renderable == GrRenderable::kYes &&
                    !caps->isFormatAsColorTypeRenderable(combo.fColorType, combo.fFormat)) {
                    continue;
                }

                for (auto fit : {SkBackingFit::kApprox, SkBackingFit::kExact}) {

                    // Does directly allocating a texture clear it?
                    {
                        auto proxy = proxyProvider->testingOnly_createInstantiatedProxy(
                                {kSize, kSize},
                                combo.fFormat,
                                renderable,
                                1,
                                fit,
                                skgpu::Budgeted::kYes,
                                GrProtected::kNo);
                        if (proxy) {
                            skgpu::Swizzle swizzle = caps->getReadSwizzle(combo.fFormat,
                                                                          combo.fColorType);
                            GrSurfaceProxyView view(std::move(proxy), kTopLeft_GrSurfaceOrigin,
                                                    swizzle);
                            GrColorInfo info(combo.fColorType, kPremul_SkAlphaType, nullptr);
                            auto texCtx = dContext->priv().makeSC(std::move(view), info);

                            readback.erase(kClearColor);
                            if (texCtx->readPixels(dContext, readback, {0, 0})) {
                                for (int i = 0; i < kSize * kSize; ++i) {
                                    if (!checkColor(combo, readback.addr32()[i])) {
                                        break;
                                    }
                                }
                            }
                        }

                        dContext->priv().getResourceCache()->purgeUnlockedResources(
                                GrPurgeResourceOptions::kAllResources);
                    }

                    // Try creating the texture as a deferred proxy.
                    {
                        GrImageInfo info(combo.fColorType,
                                         GrColorTypeHasAlpha(combo.fColorType)
                                                                            ? kPremul_SkAlphaType
                                                                            : kOpaque_SkAlphaType,
                                         nullptr,
                                         {desc.fHeight, desc.fHeight});

                        auto sc = dContext->priv().makeSC(info,
                                                          combo.fFormat,
                                                          /*label=*/{},
                                                          fit,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          renderable);
                        if (!sc) {
                            continue;
                        }

                        readback.erase(kClearColor);
                        if (sc->readPixels(dContext, readback, {0, 0})) {
                            for (int i = 0; i < kSize * kSize; ++i) {
                                if (!checkColor(combo, readback.addr32()[i])) {
                                    break;
                                }
                            }
                        }
                        dContext->priv().getResourceCache()->purgeUnlockedResources(
                                GrPurgeResourceOptions::kAllResources);
                    }
                }
            }
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ReadOnlyTexture,
                                       reporter,
                                       context_info,
                                       CtsEnforcement::kApiLevel_T) {
    auto fillPixels = [](SkPixmap* p, const std::function<uint32_t(int x, int y)>& f) {
        for (int y = 0; y < p->height(); ++y) {
            for (int x = 0; x < p->width(); ++x) {
                *p->writable_addr32(x, y) = f(x, y);
            }
        }
    };

    auto comparePixels = [](const SkPixmap& p1, const SkPixmap& p2, skiatest::Reporter* reporter) {
        SkASSERT(p1.info() == p2.info());
        for (int y = 0; y < p1.height(); ++y) {
            for (int x = 0; x < p1.width(); ++x) {
                REPORTER_ASSERT(reporter, p1.getColor(x, y) == p2.getColor(x, y));
                if (p1.getColor(x, y) != p2.getColor(x, y)) {
                    return;
                }
            }
        }
    };

    static constexpr int kSize = 100;
    SkImageInfo ii = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkAutoPixmapStorage srcPixmap;
    srcPixmap.alloc(ii);
    fillPixels(&srcPixmap,
               [](int x, int y) {
                    return (0xFFU << 24) | (x << 16) | (y << 8) | uint8_t((x * y) & 0xFF);
               });

    auto dContext = context_info.directContext();
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    // We test both kRW in addition to kRead mostly to ensure that the calls are structured such
    // that they'd succeed if the texture wasn't kRead. We want to be sure we're failing with
    // kRead for the right reason.
    for (auto ioType : {kRead_GrIOType, kRW_GrIOType}) {
        auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithData(dContext,
                                                                     srcPixmap,
                                                                     kTopLeft_GrSurfaceOrigin,
                                                                     GrRenderable::kNo,
                                                                     GrProtected::kNo);
        if (!mbet) {
            ERRORF(reporter, "Could not make texture.");
            return;
        }
        auto proxy = proxyProvider->wrapBackendTexture(mbet->texture(), kBorrow_GrWrapOwnership,
                                                       GrWrapCacheable::kNo, ioType,
                                                       mbet->refCountedCallback());
        skgpu::Swizzle swizzle = dContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                         GrColorType::kRGBA_8888);
        GrSurfaceProxyView view(proxy, kTopLeft_GrSurfaceOrigin, swizzle);
        auto surfContext = dContext->priv().makeSC(std::move(view), ii.colorInfo());
        // Read pixels should work with a read-only texture.
        {
            SkAutoPixmapStorage read;
            read.alloc(srcPixmap.info());
            auto readResult = surfContext->readPixels(dContext, read, {0, 0});
            REPORTER_ASSERT(reporter, readResult);
            if (readResult) {
                comparePixels(srcPixmap, read, reporter);
            }
        }

        // Write pixels should not work with a read-only texture.
        SkAutoPixmapStorage write;
        write.alloc(srcPixmap.info());
        fillPixels(&write, [&srcPixmap](int x, int y) { return ~*srcPixmap.addr32(); });
        auto writeResult = surfContext->writePixels(dContext, write, {0, 0});
        REPORTER_ASSERT(reporter, writeResult == (ioType == kRW_GrIOType));
        // Try the low level write.
        dContext->flushAndSubmit();
        auto gpuWriteResult = dContext->priv().getGpu()->writePixels(
                proxy->peekTexture(),
                SkIRect::MakeWH(kSize, kSize),
                GrColorType::kRGBA_8888,
                GrColorType::kRGBA_8888,
                write.addr32(),
                kSize*GrColorTypeBytesPerPixel(GrColorType::kRGBA_8888));
        REPORTER_ASSERT(reporter, gpuWriteResult == (ioType == kRW_GrIOType));

        SkBitmap copySrcBitmap;
        copySrcBitmap.installPixels(write);
        copySrcBitmap.setImmutable();

        auto copySrc = std::get<0>(GrMakeUncachedBitmapProxyView(dContext, copySrcBitmap));

        REPORTER_ASSERT(reporter, copySrc);
        auto copyResult = surfContext->testCopy(copySrc.refProxy());
        REPORTER_ASSERT(reporter, copyResult == (ioType == kRW_GrIOType));
        // Try the low level copy.
        dContext->flushAndSubmit();
        auto gpuCopyResult = dContext->priv().getGpu()->copySurface(
                proxy->peekSurface(),
                SkIRect::MakeWH(kSize, kSize),
                copySrc.proxy()->peekSurface(),
                SkIRect::MakeWH(kSize, kSize),
                GrSamplerState::Filter::kNearest);
        REPORTER_ASSERT(reporter, gpuCopyResult == (ioType == kRW_GrIOType));

        // Mip regen should not work with a read only texture.
        if (dContext->priv().caps()->mipmapSupport()) {
            mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(dContext,
                                                                       kSize,
                                                                       kSize,
                                                                       kRGBA_8888_SkColorType,
                                                                       skgpu::Mipmapped::kYes,
                                                                       GrRenderable::kNo,
                                                                       GrProtected::kNo);
            proxy = proxyProvider->wrapBackendTexture(mbet->texture(), kBorrow_GrWrapOwnership,
                                                      GrWrapCacheable::kNo, ioType,
                                                      mbet->refCountedCallback());
            dContext->flushAndSubmit();
            proxy->peekTexture()->markMipmapsDirty();  // avoids assert in GrGpu.
            auto regenResult =
                    dContext->priv().getGpu()->regenerateMipMapLevels(proxy->peekTexture());
            REPORTER_ASSERT(reporter, regenResult == (ioType == kRW_GrIOType));
        }
    }
}
