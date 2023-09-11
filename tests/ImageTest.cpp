/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMemset.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkImage_GaneshYUVA.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkImageShader.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/FenceSync.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/ProxyUtils.h"
#include "tools/gpu/TestContext.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class GrContextThreadSafeProxy;
class GrRecordingContext;
struct GrContextOptions;

using namespace sk_gpu_test;

SkImageInfo read_pixels_info(SkImage* image) {
    if (image->colorSpace()) {
        return SkImageInfo::MakeS32(image->width(), image->height(), image->alphaType());
    }

    return SkImageInfo::MakeN32(image->width(), image->height(), image->alphaType());
}

// image `b` is assumed to be raster
static void assert_equal(skiatest::Reporter* reporter, GrDirectContext* dContextA, SkImage* a,
                         const SkIRect* subsetA, SkImage* b) {
    const int widthA = subsetA ? subsetA->width() : a->width();
    const int heightA = subsetA ? subsetA->height() : a->height();

    REPORTER_ASSERT(reporter, widthA == b->width());
    REPORTER_ASSERT(reporter, heightA == b->height());

    // see https://bug.skia.org/3965
    //REPORTER_ASSERT(reporter, a->isOpaque() == b->isOpaque());

    SkAutoPixmapStorage pmapA, pmapB;
    pmapA.alloc(read_pixels_info(a));
    pmapB.alloc(read_pixels_info(b));

    const int srcX = subsetA ? subsetA->x() : 0;
    const int srcY = subsetA ? subsetA->y() : 0;

    REPORTER_ASSERT(reporter, a->readPixels(dContextA, pmapA, srcX, srcY));
    REPORTER_ASSERT(reporter, b->readPixels(nullptr, pmapB, 0, 0));

    const size_t widthBytes = widthA * 4;
    for (int y = 0; y < heightA; ++y) {
        REPORTER_ASSERT(reporter, !memcmp(pmapA.addr32(0, y), pmapB.addr32(0, y), widthBytes));
    }
}
static void draw_image_test_pattern(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeXYWH(5, 5, 10, 10), paint);
}
static sk_sp<SkImage> create_image() {
    const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    auto surface(SkSurfaces::Raster(info));
    draw_image_test_pattern(surface->getCanvas());
    return surface->makeImageSnapshot();
}
static sk_sp<SkData> create_image_data(SkImageInfo* info) {
    *info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    const size_t rowBytes = info->minRowBytes();
    sk_sp<SkData> data(SkData::MakeUninitialized(rowBytes * info->height()));
    {
        SkBitmap bm;
        bm.installPixels(*info, data->writable_data(), rowBytes);
        SkCanvas canvas(bm);
        draw_image_test_pattern(&canvas);
    }
    return data;
}
static sk_sp<SkImage> create_data_image() {
    SkImageInfo info;
    sk_sp<SkData> data(create_image_data(&info));
    return SkImages::RasterFromData(info, std::move(data), info.minRowBytes());
}
static sk_sp<SkImage> create_image_large(int maxTextureSize) {
    const SkImageInfo info = SkImageInfo::MakeN32(maxTextureSize + 1, 32, kOpaque_SkAlphaType);
    auto surface(SkSurfaces::Raster(info));
    surface->getCanvas()->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    surface->getCanvas()->drawRect(SkRect::MakeXYWH(4000, 2, 28000, 30), paint);
    return surface->makeImageSnapshot();
}
static sk_sp<SkImage> create_picture_image() {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);
    canvas->clear(SK_ColorCYAN);
    return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                         SkISize::Make(10, 10),
                                         nullptr,
                                         nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}
// Want to ensure that our Release is called when the owning image is destroyed
struct RasterDataHolder {
    RasterDataHolder() : fReleaseCount(0) {}
    sk_sp<SkData> fData;
    int fReleaseCount;
    static void Release(const void* pixels, void* context) {
        RasterDataHolder* self = static_cast<RasterDataHolder*>(context);
        self->fReleaseCount++;
        self->fData.reset();
    }
};
static sk_sp<SkImage> create_rasterproc_image(RasterDataHolder* dataHolder) {
    SkASSERT(dataHolder);
    SkImageInfo info;
    dataHolder->fData = create_image_data(&info);
    return SkImages::RasterFromPixmap(SkPixmap(info, dataHolder->fData->data(), info.minRowBytes()),
                                      RasterDataHolder::Release,
                                      dataHolder);
}
static sk_sp<SkImage> create_codec_image() {
    SkImageInfo info;
    sk_sp<SkData> data(create_image_data(&info));
    SkBitmap bitmap;
    bitmap.installPixels(info, data->writable_data(), info.minRowBytes());
    SkDynamicMemoryWStream stream;
    SkASSERT_RELEASE(SkPngEncoder::Encode(&stream, bitmap.pixmap(), {}));
    return SkImages::DeferredFromEncodedData(stream.detachAsData());
}
static sk_sp<SkImage> create_gpu_image(GrRecordingContext* rContext,
                                       bool withMips = false,
                                       skgpu::Budgeted budgeted = skgpu::Budgeted::kYes) {
    const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(
            rContext, budgeted, info, 0, kBottomLeft_GrSurfaceOrigin, nullptr, withMips);
    draw_image_test_pattern(surface->getCanvas());
    return surface->makeImageSnapshot();
}

static void test_encode(skiatest::Reporter* reporter, GrDirectContext* dContext, SkImage* image) {
    const SkIRect ir = SkIRect::MakeXYWH(5, 5, 10, 10);
    sk_sp<SkData> origEncoded = SkPngEncoder::Encode(dContext, image, {});
    REPORTER_ASSERT(reporter, origEncoded);
    REPORTER_ASSERT(reporter, origEncoded->size() > 0);

    sk_sp<SkImage> decoded(SkImages::DeferredFromEncodedData(origEncoded));
    if (!decoded) {
        ERRORF(reporter, "failed to decode image!");
        return;
    }
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, dContext, image, nullptr, decoded.get());

    // Now see if we can instantiate an image from a subset of the surface/origEncoded

    decoded = SkImages::DeferredFromEncodedData(origEncoded)->makeSubset(nullptr, ir);
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, dContext, image, &ir, decoded.get());
}

DEF_TEST(ImageEncode, reporter) {
    test_encode(reporter, nullptr, create_image().get());
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageEncode_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    test_encode(reporter, dContext, create_gpu_image(dContext).get());
}

DEF_TEST(Image_MakeFromRasterBitmap, reporter) {
    const struct {
        SkCopyPixelsMode fCPM;
        bool            fExpectSameAsMutable;
        bool            fExpectSameAsImmutable;
    } recs[] = {
        { kIfMutable_SkCopyPixelsMode,  false,  true },
        { kAlways_SkCopyPixelsMode,     false,  false },
        { kNever_SkCopyPixelsMode,      true,   true },
    };
    for (auto rec : recs) {
        SkPixmap pm;
        SkBitmap bm;
        bm.allocN32Pixels(100, 100);

        auto img = SkMakeImageFromRasterBitmap(bm, rec.fCPM);
        REPORTER_ASSERT(reporter, img->peekPixels(&pm));
        const bool sameMutable = pm.addr32(0, 0) == bm.getAddr32(0, 0);
        REPORTER_ASSERT(reporter, rec.fExpectSameAsMutable == sameMutable);
        REPORTER_ASSERT(reporter, (bm.getGenerationID() == img->uniqueID()) == sameMutable);

        bm.notifyPixelsChanged();   // force a new generation ID

        bm.setImmutable();
        img = SkMakeImageFromRasterBitmap(bm, rec.fCPM);
        REPORTER_ASSERT(reporter, img->peekPixels(&pm));
        const bool sameImmutable = pm.addr32(0, 0) == bm.getAddr32(0, 0);
        REPORTER_ASSERT(reporter, rec.fExpectSameAsImmutable == sameImmutable);
        REPORTER_ASSERT(reporter, (bm.getGenerationID() == img->uniqueID()) == sameImmutable);
    }
}

