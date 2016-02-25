/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include "DMGpuSupport.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkImage_Base.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPixelSerializer.h"
#include "SkRRect.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkUtils.h"
#include "Test.h"

static void assert_equal(skiatest::Reporter* reporter, SkImage* a, const SkIRect* subsetA,
                         SkImage* b) {
    const int widthA = subsetA ? subsetA->width() : a->width();
    const int heightA = subsetA ? subsetA->height() : a->height();

    REPORTER_ASSERT(reporter, widthA == b->width());
    REPORTER_ASSERT(reporter, heightA == b->height());
#if 0
    // see https://bug.skia.org/3965
    bool AO = a->isOpaque();
    bool BO = b->isOpaque();
    REPORTER_ASSERT(reporter, AO == BO);
#endif

    SkImageInfo info = SkImageInfo::MakeN32(widthA, heightA,
                                        a->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    SkAutoPixmapStorage pmapA, pmapB;
    pmapA.alloc(info);
    pmapB.alloc(info);

    const int srcX = subsetA ? subsetA->x() : 0;
    const int srcY = subsetA ? subsetA->y() : 0;

    REPORTER_ASSERT(reporter, a->readPixels(pmapA, srcX, srcY));
    REPORTER_ASSERT(reporter, b->readPixels(pmapB, 0, 0));

    const size_t widthBytes = widthA * info.bytesPerPixel();
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
static SkImage* create_image() {
    const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    draw_image_test_pattern(surface->getCanvas());
    return surface->newImageSnapshot();
}
static SkData* create_image_data(SkImageInfo* info) {
    *info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    const size_t rowBytes = info->minRowBytes();
    SkAutoTUnref<SkData> data(SkData::NewUninitialized(rowBytes * info->height()));
    {
        SkBitmap bm;
        bm.installPixels(*info, data->writable_data(), rowBytes);
        SkCanvas canvas(bm);
        draw_image_test_pattern(&canvas);
    }
    return data.release();
}
static SkImage* create_data_image() {
    SkImageInfo info;
    SkAutoTUnref<SkData> data(create_image_data(&info));
    return SkImage::NewRasterData(info, data, info.minRowBytes());
}
#if SK_SUPPORT_GPU // not gpu-specific but currently only used in GPU tests
static SkImage* create_picture_image() {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);
    canvas->clear(SK_ColorCYAN);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    return SkImage::NewFromPicture(picture, SkISize::Make(10, 10), nullptr, nullptr);
};
#endif
// Want to ensure that our Release is called when the owning image is destroyed
struct RasterDataHolder {
    RasterDataHolder() : fReleaseCount(0) {}
    SkAutoTUnref<SkData> fData;
    int fReleaseCount;
    static void Release(const void* pixels, void* context) {
        RasterDataHolder* self = static_cast<RasterDataHolder*>(context);
        self->fReleaseCount++;
        self->fData.reset();
    }
};
static SkImage* create_rasterproc_image(RasterDataHolder* dataHolder) {
    SkASSERT(dataHolder);
    SkImageInfo info;
    SkAutoTUnref<SkData> data(create_image_data(&info));
    dataHolder->fData.reset(SkRef(data.get()));
    return SkImage::NewFromRaster(info, data->data(), info.minRowBytes(),
                                  RasterDataHolder::Release, dataHolder);
}
static SkImage* create_codec_image() {
    SkImageInfo info;
    SkAutoTUnref<SkData> data(create_image_data(&info));
    SkBitmap bitmap;
    bitmap.installPixels(info, data->writable_data(), info.minRowBytes());
    SkAutoTUnref<SkData> src(
        SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type, 100));
    return SkImage::NewFromEncoded(src);
}
#if SK_SUPPORT_GPU
static SkImage* create_gpu_image(GrContext* context) {
    const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(context, SkBudgeted::kNo,
                                                               info));
    draw_image_test_pattern(surface->getCanvas());
    return surface->newImageSnapshot();
}
#endif

