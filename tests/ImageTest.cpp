/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include <initializer_list>
#include <vector>

#include "SkAutoPixmapStorage.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkMakeUnique.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPixelSerializer.h"
#include "SkRRect.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkUtils.h"
#include "Test.h"

#include "Resources.h"
#include "sk_tool_utils.h"

#if SK_SUPPORT_GPU
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceCache.h"
#include "GrTest.h"
#include "GrTexture.h"
#endif

using namespace sk_gpu_test;

SkImageInfo read_pixels_info(SkImage* image) {
    if (as_IB(image)->onImageInfo().colorSpace()) {
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
#if SK_SUPPORT_GPU // not gpu-specific but currently only used in GPU tests
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
#endif
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
    sk_sp<SkData> src(sk_tool_utils::EncodeImageToData(bitmap, SkEncodedImageFormat::kPNG, 100));
    return SkImage::MakeFromEncoded(std::move(src));
}
#if SK_SUPPORT_GPU
static sk_sp<SkImage> create_gpu_image(GrContext* context) {
    const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    draw_image_test_pattern(surface->getCanvas());
    return surface->makeImageSnapshot();
}
#endif

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

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageEncode_Gpu, reporter, ctxInfo) {
    test_encode(reporter, create_gpu_image(ctxInfo.grContext()).get());
}
#endif

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

namespace {

const char* kSerializedData = "serialized";

class MockSerializer : public SkPixelSerializer {
public:
    MockSerializer(sk_sp<SkData> (*func)()) : fFunc(func), fDidEncode(false) { }

    bool didEncode() const { return fDidEncode; }

protected:
    bool onUseEncodedData(const void*, size_t) override {
        return false;
    }

    SkData* onEncode(const SkPixmap&) override {
        fDidEncode = true;
        return fFunc().release();
    }

private:
    sk_sp<SkData> (*fFunc)();
    bool fDidEncode;

    typedef SkPixelSerializer INHERITED;
};

} // anonymous namespace