// Test that image encoding failures do not break picture serialization/deserialization.
DEF_TEST(Image_Serialize_Encoding_Failure, reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100)));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(100, 100);
    canvas->drawImage(image.get(), 0, 0, SkSamplingOptions());
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    REPORTER_ASSERT(reporter, picture);
    REPORTER_ASSERT(reporter, picture->approximateOpCount() > 0);

    bool was_called = false;
    SkSerialProcs procs;
    procs.fImageProc = [](SkImage*, void* called) {
        *(bool*)called = true;
        return SkData::MakeEmpty();
    };
    procs.fImageCtx = &was_called;

    REPORTER_ASSERT(reporter, !was_called);
    auto data = picture->serialize(&procs);
    REPORTER_ASSERT(reporter, was_called);
    REPORTER_ASSERT(reporter, data && data->size() > 0);

    auto deserialized = SkPicture::MakeFromData(data->data(), data->size());
    REPORTER_ASSERT(reporter, deserialized);
    REPORTER_ASSERT(reporter, deserialized->approximateOpCount() > 0);
}

// Test that a draw that only partially covers the drawing surface isn't
// interpreted as covering the entire drawing surface (i.e., exercise one of the
// conditions of SkCanvas::wouldOverwriteEntireSurface()).
DEF_TEST(Image_RetainSnapshot, reporter) {
    const SkPMColor red   = SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkPMColor green = SkPackARGB32(0xFF, 0, 0xFF, 0);
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);
    auto surface(SkSurfaces::Raster(info));
    surface->getCanvas()->clear(0xFF00FF00);

    SkPMColor pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));   // init with values we don't expect
    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(2, 2);
    const size_t dstRowBytes = 2 * sizeof(SkPMColor);

    sk_sp<SkImage> image1(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image1->readPixels(nullptr, dstInfo, pixels, dstRowBytes, 0, 0));
    for (size_t i = 0; i < std::size(pixels); ++i) {
        REPORTER_ASSERT(reporter, pixels[i] == green);
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(SK_ColorRED);

    surface->getCanvas()->drawRect(SkRect::MakeXYWH(1, 1, 1, 1), paint);

    sk_sp<SkImage> image2(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image2->readPixels(nullptr, dstInfo, pixels, dstRowBytes, 0, 0));
    REPORTER_ASSERT(reporter, pixels[0] == green);
    REPORTER_ASSERT(reporter, pixels[1] == green);
    REPORTER_ASSERT(reporter, pixels[2] == green);
    REPORTER_ASSERT(reporter, pixels[3] == red);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static void make_bitmap_mutable(SkBitmap* bm) {
    bm->allocN32Pixels(10, 10);
}

static void make_bitmap_immutable(SkBitmap* bm) {
    bm->allocN32Pixels(10, 10);
    bm->setImmutable();
}

DEF_TEST(image_newfrombitmap, reporter) {
    const struct {
        void (*fMakeProc)(SkBitmap*);
        bool fExpectPeekSuccess;
        bool fExpectSharedID;
        bool fExpectLazy;
    } rec[] = {
        { make_bitmap_mutable,      true,   false, false },
        { make_bitmap_immutable,    true,   true,  false },
    };

    for (size_t i = 0; i < std::size(rec); ++i) {
        SkBitmap bm;
        rec[i].fMakeProc(&bm);

        sk_sp<SkImage> image(bm.asImage());
        SkPixmap pmap;

        const bool sharedID = (image->uniqueID() == bm.getGenerationID());
        REPORTER_ASSERT(reporter, sharedID == rec[i].fExpectSharedID);

        const bool peekSuccess = image->peekPixels(&pmap);
        REPORTER_ASSERT(reporter, peekSuccess == rec[i].fExpectPeekSuccess);

        const bool lazy = image->isLazyGenerated();
        REPORTER_ASSERT(reporter, lazy == rec[i].fExpectLazy);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  This tests the caching (and preemptive purge) of the raster equivalent of a gpu-image.
 *  We cache it for performance when drawing into a raster surface.
 *
 *  A cleaner test would know if each drawImage call triggered a read-back from the gpu,
 *  but we don't have that facility (at the moment) so we use a little internal knowledge
 *  of *how* the raster version is cached, and look for that.
 */
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkImage_Ganesh2Cpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.directContext()));
    const auto desc = SkBitmapCacheDesc::Make(image.get());

    auto surface(SkSurfaces::Raster(info));

    // now we can test drawing a gpu-backed image into a cpu-backed surface

    {
        SkBitmap cachedBitmap;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(desc, &cachedBitmap));
    }

    surface->getCanvas()->drawImage(image, 0, 0);
    {
        SkBitmap cachedBitmap;
        if (SkBitmapCache::Find(desc, &cachedBitmap)) {
            REPORTER_ASSERT(reporter, cachedBitmap.isImmutable());
            REPORTER_ASSERT(reporter, cachedBitmap.getPixels());
        } else {
            // unexpected, but not really a bug, since the cache is global and this test may be
            // run w/ other threads competing for its budget.
            SkDebugf("SkImage_Ganesh2Cpu : cachedBitmap was already purged\n");
        }
    }

    image.reset(nullptr);
    {
        SkBitmap cachedBitmap;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(desc, &cachedBitmap));
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkImage_makeTextureImage,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = contextInfo.directContext();
    sk_gpu_test::TestContext* testContext = contextInfo.testContext();
    GrContextFactory otherFactory;
    ContextInfo otherContextInfo = otherFactory.getContextInfo(contextInfo.type());
    testContext->makeCurrent();
    std::function<sk_sp<SkImage>()> imageFactories[] = {
            create_image,
            create_codec_image,
            create_data_image,
            // Create an image from a picture.
            create_picture_image,
            // Create a texture image.
            [dContext] { return create_gpu_image(dContext, true, skgpu::Budgeted::kYes); },
            [dContext] { return create_gpu_image(dContext, false, skgpu::Budgeted::kNo); },
            // Create a texture image in a another context.
            [otherContextInfo] {
                auto restore = otherContextInfo.testContext()->makeCurrentAndAutoRestore();
                auto otherContextImage = create_gpu_image(otherContextInfo.directContext());
                otherContextInfo.directContext()->flushAndSubmit();
                return otherContextImage;
            }};
    for (auto mipmapped : {skgpu::Mipmapped::kNo, skgpu::Mipmapped::kYes}) {
        for (const auto& factory : imageFactories) {
            sk_sp<SkImage> image(factory());
            if (!image) {
                ERRORF(reporter, "Error creating image.");
                continue;
            }
            GrTextureProxy* origProxy = nullptr;
            bool origIsMippedTexture = false;

            if ((origProxy = sk_gpu_test::GetTextureImageProxy(image.get(), dContext))) {
                REPORTER_ASSERT(
                        reporter,
                        (origProxy->mipmapped() == skgpu::Mipmapped::kYes) == image->hasMipmaps());
                origIsMippedTexture = image->hasMipmaps();
            }
            for (auto budgeted : {skgpu::Budgeted::kNo, skgpu::Budgeted::kYes}) {
                auto texImage = SkImages::TextureFromImage(dContext, image, mipmapped, budgeted);
                if (!texImage) {
                    auto imageContext = as_IB(image)->context();
                    // We expect to fail if image comes from a different context
                    if (!image->isTextureBacked() || imageContext->priv().matches(dContext)) {
                        ERRORF(reporter, "makeTextureImage failed.");
                    }
                    continue;
                }
                if (!texImage->isTextureBacked()) {
                    ERRORF(reporter, "makeTextureImage returned non-texture image.");
                    continue;
                }

                GrTextureProxy* copyProxy = sk_gpu_test::GetTextureImageProxy(texImage.get(),
                                                                              dContext);
                SkASSERT(copyProxy);
                // Did we ask for MIPs on a context that supports them?
                bool validRequestForMips = (mipmapped == skgpu::Mipmapped::kYes &&
                                            dContext->priv().caps()->mipmapSupport());
                // Do we expect the "copy" to have MIPs?
                bool shouldBeMipped = origIsMippedTexture || validRequestForMips;
                REPORTER_ASSERT(reporter, shouldBeMipped == texImage->hasMipmaps());
                REPORTER_ASSERT(
                        reporter,
                        shouldBeMipped == (copyProxy->mipmapped() == skgpu::Mipmapped::kYes));

                // We should only make a copy of an already texture-backed image if it didn't
                // already have MIPs but we asked for MIPs and the context supports it.
                if (image->isTextureBacked() && (!validRequestForMips || origIsMippedTexture)) {
                    if (origProxy->underlyingUniqueID() != copyProxy->underlyingUniqueID()) {
                        ERRORF(reporter, "makeTextureImage made unnecessary texture copy.");
                    }
                } else {
                    GrTextureProxy* texProxy = sk_gpu_test::GetTextureImageProxy(texImage.get(),
                                                                                 dContext);
                    REPORTER_ASSERT(reporter, !texProxy->getUniqueKey().isValid());
                    REPORTER_ASSERT(reporter, texProxy->isBudgeted() == budgeted);
                }
                if (image->width() != texImage->width() || image->height() != texImage->height()) {
                    ERRORF(reporter, "makeTextureImage changed the image size.");
                }
                if (image->alphaType() != texImage->alphaType()) {
                    ERRORF(reporter, "makeTextureImage changed image alpha type.");
                }
            }
        }
    }
    dContext->flushAndSubmit();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkImage_makeNonTextureImage,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = contextInfo.directContext();

    std::function<sk_sp<SkImage>()> imageFactories[] = {
        create_image,
        create_codec_image,
        create_data_image,
        create_picture_image,
        [dContext] { return create_gpu_image(dContext); },
    };
    for (const auto& factory : imageFactories) {
        sk_sp<SkImage> image = factory();
        if (!image->isTextureBacked()) {
            REPORTER_ASSERT(reporter, image->makeNonTextureImage().get() == image.get());
            if (!(image = SkImages::TextureFromImage(dContext, image))) {
                continue;
            }
        }
        auto rasterImage = image->makeNonTextureImage();
        if (!rasterImage) {
            ERRORF(reporter, "makeNonTextureImage failed for texture-backed image.");
        }
        REPORTER_ASSERT(reporter, !rasterImage->isTextureBacked());
        assert_equal(reporter, dContext, image.get(), nullptr, rasterImage.get());
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrContext_colorTypeSupportedAsImage,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    static constexpr int kSize = 10;

    for (int ct = 0; ct < kLastEnum_SkColorType; ++ct) {
        SkColorType colorType = static_cast<SkColorType>(ct);
        bool can = dContext->colorTypeSupportedAsImage(colorType);

        auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
                dContext, kSize, kSize, colorType, skgpu::Mipmapped::kNo, GrRenderable::kNo);
        sk_sp<SkImage> img;
        if (mbet) {
            img = SkImages::BorrowTextureFrom(dContext,
                                              mbet->texture(),
                                              kTopLeft_GrSurfaceOrigin,
                                              colorType,
                                              kOpaque_SkAlphaType,
                                              nullptr);
        }
        REPORTER_ASSERT(reporter, can == SkToBool(img),
                        "colorTypeSupportedAsImage:%d, actual:%d, ct:%d", can, SkToBool(img),
                        colorType);
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(UnpremulTextureImage,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    SkBitmap bmp;
    bmp.allocPixels(
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr));
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            *bmp.getAddr32(x, y) =
                    SkColorSetARGB((U8CPU)y, 255 - (U8CPU)y, (U8CPU)x, 255 - (U8CPU)x);
        }
    }
    auto dContext = ctxInfo.directContext();
    auto texImage = SkImages::TextureFromImage(dContext, bmp.asImage());
    if (!texImage || texImage->alphaType() != kUnpremul_SkAlphaType) {
        ERRORF(reporter, "Failed to make unpremul texture image.");
        return;
    }
    SkBitmap unpremul;
    unpremul.allocPixels(SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType,
                                           kUnpremul_SkAlphaType, nullptr));
    if (!texImage->readPixels(dContext, unpremul.info(), unpremul.getPixels(), unpremul.rowBytes(),
                              0, 0)) {
        ERRORF(reporter, "Unpremul readback failed.");
        return;
    }
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            if (*bmp.getAddr32(x, y) != *unpremul.getAddr32(x, y)) {
                ERRORF(reporter, "unpremul(0x%08x)->unpremul(0x%08x) at %d, %d.",
                       *bmp.getAddr32(x, y), *unpremul.getAddr32(x, y), x, y);
                return;
            }
        }
    }
    SkBitmap premul;
    premul.allocPixels(
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr));
    if (!texImage->readPixels(dContext, premul.info(), premul.getPixels(), premul.rowBytes(),
                              0, 0)) {
        ERRORF(reporter, "Unpremul readback failed.");
        return;
    }
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            uint32_t origColor = *bmp.getAddr32(x, y);
            int32_t origA = (origColor >> 24) & 0xff;
            float a = origA / 255.f;
            int32_t origB = sk_float_round2int(((origColor >> 16) & 0xff) * a);
            int32_t origG = sk_float_round2int(((origColor >>  8) & 0xff) * a);
            int32_t origR = sk_float_round2int(((origColor >>  0) & 0xff) * a);

            uint32_t read = *premul.getAddr32(x, y);
            int32_t readA = (read >> 24) & 0xff;
            int32_t readB = (read >> 16) & 0xff;
            int32_t readG = (read >>  8) & 0xff;
            int32_t readR = (read >>  0) & 0xff;
            // We expect that alpha=1 and alpha=0 should come out exact. Otherwise allow a little
            // bit of tolerance for GPU vs CPU premul math.
            int32_t tol = (origA == 0 || origA == 255) ? 0 : 1;
            if (origA != readA || SkTAbs(readB - origB) > tol || SkTAbs(readG - origG) > tol ||
                SkTAbs(readR - origR) > tol) {
                ERRORF(reporter, "unpremul(0x%08x)->premul(0x%08x) expected(0x%08x) at %d, %d.",
                       *bmp.getAddr32(x, y), *premul.getAddr32(x, y), origColor, x, y);
                return;
            }
        }
    }
}

