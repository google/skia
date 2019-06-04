/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include <initializer_list>
#include <vector>

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrTexture.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuYUVA.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

using namespace sk_gpu_test;

SkImageInfo read_pixels_info(SkImage* image) {
    if (image->colorSpace()) {
        return SkImageInfo::MakeS32(image->width(), image->height(), image->alphaType());
    }

    return SkImageInfo::MakeN32(image->width(), image->height(), image->alphaType());
}

static void assert_equal(skiatest::Reporter* reporter, SkImage* a, const SkIRect* subsetA,
                         SkImage* b) {
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

    REPORTER_ASSERT(reporter, a->readPixels(pmapA, srcX, srcY));
    REPORTER_ASSERT(reporter, b->readPixels(pmapB, 0, 0));

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
    auto surface(SkSurface::MakeRaster(info));
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
    return SkImage::MakeRasterData(info, std::move(data), info.minRowBytes());
}
static sk_sp<SkImage> create_image_large(int maxTextureSize) {
    const SkImageInfo info = SkImageInfo::MakeN32(maxTextureSize + 1, 32, kOpaque_SkAlphaType);
    auto surface(SkSurface::MakeRaster(info));
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
    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(), SkISize::Make(10, 10),
                                    nullptr, nullptr, SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
};
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
    return SkImage::MakeFromRaster(SkPixmap(info, dataHolder->fData->data(), info.minRowBytes()),
                                   RasterDataHolder::Release, dataHolder);
}
static sk_sp<SkImage> create_codec_image() {
    SkImageInfo info;
    sk_sp<SkData> data(create_image_data(&info));
    SkBitmap bitmap;
    bitmap.installPixels(info, data->writable_data(), info.minRowBytes());
    auto src = SkEncodeBitmap(bitmap, SkEncodedImageFormat::kPNG, 100);
    return SkImage::MakeFromEncoded(std::move(src));
}
static sk_sp<SkImage> create_gpu_image(GrContext* context, bool withMips = false) {
    const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, 0,
                                             kBottomLeft_GrSurfaceOrigin, nullptr, withMips));
    draw_image_test_pattern(surface->getCanvas());
    return surface->makeImageSnapshot();
}

static void test_encode(skiatest::Reporter* reporter, SkImage* image) {
    const SkIRect ir = SkIRect::MakeXYWH(5, 5, 10, 10);
    sk_sp<SkData> origEncoded = image->encodeToData();
    REPORTER_ASSERT(reporter, origEncoded);
    REPORTER_ASSERT(reporter, origEncoded->size() > 0);

    sk_sp<SkImage> decoded(SkImage::MakeFromEncoded(origEncoded));
    if (!decoded) {
        ERRORF(reporter, "failed to decode image!");
        return;
    }
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, image, nullptr, decoded.get());

    // Now see if we can instantiate an image from a subset of the surface/origEncoded

    decoded = SkImage::MakeFromEncoded(origEncoded, &ir);
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, image, &ir, decoded.get());
}