static void test_encode(skiatest::Reporter* reporter, SkImage* image) {
    const SkIRect ir = SkIRect::MakeXYWH(5, 5, 10, 10);
    SkAutoTUnref<SkData> origEncoded(image->encode());
    REPORTER_ASSERT(reporter, origEncoded);
    REPORTER_ASSERT(reporter, origEncoded->size() > 0);

    SkAutoTUnref<SkImage> decoded(SkImage::NewFromEncoded(origEncoded));
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, image, nullptr, decoded);

    // Now see if we can instantiate an image from a subset of the surface/origEncoded
    
    decoded.reset(SkImage::NewFromEncoded(origEncoded, &ir));
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, image, &ir, decoded);
}

DEF_TEST(ImageEncode, reporter) {
    SkAutoTUnref<SkImage> image(create_image());
    test_encode(reporter, image);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageEncode_Gpu, reporter, context) {
    SkAutoTUnref<SkImage> image(create_gpu_image(context));
    test_encode(reporter, image);
}
#endif

namespace {

const char* kSerializedData = "serialized";

class MockSerializer : public SkPixelSerializer {
public:
    MockSerializer(SkData* (*func)()) : fFunc(func), fDidEncode(false) { }

    bool didEncode() const { return fDidEncode; }

protected:
    bool onUseEncodedData(const void*, size_t) override {
        return false;
    }

    SkData* onEncode(const SkPixmap&) override {
        fDidEncode = true;
        return fFunc();
    }

private:
    SkData* (*fFunc)();
    bool fDidEncode;

    typedef SkPixelSerializer INHERITED;
};

} // anonymous namespace

// Test that SkImage encoding observes custom pixel serializers.
DEF_TEST(Image_Encode_Serializer, reporter) {
    MockSerializer serializer([]() -> SkData* { return SkData::NewWithCString(kSerializedData); });
    SkAutoTUnref<SkImage> image(create_image());
    SkAutoTUnref<SkData> encoded(image->encode(&serializer));
    SkAutoTUnref<SkData> reference(SkData::NewWithCString(kSerializedData));

    REPORTER_ASSERT(reporter, serializer.didEncode());
    REPORTER_ASSERT(reporter, encoded);
    REPORTER_ASSERT(reporter, encoded->size() > 0);
    REPORTER_ASSERT(reporter, encoded->equals(reference));
}

// Test that image encoding failures do not break picture serialization/deserialization.
DEF_TEST(Image_Serialize_Encoding_Failure, reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    surface->getCanvas()->clear(SK_ColorGREEN);
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(100, 100);
    canvas->drawImage(image, 0, 0);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    REPORTER_ASSERT(reporter, picture);
    REPORTER_ASSERT(reporter, picture->approximateOpCount() > 0);

    MockSerializer emptySerializer([]() -> SkData* { return SkData::NewEmpty(); });
    MockSerializer nullSerializer([]() -> SkData* { return nullptr; });
    MockSerializer* serializers[] = { &emptySerializer, &nullSerializer };

    for (size_t i = 0; i < SK_ARRAY_COUNT(serializers); ++i) {
        SkDynamicMemoryWStream wstream;
        REPORTER_ASSERT(reporter, !serializers[i]->didEncode());
        picture->serialize(&wstream, serializers[i]);
        REPORTER_ASSERT(reporter, serializers[i]->didEncode());

        SkAutoTDelete<SkStream> rstream(wstream.detachAsStream());
        SkAutoTUnref<SkPicture> deserialized(SkPicture::CreateFromStream(rstream));
        REPORTER_ASSERT(reporter, deserialized);
        REPORTER_ASSERT(reporter, deserialized->approximateOpCount() > 0);
    }
}