DEF_GANESH_TEST(AbandonedContextImage, reporter, options, CtsEnforcement::kApiLevel_T) {
    using Factory = sk_gpu_test::GrContextFactory;
    for (int ct = 0; ct < skgpu::kContextTypeCount; ++ct) {
        auto type = static_cast<Factory::ContextType>(ct);
        std::unique_ptr<Factory> factory(new Factory);
        if (!factory->get(type)) {
            continue;
        }

        sk_sp<SkImage> img;
        auto gsurf = SkSurfaces::RenderTarget(
                factory->get(type),
                skgpu::Budgeted::kYes,
                SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                1,
                nullptr);
        if (!gsurf) {
            continue;
        }
        img = gsurf->makeImageSnapshot();
        gsurf.reset();

        auto rsurf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));

        REPORTER_ASSERT(reporter, img->isValid(factory->get(type)));
        REPORTER_ASSERT(reporter, img->isValid(rsurf->getCanvas()->recordingContext()));

        factory->get(type)->abandonContext();
        REPORTER_ASSERT(reporter, !img->isValid(factory->get(type)));
        REPORTER_ASSERT(reporter, !img->isValid(rsurf->getCanvas()->recordingContext()));
        // This shouldn't crash.
        rsurf->getCanvas()->drawImage(img, 0, 0);

        // Give up all other refs on the context.
        factory.reset(nullptr);
        REPORTER_ASSERT(reporter, !img->isValid(rsurf->getCanvas()->recordingContext()));
        // This shouldn't crash.
        rsurf->getCanvas()->drawImage(img, 0, 0);
    }
}