// Test that SkImage encoding observes custom pixel serializers.
DEF_TEST(Image_Encode_Serializer, reporter) {
    MockSerializer serializer([]() -> sk_sp<SkData> {
        return SkData::MakeWithCString(kSerializedData);
    });
    sk_sp<SkImage> image(create_image());
    sk_sp<SkData> encoded = image->encodeToData(&serializer);
    sk_sp<SkData> reference(SkData::MakeWithCString(kSerializedData));

    REPORTER_ASSERT(reporter, serializer.didEncode());
    REPORTER_ASSERT(reporter, encoded);
    REPORTER_ASSERT(reporter, encoded->size() > 0);
    REPORTER_ASSERT(reporter, encoded->equals(reference.get()));
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

    MockSerializer emptySerializer([]() -> sk_sp<SkData> { return SkData::MakeEmpty(); });
    MockSerializer nullSerializer([]() -> sk_sp<SkData> { return nullptr; });
    MockSerializer* serializers[] = { &emptySerializer, &nullSerializer };

    for (size_t i = 0; i < SK_ARRAY_COUNT(serializers); ++i) {
        SkDynamicMemoryWStream wstream;
        REPORTER_ASSERT(reporter, !serializers[i]->didEncode());
        picture->serialize(&wstream, serializers[i]);
        REPORTER_ASSERT(reporter, serializers[i]->didEncode());

        std::unique_ptr<SkStream> rstream(wstream.detachAsStream());
        sk_sp<SkPicture> deserialized(SkPicture::MakeFromStream(rstream.get()));
        REPORTER_ASSERT(reporter, deserialized);
        REPORTER_ASSERT(reporter, deserialized->approximateOpCount() > 0);
    }
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
#if SK_SUPPORT_GPU

#include "SkBitmapCache.h"

/*
 *  This tests the caching (and preemptive purge) of the raster equivalent of a gpu-image.
 *  We cache it for performance when drawing into a raster surface.
 *
 *  A cleaner test would know if each drawImage call triggered a read-back from the gpu,
 *  but we don't have that facility (at the moment) so we use a little internal knowledge
 *  of *how* the raster version is cached, and look for that.
 */
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(c, reporter, ctxInfo) {
    SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.grContext()));
    const uint32_t uniqueID = image->uniqueID();
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
            REPORTER_ASSERT(reporter, cachedBitmap.getGenerationID() == uniqueID);
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
        // Create a texture image in a another GrContext.
        [testContext, otherContextInfo] {
            otherContextInfo.testContext()->makeCurrent();
            sk_sp<SkImage> otherContextImage = create_gpu_image(otherContextInfo.grContext());
            testContext->makeCurrent();
            return otherContextImage;
        }
    };

    sk_sp<SkColorSpace> dstColorSpaces[] ={
        nullptr,
        SkColorSpace::MakeSRGB(),
    };

    for (auto& dstColorSpace : dstColorSpaces) {
        for (auto factory : imageFactories) {
            sk_sp<SkImage> image(factory());
            if (!image) {
                ERRORF(reporter, "Error creating image.");
                continue;
            }

            sk_sp<SkImage> texImage(image->makeTextureImage(context, dstColorSpace.get()));
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
            if (image->isTextureBacked()) {
                GrSurfaceProxy* origProxy = as_IB(image)->peekProxy();
                GrSurfaceProxy* copyProxy = as_IB(texImage)->peekProxy();

                if (origProxy->underlyingUniqueID() != copyProxy->underlyingUniqueID()) {
                    ERRORF(reporter, "makeTextureImage made unnecessary texture copy.");
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

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkImage_drawAbandonedGpuImage, reporter, contextInfo) {
    auto context = contextInfo.grContext();
    auto image = create_gpu_image(context);
    auto info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    image->getTexture()->abandon();
    surface->getCanvas()->drawImage(image, 0, 0);
}

#endif

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
    size_t size = info.getSafeSize(rowBytes);
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
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageReadPixels_Gpu, reporter, ctxInfo) {
    image_test_read_pixels(reporter, create_gpu_image(ctxInfo.grContext()).get());
}
#endif

static void check_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image,
                                const SkBitmap& bitmap, SkImage::LegacyBitmapMode mode) {
    REPORTER_ASSERT(reporter, image->width() == bitmap.width());
    REPORTER_ASSERT(reporter, image->height() == bitmap.height());
    REPORTER_ASSERT(reporter, image->alphaType() == bitmap.alphaType());

    if (SkImage::kRO_LegacyBitmapMode == mode) {
        REPORTER_ASSERT(reporter, bitmap.isImmutable());
    }

    REPORTER_ASSERT(reporter, bitmap.getPixels());

    const SkImageInfo info = SkImageInfo::MakeN32(1, 1, bitmap.alphaType());
    SkPMColor imageColor;
    REPORTER_ASSERT(reporter, image->readPixels(info, &imageColor, sizeof(SkPMColor), 0, 0));
    REPORTER_ASSERT(reporter, imageColor == *bitmap.getAddr32(0, 0));
}

static void test_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image, SkImage::LegacyBitmapMode mode) {
    if (!image) {
        ERRORF(reporter, "Failed to create image.");
        return;
    }
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter, image->asLegacyBitmap(&bitmap, mode));
    check_legacy_bitmap(reporter, image, bitmap, mode);

    // Test subsetting to exercise the rowBytes logic.
    SkBitmap tmp;
    REPORTER_ASSERT(reporter, bitmap.extractSubset(&tmp, SkIRect::MakeWH(image->width() / 2,
                                                                         image->height() / 2)));
    sk_sp<SkImage> subsetImage(SkImage::MakeFromBitmap(tmp));
    REPORTER_ASSERT(reporter, subsetImage.get());

    SkBitmap subsetBitmap;
    REPORTER_ASSERT(reporter, subsetImage->asLegacyBitmap(&subsetBitmap, mode));
    check_legacy_bitmap(reporter, subsetImage.get(), subsetBitmap, mode);
}
DEF_TEST(ImageLegacyBitmap, reporter) {
    const SkImage::LegacyBitmapMode modes[] = {
        SkImage::kRO_LegacyBitmapMode,
        SkImage::kRW_LegacyBitmapMode,
    };
    for (auto& mode : modes) {
        sk_sp<SkImage> image(create_image());
        test_legacy_bitmap(reporter, image.get(), mode);

        image = create_data_image();
        test_legacy_bitmap(reporter, image.get(), mode);

        RasterDataHolder dataHolder;
        image = create_rasterproc_image(&dataHolder);
        test_legacy_bitmap(reporter, image.get(), mode);
        image.reset();
        REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

        image = create_codec_image();
        test_legacy_bitmap(reporter, image.get(), mode);
    }
}
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageLegacyBitmap_Gpu, reporter, ctxInfo) {
    const SkImage::LegacyBitmapMode modes[] = {
        SkImage::kRO_LegacyBitmapMode,
        SkImage::kRW_LegacyBitmapMode,
    };
    for (auto& mode : modes) {
        sk_sp<SkImage> image(create_gpu_image(ctxInfo.grContext()));
        test_legacy_bitmap(reporter, image.get(), mode);
    }
}
#endif

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
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImagePeek_Gpu, reporter, ctxInfo) {
    sk_sp<SkImage> image(create_gpu_image(ctxInfo.grContext()));
    test_peek(reporter, image.get(), false);
}
#endif