DEF_TEST(Image_NewRasterCopy, reporter) {
    const SkPMColor red =   SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkPMColor green = SkPackARGB32(0xFF, 0, 0xFF, 0);
    const SkPMColor blue =  SkPackARGB32(0xFF, 0, 0, 0xFF);
    SkPMColor colors[] = { red, green, blue, 0 };
    SkAutoTUnref<SkColorTable> ctable(new SkColorTable(colors, SK_ARRAY_COUNT(colors)));
    // The colortable made a copy, so we can trash the original colors
    memset(colors, 0xFF, sizeof(colors));

    const SkImageInfo srcInfo = SkImageInfo::Make(2, 2, kIndex_8_SkColorType, kPremul_SkAlphaType);
    const size_t srcRowBytes = 2 * sizeof(uint8_t);
    uint8_t indices[] = { 0, 1, 2, 3 };
    SkAutoTUnref<SkImage> image(SkImage::NewRasterCopy(srcInfo, indices, srcRowBytes, ctable));
    // The image made a copy, so we can trash the original indices
    memset(indices, 0xFF, sizeof(indices));

    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(2, 2);
    const size_t dstRowBytes = 2 * sizeof(SkPMColor);
    SkPMColor pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));   // init with values we don't expect
    image->readPixels(dstInfo, pixels, dstRowBytes, 0, 0);
    REPORTER_ASSERT(reporter, red == pixels[0]);
    REPORTER_ASSERT(reporter, green == pixels[1]);
    REPORTER_ASSERT(reporter, blue == pixels[2]);
    REPORTER_ASSERT(reporter, 0 == pixels[3]);
}

// Test that a draw that only partially covers the drawing surface isn't
// interpreted as covering the entire drawing surface (i.e., exercise one of the
// conditions of SkCanvas::wouldOverwriteEntireSurface()).
DEF_TEST(Image_RetainSnapshot, reporter) {
    const SkPMColor red   = SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkPMColor green = SkPackARGB32(0xFF, 0, 0xFF, 0);
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    surface->getCanvas()->clear(0xFF00FF00);

    SkPMColor pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));   // init with values we don't expect
    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(2, 2);
    const size_t dstRowBytes = 2 * sizeof(SkPMColor);

    SkAutoTUnref<SkImage> image1(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image1->readPixels(dstInfo, pixels, dstRowBytes, 0, 0));
    for (size_t i = 0; i < SK_ARRAY_COUNT(pixels); ++i) {
        REPORTER_ASSERT(reporter, pixels[i] == green);
    }

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColor(SK_ColorRED);

    surface->getCanvas()->drawRect(SkRect::MakeXYWH(1, 1, 1, 1), paint);

    SkAutoTUnref<SkImage> image2(surface->newImageSnapshot());
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

        SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bm));
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
DEF_GPUTEST_FOR_NATIVE_CONTEXT(SkImage_Gpu2Cpu, reporter, context) {
    SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
    SkAutoTUnref<SkImage> image(create_gpu_image(context));
    const uint32_t uniqueID = image->uniqueID();

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));

    // now we can test drawing a gpu-backed image into a cpu-backed surface

    {
        SkBitmap cachedBitmap;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(uniqueID, &cachedBitmap));
    }

    surface->getCanvas()->drawImage(image, 0, 0);
    {
        SkBitmap cachedBitmap;
        if (SkBitmapCache::Find(uniqueID, &cachedBitmap)) {
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
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(uniqueID, &cachedBitmap));
    }
}