class EmptyGenerator : public SkImageGenerator {
public:
    EmptyGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageEmpty, reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    SkPixmap pmap(info, nullptr, 0);
    REPORTER_ASSERT(reporter, nullptr == SkImages::RasterFromPixmapCopy(pmap));
    REPORTER_ASSERT(reporter, nullptr == SkImages::RasterFromData(info, nullptr, 0));
    REPORTER_ASSERT(reporter, nullptr == SkImages::RasterFromPixmap(pmap, nullptr, nullptr));
    REPORTER_ASSERT(reporter,
                    nullptr == SkImages::DeferredFromGenerator(std::make_unique<EmptyGenerator>()));
}

DEF_TEST(ImageDataRef, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.computeByteSize(rowBytes);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    REPORTER_ASSERT(reporter, data->unique());
    sk_sp<SkImage> image = SkImages::RasterFromData(info, data, rowBytes);
    REPORTER_ASSERT(reporter, !data->unique());
    image.reset();
    REPORTER_ASSERT(reporter, data->unique());
}

static bool has_pixels(const SkPMColor pixels[], int count, SkPMColor expected) {
    for (int i = 0; i < count; ++i) {
        if (pixels[i] != expected) {
            return false;
        }
    }
    return true;
}

static void image_test_read_pixels(GrDirectContext* dContext, skiatest::Reporter* reporter,
                                   SkImage* image) {
    if (!image) {
        ERRORF(reporter, "Failed to create image!");
        return;
    }
    const SkPMColor expected = SkPreMultiplyColor(SK_ColorWHITE);
    const SkPMColor notExpected = ~expected;

    const int w = 2, h = 2;
    const size_t rowBytes = w * sizeof(SkPMColor);
    SkPMColor pixels[w*h];

    SkImageInfo info;

    info = SkImageInfo::MakeUnknown(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(dContext, info, pixels, rowBytes, 0, 0));

    // out-of-bounds should fail
    info = SkImageInfo::MakeN32Premul(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(dContext, info, pixels, rowBytes, -w, 0));
    REPORTER_ASSERT(reporter, !image->readPixels(dContext, info, pixels, rowBytes, 0, -h));
    REPORTER_ASSERT(reporter, !image->readPixels(dContext, info, pixels, rowBytes,
                                                 image->width(), 0));
    REPORTER_ASSERT(reporter, !image->readPixels(dContext, info, pixels, rowBytes,
                                                 0, image->height()));

    // top-left should succeed
    SkOpts::memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(dContext, info, pixels, rowBytes, 0, 0));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // bottom-right should succeed
    SkOpts::memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(dContext, info, pixels, rowBytes,
                                                image->width() - w, image->height() - h));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // partial top-left should succeed
    SkOpts::memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(dContext, info, pixels, rowBytes, -1, -1));
    REPORTER_ASSERT(reporter, pixels[3] == expected);
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h - 1, notExpected));

    // partial bottom-right should succeed
    SkOpts::memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(dContext, info, pixels, rowBytes,
                                                image->width() - 1, image->height() - 1));
    REPORTER_ASSERT(reporter, pixels[0] == expected);
    REPORTER_ASSERT(reporter, has_pixels(&pixels[1], w*h - 1, notExpected));
}
DEF_TEST(ImageReadPixels, reporter) {
    sk_sp<SkImage> image(create_image());
    image_test_read_pixels(nullptr, reporter, image.get());

    image = create_data_image();
    image_test_read_pixels(nullptr, reporter, image.get());

    RasterDataHolder dataHolder;
    image = create_rasterproc_image(&dataHolder);
    image_test_read_pixels(nullptr, reporter, image.get());
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image = create_codec_image();
    image_test_read_pixels(nullptr, reporter, image.get());
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageReadPixels_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    image_test_read_pixels(dContext, reporter, create_gpu_image(dContext).get());
}

static void check_legacy_bitmap(skiatest::Reporter* reporter, GrDirectContext* dContext,
                                const SkImage* image, const SkBitmap& bitmap) {
    REPORTER_ASSERT(reporter, image->width() == bitmap.width());
    REPORTER_ASSERT(reporter, image->height() == bitmap.height());
    REPORTER_ASSERT(reporter, image->alphaType() == bitmap.alphaType());

    REPORTER_ASSERT(reporter, bitmap.isImmutable());

    REPORTER_ASSERT(reporter, bitmap.getPixels());

    const SkImageInfo info = SkImageInfo::MakeN32(1, 1, bitmap.alphaType());
    SkPMColor imageColor;
    REPORTER_ASSERT(reporter, image->readPixels(dContext, info, &imageColor, sizeof(SkPMColor),
                                                0, 0));
    REPORTER_ASSERT(reporter, imageColor == *bitmap.getAddr32(0, 0));
}

static void test_legacy_bitmap(skiatest::Reporter* reporter, GrDirectContext* dContext,
                               const SkImage* image) {
    if (!image) {
        ERRORF(reporter, "Failed to create image.");
        return;
    }
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter, image->asLegacyBitmap(&bitmap));
    check_legacy_bitmap(reporter, dContext, image, bitmap);

    // Test subsetting to exercise the rowBytes logic.
    SkBitmap tmp;
    REPORTER_ASSERT(reporter, bitmap.extractSubset(&tmp, SkIRect::MakeWH(image->width() / 2,
                                                                         image->height() / 2)));
    sk_sp<SkImage> subsetImage(tmp.asImage());
    REPORTER_ASSERT(reporter, subsetImage.get());

    SkBitmap subsetBitmap;
    REPORTER_ASSERT(reporter, subsetImage->asLegacyBitmap(&subsetBitmap));
    check_legacy_bitmap(reporter, nullptr, subsetImage.get(), subsetBitmap);
}
DEF_TEST(ImageLegacyBitmap, reporter) {
    sk_sp<SkImage> image(create_image());
    test_legacy_bitmap(reporter, nullptr, image.get());

    image = create_data_image();
    test_legacy_bitmap(reporter, nullptr, image.get());

    RasterDataHolder dataHolder;
    image = create_rasterproc_image(&dataHolder);
    test_legacy_bitmap(reporter, nullptr, image.get());
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image = create_codec_image();
    test_legacy_bitmap(reporter, nullptr, image.get());
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageLegacyBitmap_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    sk_sp<SkImage> image(create_gpu_image(dContext));
    test_legacy_bitmap(reporter, dContext, image.get());
}