DEF_TEST(ImageEncode, reporter) {
    test_encode(reporter, create_image().get());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageEncode_Gpu, reporter, ctxInfo) {
    test_encode(reporter, create_gpu_image(ctxInfo.grContext()).get());
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
    auto surface(SkSurface::MakeRasterN32Premul(100, 100));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(100, 100);
    canvas->drawImage(image, 0, 0);
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
    auto surface(SkSurface::MakeRaster(info));
    surface->getCanvas()->clear(0xFF00FF00);

    SkPMColor pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));   // init with values we don't expect
    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(2, 2);
    const size_t dstRowBytes = 2 * sizeof(SkPMColor);

    sk_sp<SkImage> image1(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image1->readPixels(dstInfo, pixels, dstRowBytes, 0, 0));
    for (size_t i = 0; i < SK_ARRAY_COUNT(pixels); ++i) {
        REPORTER_ASSERT(reporter, pixels[i] == green);
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(SK_ColorRED);

    surface->getCanvas()->drawRect(SkRect::MakeXYWH(1, 1, 1, 1), paint);

    sk_sp<SkImage> image2(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image2->readPixels(dstInfo, pixels, dstRowBytes, 0, 0));
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

    for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
        SkBitmap bm;
        rec[i].fMakeProc(&bm);

        sk_sp<SkImage> image(SkImage::MakeFromBitmap(bm));
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

#include "src/core/SkBitmapCache.h"

/*
 *  This tests the caching (and preemptive purge) of the raster equivalent of a gpu-image.
 *  We cache it for performance when drawing into a raster surface.
 *
 *  A cleaner test would know if each drawImage call triggered a read-back from the gpu,
 *  but we don't have that facility (at the moment) so we use a little internal knowledge
 *  of *how* the raster version is cached, and look for that.
 */
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkImage_Gpu2Cpu, reporter, ctxInfo) {
    SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.grContext()));
    const auto desc = SkBitmapCacheDesc::Make(image.get());

    auto surface(SkSurface::MakeRaster(info));

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
            SkDebugf("SkImage_Gpu2Cpu : cachedBitmap was already purged\n");
        }
    }

    image.reset(nullptr);
    {
        SkBitmap cachedBitmap;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(desc, &cachedBitmap));
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkImage_makeTextureImage, reporter, contextInfo) {
    GrContext* context = contextInfo.grContext();
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
        [context] { return create_gpu_image(context); },
        // Create a texture image with mips
        //[context] { return create_gpu_image(context, true); },
        // Create a texture image in a another GrContext.
        [otherContextInfo] {
            auto restore = otherContextInfo.testContext()->makeCurrentAndAutoRestore();
            sk_sp<SkImage> otherContextImage = create_gpu_image(otherContextInfo.grContext());
            otherContextInfo.grContext()->flush();
            return otherContextImage;
        }
    };

    sk_sp<SkColorSpace> dstColorSpaces[] ={
        nullptr,
        SkColorSpace::MakeSRGB(),
    };

    for (auto& dstColorSpace : dstColorSpaces) {
        for (auto mipMapped : {GrMipMapped::kNo, GrMipMapped::kYes}) {
            for (auto factory : imageFactories) {
                sk_sp<SkImage> image(factory());
                if (!image) {
                    ERRORF(reporter, "Error creating image.");
                    continue;
                }

                sk_sp<SkImage> texImage(image->makeTextureImage(context, dstColorSpace.get(),
                                                                mipMapped));
                if (!texImage) {
                    GrContext* imageContext = as_IB(image)->context();

                    // We expect to fail if image comes from a different GrContext.
                    if (!image->isTextureBacked() || imageContext == context) {
                        ERRORF(reporter, "makeTextureImage failed.");
                    }
                    continue;
                }
                if (!texImage->isTextureBacked()) {
                    ERRORF(reporter, "makeTextureImage returned non-texture image.");
                    continue;
                }
                if (GrMipMapped::kYes == mipMapped &&
                    as_IB(texImage)->peekProxy()->mipMapped() != mipMapped &&
                    context->priv().caps()->mipMapSupport()) {
                    ERRORF(reporter, "makeTextureImage returned non-mipmapped texture.");
                    continue;
                }
                if (image->isTextureBacked()) {
                    GrSurfaceProxy* origProxy = as_IB(image)->peekProxy();
                    GrSurfaceProxy* copyProxy = as_IB(texImage)->peekProxy();

                    if (origProxy->underlyingUniqueID() != copyProxy->underlyingUniqueID()) {
                        SkASSERT(origProxy->asTextureProxy());
                        if (GrMipMapped::kNo == mipMapped ||
                            GrMipMapped::kYes == origProxy->asTextureProxy()->mipMapped()) {
                            ERRORF(reporter, "makeTextureImage made unnecessary texture copy.");
                        }
                    }
                }
                if (image->width() != texImage->width() || image->height() != texImage->height()) {
                    ERRORF(reporter, "makeTextureImage changed the image size.");
                }
                if (image->alphaType() != texImage->alphaType()) {
                    ERRORF(reporter, "makeTextureImage changed image alpha type.");
                }
            }
        }
        context->flush();
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkImage_makeNonTextureImage, reporter, contextInfo) {
    GrContext* context = contextInfo.grContext();

    std::function<sk_sp<SkImage>()> imageFactories[] = {
        create_image,
        create_codec_image,
        create_data_image,
        create_picture_image,
        [context] { return create_gpu_image(context); },
    };
    SkColorSpace* legacyColorSpace = nullptr;
    for (auto factory : imageFactories) {
        sk_sp<SkImage> image = factory();
        if (!image->isTextureBacked()) {
            REPORTER_ASSERT(reporter, image->makeNonTextureImage().get() == image.get());
            if (!(image = image->makeTextureImage(context, legacyColorSpace))) {
                continue;
            }
        }
        auto rasterImage = image->makeNonTextureImage();
        if (!rasterImage) {
            ERRORF(reporter, "makeNonTextureImage failed for texture-backed image.");
        }
        REPORTER_ASSERT(reporter, !rasterImage->isTextureBacked());
        assert_equal(reporter, image.get(), nullptr, rasterImage.get());
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrContext_colorTypeSupportedAsImage, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    static constexpr int kSize = 10;

    for (int ct = 0; ct < kLastEnum_SkColorType; ++ct) {
        SkColorType colorType = static_cast<SkColorType>(ct);
        bool can = context->colorTypeSupportedAsImage(colorType);

        GrBackendTexture backendTex = context->createBackendTexture(
                kSize, kSize, colorType, SkColors::kTransparent,
                GrMipMapped::kNo, GrRenderable::kNo);

        auto img = SkImage::MakeFromTexture(context, backendTex, kTopLeft_GrSurfaceOrigin,
                                            colorType, kOpaque_SkAlphaType, nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(img),
                        "colorTypeSupportedAsImage:%d, actual:%d, ct:%d", can, SkToBool(img),
                        colorType);

        img.reset();
        context->flush();
        context->deleteBackendTexture(backendTex);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(UnpremulTextureImage, reporter, ctxInfo) {
    SkBitmap bmp;
    bmp.allocPixels(
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr));
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            *bmp.getAddr32(x, y) =
                    SkColorSetARGB((U8CPU)y, 255 - (U8CPU)y, (U8CPU)x, 255 - (U8CPU)x);
        }
    }
    auto texImage = SkImage::MakeFromBitmap(bmp)->makeTextureImage(ctxInfo.grContext(), nullptr);
    if (!texImage || texImage->alphaType() != kUnpremul_SkAlphaType) {
        ERRORF(reporter, "Failed to make unpremul texture image.");
        return;
    }
    // The GPU backend always unpremuls the values stored in the texture because it assumes they
    // are premul values. (skbug.com/7580).
    if (false) {
        SkBitmap unpremul;
        unpremul.allocPixels(SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType,
                                               kUnpremul_SkAlphaType, nullptr));
        if (!texImage->readPixels(unpremul.info(), unpremul.getPixels(), unpremul.rowBytes(), 0,
                                  0)) {
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
    }
    SkBitmap premul;
    premul.allocPixels(
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr));
    if (!texImage->readPixels(premul.info(), premul.getPixels(), premul.rowBytes(), 0, 0)) {
        ERRORF(reporter, "Unpremul readback failed.");
        return;
    }
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            // Treat bmp's color as a pm color even though it may be the r/b swap of a PM color.
            // SkPremultiplyColor acts the same on both channels.
            uint32_t origColor = SkPreMultiplyColor(*bmp.getAddr32(x, y));
            int32_t origA = (origColor >> 24) & 0xff;
            int32_t origB = (origColor >> 16) & 0xff;
            int32_t origG = (origColor >>  8) & 0xff;
            int32_t origR = (origColor >>  0) & 0xff;
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
                ERRORF(reporter, "unpremul(0x%08x)->premul(0x%08x) at %d, %d.",
                       *bmp.getAddr32(x, y), *premul.getAddr32(x, y), x, y);
                return;
            }
        }
    }
}