#if SK_SUPPORT_GPU
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
    std::unique_ptr<uint32_t[]> pixels(new uint32_t[kWidth * kHeight]);

    GrContext* ctx = ctxInfo.grContext();

    GrBackendObject backendTexHandle =
            ctxInfo.grContext()->getGpu()->createTestingOnlyBackendTexture(
                    pixels.get(), kWidth, kHeight, kRGBA_8888_GrPixelConfig, true);

    GrBackendTexture backendTex = GrTest::CreateBackendTexture(ctx->contextPriv().getBackend(),
                                                               kWidth,
                                                               kHeight,
                                                               kRGBA_8888_GrPixelConfig,
                                                               backendTexHandle);

    TextureReleaseChecker releaseChecker;
    GrSurfaceOrigin texOrigin = kBottomLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(
        SkImage::MakeFromTexture(ctx, backendTex, texOrigin, kPremul_SkAlphaType, nullptr,
                                 TextureReleaseChecker::Release, &releaseChecker));

    GrSurfaceOrigin readBackOrigin;
    GrBackendObject readBackHandle = refImg->getTextureHandle(false, &readBackOrigin);
    // TODO: Make it so we can check this (see skbug.com/5019)
#if 0
    if (*readBackHandle != *(backendTexHandle)) {
        ERRORF(reporter, "backend mismatch %d %d\n",
                       (int)readBackHandle, (int)backendTexHandle);
    }
    REPORTER_ASSERT(reporter, readBackHandle == backendTexHandle);
#else
    REPORTER_ASSERT(reporter, SkToBool(readBackHandle));
#endif
    if (readBackOrigin != texOrigin) {
        ERRORF(reporter, "origin mismatch %d %d\n", readBackOrigin, texOrigin);
    }
    REPORTER_ASSERT(reporter, readBackOrigin == texOrigin);

    // Now exercise the release proc
    REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
    refImg.reset(nullptr); // force a release of the image
    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);

    ctxInfo.grContext()->getGpu()->deleteTestingOnlyBackendTexture(backendTexHandle);
}