static void test_peek(skiatest::Reporter* reporter, SkImage* image, bool expectPeekSuccess) {
    if (!image) {
        ERRORF(reporter, "Failed to create image!");
        return;
    }
    SkPixmap pm;
    bool success = image->peekPixels(&pm);
    REPORTER_ASSERT(reporter, expectPeekSuccess == success);
    if (success) {
        const SkImageInfo& info = pm.info();
        REPORTER_ASSERT(reporter, 20 == info.width());
        REPORTER_ASSERT(reporter, 20 == info.height());
        REPORTER_ASSERT(reporter, kN32_SkColorType == info.colorType());
        REPORTER_ASSERT(reporter, kPremul_SkAlphaType == info.alphaType() ||
                        kOpaque_SkAlphaType == info.alphaType());
        REPORTER_ASSERT(reporter, info.minRowBytes() <= pm.rowBytes());
        REPORTER_ASSERT(reporter, SkPreMultiplyColor(SK_ColorWHITE) == *pm.addr32(0, 0));
    }
}
DEF_TEST(ImagePeek, reporter) {
    sk_sp<SkImage> image(create_image());
    test_peek(reporter, image.get(), true);

    image = create_data_image();
    test_peek(reporter, image.get(), true);

    RasterDataHolder dataHolder;
    image = create_rasterproc_image(&dataHolder);
    test_peek(reporter, image.get(), true);
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image = create_codec_image();
    test_peek(reporter, image.get(), false);
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImagePeek_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.directContext()));
    test_peek(reporter, image.get(), false);
}

struct TextureReleaseChecker {
    TextureReleaseChecker() : fReleaseCount(0) {}
    int fReleaseCount;
    static void Release(void* self) {
        static_cast<TextureReleaseChecker*>(self)->fReleaseCount++;
    }
};

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkImage_NewFromTextureRelease,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    const int kWidth = 10;
    const int kHeight = 10;

    auto dContext = ctxInfo.directContext();

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(dContext,
                                                                    kWidth,
                                                                    kHeight,
                                                                    kRGBA_8888_SkColorType,
                                                                    skgpu::Mipmapped::kNo,
                                                                    GrRenderable::kNo,
                                                                    GrProtected::kNo);
    if (!mbet) {
        ERRORF(reporter, "couldn't create backend texture\n");
        return;
    }

    TextureReleaseChecker releaseChecker;
    GrSurfaceOrigin texOrigin = kBottomLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg = SkImages::BorrowTextureFrom(
            dContext,
            mbet->texture(),
            texOrigin,
            kRGBA_8888_SkColorType,
            kPremul_SkAlphaType,
            /*color space*/ nullptr,
            sk_gpu_test::ManagedBackendTexture::ReleaseProc,
            mbet->releaseContext(TextureReleaseChecker::Release, &releaseChecker));

    GrSurfaceOrigin readBackOrigin;
    GrBackendTexture readBackBackendTex;
    REPORTER_ASSERT(reporter,
                    SkImages::GetBackendTextureFromImage(
                            refImg, &readBackBackendTex, false, &readBackOrigin),
                    "Did not get backend texture");
    if (!GrBackendTexture::TestingOnly_Equals(readBackBackendTex, mbet->texture())) {
        ERRORF(reporter, "backend mismatch\n");
    }
    REPORTER_ASSERT(reporter,
                    GrBackendTexture::TestingOnly_Equals(readBackBackendTex, mbet->texture()));
    if (readBackOrigin != texOrigin) {
        ERRORF(reporter, "origin mismatch %d %d\n", readBackOrigin, texOrigin);
    }
    REPORTER_ASSERT(reporter, readBackOrigin == texOrigin);

    // Now exercise the release proc
    REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
    refImg.reset(nullptr); // force a release of the image
    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
}

static void test_cross_context_image(skiatest::Reporter* reporter, const GrContextOptions& options,
                                     const char* testName,
                                     std::function<sk_sp<SkImage>(GrDirectContext*)> imageMaker) {
    for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
        GrContextFactory testFactory(options);
        skgpu::ContextType ctxType = static_cast<skgpu::ContextType>(i);
        ContextInfo ctxInfo = testFactory.getContextInfo(ctxType);
        auto dContext = ctxInfo.directContext();
        if (!dContext) {
            continue;
        }

        // If we don't have proper support for this feature, the factory will fallback to returning
        // codec-backed images. Those will "work", but some of our checks will fail because we
        // expect the cross-context images not to work on multiple contexts at once.
        if (!dContext->priv().caps()->crossContextTextureSupport()) {
            continue;
        }

        // We test three lifetime patterns for a single context:
        // 1) Create image, free image
        // 2) Create image, draw, flush, free image
        // 3) Create image, draw, free image, flush
        // ... and then repeat the last two patterns with drawing on a second* context:
        // 4) Create image, draw*, flush*, free image
        // 5) Create image, draw*, free image, flush*

        // Case #1: Create image, free image
        {
            sk_sp<SkImage> refImg(imageMaker(dContext));
            refImg.reset(nullptr); // force a release of the image
        }

        SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
        if (!surface) {
            ERRORF(reporter, "SkSurfaces::RenderTarget failed for %s.", testName);
            continue;
        }

        SkCanvas* canvas = surface->getCanvas();

        // Case #2: Create image, draw, flush, free image
        {
            sk_sp<SkImage> refImg(imageMaker(dContext));

            canvas->drawImage(refImg, 0, 0);
            dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);

            refImg.reset(nullptr); // force a release of the image
        }

        // Case #3: Create image, draw, free image, flush
        {
            sk_sp<SkImage> refImg(imageMaker(dContext));

            canvas->drawImage(refImg, 0, 0);
            refImg.reset(nullptr); // force a release of the image

            dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        }

        // Configure second context
        sk_gpu_test::TestContext* testContext = ctxInfo.testContext();

        ContextInfo otherContextInfo = testFactory.getSharedContextInfo(dContext);
        auto otherCtx = otherContextInfo.directContext();
        sk_gpu_test::TestContext* otherTestContext = otherContextInfo.testContext();

        // Creating a context in a share group may fail
        if (!otherCtx) {
            continue;
        }

        surface = SkSurfaces::RenderTarget(otherCtx, skgpu::Budgeted::kNo, info);
        canvas = surface->getCanvas();

        // Case #4: Create image, draw*, flush*, free image
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(imageMaker(dContext));

            otherTestContext->makeCurrent();
            canvas->drawImage(refImg, 0, 0);
            otherCtx->flushAndSubmit(surface.get(), GrSyncCpu::kNo);

            testContext->makeCurrent();
            refImg.reset(nullptr); // force a release of the image
        }

        // Case #5: Create image, draw*, free image, flush*
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(imageMaker(dContext));

            otherTestContext->makeCurrent();
            canvas->drawImage(refImg, 0, 0);

            testContext->makeCurrent();
            refImg.reset(nullptr); // force a release of the image

            otherTestContext->makeCurrent();
            // Sync is specifically here for vulkan to guarantee the command buffer will finish
            // which is when we call the ReleaseProc.
            otherCtx->flushAndSubmit(surface.get(), GrSyncCpu::kYes);
        }

        // Case #6: Verify that only one context can be using the image at a time
        {
            // Suppress warnings about trying to use a texture in two contexts.
            GrRecordingContextPriv::AutoSuppressWarningMessages aswm(otherCtx);

            testContext->makeCurrent();
            sk_sp <SkImage> refImg(imageMaker(dContext));
            GrSurfaceProxyView view, otherView, viewSecondRef;

            // Any context should be able to borrow the texture at this point

            std::tie(view, std::ignore) =
                    skgpu::ganesh::AsView(dContext, refImg, skgpu::Mipmapped::kNo);
            REPORTER_ASSERT(reporter, view);

            // But once it's borrowed, no other context should be able to borrow
            otherTestContext->makeCurrent();
            std::tie(otherView, std::ignore) =
                    skgpu::ganesh::AsView(otherCtx, refImg, skgpu::Mipmapped::kNo);
            REPORTER_ASSERT(reporter, !otherView);

            // Original context (that's already borrowing) should be okay
            testContext->makeCurrent();
            std::tie(viewSecondRef, std::ignore) =
                    skgpu::ganesh::AsView(dContext, refImg, skgpu::Mipmapped::kNo);
            REPORTER_ASSERT(reporter, viewSecondRef);

            // Release first ref from the original context
            view.reset();

            // We released one proxy but not the other from the current borrowing context. Make sure
            // a new context is still not able to borrow the texture.
            otherTestContext->makeCurrent();
            std::tie(otherView, std::ignore) =
                    skgpu::ganesh::AsView(otherCtx, refImg, skgpu::Mipmapped::kNo);
            REPORTER_ASSERT(reporter, !otherView);

            // Release second ref from the original context
            testContext->makeCurrent();
            viewSecondRef.reset();

            // Now we should be able to borrow the texture from the other context
            otherTestContext->makeCurrent();
            std::tie(otherView, std::ignore) =
                    skgpu::ganesh::AsView(otherCtx, refImg, skgpu::Mipmapped::kNo);
            REPORTER_ASSERT(reporter, otherView);

            // Release everything
            otherView.reset();
            refImg.reset(nullptr);
        }
    }
}