DEF_GPUTEST_FOR_NATIVE_CONTEXT(SkImage_newTextureImage, reporter, context, glContext) {
    GrContextFactory otherFactory;
    GrContextFactory::ContextInfo otherContextInfo =
        otherFactory.getContextInfo(GrContextFactory::kNative_GLContextType);
    glContext->makeCurrent();

    std::function<SkImage*()> imageFactories[] = {
        create_image,
        create_codec_image,
        create_data_image,
        // Create an image from a picture.
        create_picture_image,
        // Create a texture image.
        [context] { return create_gpu_image(context); },
        // Create a texture image in a another GrContext.
        [glContext, otherContextInfo] {
            otherContextInfo.fGLContext->makeCurrent();
            SkImage* otherContextImage = create_gpu_image(otherContextInfo.fGrContext);
            glContext->makeCurrent();
            return otherContextImage;
        }
    };

    for (auto factory : imageFactories) {
        SkAutoTUnref<SkImage> image(factory());
        if (!image) {
            ERRORF(reporter, "Error creating image.");
            continue;
        }
        GrTexture* origTexture = as_IB(image)->peekTexture();

        SkAutoTUnref<SkImage> texImage(image->newTextureImage(context));
        if (!texImage) {
            // We execpt to fail if image comes from a different GrContext.
            if (!origTexture || origTexture->getContext() == context) {
                ERRORF(reporter, "newTextureImage failed.");
            }
            continue;
        }
        GrTexture* copyTexture = as_IB(texImage)->peekTexture();
        if (!copyTexture) {
            ERRORF(reporter, "newTextureImage returned non-texture image.");
            continue;
        }
        if (origTexture) {
            if (origTexture != copyTexture) {
                ERRORF(reporter, "newTextureImage made unnecessary texture copy.");
            }
        }
        if (image->width() != texImage->width() || image->height() != texImage->height()) {
            ERRORF(reporter, "newTextureImage changed the image size.");
        }
        if (image->isOpaque() != texImage->isOpaque()) {
            ERRORF(reporter, "newTextureImage changed image opaqueness.");
        }
    }
}
#endif

// https://bug.skia.org/4390
DEF_TEST(ImageFromIndex8Bitmap, r) {
    SkPMColor pmColors[1] = {SkPreMultiplyColor(SK_ColorWHITE)};
    SkBitmap bm;
    SkAutoTUnref<SkColorTable> ctable(
            new SkColorTable(pmColors, SK_ARRAY_COUNT(pmColors)));
    SkImageInfo info =
            SkImageInfo::Make(1, 1, kIndex_8_SkColorType, kPremul_SkAlphaType);
    bm.allocPixels(info, nullptr, ctable);
    SkAutoLockPixels autoLockPixels(bm);
    *bm.getAddr8(0, 0) = 0;
    SkAutoTUnref<SkImage> img(SkImage::NewFromBitmap(bm));
    REPORTER_ASSERT(r, img.get() != nullptr);
}

class EmptyGenerator : public SkImageGenerator {
public:
    EmptyGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageEmpty, reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewRasterCopy(info, nullptr, 0));
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewRasterData(info, nullptr, 0));
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewFromRaster(info, nullptr, 0, nullptr, nullptr));
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewFromGenerator(new EmptyGenerator));
}

DEF_TEST(ImageDataRef, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.getSafeSize(rowBytes);
    SkData* data = SkData::NewUninitialized(size);
    REPORTER_ASSERT(reporter, data->unique());
    SkImage* image = SkImage::NewRasterData(info, data, rowBytes);
    REPORTER_ASSERT(reporter, !data->unique());
    image->unref();
    REPORTER_ASSERT(reporter, data->unique());
    data->unref();
}

static bool has_pixels(const SkPMColor pixels[], int count, SkPMColor expected) {
    for (int i = 0; i < count; ++i) {
        if (pixels[i] != expected) {
            return false;
        }
    }
    return true;
}

static void test_read_pixels(skiatest::Reporter* reporter, SkImage* image) {
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
    SkAutoTUnref<SkImage> image(create_image());
    test_read_pixels(reporter, image);

    image.reset(create_data_image());
    test_read_pixels(reporter, image);

    RasterDataHolder dataHolder;
    image.reset(create_rasterproc_image(&dataHolder));
    test_read_pixels(reporter, image);
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image.reset(create_codec_image());
    test_read_pixels(reporter, image);
}
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageReadPixels_Gpu, reporter, context) {
    SkAutoTUnref<SkImage> image(create_gpu_image(context));
    test_read_pixels(reporter, image);
}
#endif