DEF_GPUTEST(SkImage_MakeCrossContextRelease, reporter, /*factory*/) {
    GrContextFactory testFactory;

    sk_sp<SkData> data = GetResourceAsData("mandrill_128.png");
    SkASSERT(data.get());

    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
        ContextInfo ctxInfo = testFactory.getContextInfo(ctxType);
        GrContext* ctx = ctxInfo.grContext();
        if (!ctx) {
            continue;
        }

        // If we don't have proper support for this feature, the factory will fallback to returning
        // codec-backed images. Those will "work", but some of our checks will fail because we
        // expect the cross-context images not to work on multiple contexts at once.
        if (!ctx->caps()->crossContextTextureSupport()) {
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
            sk_sp<SkImage> refImg(SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr));
            refImg.reset(nullptr); // force a release of the image
        }

        SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
        SkCanvas* canvas = surface->getCanvas();

        // Case #2: Create image, draw, flush, free image
        {
            sk_sp<SkImage> refImg(SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr));

            canvas->drawImage(refImg, 0, 0);
            canvas->flush();

            refImg.reset(nullptr); // force a release of the image
        }

        // Case #3: Create image, draw, free image, flush
        {
            sk_sp<SkImage> refImg(SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr));

            canvas->drawImage(refImg, 0, 0);
            refImg.reset(nullptr); // force a release of the image

            canvas->flush();
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
            sk_sp<SkImage> refImg(SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr));

            otherTestContext->makeCurrent();
            canvas->drawImage(refImg, 0, 0);
            canvas->flush();

            testContext->makeCurrent();
            refImg.reset(nullptr); // force a release of the image
        }

        // Case #5: Create image, draw*, free image, flush*
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr));

            otherTestContext->makeCurrent();
            canvas->drawImage(refImg, 0, 0);

            testContext->makeCurrent();
            refImg.reset(nullptr); // force a release of the image

            otherTestContext->makeCurrent();
            canvas->flush();
        }

        // Case #6: Verify that only one context can be using the image at a time
        {
            testContext->makeCurrent();
            sk_sp<SkImage> refImg(SkImage::MakeCrossContextFromEncoded(ctx, data, false, nullptr));

            // Any context should be able to borrow the texture at this point
            sk_sp<SkColorSpace> texColorSpace;
            sk_sp<GrTextureProxy> proxy = as_IB(refImg)->asTextureProxyRef(
                ctx, GrSamplerParams::ClampNoFilter(), nullptr, &texColorSpace, nullptr);
            REPORTER_ASSERT(reporter, proxy);

            // But once it's borrowed, no other context should be able to borrow
            otherTestContext->makeCurrent();
            sk_sp<GrTextureProxy> otherProxy = as_IB(refImg)->asTextureProxyRef(
                otherCtx, GrSamplerParams::ClampNoFilter(), nullptr, &texColorSpace, nullptr);
            REPORTER_ASSERT(reporter, !otherProxy);

            // Original context (that's already borrowing) should be okay
            testContext->makeCurrent();
            sk_sp<GrTextureProxy> proxySecondRef = as_IB(refImg)->asTextureProxyRef(
                ctx, GrSamplerParams::ClampNoFilter(), nullptr, &texColorSpace, nullptr);
            REPORTER_ASSERT(reporter, proxySecondRef);

            // Releae all refs from the original context
            proxy.reset(nullptr);
            proxySecondRef.reset(nullptr);

            // Now we should be able to borrow the texture from the other context
            otherTestContext->makeCurrent();
            otherProxy = as_IB(refImg)->asTextureProxyRef(
                otherCtx, GrSamplerParams::ClampNoFilter(), nullptr, &texColorSpace, nullptr);
            REPORTER_ASSERT(reporter, otherProxy);

            // Release everything
            otherProxy.reset(nullptr);
            refImg.reset(nullptr);
        }
    }
}