DEF_GPUTEST(AbandonedContextImage, reporter, options) {
    using Factory = sk_gpu_test::GrContextFactory;
    for (int ct = 0; ct < Factory::kContextTypeCnt; ++ct) {
        auto type = static_cast<Factory::ContextType>(ct);
        std::unique_ptr<Factory> factory(new Factory);
        if (!factory->get(type)) {
            continue;
        }

        sk_sp<SkImage> img;
        auto gsurf = SkSurface::MakeRenderTarget(
                factory->get(type), SkBudgeted::kYes,
                SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType), 1,
                nullptr);
        if (!gsurf) {
            continue;
        }
        img = gsurf->makeImageSnapshot();
        gsurf.reset();

        auto rsurf = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(100, 100));

        REPORTER_ASSERT(reporter, img->isValid(factory->get(type)));
        REPORTER_ASSERT(reporter, img->isValid(rsurf->getCanvas()->getGrContext()));

        factory->get(type)->abandonContext();
        REPORTER_ASSERT(reporter, !img->isValid(factory->get(type)));
        REPORTER_ASSERT(reporter, !img->isValid(rsurf->getCanvas()->getGrContext()));
        // This shouldn't crash.
        rsurf->getCanvas()->drawImage(img, 0, 0);

        // Give up all other refs on GrContext.
        factory.reset(nullptr);
        REPORTER_ASSERT(reporter, !img->isValid(rsurf->getCanvas()->getGrContext()));
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
    REPORTER_ASSERT(reporter, nullptr == SkImage::MakeRasterCopy(pmap));
    REPORTER_ASSERT(reporter, nullptr == SkImage::MakeRasterData(info, nullptr, 0));
    REPORTER_ASSERT(reporter, nullptr == SkImage::MakeFromRaster(pmap, nullptr, nullptr));
    REPORTER_ASSERT(reporter, nullptr == SkImage::MakeFromGenerator(
                                                            skstd::make_unique<EmptyGenerator>()));
}

DEF_TEST(ImageDataRef, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.computeByteSize(rowBytes);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    REPORTER_ASSERT(reporter, data->unique());
    sk_sp<SkImage> image = SkImage::MakeRasterData(info, data, rowBytes);
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

static void image_test_read_pixels(skiatest::Reporter* reporter, SkImage* image) {
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
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, 0));

    // out-of-bounds should fail
    info = SkImageInfo::MakeN32Premul(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, -w, 0));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, -h));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, image->width(), 0));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, image->height()));

    // top-left should succeed
    sk_memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, 0, 0));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // bottom-right should succeed
    sk_memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - w, image->height() - h));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // partial top-left should succeed
    sk_memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, -1, -1));
    REPORTER_ASSERT(reporter, pixels[3] == expected);
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h - 1, notExpected));

    // partial bottom-right should succeed
    sk_memset32(pixels, notExpected, w*h);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - 1, image->height() - 1));
    REPORTER_ASSERT(reporter, pixels[0] == expected);
    REPORTER_ASSERT(reporter, has_pixels(&pixels[1], w*h - 1, notExpected));
}
DEF_TEST(ImageReadPixels, reporter) {
    sk_sp<SkImage> image(create_image());
    image_test_read_pixels(reporter, image.get());

    image = create_data_image();
    image_test_read_pixels(reporter, image.get());

    RasterDataHolder dataHolder;
    image = create_rasterproc_image(&dataHolder);
    image_test_read_pixels(reporter, image.get());
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image = create_codec_image();
    image_test_read_pixels(reporter, image.get());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageReadPixels_Gpu, reporter, ctxInfo) {
    image_test_read_pixels(reporter, create_gpu_image(ctxInfo.grContext()).get());
}