static void check_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image,
                                const SkBitmap& bitmap, SkImage::LegacyBitmapMode mode) {
    REPORTER_ASSERT(reporter, image->width() == bitmap.width());
    REPORTER_ASSERT(reporter, image->height() == bitmap.height());
    REPORTER_ASSERT(reporter, image->isOpaque() == bitmap.isOpaque());

    if (SkImage::kRO_LegacyBitmapMode == mode) {
        REPORTER_ASSERT(reporter, bitmap.isImmutable());
    }

    SkAutoLockPixels alp(bitmap);
    REPORTER_ASSERT(reporter, bitmap.getPixels());

    const SkImageInfo info = SkImageInfo::MakeN32(1, 1, bitmap.alphaType());
    SkPMColor imageColor;
    REPORTER_ASSERT(reporter, image->readPixels(info, &imageColor, sizeof(SkPMColor), 0, 0));
    REPORTER_ASSERT(reporter, imageColor == *bitmap.getAddr32(0, 0));
}

static void test_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image, SkImage::LegacyBitmapMode mode) {
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter, image->asLegacyBitmap(&bitmap, mode));
    check_legacy_bitmap(reporter, image, bitmap, mode);

    // Test subsetting to exercise the rowBytes logic.
    SkBitmap tmp;
    REPORTER_ASSERT(reporter, bitmap.extractSubset(&tmp, SkIRect::MakeWH(image->width() / 2,
                                                                         image->height() / 2)));
    SkAutoTUnref<SkImage> subsetImage(SkImage::NewFromBitmap(tmp));
    REPORTER_ASSERT(reporter, subsetImage);

    SkBitmap subsetBitmap;
    REPORTER_ASSERT(reporter, subsetImage->asLegacyBitmap(&subsetBitmap, mode));
    check_legacy_bitmap(reporter, subsetImage, subsetBitmap, mode);
}
DEF_TEST(ImageLegacyBitmap, reporter) {
    const SkImage::LegacyBitmapMode modes[] = {
        SkImage::kRO_LegacyBitmapMode,
        SkImage::kRW_LegacyBitmapMode,
    };
    for (auto& mode : modes) {
        SkAutoTUnref<SkImage> image(create_image());
        test_legacy_bitmap(reporter, image, mode);

        image.reset(create_data_image());
        test_legacy_bitmap(reporter, image, mode);

        RasterDataHolder dataHolder;
        image.reset(create_rasterproc_image(&dataHolder));
        test_legacy_bitmap(reporter, image, mode);
        image.reset();
        REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

        image.reset(create_codec_image());
        test_legacy_bitmap(reporter, image, mode);
    }
}
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageLegacyBitmap_Gpu, reporter, context) {
    const SkImage::LegacyBitmapMode modes[] = {
        SkImage::kRO_LegacyBitmapMode,
        SkImage::kRW_LegacyBitmapMode,
    };
    for (auto& mode : modes) {
        SkAutoTUnref<SkImage> image(create_gpu_image(context));
        test_legacy_bitmap(reporter, image, mode);
    }
}
#endif