DEF_GANESH_TEST(SkImage_MakeCrossContextFromPixmapRelease,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    SkBitmap bitmap;
    SkPixmap pixmap;
    if (!GetResourceAsBitmap("images/mandrill_128.png", &bitmap) || !bitmap.peekPixels(&pixmap)) {
        ERRORF(reporter, "missing resource");
        return;
    }
    test_cross_context_image(reporter,
                             options,
                             "SkImage_MakeCrossContextFromPixmapRelease",
                             [&pixmap](GrDirectContext* dContext) {
                                 return SkImages::CrossContextTextureFromPixmap(
                                         dContext, pixmap, false);
                             });
}

DEF_GANESH_TEST(SkImage_CrossContextGrayAlphaConfigs,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    for (SkColorType ct : { kGray_8_SkColorType, kAlpha_8_SkColorType }) {
        SkAutoPixmapStorage pixmap;
        pixmap.alloc(SkImageInfo::Make(4, 4, ct, kPremul_SkAlphaType));

        for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
            GrContextFactory testFactory(options);
            skgpu::ContextType ctxType = static_cast<skgpu::ContextType>(i);
            ContextInfo ctxInfo = testFactory.getContextInfo(ctxType);
            auto dContext = ctxInfo.directContext();
            if (!dContext || !dContext->priv().caps()->crossContextTextureSupport()) {
                continue;
            }

            sk_sp<SkImage> image = SkImages::CrossContextTextureFromPixmap(dContext, pixmap, false);
            REPORTER_ASSERT(reporter, image);

            auto [view, viewCT] = skgpu::ganesh::AsView(dContext, image, skgpu::Mipmapped::kNo);
            REPORTER_ASSERT(reporter, view);
            REPORTER_ASSERT(reporter, GrColorTypeToSkColorType(viewCT) == ct);

            bool expectAlpha = kAlpha_8_SkColorType == ct;
            GrColorType grCT = SkColorTypeToGrColorType(image->colorType());
            REPORTER_ASSERT(reporter, expectAlpha == GrColorTypeIsAlphaOnly(grCT));
        }
    }
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(makeBackendTexture, reporter, ctxInfo, CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    sk_gpu_test::TestContext* testContext = ctxInfo.testContext();
    sk_sp<GrContextThreadSafeProxy> proxy = context->threadSafeProxy();

    GrContextFactory otherFactory;
    ContextInfo otherContextInfo = otherFactory.getContextInfo(ctxInfo.type());

    testContext->makeCurrent();
    REPORTER_ASSERT(reporter, proxy);
    auto createLarge = [context] {
        return create_image_large(context->priv().caps()->maxTextureSize());
    };
    struct TestCase {
        std::function<sk_sp<SkImage>()> fImageFactory;
        bool                            fExpectation;
        bool                            fCanTakeDirectly;
    };
    TestCase testCases[] = {
        { create_image, true, false },
        { create_codec_image, true, false },
        { create_data_image, true, false },
        { create_picture_image, true, false },
        { [context] { return create_gpu_image(context); }, true, true },
        // Create a texture image in a another context.
        { [otherContextInfo] {
            auto restore = otherContextInfo.testContext()->makeCurrentAndAutoRestore();
            sk_sp<SkImage> otherContextImage = create_gpu_image(otherContextInfo.directContext());
            otherContextInfo.directContext()->flushAndSubmit();
            return otherContextImage;
          }, false, false },
        // Create an image that is too large to be texture backed.
        { createLarge, false, false }
    };

    for (const TestCase& testCase : testCases) {
        sk_sp<SkImage> image(testCase.fImageFactory());
        if (!image) {
            ERRORF(reporter, "Failed to create image!");
            continue;
        }

        GrBackendTexture origBackend;
        SkImages::GetBackendTextureFromImage(image, &origBackend, true);
        if (testCase.fCanTakeDirectly) {
            SkASSERT(origBackend.isValid());
        }

        GrBackendTexture newBackend;
        SkImages::BackendTextureReleaseProc proc;
        bool result = SkImages::MakeBackendTextureFromImage(
                context, std::move(image), &newBackend, &proc);
        if (result != testCase.fExpectation) {
            static const char *const kFS[] = { "fail", "succeed" };
            ERRORF(reporter, "This image was expected to %s but did not.",
            kFS[testCase.fExpectation]);
        }

        if (result) {
            SkASSERT(newBackend.isValid());
        }

        bool tookDirectly = result && GrBackendTexture::TestingOnly_Equals(origBackend, newBackend);
        if (testCase.fCanTakeDirectly != tookDirectly) {
            static const char *const kExpectedState[] = { "not expected", "expected" };
            ERRORF(reporter, "This backend texture was %s to be taken directly.",
            kExpectedState[testCase.fCanTakeDirectly]);
        }

        context->flushAndSubmit();
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageBackendAccessAbandoned_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.directContext()));
    if (!image) {
        return;
    }

    GrBackendTexture beTex;
    bool ok = SkImages::GetBackendTextureFromImage(image, &beTex, true);
    REPORTER_ASSERT(reporter, ok);
    REPORTER_ASSERT(reporter, beTex.isValid());

    dContext->abandonContext();

    // After abandoning the context the backend texture should not be valid.
    ok = SkImages::GetBackendTextureFromImage(image, &beTex, true);
    REPORTER_ASSERT(reporter, !ok);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> create_picture_image(sk_sp<SkColorSpace> space) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);
    canvas->clear(SK_ColorCYAN);
    return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                         SkISize::Make(10, 10),
                                         nullptr,
                                         nullptr,
                                         SkImages::BitDepth::kU8,
                                         std::move(space));
}