static void check_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image,
                                const SkBitmap& bitmap) {
    REPORTER_ASSERT(reporter, image->width() == bitmap.width());
    REPORTER_ASSERT(reporter, image->height() == bitmap.height());
    REPORTER_ASSERT(reporter, image->alphaType() == bitmap.alphaType());

    REPORTER_ASSERT(reporter, bitmap.isImmutable());

    REPORTER_ASSERT(reporter, bitmap.getPixels());

    const SkImageInfo info = SkImageInfo::MakeN32(1, 1, bitmap.alphaType());
    SkPMColor imageColor;
    REPORTER_ASSERT(reporter, image->readPixels(info, &imageColor, sizeof(SkPMColor), 0, 0));
    REPORTER_ASSERT(reporter, imageColor == *bitmap.getAddr32(0, 0));
}

static void test_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image) {
    if (!image) {
        ERRORF(reporter, "Failed to create image.");
        return;
    }
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter, image->asLegacyBitmap(&bitmap));
    check_legacy_bitmap(reporter, image, bitmap);

    // Test subsetting to exercise the rowBytes logic.
    SkBitmap tmp;
    REPORTER_ASSERT(reporter, bitmap.extractSubset(&tmp, SkIRect::MakeWH(image->width() / 2,
                                                                         image->height() / 2)));
    sk_sp<SkImage> subsetImage(SkImage::MakeFromBitmap(tmp));
    REPORTER_ASSERT(reporter, subsetImage.get());

    SkBitmap subsetBitmap;
    REPORTER_ASSERT(reporter, subsetImage->asLegacyBitmap(&subsetBitmap));
    check_legacy_bitmap(reporter, subsetImage.get(), subsetBitmap);
}
DEF_TEST(ImageLegacyBitmap, reporter) {
    sk_sp<SkImage> image(create_image());
    test_legacy_bitmap(reporter, image.get());

    image = create_data_image();
    test_legacy_bitmap(reporter, image.get());

    RasterDataHolder dataHolder;
    image = create_rasterproc_image(&dataHolder);
    test_legacy_bitmap(reporter, image.get());
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image = create_codec_image();
    test_legacy_bitmap(reporter, image.get());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageLegacyBitmap_Gpu, reporter, ctxInfo) {
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.grContext()));
    test_legacy_bitmap(reporter, image.get());
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
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImagePeek_Gpu, reporter, ctxInfo) {
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.grContext()));
    test_peek(reporter, image.get(), false);
}

struct TextureReleaseChecker {
    TextureReleaseChecker() : fReleaseCount(0) {}
    int fReleaseCount;
    static void Release(void* self) {
        static_cast<TextureReleaseChecker*>(self)->fReleaseCount++;
    }
};

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkImage_NewFromTextureRelease, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();

    SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, SkColorType::kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    GrBackendTexture backendTex;

    if (!create_backend_texture(ctx, &backendTex, ii, GrMipMapped::kNo, SK_ColorRED,
                                GrRenderable::kNo)) {
        ERRORF(reporter, "couldn't create backend texture\n");
    }

    TextureReleaseChecker releaseChecker;
    GrSurfaceOrigin texOrigin = kBottomLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(
        SkImage::MakeFromTexture(ctx, backendTex, texOrigin, kRGBA_8888_SkColorType,
                                 kPremul_SkAlphaType, nullptr,
                                 TextureReleaseChecker::Release, &releaseChecker));

    GrSurfaceOrigin readBackOrigin;
    GrBackendTexture readBackBackendTex = refImg->getBackendTexture(false, &readBackOrigin);
    readBackBackendTex.setPixelConfig(kRGBA_8888_GrPixelConfig);
    if (!GrBackendTexture::TestingOnly_Equals(readBackBackendTex, backendTex)) {
        ERRORF(reporter, "backend mismatch\n");
    }
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(readBackBackendTex, backendTex));
    if (readBackOrigin != texOrigin) {
        ERRORF(reporter, "origin mismatch %d %d\n", readBackOrigin, texOrigin);
    }
    REPORTER_ASSERT(reporter, readBackOrigin == texOrigin);

    // Now exercise the release proc
    REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
    refImg.reset(nullptr); // force a release of the image
    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);

    delete_backend_texture(ctx, backendTex);
}

