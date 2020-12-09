/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <set>
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/BackendTextureImageFactory.h"
#include "tools/gpu/ManagedBackendTexture.h"

// Tests that GrSurface::asTexture(), GrSurface::asRenderTarget(), and static upcasting of texture
// and render targets to GrSurface all work as expected.
DEF_GPUTEST_FOR_MOCK_CONTEXT(GrSurface, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    auto resourceProvider = context->priv().resourceProvider();

    static constexpr SkISize kDesc = {256, 256};
    auto format = context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                  GrRenderable::kYes);
    sk_sp<GrSurface> texRT1 =
            resourceProvider->createTexture(kDesc, format, GrRenderable::kYes, 1, GrMipmapped::kNo,
                                            SkBudgeted::kNo, GrProtected::kNo);

    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    texRT1->asTexture());
    REPORTER_ASSERT(reporter, texRT1->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT1->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT1->asTexture()));

    sk_sp<GrTexture> tex1 =
            resourceProvider->createTexture(kDesc, format, GrRenderable::kNo, 1, GrMipmapped::kNo,
                                            SkBudgeted::kNo, GrProtected::kNo);
    REPORTER_ASSERT(reporter, nullptr == tex1->asRenderTarget());
    REPORTER_ASSERT(reporter, tex1.get() == tex1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(tex1.get()) == tex1->asTexture());

    GrBackendTexture backendTex = context->createBackendTexture(
        256, 256, kRGBA_8888_SkColorType,
        SkColors::kTransparent, GrMipmapped::kNo, GrRenderable::kNo, GrProtected::kNo);

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
DEF_GPUTEST_FOR_ALL_CONTEXTS(GrSurfaceRenderability, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    const GrCaps* caps = context->priv().caps();

    // TODO: Should only need format here but need to determine compression type from format
    // without config.
    auto createTexture = [](SkISize dimensions, GrColorType colorType,
                            const GrBackendFormat& format, GrRenderable renderable,
                            GrResourceProvider* rp) -> sk_sp<GrTexture> {
        SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);
        if (compression != SkImage::CompressionType::kNone) {
            if (renderable == GrRenderable::kYes) {
                return nullptr;
            }
            auto size = SkCompressedDataSize(compression, dimensions, nullptr, false);
            auto data = SkData::MakeUninitialized(size);
            SkColor4f color = {0, 0, 0, 0};
            GrFillInCompressedData(compression, dimensions, GrMipmapped::kNo,
                                   (char*)data->writable_data(), color);
            return rp->createCompressedTexture(dimensions, format, SkBudgeted::kNo,
                                               GrMipmapped::kNo, GrProtected::kNo, data.get());
        } else {
            return rp->createTexture(dimensions, format, renderable, 1, GrMipmapped::kNo,
                                     SkBudgeted::kNo, GrProtected::kNo);
        }
    };

    static constexpr SkISize kDims = {64, 64};

    const std::vector<GrCaps::TestFormatColorTypeCombination>& combos =
            caps->getTestingCombinations();

    for (const GrCaps::TestFormatColorTypeCombination& combo : combos) {

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
            bool isTexturable = caps->isFormatTexturable(combo.fFormat);

            sk_sp<GrSurface> tex = createTexture(kDims, combo.fColorType, combo.fFormat,
                                                 GrRenderable::kNo, resourceProvider);
            REPORTER_ASSERT(reporter, SkToBool(tex) == isTexturable,
                            "ct:%s format:%s, tex:%d, isTexturable:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(tex), isTexturable);

            // Check that the lack of mipmap support blocks the creation of mipmapped
            // proxies
            bool expectedMipMapability = isTexturable && caps->mipmapSupport() && !isCompressed;

            sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                    combo.fFormat, kDims, GrRenderable::kNo, 1, GrMipmapped::kYes,
                    SkBackingFit::kExact, SkBudgeted::kNo, GrProtected::kNo);
            REPORTER_ASSERT(reporter, SkToBool(proxy.get()) == expectedMipMapability,
                            "ct:%s format:%s, tex:%d, expectedMipMapability:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(proxy.get()), expectedMipMapability);
        }

        // Check if 'isFormatAsColorTypeRenderable' agrees with 'createTexture' (w/o MSAA)
        {
            bool isRenderable = caps->isFormatRenderable(combo.fFormat, 1);

            sk_sp<GrSurface> tex = resourceProvider->createTexture(
                    kDims, combo.fFormat, GrRenderable::kYes, 1, GrMipmapped::kNo, SkBudgeted::kNo,
                    GrProtected::kNo);
            REPORTER_ASSERT(reporter, SkToBool(tex) == isRenderable,
                            "ct:%s format:%s, tex:%d, isRenderable:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(tex), isRenderable);
        }

        // Check if 'isFormatAsColorTypeRenderable' agrees with 'createTexture' w/ MSAA
        {
            bool isRenderable = caps->isFormatRenderable(combo.fFormat, 2);

            sk_sp<GrSurface> tex = resourceProvider->createTexture(
                    kDims, combo.fFormat, GrRenderable::kYes, 2, GrMipmapped::kNo, SkBudgeted::kNo,
                    GrProtected::kNo);
            REPORTER_ASSERT(reporter, SkToBool(tex) == isRenderable,
                            "ct:%s format:%s, tex:%d, isRenderable:%d",
                            GrColorTypeToStr(combo.fColorType), combo.fFormat.toStr().c_str(),
                            SkToBool(tex), isRenderable);
        }
    }
}