static void check_images_same(skiatest::Reporter* reporter, const SkImage* a, const SkImage* b) {
    if (a->width() != b->width() || a->height() != b->height()) {
        ERRORF(reporter, "Images must have the same size");
        return;
    }
    if (a->alphaType() != b->alphaType()) {
        ERRORF(reporter, "Images must have the same alpha type");
        return;
    }

    SkImageInfo info = SkImageInfo::MakeN32Premul(a->width(), a->height());
    SkAutoPixmapStorage apm;
    SkAutoPixmapStorage bpm;

    apm.alloc(info);
    bpm.alloc(info);

    if (!a->readPixels(apm, 0, 0)) {
        ERRORF(reporter, "Could not read image a's pixels");
        return;
    }
    if (!b->readPixels(bpm, 0, 0)) {
        ERRORF(reporter, "Could not read image b's pixels");
        return;
    }

    for (auto y = 0; y < info.height(); ++y) {
        for (auto x = 0; x < info.width(); ++x) {
            uint32_t pixelA = *apm.addr32(x, y);
            uint32_t pixelB = *bpm.addr32(x, y);
            if (pixelA != pixelB) {
                ERRORF(reporter, "Expected image pixels to be the same. At %d,%d 0x%08x != 0x%08x",
                       x, y, pixelA, pixelB);
                return;
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DeferredTextureImage, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    sk_gpu_test::TestContext* testContext = ctxInfo.testContext();
    sk_sp<GrContextThreadSafeProxy> proxy = context->threadSafeProxy();

    GrContextFactory otherFactory;
    ContextInfo otherContextInfo = otherFactory.getContextInfo(ctxInfo.type());

    testContext->makeCurrent();
    REPORTER_ASSERT(reporter, proxy);
    auto createLarge = [context] {
        return create_image_large(context->caps()->maxTextureSize());
    };
    struct {
        std::function<sk_sp<SkImage> ()>                      fImageFactory;
        std::vector<SkImage::DeferredTextureImageUsageParams> fParams;
        sk_sp<SkColorSpace>                                   fColorSpace;
        SkColorType                                           fColorType;
        SkFilterQuality                                       fExpectedQuality;
        int                                                   fExpectedScaleFactor;
        bool                                                  fExpectation;
    } testCases[] = {
        { create_image,          {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, true },
        { create_codec_image,    {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, true },
        { create_data_image,     {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, true },
        { create_picture_image,  {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, false },
        { [context] { return create_gpu_image(context); },
          {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, false },
        // Create a texture image in a another GrContext.
        { [testContext, otherContextInfo] {
            otherContextInfo.testContext()->makeCurrent();
            sk_sp<SkImage> otherContextImage = create_gpu_image(otherContextInfo.grContext());
            testContext->makeCurrent();
            return otherContextImage;
          }, {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, false },
        // Create an image that is too large to upload.
        { createLarge, {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kN32_SkColorType, kNone_SkFilterQuality, 1, false },
        // Create an image that is too large, but is scaled to an acceptable size.
        { createLarge, {{SkMatrix::I(), kMedium_SkFilterQuality, 4}},
          nullptr, kN32_SkColorType, kMedium_SkFilterQuality, 16, true},
        // Create an image with multiple low filter qualities, make sure we round up.
        { createLarge, {{SkMatrix::I(), kNone_SkFilterQuality, 4},
                        {SkMatrix::I(), kMedium_SkFilterQuality, 4}},
          nullptr, kN32_SkColorType, kMedium_SkFilterQuality, 16, true},
        // Create an image with multiple prescale levels, make sure we chose the minimum scale.
        { createLarge, {{SkMatrix::I(), kMedium_SkFilterQuality, 5},
                        {SkMatrix::I(), kMedium_SkFilterQuality, 4}},
          nullptr, kN32_SkColorType, kMedium_SkFilterQuality, 16, true},
        // Create a images which are decoded to a 4444 backing.
        { create_image,       {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kARGB_4444_SkColorType, kNone_SkFilterQuality, 1, true },
        { create_codec_image, {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kARGB_4444_SkColorType, kNone_SkFilterQuality, 1, true },
        { create_data_image,  {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          nullptr, kARGB_4444_SkColorType, kNone_SkFilterQuality, 1, true },
        // Valid SkColorSpace and SkColorType.
        { create_data_image,  {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          SkColorSpace::MakeSRGB(), kN32_SkColorType, kNone_SkFilterQuality, 1, true },
        // Invalid SkColorSpace and SkColorType.
        { create_data_image,  {{SkMatrix::I(), kNone_SkFilterQuality, 0}},
          SkColorSpace::MakeSRGB(), kARGB_4444_SkColorType, kNone_SkFilterQuality, 1, false },
    };


    for (auto testCase : testCases) {
        sk_sp<SkImage> image(testCase.fImageFactory());
        if (!image) {
            ERRORF(reporter, "Failed to create image!");
            continue;
        }

        size_t size = image->getDeferredTextureImageData(*proxy, testCase.fParams.data(),
                                                         static_cast<int>(testCase.fParams.size()),
                                                         nullptr, testCase.fColorSpace.get(),
                                                         testCase.fColorType);
        static const char *const kFS[] = { "fail", "succeed" };
        if (SkToBool(size) != testCase.fExpectation) {
            ERRORF(reporter,  "This image was expected to %s but did not.",
                   kFS[testCase.fExpectation]);
        }
        if (size) {
            void* buffer = sk_malloc_throw(size);
            void* misaligned = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(buffer) + 3);
            if (image->getDeferredTextureImageData(*proxy, testCase.fParams.data(),
                                                   static_cast<int>(testCase.fParams.size()),
                                                   misaligned, testCase.fColorSpace.get(),
                                                   testCase.fColorType)) {
                ERRORF(reporter, "Should fail when buffer is misaligned.");
            }
            if (!image->getDeferredTextureImageData(*proxy, testCase.fParams.data(),
                                                    static_cast<int>(testCase.fParams.size()),
                                                    buffer, testCase.fColorSpace.get(),
                                                   testCase.fColorType)) {
                ERRORF(reporter, "deferred image size succeeded but creation failed.");
            } else {
                for (auto budgeted : { SkBudgeted::kNo, SkBudgeted::kYes }) {
                    sk_sp<SkImage> newImage(
                        SkImage::MakeFromDeferredTextureImageData(context, buffer, budgeted));
                    REPORTER_ASSERT(reporter, newImage != nullptr);
                    if (newImage) {
                        // Scale the image in software for comparison.
                        SkImageInfo scaled_info = SkImageInfo::MakeN32(
                                                    image->width() / testCase.fExpectedScaleFactor,
                                                    image->height() / testCase.fExpectedScaleFactor,
                                                    image->alphaType());
                        SkAutoPixmapStorage scaled;
                        scaled.alloc(scaled_info);
                        image->scalePixels(scaled, testCase.fExpectedQuality);
                        sk_sp<SkImage> scaledImage = SkImage::MakeRasterCopy(scaled);
                        check_images_same(reporter, scaledImage.get(), newImage.get());
                    }
                    // The other context should not be able to create images from texture data
                    // created by the original context.
                    sk_sp<SkImage> newImage2(SkImage::MakeFromDeferredTextureImageData(
                        otherContextInfo.grContext(), buffer, budgeted));
                    REPORTER_ASSERT(reporter, !newImage2);
                    testContext->makeCurrent();
                }
            }
            sk_free(buffer);
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> create_picture_image(sk_sp<SkColorSpace> space) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);
    canvas->clear(SK_ColorCYAN);
    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(), SkISize::Make(10, 10),
                                    nullptr, nullptr, SkImage::BitDepth::kU8, std::move(space));
};

static inline bool almost_equal(int a, int b) {
    return SkTAbs(a - b) <= 1;
}

DEF_TEST(Image_ColorSpace, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    sk_sp<SkImage> image = GetResourceAsImage("mandrill_512_q075.jpg");
    REPORTER_ASSERT(r, srgb.get() == image->colorSpace());

    image = GetResourceAsImage("webp-color-profile-lossy.webp");
    SkColorSpaceTransferFn fn;
    bool success = image->colorSpace()->isNumericalTransferFn(&fn);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, color_space_almost_equal(1.8f, fn.fG));

    sk_sp<SkColorSpace> rec2020 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                        SkColorSpace::kRec2020_Gamut);
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
    sk_sp<SkColorSpace> p3 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                   SkColorSpace::kDCIP3_D65_Gamut);
    SkColorSpaceTransferFn fn;
    fn.fA = 1.f; fn.fB = 0.f; fn.fC = 0.f; fn.fD = 0.f; fn.fE = 0.f; fn.fF = 0.f; fn.fG = 1.8f;
    sk_sp<SkColorSpace> adobeGamut = SkColorSpace::MakeRGB(fn, SkColorSpace::kAdobeRGB_Gamut);

    SkBitmap srgbBitmap;
    srgbBitmap.allocPixels(SkImageInfo::MakeS32(1, 1, kOpaque_SkAlphaType));
    *srgbBitmap.getAddr32(0, 0) = SkSwizzle_RGBA_to_PMColor(0xFF604020);
    srgbBitmap.setImmutable();
    sk_sp<SkImage> srgbImage = SkImage::MakeFromBitmap(srgbBitmap);
    sk_sp<SkImage> p3Image = srgbImage->makeColorSpace(p3, SkTransferFunctionBehavior::kIgnore);
    SkBitmap p3Bitmap;
    bool success = p3Image->asLegacyBitmap(&p3Bitmap, SkImage::kRO_LegacyBitmapMode);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x28, SkGetPackedR32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x40, SkGetPackedG32(*p3Bitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x5E, SkGetPackedB32(*p3Bitmap.getAddr32(0, 0))));

    sk_sp<SkImage> adobeImage = srgbImage->makeColorSpace(adobeGamut,
                                                          SkTransferFunctionBehavior::kIgnore);
    SkBitmap adobeBitmap;
    success = adobeImage->asLegacyBitmap(&adobeBitmap, SkImage::kRO_LegacyBitmapMode);
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, almost_equal(0x21, SkGetPackedR32(*adobeBitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x31, SkGetPackedG32(*adobeBitmap.getAddr32(0, 0))));
    REPORTER_ASSERT(r, almost_equal(0x4C, SkGetPackedB32(*adobeBitmap.getAddr32(0, 0))));

    srgbImage = GetResourceAsImage("1x1.png");
    p3Image = srgbImage->makeColorSpace(p3, SkTransferFunctionBehavior::kIgnore);
    success = p3Image->asLegacyBitmap(&p3Bitmap, SkImage::kRO_LegacyBitmapMode);
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

#if SK_SUPPORT_GPU
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
#endif