static void test_cross_context_image(skiatest::Reporter* reporter, const GrContextOptions& options,
                                     const char* testName,
                                     std::function<sk_sp<SkImage>(GrContext*)> imageMaker) {
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory testFactory(options);
        GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
        ContextInfo ctxInfo = testFactory.getContextInfo(ctxType);
        GrContext* ctx = ctxInfo.grContext();
        if (!ctx) {
            continue;
        }

        // If we don't have proper support for this feature, the factory will fallback to returning
        // codec-backed images. Those will "work", but some of our checks will fail because we
        // expect the cross-context images not to work on multiple contexts at once.
        if (!ctx->priv().caps()->crossContextTextureSupport()) {
            continue;
        }

        // We test three lifetime patterns for a single context:
        // 1) Create image, free image
        // 2) Create image, draw, flush, free image
        // 3) Create image, draw, free image, flush
        // ... and then repeat the last two patterns with drawing on a second* context:
        // 4) Create image, draw*, flush*, free image
        // 5) Create image, draw*, free iamge, flush*

        // Case #1: Create image, free image
        {
            sk_sp<SkImage> refImg(imageMaker(ctx));
            refImg.reset(nullptr); // force a release of the image
        }

        SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
        if (!surface) {
            ERRORF(reporter, "SkSurface::MakeRenderTarget failed for %s.", testName);
            continue;
        }

        SkCanvas* canvas = surface->getCanvas();

        // Case #2: Create image, draw, flush, free image
        {
            sk_sp<SkImage> refImg(imageMaker(ctx));

            canvas->drawImage(refImg, 0, 0);
            surface->flush();

            refImg.reset(nullptr); // force a release of the image
        }

        // Case #3: Create image, draw, free image, flush
        {
            sk_sp<SkImage> refImg(imageMaker(ctx));

            canvas->drawImage(refImg, 0, 0);
            refImg.reset(nullptr); // force a release of the image

            surface->flush();
        }

        // Configure second context
        sk_gpu_test::TestContext* testContext = ctxInfo.testContext();

        ContextInfo otherContextInfo = testFactory.getSharedContextInfo(ctx);
        GrContext* otherCtx = otherContextInfo.grContext();
        sk_gpu_test::TestContext* otherTestContext = otherContextInfo.testContext();

        // Creating a context in a share group may fail
        if (!otherCtx) {
            continue;
        }

        surface = SkSurface::MakeRenderTarget(otherCtx, SkBudgeted::kNo, info);
        canvas = surface->getCanvas();

        // Case #4: Create image, draw*, flush*, free image
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(imageMaker(ctx));

            otherTestContext->makeCurrent();
            canvas->drawImage(refImg, 0, 0);
            surface->flush();

            testContext->makeCurrent();
            refImg.reset(nullptr); // force a release of the image
        }

        // Case #5: Create image, draw*, free image, flush*
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(imageMaker(ctx));

            otherTestContext->makeCurrent();
            canvas->drawImage(refImg, 0, 0);

            testContext->makeCurrent();
            refImg.reset(nullptr); // force a release of the image

            otherTestContext->makeCurrent();
            surface->flush();

            // This is specifically here for vulkan to guarantee the command buffer will finish
            // which is when we call the ReleaseProc.
            otherCtx->priv().getGpu()->testingOnly_flushGpuAndSync();
        }

        // Case #6: Verify that only one context can be using the image at a time
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(imageMaker(ctx));

            // Any context should be able to borrow the texture at this point
            sk_sp<GrTextureProxy> proxy = as_IB(refImg)->asTextureProxyRef(
                    ctx, GrSamplerState::ClampNearest(), nullptr);
            REPORTER_ASSERT(reporter, proxy);

            // But once it's borrowed, no other context should be able to borrow
            otherTestContext->makeCurrent();
            sk_sp<GrTextureProxy> otherProxy = as_IB(refImg)->asTextureProxyRef(
                    otherCtx, GrSamplerState::ClampNearest(), nullptr);
            REPORTER_ASSERT(reporter, !otherProxy);

            // Original context (that's already borrowing) should be okay
            testContext->makeCurrent();
            sk_sp<GrTextureProxy> proxySecondRef = as_IB(refImg)->asTextureProxyRef(
                    ctx, GrSamplerState::ClampNearest(), nullptr);
            REPORTER_ASSERT(reporter, proxySecondRef);

            // Release first ref from the original context
            proxy.reset(nullptr);

            // We released one proxy but not the other from the current borrowing context. Make sure
            // a new context is still not able to borrow the texture.
            otherTestContext->makeCurrent();
            otherProxy = as_IB(refImg)->asTextureProxyRef(otherCtx, GrSamplerState::ClampNearest(),
                                                          nullptr);
            REPORTER_ASSERT(reporter, !otherProxy);

            // Release second ref from the original context
            testContext->makeCurrent();
            proxySecondRef.reset(nullptr);

            // Now we should be able to borrow the texture from the other context
            otherTestContext->makeCurrent();
            otherProxy = as_IB(refImg)->asTextureProxyRef(otherCtx, GrSamplerState::ClampNearest(),
                                                          nullptr);
            REPORTER_ASSERT(reporter, otherProxy);

            // Release everything
            otherProxy.reset(nullptr);
            refImg.reset(nullptr);
        }
    }
}