#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrSurfaceProxy.h"

// For each context, set it to always clear the textures and then run through all the
// supported formats checking that the textures are actually cleared
DEF_GPUTEST(InitialTextureClear, reporter, baseOptions) {
    GrContextOptions options = baseOptions;
    options.fClearAllTextures = true;

    static constexpr int kSize = 100;
    static constexpr SkColor kClearColor = 0xABABABAB;

    const SkImageInfo info = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                               kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(info);

    SkISize desc;
    desc.fWidth = desc.fHeight = kSize;

    for (int ct = 0; ct < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++ct) {
        sk_gpu_test::GrContextFactory factory(options);
        auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(ct);
        if (!sk_gpu_test::GrContextFactory::IsRenderingContext(contextType)) {
            continue;
        }
        auto dContext = factory.get(contextType);
        if (!dContext) {
            continue;
        }

        GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
        const GrCaps* caps = dContext->priv().caps();

        const std::vector<GrCaps::TestFormatColorTypeCombination>& combos =
                caps->getTestingCombinations();

        for (const GrCaps::TestFormatColorTypeCombination& combo : combos) {

            SkASSERT(combo.fColorType != GrColorType::kUnknown);
            SkASSERT(combo.fFormat.isValid());

            if (!caps->isFormatTexturable(combo.fFormat)) {
                continue;
            }

            auto checkColor = [reporter](const GrCaps::TestFormatColorTypeCombination& combo,
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
                                {kSize, kSize}, combo.fFormat, renderable, 1, fit, SkBudgeted::kYes,
                                GrProtected::kNo);
                        if (proxy) {
                            GrSwizzle swizzle = caps->getReadSwizzle(combo.fFormat,
                                                                     combo.fColorType);
                            GrSurfaceProxyView view(std::move(proxy), kTopLeft_GrSurfaceOrigin,
                                                    swizzle);
                            GrColorInfo info(combo.fColorType, kPremul_SkAlphaType, nullptr);
                            auto texCtx = GrSurfaceContext::Make(dContext, std::move(view), info);

                            readback.erase(kClearColor);
                            if (texCtx->readPixels(
                                    dContext, readback.info(), readback.writable_addr(),
                                    readback.rowBytes(), {0, 0})) {
                                for (int i = 0; i < kSize * kSize; ++i) {
                                    if (!checkColor(combo, readback.addr32()[i])) {
                                        break;
                                    }
                                }
                            }
                        }

                        dContext->priv().testingOnly_purgeAllUnlockedResources();
                    }

                    // Try creating the texture as a deferred proxy.
                    {
                        std::unique_ptr<GrSurfaceContext> surfCtx;
                        if (renderable == GrRenderable::kYes) {
                            surfCtx = GrSurfaceDrawContext::Make(
                                    dContext, combo.fColorType, nullptr, fit,
                                    {desc.fWidth, desc.fHeight}, 1, GrMipmapped::kNo,
                                    GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
                        } else {
                            GrImageInfo info(combo.fColorType,
                                             kUnknown_SkAlphaType,
                                             nullptr,
                                             {desc.fHeight, desc.fHeight});
                            surfCtx = GrSurfaceContext::Make(dContext, info, combo.fFormat, fit);
                        }
                        if (!surfCtx) {
                            continue;
                        }

                        readback.erase(kClearColor);
                        if (surfCtx->readPixels(dContext, readback.info(), readback.writable_addr(),
                                                readback.rowBytes(), {0, 0})) {
                            for (int i = 0; i < kSize * kSize; ++i) {
                                if (!checkColor(combo, readback.addr32()[i])) {
                                    break;
                                }
                            }
                        }
                        dContext->priv().testingOnly_purgeAllUnlockedResources();
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadOnlyTexture, reporter, context_info) {
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
        auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithData(
                dContext, srcPixmap, kTopLeft_GrSurfaceOrigin, GrRenderable::kNo, GrProtected::kNo);
        if (!mbet) {
            ERRORF(reporter, "Could not make texture.");
            return;
        }
        auto proxy = proxyProvider->wrapBackendTexture(mbet->texture(), kBorrow_GrWrapOwnership,
                                                       GrWrapCacheable::kNo, ioType,
                                                       mbet->refCountedCallback());
        GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                    GrColorType::kRGBA_8888);
        GrSurfaceProxyView view(proxy, kTopLeft_GrSurfaceOrigin, swizzle);
        auto surfContext = GrSurfaceContext::Make(dContext, std::move(view), ii.colorInfo());
        // Read pixels should work with a read-only texture.
        {
            SkAutoPixmapStorage read;
            read.alloc(srcPixmap.info());
            auto readResult = surfContext->readPixels(dContext, srcPixmap.info(),
                                                      read.writable_addr(), 0, { 0, 0 });
            REPORTER_ASSERT(reporter, readResult);
            if (readResult) {
                comparePixels(srcPixmap, read, reporter);
            }
        }

        // Write pixels should not work with a read-only texture.
        SkAutoPixmapStorage write;
        write.alloc(srcPixmap.info());
        fillPixels(&write, [&srcPixmap](int x, int y) { return ~*srcPixmap.addr32(); });
        auto writeResult = surfContext->writePixels(dContext, srcPixmap.info(), write.addr(),
                                                    0, {0, 0});
        REPORTER_ASSERT(reporter, writeResult == (ioType == kRW_GrIOType));
        // Try the low level write.
        dContext->flushAndSubmit();
        auto gpuWriteResult = dContext->priv().getGpu()->writePixels(
                proxy->peekTexture(), 0, 0, kSize, kSize, GrColorType::kRGBA_8888,
                GrColorType::kRGBA_8888, write.addr32(),
                kSize * GrColorTypeBytesPerPixel(GrColorType::kRGBA_8888));
        REPORTER_ASSERT(reporter, gpuWriteResult == (ioType == kRW_GrIOType));

        SkBitmap copySrcBitmap;
        copySrcBitmap.installPixels(write);
        copySrcBitmap.setImmutable();

        GrBitmapTextureMaker maker(dContext, copySrcBitmap,
                                   GrImageTexGenPolicy::kNew_Uncached_Budgeted);
        auto copySrc = maker.view(GrMipmapped::kNo);

        REPORTER_ASSERT(reporter, copySrc.proxy());
        auto copyResult = surfContext->testCopy(copySrc.proxy());
        REPORTER_ASSERT(reporter, copyResult == (ioType == kRW_GrIOType));
        // Try the low level copy.
        dContext->flushAndSubmit();
        auto gpuCopyResult = dContext->priv().getGpu()->copySurface(
                proxy->peekSurface(), copySrc.proxy()->peekSurface(), SkIRect::MakeWH(kSize, kSize),
                {0, 0});
        REPORTER_ASSERT(reporter, gpuCopyResult == (ioType == kRW_GrIOType));

        // Mip regen should not work with a read only texture.
        if (dContext->priv().caps()->mipmapSupport()) {
            mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(dContext,
                                                                       kSize,
                                                                       kSize,
                                                                       kRGBA_8888_SkColorType,
                                                                       GrMipmapped::kYes,
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

static const int kSurfSize = 10;

static sk_sp<GrTexture> make_wrapped_texture(GrDirectContext* dContext, GrRenderable renderable) {
    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, kSurfSize, kSurfSize, kRGBA_8888_SkColorType, GrMipmapped::kNo, renderable);
    SkASSERT(mbet);
    sk_sp<GrTextureProxy> proxy;
    if (renderable == GrRenderable::kYes) {
        proxy = dContext->priv().proxyProvider()->wrapRenderableBackendTexture(
                mbet->texture(), 1, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                mbet->refCountedCallback());
    } else {
        proxy = dContext->priv().proxyProvider()->wrapBackendTexture(
                mbet->texture(), kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType,
                mbet->refCountedCallback());
    }
    if (!proxy) {
        return nullptr;
    }
    return sk_ref_sp(proxy->peekTexture());
}

static sk_sp<GrTexture> make_normal_texture(GrDirectContext* dContext, GrRenderable renderable) {
    SkISize desc;
    desc.fWidth = desc.fHeight = kSurfSize;
    auto format = dContext->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                   renderable);
    return dContext->priv().resourceProvider()->createTexture(
            desc, format, renderable, 1, GrMipmapped::kNo, SkBudgeted::kNo, GrProtected::kNo);
}

DEF_GPUTEST(TextureIdleProcTest, reporter, options) {
    // Various ways of making textures.
    auto makeWrapped = [](GrDirectContext* dContext) {
        return make_wrapped_texture(dContext, GrRenderable::kNo);
    };
    auto makeWrappedRenderable = [](GrDirectContext* dContext) {
        return make_wrapped_texture(dContext, GrRenderable::kYes);
    };
    auto makeNormal = [](GrDirectContext* dContext) {
        return make_normal_texture(dContext, GrRenderable::kNo);
    };
    auto makeRenderable = [](GrDirectContext* dContext) {
        return make_normal_texture(dContext, GrRenderable::kYes);
    };

    std::function<sk_sp<GrTexture>(GrDirectContext*)> makers[] = {
        makeWrapped,
        makeWrappedRenderable,
        makeNormal,
        makeRenderable
    };

    // Add a unique key, or not.
    auto addKey = [](GrTexture* texture) {
        static uint32_t gN = 0;
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 1);
        builder[0] = gN++;
        builder.finish();
        texture->resourcePriv().setUniqueKey(key);
    };
    auto dontAddKey = [](GrTexture* texture) {};
    std::function<void(GrTexture*)> keyAdders[] = {addKey, dontAddKey};

    for (const auto& m : makers) {
        for (const auto& keyAdder : keyAdders) {
            for (int type = 0; type < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++type) {
                sk_gpu_test::GrContextFactory factory;
                auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(type);
                auto dContext = factory.get(contextType);
                if (!dContext) {
                    continue;
                }

                // The callback we add simply adds an integer to a set.
                std::set<int> idleIDs;
                struct Context {
                    std::set<int>* fIdleIDs;
                    int fNum;
                };
                auto proc = [](void* context) {
                    static_cast<Context*>(context)->fIdleIDs->insert(
                            static_cast<Context*>(context)->fNum);
                    delete static_cast<Context*>(context);
                };

                // Makes a texture, possibly adds a key, and sets the callback.
                auto make = [&m, &keyAdder, &proc, &idleIDs](GrDirectContext* dContext, int num) {
                    sk_sp<GrTexture> texture = m(dContext);
                    texture->addIdleProc(proc, new Context{&idleIDs, num});
                    keyAdder(texture.get());
                    return texture;
                };

                auto texture = make(dContext, 1);
                REPORTER_ASSERT(reporter, idleIDs.find(1) == idleIDs.end());
                auto renderable = GrRenderable(SkToBool(texture->asRenderTarget()));
                auto backendFormat = texture->backendFormat();
                texture.reset();
                REPORTER_ASSERT(reporter, idleIDs.find(1) != idleIDs.end());

                texture = make(dContext, 2);
                int w = texture->width();
                int h = texture->height();
                SkImageInfo info =
                        SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                auto rt = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info, 0, nullptr);
                auto rtc = rt->getCanvas()->internal_private_accessTopLayerRenderTargetContext();
                auto singleUseLazyCB = [&texture](GrResourceProvider*,
                                                  const GrSurfaceProxy::LazySurfaceDesc&) {
                    auto mode = GrSurfaceProxy::LazyInstantiationKeyMode::kSynced;
                    if (texture->getUniqueKey().isValid()) {
                        mode = GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced;
                    }
                    return GrSurfaceProxy::LazyCallbackResult{std::move(texture), true, mode};
                };
                SkISize desc;
                desc.fWidth = w;
                desc.fHeight = h;
                SkBudgeted budgeted;
                if (texture->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted) {
                    budgeted = SkBudgeted::kYes;
                } else {
                    budgeted = SkBudgeted::kNo;
                }
                sk_sp<GrSurfaceProxy> proxy;
                if (renderable == GrRenderable::kYes) {
                    static const GrProxyProvider::TextureInfo kTexInfo = {GrMipMapped::kNo,
                                                                          GrTextureType::k2D};
                    proxy = dContext->priv().proxyProvider()->createLazyRenderTargetProxy(
                            singleUseLazyCB, backendFormat, desc, 1,
                            dContext->priv().caps()->getExtraSurfaceFlagsForDeferredRT(),
                            &kTexInfo,
                            GrMipmapStatus::kNotAllocated,
                            SkBackingFit::kExact, budgeted, GrProtected::kNo, false,
                            GrSurfaceProxy::UseAllocator::kYes);
                } else {
                    proxy = dContext->priv().proxyProvider()->createLazyProxy(
                            singleUseLazyCB, backendFormat, desc, GrMipmapped::kNo,
                            GrMipmapStatus::kNotAllocated, GrInternalSurfaceFlags ::kNone,
                            SkBackingFit::kExact, budgeted, GrProtected::kNo,
                            GrSurfaceProxy::UseAllocator::kYes);
                }
                GrSwizzle readSwizzle = dContext->priv().caps()->getReadSwizzle(
                        backendFormat, GrColorType::kRGBA_8888);
                GrSurfaceProxyView view(std::move(proxy), kTopLeft_GrSurfaceOrigin, readSwizzle);
                rtc->drawTexture(nullptr,
                                 view,
                                 kPremul_SkAlphaType,
                                 GrSamplerState::Filter::kNearest,
                                 GrSamplerState::MipmapMode::kNone,
                                 SkBlendMode::kSrcOver,
                                 SkPMColor4f(),
                                 SkRect::MakeWH(w, h),
                                 SkRect::MakeWH(w, h),
                                 GrAA::kNo,
                                 GrQuadAAFlags::kNone,
                                 SkCanvas::kFast_SrcRectConstraint,
                                 SkMatrix::I(),
                                 nullptr);
                // We still have the proxy, which should remain instantiated, thereby keeping the
                // texture not purgeable.
                REPORTER_ASSERT(reporter, idleIDs.find(2) == idleIDs.end());
                dContext->flushAndSubmit();
                REPORTER_ASSERT(reporter, idleIDs.find(2) == idleIDs.end());
                dContext->priv().getGpu()->testingOnly_flushGpuAndSync();
                REPORTER_ASSERT(reporter, idleIDs.find(2) == idleIDs.end());

                // This time we move the proxy into the draw.
                rtc->drawTexture(nullptr,
                                 std::move(view),
                                 kPremul_SkAlphaType,
                                 GrSamplerState::Filter::kNearest,
                                 GrSamplerState::MipmapMode::kNone,
                                 SkBlendMode::kSrcOver,
                                 SkPMColor4f(),
                                 SkRect::MakeWH(w, h),
                                 SkRect::MakeWH(w, h),
                                 GrAA::kNo,
                                 GrQuadAAFlags::kNone,
                                 SkCanvas::kFast_SrcRectConstraint,
                                 SkMatrix::I(),
                                 nullptr);
                REPORTER_ASSERT(reporter, idleIDs.find(2) == idleIDs.end());
                dContext->flushAndSubmit();
                dContext->priv().getGpu()->testingOnly_flushGpuAndSync();
                // Now that the draw is fully consumed by the GPU, the texture should be idle.
                REPORTER_ASSERT(reporter, idleIDs.find(2) != idleIDs.end());

                // Make sure we make the call during various shutdown scenarios where the texture
                // might persist after context is destroyed, abandoned, etc. We test three
                // variations of each scenario. One where the texture is just created. Another,
                // where the texture has been used in a draw and then the context is flushed. And
                // one where the the texture was drawn but the context is not flushed.
                // In each scenario we test holding a ref beyond the context shutdown and not.

                // These tests are difficult to get working with Vulkan, Direct3D, and Dawn.
                // See http://skbug.com/8705, http://skbug.com/8277, and http://skbug.com/10326
                GrBackendApi api = sk_gpu_test::GrContextFactory::ContextTypeBackend(contextType);
                if (api == GrBackendApi::kVulkan || api == GrBackendApi::kDirect3D ||
                    api == GrBackendApi::kDawn) {
                    continue;
                }
                int id = 3;
                enum class DrawType {
                    kNoDraw,
                    kDraw,
                    kDrawAndFlush,
                };
                for (auto drawType :
                     {DrawType::kNoDraw, DrawType::kDraw, DrawType::kDrawAndFlush}) {
                    for (bool unrefFirst : {false, true}) {
                        auto possiblyDrawAndFlush = [&dContext, &texture, drawType, unrefFirst, w,
                                                     h] {
                            if (drawType == DrawType::kNoDraw) {
                                return;
                            }
                            SkImageInfo info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType,
                                                                 kPremul_SkAlphaType);
                            auto rt = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info, 0,
                                                                  nullptr);
                            auto rtc = rt->getCanvas()
                                            ->internal_private_accessTopLayerRenderTargetContext();
                            auto proxy = dContext->priv().proxyProvider()->testingOnly_createWrapped(
                                    texture);
                            GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(
                                    proxy->backendFormat(), GrColorType::kRGBA_8888);
                            GrSurfaceProxyView view(std::move(proxy), kTopLeft_GrSurfaceOrigin,
                                                    swizzle);
                            rtc->drawTexture(nullptr,
                                             std::move(view),
                                             kPremul_SkAlphaType,
                                             GrSamplerState::Filter::kNearest,
                                             GrSamplerState::MipmapMode::kNone,
                                             SkBlendMode::kSrcOver,
                                             SkPMColor4f(),
                                             SkRect::MakeWH(w, h),
                                             SkRect::MakeWH(w, h),
                                             GrAA::kNo,
                                             GrQuadAAFlags::kNone,
                                             SkCanvas::kFast_SrcRectConstraint,
                                             SkMatrix::I(),
                                             nullptr);
                            if (drawType == DrawType::kDrawAndFlush) {
                                dContext->flushAndSubmit();
                            }
                            if (unrefFirst) {
                                texture.reset();
                            }
                        };
                        texture = make(dContext, id);
                        possiblyDrawAndFlush();
                        dContext->abandonContext();
                        texture.reset();
                        REPORTER_ASSERT(reporter, idleIDs.find(id) != idleIDs.end());
                        factory.destroyContexts();
                        dContext = factory.get(contextType);
                        ++id;

                        // Similar to previous, but reset the texture after the context was
                        // abandoned and then destroyed.
                        texture = make(dContext, id);
                        possiblyDrawAndFlush();
                        dContext->abandonContext();
                        factory.destroyContexts();
                        texture.reset();
                        REPORTER_ASSERT(reporter, idleIDs.find(id) != idleIDs.end());
                        dContext = factory.get(contextType);
                        id++;

                        texture = make(dContext, id);
                        possiblyDrawAndFlush();
                        factory.destroyContexts();
                        texture.reset();
                        REPORTER_ASSERT(reporter, idleIDs.find(id) != idleIDs.end());
                        dContext = factory.get(contextType);
                        id++;

                        texture = make(dContext, id);
                        possiblyDrawAndFlush();
                        factory.releaseResourcesAndAbandonContexts();
                        texture.reset();
                        REPORTER_ASSERT(reporter, idleIDs.find(id) != idleIDs.end());
                        dContext = factory.get(contextType);
                        id++;
                    }
                }
            }
        }
    }
}