DEF_TEST(Image_ColorSpace, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    sk_sp<SkImage> image = GetResourceAsImage("images/mandrill_512_q075.jpg");
    REPORTER_ASSERT(r, srgb.get() == image->colorSpace());

    image = GetResourceAsImage("images/webp-color-profile-lossy.webp");
    skcms_TransferFunction fn;
    bool success = image->colorSpace()->isNumericalTransferFn(&fn);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, color_space_almost_equal(1.8f, fn.g));

    sk_sp<SkColorSpace> rec2020 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                        SkNamedGamut::kRec2020);
    image = create_picture_image(rec2020);
    REPORTER_ASSERT(r, SkColorSpace::Equals(rec2020.get(), image->colorSpace()));

    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::MakeN32(10, 10, kPremul_SkAlphaType, rec2020);
    bitmap.allocPixels(info);
    image = bitmap.asImage();
    REPORTER_ASSERT(r, SkColorSpace::Equals(rec2020.get(), image->colorSpace()));

    sk_sp<SkSurface> surface =
            SkSurfaces::Raster(SkImageInfo::MakeN32Premul(SkISize::Make(10, 10)));
    image = surface->makeImageSnapshot();
    REPORTER_ASSERT(r, nullptr == image->colorSpace());

    surface = SkSurfaces::Raster(info);
    image = surface->makeImageSnapshot();
    REPORTER_ASSERT(r, SkColorSpace::Equals(rec2020.get(), image->colorSpace()));
}

DEF_TEST(Image_makeColorSpace, r) {
    sk_sp<SkColorSpace> p3 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
    skcms_TransferFunction fn;
    fn.a = 1.f; fn.b = 0.f; fn.c = 0.f; fn.d = 0.f; fn.e = 0.f; fn.f = 0.f; fn.g = 1.8f;
    sk_sp<SkColorSpace> adobeGamut = SkColorSpace::MakeRGB(fn, SkNamedGamut::kAdobeRGB);

    SkBitmap srgbBitmap;
    srgbBitmap.allocPixels(SkImageInfo::MakeS32(1, 1, kOpaque_SkAlphaType));
    *srgbBitmap.getAddr32(0, 0) = SkSwizzle_RGBA_to_PMColor(0xFF604020);
    srgbBitmap.setImmutable();
    sk_sp<SkImage> srgbImage = srgbBitmap.asImage();
    sk_sp<SkImage> p3Image = srgbImage->makeColorSpace(nullptr, p3);
    SkBitmap p3Bitmap;
    bool success = p3Image->asLegacyBitmap(&p3Bitmap);

    auto almost_equal = [](int a, int b) { return SkTAbs(a - b) <= 2; };

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x28, SkGetPackedR32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x40, SkGetPackedG32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x5E, SkGetPackedB32(*p3Bitmap.getAddr32(0, 0))));

    sk_sp<SkImage> adobeImage = srgbImage->makeColorSpace(nullptr, adobeGamut);
    SkBitmap adobeBitmap;
    success = adobeImage->asLegacyBitmap(&adobeBitmap);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x21, SkGetPackedR32(*adobeBitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x31, SkGetPackedG32(*adobeBitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x4C, SkGetPackedB32(*adobeBitmap.getAddr32(0, 0))));

    srgbImage = GetResourceAsImage("images/1x1.png");
    p3Image = srgbImage->makeColorSpace(nullptr, p3);
    success = p3Image->asLegacyBitmap(&p3Bitmap);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x8B, SkGetPackedR32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x82, SkGetPackedG32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x77, SkGetPackedB32(*p3Bitmap.getAddr32(0, 0))));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void make_all_premul(SkBitmap* bm) {
    bm->allocPixels(SkImageInfo::MakeN32(256, 256, kPremul_SkAlphaType));
    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            // make all valid premul combinations
            int c = std::min(a, r);
            *bm->getAddr32(a, r) = SkPackARGB32(a, c, c, c);
        }
    }
}

static bool equal(const SkBitmap& a, const SkBitmap& b) {
    SkASSERT(a.width() == b.width());
    SkASSERT(a.height() == b.height());
    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            SkPMColor pa = *a.getAddr32(x, y);
            SkPMColor pb = *b.getAddr32(x, y);
            if (pa != pb) {
                return false;
            }
        }
    }
    return true;
}

DEF_TEST(image_roundtrip_encode, reporter) {
    SkBitmap bm0;
    make_all_premul(&bm0);

    auto img0 = bm0.asImage();
    sk_sp<SkData> data = SkPngEncoder::Encode(nullptr, img0.get(), {});
    auto img1 = SkImages::DeferredFromEncodedData(data);

    SkBitmap bm1;
    bm1.allocPixels(SkImageInfo::MakeN32(256, 256, kPremul_SkAlphaType));
    img1->readPixels(nullptr, bm1.info(), bm1.getPixels(), bm1.rowBytes(), 0, 0);

    REPORTER_ASSERT(reporter, equal(bm0, bm1));
}

DEF_TEST(image_roundtrip_premul, reporter) {
    SkBitmap bm0;
    make_all_premul(&bm0);

    SkBitmap bm1;
    bm1.allocPixels(SkImageInfo::MakeN32(256, 256, kUnpremul_SkAlphaType));
    bm0.readPixels(bm1.info(), bm1.getPixels(), bm1.rowBytes(), 0, 0);

    SkBitmap bm2;
    bm2.allocPixels(SkImageInfo::MakeN32(256, 256, kPremul_SkAlphaType));
    bm1.readPixels(bm2.info(), bm2.getPixels(), bm2.rowBytes(), 0, 0);

    REPORTER_ASSERT(reporter, equal(bm0, bm2));
}

DEF_TEST(image_from_encoded_alphatype_override, reporter) {
    sk_sp<SkData> data = GetResourceAsData("images/mandrill_32.png");

    // Ensure that we can decode the image when we specifically request premul or unpremul, but
    // not when we request kOpaque
    REPORTER_ASSERT(reporter, SkImages::DeferredFromEncodedData(data, kPremul_SkAlphaType));
    REPORTER_ASSERT(reporter, SkImages::DeferredFromEncodedData(data, kUnpremul_SkAlphaType));
    REPORTER_ASSERT(reporter, !SkImages::DeferredFromEncodedData(data, kOpaque_SkAlphaType));

    // Same tests as above, but using SkImageGenerators::MakeFromEncoded
    REPORTER_ASSERT(reporter, SkImageGenerators::MakeFromEncoded(data, kPremul_SkAlphaType));
    REPORTER_ASSERT(reporter, SkImageGenerators::MakeFromEncoded(data, kUnpremul_SkAlphaType));
    REPORTER_ASSERT(reporter, !SkImageGenerators::MakeFromEncoded(data, kOpaque_SkAlphaType));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void check_scaled_pixels(skiatest::Reporter* reporter, SkPixmap* pmap, uint32_t expected) {
    // Verify that all pixels contain the original test color
    for (auto y = 0; y < pmap->height(); ++y) {
        for (auto x = 0; x < pmap->width(); ++x) {
            uint32_t pixel = *pmap->addr32(x, y);
            if (pixel != expected) {
                ERRORF(reporter, "Expected scaled pixels to be the same. At %d,%d 0x%08x != 0x%08x",
                       x, y, pixel, expected);
                return;
            }
        }
    }
}

static void test_scale_pixels(skiatest::Reporter* reporter, const SkImage* image,
                              uint32_t expected) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width() * 2, image->height() * 2);

    // Make sure to test kDisallow first, so we don't just get a cache hit in that case
    for (auto chint : { SkImage::kDisallow_CachingHint, SkImage::kAllow_CachingHint }) {
        SkAutoPixmapStorage scaled;
        scaled.alloc(info);
        if (!image->scalePixels(scaled, SkSamplingOptions(SkFilterMode::kLinear), chint)) {
            ERRORF(reporter, "Failed to scale image");
            continue;
        }

        check_scaled_pixels(reporter, &scaled, expected);
    }
}