DEF_GPUTEST(SkImage_MakeCrossContextFromEncodedRelease, reporter, options) {
    sk_sp<SkData> data = GetResourceAsData("images/mandrill_128.png");
    if (!data) {
       ERRORF(reporter, "missing resource");
       return;
    }

    test_cross_context_image(reporter, options, "SkImage_MakeCrossContextFromEncodedRelease",
                             [&data](GrContext* ctx) {
        return SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr);
    });
}

DEF_GPUTEST(SkImage_MakeCrossContextFromPixmapRelease, reporter, options) {
    SkBitmap bitmap;
    SkPixmap pixmap;
    if (!GetResourceAsBitmap("images/mandrill_128.png", &bitmap) || !bitmap.peekPixels(&pixmap)) {
        ERRORF(reporter, "missing resource");
        return;
    }
    test_cross_context_image(reporter, options, "SkImage_MakeCrossContextFromPixmapRelease",
                             [&pixmap](GrContext* ctx) {
        return SkImage::MakeCrossContextFromPixmap(ctx, pixmap, false, nullptr);
    });
}

DEF_GPUTEST(SkImage_CrossContextGrayAlphaConfigs, reporter, options) {

    for (SkColorType ct : { kGray_8_SkColorType, kAlpha_8_SkColorType }) {
        SkAutoPixmapStorage pixmap;
        pixmap.alloc(SkImageInfo::Make(4, 4, ct, kPremul_SkAlphaType));

        for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
            GrContextFactory testFactory(options);
            GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
            ContextInfo ctxInfo = testFactory.getContextInfo(ctxType);
            GrContext* ctx = ctxInfo.grContext();
            if (!ctx || !ctx->priv().caps()->crossContextTextureSupport()) {
                continue;
            }

            sk_sp<SkImage> image = SkImage::MakeCrossContextFromPixmap(ctx, pixmap, false, nullptr);
            REPORTER_ASSERT(reporter, image);

            sk_sp<GrTextureProxy> proxy = as_IB(image)->asTextureProxyRef(
                ctx, GrSamplerState::ClampNearest(), nullptr);
            REPORTER_ASSERT(reporter, proxy);

            bool expectAlpha = kAlpha_8_SkColorType == ct;
            REPORTER_ASSERT(reporter, expectAlpha == GrPixelConfigIsAlphaOnly(proxy->config()));
        }
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(makeBackendTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    sk_gpu_test::TestContext* testContext = ctxInfo.testContext();
    sk_sp<GrContextThreadSafeProxy> proxy = context->threadSafeProxy();

    GrContextFactory otherFactory;
    ContextInfo otherContextInfo = otherFactory.getContextInfo(ctxInfo.type());

    testContext->makeCurrent();
    REPORTER_ASSERT(reporter, proxy);
    auto createLarge = [context] {
        return create_image_large(context->priv().caps()->maxTextureSize());
    };
    struct {
        std::function<sk_sp<SkImage> ()>                      fImageFactory;
        bool                                                  fExpectation;
        bool                                                  fCanTakeDirectly;
    } testCases[] = {
        { create_image, true, false },
        { create_codec_image, true, false },
        { create_data_image, true, false },
        { create_picture_image, true, false },
        { [context] { return create_gpu_image(context); }, true, true },
        // Create a texture image in a another GrContext.
        { [otherContextInfo] {
            auto restore = otherContextInfo.testContext()->makeCurrentAndAutoRestore();
            sk_sp<SkImage> otherContextImage = create_gpu_image(otherContextInfo.grContext());
            otherContextInfo.grContext()->flush();
            return otherContextImage;
          }, false, false },
        // Create an image that is too large to be texture backed.
        { createLarge, false, false }
    };

    for (auto testCase : testCases) {
        sk_sp<SkImage> image(testCase.fImageFactory());
        if (!image) {
            ERRORF(reporter, "Failed to create image!");
            continue;
        }

        GrBackendTexture origBackend = image->getBackendTexture(true);
        if (testCase.fCanTakeDirectly) {
            SkASSERT(origBackend.isValid());
        }

        GrBackendTexture newBackend;
        SkImage::BackendTextureReleaseProc proc;
        bool result = SkImage::MakeBackendTextureFromSkImage(context, std::move(image),
                                                             &newBackend, &proc);
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

        context->flush();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> create_picture_image(sk_sp<SkColorSpace> space) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);
    canvas->clear(SK_ColorCYAN);
    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(), SkISize::Make(10, 10),
                                    nullptr, nullptr, SkImage::BitDepth::kU8, std::move(space));
};

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
    image = SkImage::MakeFromBitmap(bitmap);
    REPORTER_ASSERT(r, SkColorSpace::Equals(rec2020.get(), image->colorSpace()));

    sk_sp<SkSurface> surface = SkSurface::MakeRaster(
            SkImageInfo::MakeN32Premul(SkISize::Make(10, 10)));
    image = surface->makeImageSnapshot();
    REPORTER_ASSERT(r, nullptr == image->colorSpace());

    surface = SkSurface::MakeRaster(info);
    image = surface->makeImageSnapshot();
    REPORTER_ASSERT(r, SkColorSpace::Equals(rec2020.get(), image->colorSpace()));
}