static void test_peek(skiatest::Reporter* reporter, SkImage* image, bool expectPeekSuccess) {
    SkImageInfo info;
    size_t rowBytes;
    const void* addr = image->peekPixels(&info, &rowBytes);
    bool success = SkToBool(addr);
    REPORTER_ASSERT(reporter, expectPeekSuccess == success);
    if (success) {
        REPORTER_ASSERT(reporter, 20 == info.width());
        REPORTER_ASSERT(reporter, 20 == info.height());
        REPORTER_ASSERT(reporter, kN32_SkColorType == info.colorType());
        REPORTER_ASSERT(reporter, kPremul_SkAlphaType == info.alphaType() ||
                        kOpaque_SkAlphaType == info.alphaType());
        REPORTER_ASSERT(reporter, info.minRowBytes() <= rowBytes);
        REPORTER_ASSERT(reporter, SkPreMultiplyColor(SK_ColorWHITE) == *(const SkPMColor*)addr);
    }
}
DEF_TEST(ImagePeek, reporter) {
    SkAutoTUnref<SkImage> image(create_image());
    test_peek(reporter, image, true);

    image.reset(create_data_image());
    test_peek(reporter, image, true);

    RasterDataHolder dataHolder;
    image.reset(create_rasterproc_image(&dataHolder));
    test_peek(reporter, image, true);
    image.reset();
    REPORTER_ASSERT(reporter, 1 == dataHolder.fReleaseCount);

    image.reset(create_codec_image());
    test_peek(reporter, image, false);
}
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImagePeek_Gpu, reporter, context) {
    SkAutoTUnref<SkImage> image(create_gpu_image(context));
    test_peek(reporter, image, false);
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
static void check_image_color(skiatest::Reporter* reporter, SkImage* image, SkPMColor expected) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    SkPMColor pixel;
    REPORTER_ASSERT(reporter, image->readPixels(info, &pixel, sizeof(pixel), 0, 0));
    REPORTER_ASSERT(reporter, pixel == expected);
}
DEF_GPUTEST_FOR_NATIVE_CONTEXT(SkImage_NewFromTexture, reporter, context) {
    GrTextureProvider* provider = context->textureProvider();
    const int w = 10;
    const int h = 10;
    SkPMColor storage[w * h];
    const SkPMColor expected0 = SkPreMultiplyColor(SK_ColorRED);
    sk_memset32(storage, expected0, w * h);
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;  // needs to be a rendertarget for readpixels();
    desc.fOrigin = kDefault_GrSurfaceOrigin;
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fSampleCnt = 0;
    SkAutoTUnref<GrTexture> tex(provider->createTexture(desc, SkBudgeted::kNo, storage, w * 4));
    if (!tex) {
        REPORTER_ASSERT(reporter, false);
        return;
    }

    GrBackendTextureDesc backendDesc;
    backendDesc.fConfig = kSkia8888_GrPixelConfig;
    backendDesc.fFlags = kRenderTarget_GrBackendTextureFlag;
    backendDesc.fWidth = w;
    backendDesc.fHeight = h;
    backendDesc.fSampleCnt = 0;
    backendDesc.fTextureHandle = tex->getTextureHandle();
    TextureReleaseChecker releaseChecker;
    SkAutoTUnref<SkImage> refImg(
        SkImage::NewFromTexture(context, backendDesc, kPremul_SkAlphaType,
                                TextureReleaseChecker::Release, &releaseChecker));
    SkAutoTUnref<SkImage> cpyImg(SkImage::NewFromTextureCopy(context, backendDesc,
                                                             kPremul_SkAlphaType));

    check_image_color(reporter, refImg, expected0);
    check_image_color(reporter, cpyImg, expected0);

    // Now lets jam new colors into our "external" texture, and see if the images notice
    const SkPMColor expected1 = SkPreMultiplyColor(SK_ColorBLUE);
    sk_memset32(storage, expected1, w * h);
    tex->writePixels(0, 0, w, h, kSkia8888_GrPixelConfig, storage, GrContext::kFlushWrites_PixelOp);

    // The cpy'd one should still see the old color
#if 0
    // There is no guarantee that refImg sees the new color. We are free to have made a copy. Our
    // write pixels call violated the contract with refImg and refImg is now undefined.
    check_image_color(reporter, refImg, expected1);
#endif
    check_image_color(reporter, cpyImg, expected0);

    // Now exercise the release proc
    REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
    refImg.reset(nullptr); // force a release of the image
    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
}
#endif