// Tests an idle proc that unrefs another resource down to zero.
DEF_GPUTEST_FOR_ALL_CONTEXTS(TextureIdleProcCacheManipulationTest, reporter, contextInfo) {
    auto context = contextInfo.directContext();

    // idle proc that releases another texture.
    auto idleProc = [](void* texture) { reinterpret_cast<GrTexture*>(texture)->unref(); };

    for (const auto& idleMaker : {make_wrapped_texture, make_normal_texture}) {
        for (const auto& otherMaker : {make_wrapped_texture, make_normal_texture}) {
            auto idleTexture = idleMaker(context, GrRenderable::kNo);
            auto otherTexture = otherMaker(context, GrRenderable::kNo);
            otherTexture->ref();
            idleTexture->addIdleProc(idleProc, otherTexture.get());
            otherTexture.reset();
            idleTexture.reset();
        }
    }
}

// Similar to above but more complicated. This flushes the context from the idle proc.
// crbug.com/933526.
DEF_GPUTEST_FOR_ALL_CONTEXTS(TextureIdleProcFlushTest, reporter, contextInfo) {
    auto dContext = contextInfo.directContext();

    // idle proc that flushes the context.
    auto idleProc = [](void* context) {
        reinterpret_cast<GrDirectContext*>(context)->flushAndSubmit();
    };

    for (const auto& idleMaker : {make_wrapped_texture, make_normal_texture}) {
        auto idleTexture = idleMaker(dContext, GrRenderable::kNo);
        idleTexture->addIdleProc(idleProc, dContext);
        auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        auto surf = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info, 1, nullptr);
        // We'll draw two images to the canvas. One is a normal texture-backed image. The other
        // is a wrapped-texture backed image.
        surf->getCanvas()->clear(SK_ColorWHITE);
        auto img1 = surf->makeImageSnapshot();
        auto img2 = sk_gpu_test::MakeBackendTextureImage(dContext, info, SkColors::kBlack);
        REPORTER_ASSERT(reporter, img1 && img2);
        surf->getCanvas()->drawImage(std::move(img1), 0, 0);
        surf->getCanvas()->drawImage(std::move(img2), 1, 1);
        idleTexture.reset();
    }
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(TextureIdleProcRerefTest, reporter, contextInfo) {
    auto context = contextInfo.directContext();
    // idle proc that refs the texture
    auto idleProc = [](void* texture) { reinterpret_cast<GrTexture*>(texture)->ref(); };
    // release proc to check whether the texture was released or not.
    auto releaseProc = [](void* isReleased) { *reinterpret_cast<bool*>(isReleased) = true; };
    bool isReleased = false;
    auto idleTexture = make_normal_texture(context, GrRenderable::kNo);
    // This test assumes the texture won't be cached (or else the release proc doesn't get
    // called).
    idleTexture->resourcePriv().removeScratchKey();
    context->flushAndSubmit();
    idleTexture->addIdleProc(idleProc, idleTexture.get());
    idleTexture->setRelease(releaseProc, &isReleased);
    auto* raw = idleTexture.get();
    idleTexture.reset();
    REPORTER_ASSERT(reporter, !isReleased);
    raw->unref();
    REPORTER_ASSERT(reporter, isReleased);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(TextureIdleStateTest, reporter, contextInfo) {
    auto context = contextInfo.directContext();
    for (const auto& idleMaker : {make_wrapped_texture, make_normal_texture}) {
        auto idleTexture = idleMaker(context, GrRenderable::kNo);

        bool called = false;
        auto finishProc = [](void* called) { *static_cast<bool*>(called) = true; };
        idleTexture->addIdleProc(finishProc, &called);

        // Insert a copy from idleTexture to another texture so that we have some queued IO on
        // idleTexture.
        SkImageInfo info = SkImageInfo::Make(kSurfSize, kSurfSize, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
        auto rt = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, 0, nullptr);
        auto rtc = rt->getCanvas()->internal_private_accessTopLayerRenderTargetContext();
        auto proxy =
                context->priv().proxyProvider()->testingOnly_createWrapped(std::move(idleTexture));
        context->flushAndSubmit();
        SkAssertResult(rtc->testCopy(proxy.get()));
        proxy.reset();
        REPORTER_ASSERT(reporter, !called);

        // After a flush we expect idleTexture to have reached the kFlushed state on all backends.
        // On "managed" backends we expect it to reach kFinished as well. On Vulkan, the only
        // current "unmanaged" backend, we *may* need a sync to reach kFinished.
        context->flushAndSubmit();
        if (contextInfo.backend() != kVulkan_GrBackend) {
            REPORTER_ASSERT(reporter, called);
        }
        context->submit(true);
        REPORTER_ASSERT(reporter, called);
    }
}