DEF_TEST(Image_makeColorSpace, r) {
    sk_sp<SkColorSpace> p3 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3);
    skcms_TransferFunction fn;
    fn.a = 1.f; fn.b = 0.f; fn.c = 0.f; fn.d = 0.f; fn.e = 0.f; fn.f = 0.f; fn.g = 1.8f;
    sk_sp<SkColorSpace> adobeGamut = SkColorSpace::MakeRGB(fn, SkNamedGamut::kAdobeRGB);

    SkBitmap srgbBitmap;
    srgbBitmap.allocPixels(SkImageInfo::MakeS32(1, 1, kOpaque_SkAlphaType));
    *srgbBitmap.getAddr32(0, 0) = SkSwizzle_RGBA_to_PMColor(0xFF604020);
    srgbBitmap.setImmutable();
    sk_sp<SkImage> srgbImage = SkImage::MakeFromBitmap(srgbBitmap);
    sk_sp<SkImage> p3Image = srgbImage->makeColorSpace(p3);
    SkBitmap p3Bitmap;
    bool success = p3Image->asLegacyBitmap(&p3Bitmap);

    auto almost_equal = [](int a, int b) { return SkTAbs(a - b) <= 2; };

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x28, SkGetPackedR32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x40, SkGetPackedG32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x5E, SkGetPackedB32(*p3Bitmap.getAddr32(0, 0))));

    sk_sp<SkImage> adobeImage = srgbImage->makeColorSpace(adobeGamut);
    SkBitmap adobeBitmap;
    success = adobeImage->asLegacyBitmap(&adobeBitmap);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x21, SkGetPackedR32(*adobeBitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x31, SkGetPackedG32(*adobeBitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x4C, SkGetPackedB32(*adobeBitmap.getAddr32(0, 0))));

    srgbImage = GetResourceAsImage("images/1x1.png");
    p3Image = srgbImage->makeColorSpace(p3);
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
            int c = SkTMin(a, r);
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

    auto img0 = SkImage::MakeFromBitmap(bm0);
    sk_sp<SkData> data = img0->encodeToData(SkEncodedImageFormat::kPNG, 100);
    auto img1 = SkImage::MakeFromEncoded(data);

    SkBitmap bm1;
    bm1.allocPixels(SkImageInfo::MakeN32(256, 256, kPremul_SkAlphaType));
    img1->readPixels(bm1.info(), bm1.getPixels(), bm1.rowBytes(), 0, 0);

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
        if (!image->scalePixels(scaled, kLow_SkFilterQuality, chint)) {
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
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    surface->getCanvas()->clear(red);
    sk_sp<SkImage> rasterImage = surface->makeImageSnapshot();
    test_scale_pixels(reporter, rasterImage.get(), pmRed);

    // Test encoded image
    sk_sp<SkData> data = rasterImage->encodeToData();
    sk_sp<SkImage> codecImage = SkImage::MakeFromEncoded(data);
    test_scale_pixels(reporter, codecImage.get(), pmRed);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageScalePixels_Gpu, reporter, ctxInfo) {
    const SkPMColor pmRed = SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkColor red = SK_ColorRED;

    SkImageInfo info = SkImageInfo::MakeN32Premul(16, 16);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kNo,
                                                           info);
    surface->getCanvas()->clear(red);
    sk_sp<SkImage> gpuImage = surface->makeImageSnapshot();
    test_scale_pixels(reporter, gpuImage.get(), pmRed);
}

static sk_sp<SkImage> any_image_will_do() {
    return GetResourceAsImage("images/mandrill_32.png");
}

DEF_TEST(Image_nonfinite_dst, reporter) {
    auto surf = SkSurface::MakeRasterN32Premul(10, 10);
    auto img = any_image_will_do();
    SkPaint paint;

    for (SkScalar bad : { SK_ScalarInfinity, SK_ScalarNaN}) {
        for (int bits = 1; bits <= 15; ++bits) {
            SkRect dst = { 0, 0, 10, 10 };
            if (bits & 1) dst.fLeft = bad;
            if (bits & 2) dst.fTop = bad;
            if (bits & 4) dst.fRight = bad;
            if (bits & 8) dst.fBottom = bad;

            surf->getCanvas()->drawImageRect(img, dst, &paint);

            // we should draw nothing
            ToolUtils::PixelIter iter(surf.get());
            while (void* addr = iter.next()) {
                REPORTER_ASSERT(reporter, *(SkPMColor*)addr == 0);
            }
        }
    }
}