DEF_TEST(ImageScalePixels, reporter) {
    const SkPMColor pmRed = SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkColor red = SK_ColorRED;

    // Test raster image
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    surface->getCanvas()->clear(red);
    sk_sp<SkImage> rasterImage = surface->makeImageSnapshot();
    test_scale_pixels(reporter, rasterImage.get(), pmRed);

    // Test encoded image
    sk_sp<SkData> data = SkPngEncoder::Encode(nullptr, rasterImage.get(), {});
    sk_sp<SkImage> codecImage = SkImages::DeferredFromEncodedData(data);
    test_scale_pixels(reporter, codecImage.get(), pmRed);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageScalePixels_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const SkPMColor pmRed = SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkColor red = SK_ColorRED;

    SkImageInfo info = SkImageInfo::MakeN32Premul(16, 16);
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo, info);
    surface->getCanvas()->clear(red);
    sk_sp<SkImage> gpuImage = surface->makeImageSnapshot();
    test_scale_pixels(reporter, gpuImage.get(), pmRed);
}

static sk_sp<SkImage> any_image_will_do() {
    return GetResourceAsImage("images/mandrill_32.png");
}

DEF_TEST(Image_nonfinite_dst, reporter) {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10));
    auto img = any_image_will_do();

    for (SkScalar bad : { SK_ScalarInfinity, SK_ScalarNaN}) {
        for (int bits = 1; bits <= 15; ++bits) {
            SkRect dst = { 0, 0, 10, 10 };
            if (bits & 1) dst.fLeft = bad;
            if (bits & 2) dst.fTop = bad;
            if (bits & 4) dst.fRight = bad;
            if (bits & 8) dst.fBottom = bad;

            surf->getCanvas()->drawImageRect(img, dst, SkSamplingOptions());

            // we should draw nothing
            ToolUtils::PixelIter iter(surf.get());
            while (void* addr = iter.next()) {
                REPORTER_ASSERT(reporter, *(SkPMColor*)addr == 0);
            }
        }
    }
}

static sk_sp<SkImage> make_yuva_image(GrDirectContext* dContext) {
    SkAutoPixmapStorage pm;
    pm.alloc(SkImageInfo::Make(1, 1, kAlpha_8_SkColorType, kPremul_SkAlphaType));
    SkYUVAInfo yuvaInfo({1, 1},
                        SkYUVAInfo::PlaneConfig::kY_U_V,
                        SkYUVAInfo::Subsampling::k444,
                        kJPEG_Full_SkYUVColorSpace);
    const SkPixmap pmaps[] = {pm, pm, pm};
    auto yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pmaps);

    return SkImages::TextureFromYUVAPixmaps(dContext, yuvaPixmaps);
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ImageFlush, reporter, ctxInfo, CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto s = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, ii, 1, nullptr);

    s->getCanvas()->clear(SK_ColorRED);
    auto i0 = s->makeImageSnapshot();
    s->getCanvas()->clear(SK_ColorBLUE);
    auto i1 = s->makeImageSnapshot();
    s->getCanvas()->clear(SK_ColorGREEN);
    // Make a YUVA image.
    auto i2 = make_yuva_image(dContext);

    // Flush all the setup work we did above and then make little lambda that reports the flush
    // count delta since the last time it was called.
    dContext->flushAndSubmit();
    auto numSubmits =
            [dContext,
             submitCnt = dContext->priv().getGpu()->stats()->numSubmitToGpus()]() mutable {
        int curr = dContext->priv().getGpu()->stats()->numSubmitToGpus();
        int n = curr - submitCnt;
        submitCnt = curr;
        return n;
    };

    // Images aren't used therefore flush is ignored, but submit is still called.
    dContext->flushAndSubmit(i0);
    dContext->flushAndSubmit(i1);
    dContext->flushAndSubmit(i2);
    REPORTER_ASSERT(reporter, numSubmits() == 3);

    // Syncing forces the flush to happen even if the images aren't used.
    dContext->flush(i0);
    dContext->submit(GrSyncCpu::kYes);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    dContext->flush(i1);
    dContext->submit(GrSyncCpu::kYes);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    dContext->flush(i2);
    dContext->submit(GrSyncCpu::kYes);
    REPORTER_ASSERT(reporter, numSubmits() == 1);

    // Use image 1
    s->getCanvas()->drawImage(i1, 0, 0);
    // Flushing image 0 should do nothing, but submit is still called.
    dContext->flushAndSubmit(i0);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    // Flushing image 1 should flush.
    dContext->flushAndSubmit(i1);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    // Flushing image 2 should do nothing, but submit is still called.
    dContext->flushAndSubmit(i2);
    REPORTER_ASSERT(reporter, numSubmits() == 1);

    // Use image 2
    s->getCanvas()->drawImage(i2, 0, 0);
    // Flushing image 0 should do nothing, but submit is still called.
    dContext->flushAndSubmit(i0);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    // Flushing image 1 do nothing, but submit is still called.
    dContext->flushAndSubmit(i1);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    // Flushing image 2 should flush.
    dContext->flushAndSubmit(i2);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    REPORTER_ASSERT(reporter, static_cast<SkImage_GaneshYUVA*>(as_IB(i2.get()))->isTextureBacked());
    s->getCanvas()->drawImage(i2, 0, 0);
    // Flushing image 0 should do nothing, but submit is still called.
    dContext->flushAndSubmit(i0);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    // Flushing image 1 do nothing, but submit is still called.
    dContext->flushAndSubmit(i1);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
    // Flushing image 2 should flush.
    dContext->flushAndSubmit(i2);
    REPORTER_ASSERT(reporter, numSubmits() == 1);
}

constexpr SkM44 gCentripetalCatmulRom
    (0.0f/2, -1.0f/2,  2.0f/2, -1.0f/2,
     2.0f/2,  0.0f/2, -5.0f/2,  3.0f/2,
     0.0f/2,  1.0f/2,  4.0f/2, -3.0f/2,
     0.0f/2,  0.0f/2, -1.0f/2,  1.0f/2);

constexpr SkM44 gMitchellNetravali
    ( 1.0f/18, -9.0f/18,  15.0f/18,  -7.0f/18,
     16.0f/18,  0.0f/18, -36.0f/18,  21.0f/18,
      1.0f/18,  9.0f/18,  27.0f/18, -21.0f/18,
      0.0f/18,  0.0f/18,  -6.0f/18,   7.0f/18);

DEF_TEST(image_cubicresampler, reporter) {
    auto diff = [reporter](const SkM44& a, const SkM44& b) {
        const float tolerance = 0.000001f;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                float d = std::abs(a.rc(r, c) - b.rc(r, c));
                REPORTER_ASSERT(reporter, d <= tolerance);
            }
        }
    };

    diff(SkImageShader::CubicResamplerMatrix(1.0f/3, 1.0f/3), gMitchellNetravali);

    diff(SkImageShader::CubicResamplerMatrix(0, 1.0f/2), gCentripetalCatmulRom);
}

DEF_TEST(image_subset_encode_skbug_7752, reporter) {
    sk_sp<SkImage> image = GetResourceAsImage("images/mandrill_128.png");
    const int W = image->width();
    const int H = image->height();

    auto check_roundtrip = [&](sk_sp<SkImage> img) {
        auto img2 = SkImages::DeferredFromEncodedData(SkPngEncoder::Encode(nullptr, img.get(), {}));
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img.get(), img2.get()));
    };
    check_roundtrip(image); // should trivially pass
    check_roundtrip(image->makeSubset(nullptr, {0, 0, W/2, H/2}));
    check_roundtrip(image->makeSubset(nullptr, {W/2, H/2, W, H}));
    check_roundtrip(image->makeColorSpace(nullptr, SkColorSpace::MakeSRGBLinear()));
}