static sk_sp<SkImage> make_yuva_image(GrContext* c) {
    SkAutoPixmapStorage pm;
    pm.alloc(SkImageInfo::Make(1, 1, kAlpha_8_SkColorType, kPremul_SkAlphaType));
    const SkPixmap pmaps[] = {pm, pm, pm, pm};
    SkYUVAIndex indices[] = {{0, SkColorChannel::kA},
                             {1, SkColorChannel::kA},
                             {2, SkColorChannel::kA},
                             {3, SkColorChannel::kA}};

    return SkImage::MakeFromYUVAPixmaps(c, kJPEG_SkYUVColorSpace, pmaps, indices,
                                        SkISize::Make(1, 1), kTopLeft_GrSurfaceOrigin, false);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ImageFlush, reporter, ctxInfo) {
    auto c = ctxInfo.grContext();
    auto ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto s = SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kYes, ii, 1, nullptr);

    s->getCanvas()->clear(SK_ColorRED);
    auto i0 = s->makeImageSnapshot();
    s->getCanvas()->clear(SK_ColorBLUE);
    auto i1 = s->makeImageSnapshot();
    s->getCanvas()->clear(SK_ColorGREEN);
    // Make a YUVA image.
    auto i2 = make_yuva_image(c);

    // Flush all the setup work we did above and then make little lambda that reports the flush
    // count delta since the last time it was called.
    c->flush();
    auto numFlushes = [c, flushCnt = c->priv().getGpu()->stats()->numFinishFlushes()]() mutable {
        int curr = c->priv().getGpu()->stats()->numFinishFlushes();
        int n = curr - flushCnt;
        flushCnt = curr;
        return n;
    };

    // Images aren't used therefore flush is ignored.
    i0->flush(c);
    i1->flush(c);
    i2->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);

    // Syncing forces the flush to happen even if the images aren't used.
    GrFlushInfo syncInfo;
    syncInfo.fFlags = kSyncCpu_GrFlushFlag;
    i0->flush(c, syncInfo);
    REPORTER_ASSERT(reporter, numFlushes() == 1);
    i1->flush(c, syncInfo);
    REPORTER_ASSERT(reporter, numFlushes() == 1);
    i2->flush(c, syncInfo);
    REPORTER_ASSERT(reporter, numFlushes() == 1);

    // Use image 1
    s->getCanvas()->drawImage(i1, 0, 0);
    // Flushing image 0 should do nothing.
    i0->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 1 should flush.
    i1->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 1);
    // Flushing image 2 should do nothing.
    i2->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);

    // Use image 2
    s->getCanvas()->drawImage(i2, 0, 0);
    // Flushing image 0 should do nothing.
    i0->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 1 do nothing.
    i1->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 2 should flush.
    i2->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 1);
    // Since we just did a simple image draw it should not have been flattened.
    REPORTER_ASSERT(reporter,
                    !static_cast<SkImage_GpuYUVA*>(as_IB(i2.get()))->testingOnly_IsFlattened());
    REPORTER_ASSERT(reporter, static_cast<SkImage_GpuYUVA*>(as_IB(i2.get()))->isTextureBacked());

    // Flatten it and repeat.
    as_IB(i2.get())->asTextureProxyRef(c);
    REPORTER_ASSERT(reporter,
                    static_cast<SkImage_GpuYUVA*>(as_IB(i2.get()))->testingOnly_IsFlattened());
    REPORTER_ASSERT(reporter, static_cast<SkImage_GpuYUVA*>(as_IB(i2.get()))->isTextureBacked());
    s->getCanvas()->drawImage(i2, 0, 0);
    // Flushing image 0 should do nothing.
    i0->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 1 do nothing.
    i1->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 2 should flush.
    i2->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 1);

    // Test case where flatten happens before the first flush.
    i2 = make_yuva_image(c);
    // On some systems where preferVRAMUseOverFlushes is false (ANGLE on Windows) the above may
    // actually flush in order to make textures for the YUV planes. TODO: Remove this when we
    // make the YUVA planes from backend textures rather than pixmaps that GrContext must upload.
    // Calling numFlushes rebases the flush count from here.
    numFlushes();
    as_IB(i2.get())->asTextureProxyRef(c);
    REPORTER_ASSERT(reporter,
                    static_cast<SkImage_GpuYUVA*>(as_IB(i2.get()))->testingOnly_IsFlattened());
    REPORTER_ASSERT(reporter, static_cast<SkImage_GpuYUVA*>(as_IB(i2.get()))->isTextureBacked());
    s->getCanvas()->drawImage(i2, 0, 0);
    // Flushing image 0 should do nothing.
    i0->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 1 do nothing.
    i1->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 0);
    // Flushing image 2 should flush.
    i2->flush(c);
    REPORTER_ASSERT(reporter, numFlushes() == 1);
}
