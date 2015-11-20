/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
#include "SkImage_Base.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPixelSerializer.h"
#include "SkRRect.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkUtils.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrTest.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#else
class GrContextFactory;
class GrContext;
#endif

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

static SkImage* make_image(GrContext* ctx, int w, int h, const SkIRect& ir) {
    const SkImageInfo info = SkImageInfo::MakeN32(w, h, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(ctx ?
                                    SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info) :
                                    SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::Make(ir), paint);
    return surface->newImageSnapshot();
}

static void test_encode(skiatest::Reporter* reporter, GrContext* ctx) {
    const SkIRect ir = SkIRect::MakeXYWH(5, 5, 10, 10);
    SkAutoTUnref<SkImage> orig(make_image(ctx, 20, 20, ir));
    SkAutoTUnref<SkData> origEncoded(orig->encode());
    REPORTER_ASSERT(reporter, origEncoded);
    REPORTER_ASSERT(reporter, origEncoded->size() > 0);

    SkAutoTUnref<SkImage> decoded(SkImage::NewFromEncoded(origEncoded));
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, orig, nullptr, decoded);

    // Now see if we can instantiate an image from a subset of the surface/origEncoded
    
    decoded.reset(SkImage::NewFromEncoded(origEncoded, &ir));
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, orig, &ir, decoded);
}

DEF_TEST(Image_Encode_Cpu, reporter) {
    test_encode(reporter, nullptr);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST(Image_Encode_Gpu, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (!ctx) {
        REPORTER_ASSERT(reporter, false);
        return;
    }
    test_encode(reporter, ctx);
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

    SkData* onEncodePixels(const SkImageInfo&, const void*, size_t) override {
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
    const SkIRect ir = SkIRect::MakeXYWH(5, 5, 10, 10);
    SkAutoTUnref<SkImage> image(make_image(nullptr, 20, 20, ir));
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

static SkImage* make_gpu_image(GrContext* ctx, const SkImageInfo& info, SkColor color) {
    const SkSurface::Budgeted budgeted = SkSurface::kNo_Budgeted;
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(ctx, budgeted, info, 0));
    surface->getCanvas()->drawColor(color);
    return surface->newImageSnapshot();
}

#include "SkBitmapCache.h"

/*
 *  This tests the caching (and preemptive purge) of the raster equivalent of a gpu-image.
 *  We cache it for performance when drawing into a raster surface.
 *
 *  A cleaner test would know if each drawImage call triggered a read-back from the gpu,
 *  but we don't have that facility (at the moment) so we use a little internal knowledge
 *  of *how* the raster version is cached, and look for that.
 */
DEF_GPUTEST(SkImage_Gpu2Cpu, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (!ctx) {
        REPORTER_ASSERT(reporter, false);
        return;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    SkAutoTUnref<SkImage> image(make_gpu_image(ctx, info, SK_ColorRED));
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

// TODO: The tests below were moved from SurfaceTests and should be reformatted.

enum ImageType {
    kRasterCopy_ImageType,
    kRasterData_ImageType,
    kRasterProc_ImageType,
    kGpu_ImageType,
    kCodec_ImageType,
};

#include "SkImageGenerator.h"

class EmptyGenerator : public SkImageGenerator {
public:
    EmptyGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

static void test_empty_image(skiatest::Reporter* reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewRasterCopy(info, nullptr, 0));
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewRasterData(info, nullptr, 0));
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewFromRaster(info, nullptr, 0, nullptr, nullptr));
    REPORTER_ASSERT(reporter, nullptr == SkImage::NewFromGenerator(new EmptyGenerator));
}

static void test_image(skiatest::Reporter* reporter) {
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

// Want to ensure that our Release is called when the owning image is destroyed
struct ReleaseDataContext {
    skiatest::Reporter* fReporter;
    SkData*             fData;

    static void Release(const void* pixels, void* context) {
        ReleaseDataContext* state = (ReleaseDataContext*)context;
        REPORTER_ASSERT(state->fReporter, state->fData);
        state->fData->unref();
        state->fData = nullptr;
    }
};

static SkImage* create_image(skiatest::Reporter* reporter,
                             ImageType imageType, GrContext* context, SkColor color,
                             ReleaseDataContext* releaseContext) {
    const SkPMColor pmcolor = SkPreMultiplyColor(color);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    const size_t rowBytes = info.minRowBytes();
    const size_t size = rowBytes * info.height();

    SkAutoTUnref<SkData> data(SkData::NewUninitialized(size));
    void* addr = data->writable_data();
    sk_memset32((SkPMColor*)addr, pmcolor, SkToInt(size >> 2));

    switch (imageType) {
        case kRasterCopy_ImageType:
            return SkImage::NewRasterCopy(info, addr, rowBytes);
        case kRasterData_ImageType:
            return SkImage::NewRasterData(info, data, rowBytes);
        case kRasterProc_ImageType:
            SkASSERT(releaseContext);
            releaseContext->fData = SkRef(data.get());
            return SkImage::NewFromRaster(info, addr, rowBytes,
                                          ReleaseDataContext::Release, releaseContext);
        case kGpu_ImageType: {
            SkAutoTUnref<SkSurface> surf(
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info, 0));
            surf->getCanvas()->clear(color);
            return surf->newImageSnapshot();
        }
        case kCodec_ImageType: {
            SkBitmap bitmap;
            bitmap.installPixels(info, addr, rowBytes);
            SkAutoTUnref<SkData> src(
                 SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type, 100));
            return SkImage::NewFromEncoded(src);
        }
    }
    SkASSERT(false);
    return nullptr;
}

static void set_pixels(SkPMColor pixels[], int count, SkPMColor color) {
    sk_memset32(pixels, color, count);
}
static bool has_pixels(const SkPMColor pixels[], int count, SkPMColor expected) {
    for (int i = 0; i < count; ++i) {
        if (pixels[i] != expected) {
            return false;
        }
    }
    return true;
}

static void test_image_readpixels(skiatest::Reporter* reporter, SkImage* image,
                                  SkPMColor expected) {
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
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, 0, 0));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // bottom-right should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - w, image->height() - h));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // partial top-left should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, -1, -1));
    REPORTER_ASSERT(reporter, pixels[3] == expected);
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h - 1, notExpected));

    // partial bottom-right should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - 1, image->height() - 1));
    REPORTER_ASSERT(reporter, pixels[0] == expected);
    REPORTER_ASSERT(reporter, has_pixels(&pixels[1], w*h - 1, notExpected));
}

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

static void test_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image) {
    const SkImage::LegacyBitmapMode modes[] = {
        SkImage::kRO_LegacyBitmapMode,
        SkImage::kRW_LegacyBitmapMode,
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(modes); ++i) {
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, image->asLegacyBitmap(&bitmap, modes[i]));
        check_legacy_bitmap(reporter, image, bitmap, modes[i]);

        // Test subsetting to exercise the rowBytes logic.
        SkBitmap tmp;
        REPORTER_ASSERT(reporter, bitmap.extractSubset(&tmp, SkIRect::MakeWH(image->width() / 2,
                                                                             image->height() / 2)));
        SkAutoTUnref<SkImage> subsetImage(SkImage::NewFromBitmap(tmp));
        REPORTER_ASSERT(reporter, subsetImage);

        SkBitmap subsetBitmap;
        REPORTER_ASSERT(reporter, subsetImage->asLegacyBitmap(&subsetBitmap, modes[i]));
        check_legacy_bitmap(reporter, subsetImage, subsetBitmap, modes[i]);
    }
}

static void test_imagepeek(skiatest::Reporter* reporter, GrContextFactory* factory) {
    static const struct {
        ImageType   fType;
        bool        fPeekShouldSucceed;
        const char* fName;
    } gRec[] = {
        { kRasterCopy_ImageType,    true,       "RasterCopy"    },
        { kRasterData_ImageType,    true,       "RasterData"    },
        { kRasterProc_ImageType,    true,       "RasterProc"    },
        { kGpu_ImageType,           false,      "Gpu"           },
        { kCodec_ImageType,         false,      "Codec"         },
    };

    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);

    GrContext* ctx = nullptr;
#if SK_SUPPORT_GPU
    ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (nullptr == ctx) {
        return;
    }
#endif

    ReleaseDataContext releaseCtx;
    releaseCtx.fReporter = reporter;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkImageInfo info;
        size_t rowBytes;

        releaseCtx.fData = nullptr;
        SkAutoTUnref<SkImage> image(create_image(reporter, gRec[i].fType, ctx, color, &releaseCtx));
        if (!image.get()) {
            SkDebugf("failed to createImage[%d] %s\n", i, gRec[i].fName);
            continue;   // gpu may not be enabled
        }
        if (kRasterProc_ImageType == gRec[i].fType) {
            REPORTER_ASSERT(reporter, nullptr != releaseCtx.fData);  // we are tracking the data
        } else {
            REPORTER_ASSERT(reporter, nullptr == releaseCtx.fData);  // we ignored the context
        }

        test_legacy_bitmap(reporter, image);

        const void* addr = image->peekPixels(&info, &rowBytes);
        bool success = SkToBool(addr);
        REPORTER_ASSERT(reporter, gRec[i].fPeekShouldSucceed == success);
        if (success) {
            REPORTER_ASSERT(reporter, 10 == info.width());
            REPORTER_ASSERT(reporter, 10 == info.height());
            REPORTER_ASSERT(reporter, kN32_SkColorType == info.colorType());
            REPORTER_ASSERT(reporter, kPremul_SkAlphaType == info.alphaType() ||
                            kOpaque_SkAlphaType == info.alphaType());
            REPORTER_ASSERT(reporter, info.minRowBytes() <= rowBytes);
            REPORTER_ASSERT(reporter, pmcolor == *(const SkPMColor*)addr);
        }

        test_image_readpixels(reporter, image, pmcolor);
    }
    REPORTER_ASSERT(reporter, nullptr == releaseCtx.fData);  // we released the data
}
#if SK_SUPPORT_GPU

struct ReleaseTextureContext {
    ReleaseTextureContext(skiatest::Reporter* reporter) {
        fReporter = reporter;
        fIsReleased = false;
    }

    skiatest::Reporter* fReporter;
    bool                fIsReleased;

    void doRelease() {
        REPORTER_ASSERT(fReporter, false == fIsReleased);
        fIsReleased = true;
    }

    static void ReleaseProc(void* context) {
        ((ReleaseTextureContext*)context)->doRelease();
    }
};

static SkImage* make_desc_image(GrContext* ctx, int w, int h, GrBackendObject texID,
                                ReleaseTextureContext* releaseContext) {
    GrBackendTextureDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    // need to be a rendertarget for now...
    desc.fFlags = kRenderTarget_GrBackendTextureFlag;
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fSampleCnt = 0;
    desc.fTextureHandle = texID;
    return releaseContext
                ? SkImage::NewFromTexture(ctx, desc, kPremul_SkAlphaType,
                                          ReleaseTextureContext::ReleaseProc, releaseContext)
                : SkImage::NewFromTextureCopy(ctx, desc, kPremul_SkAlphaType);
}

static void test_image_color(skiatest::Reporter* reporter, SkImage* image, SkPMColor expected) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    SkPMColor pixel;
    REPORTER_ASSERT(reporter, image->readPixels(info, &pixel, sizeof(pixel), 0, 0));
    REPORTER_ASSERT(reporter, pixel == expected);
}

DEF_GPUTEST(SkImage_NewFromTexture, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (!ctx) {
        REPORTER_ASSERT(reporter, false);
        return;
    }
    GrTextureProvider* provider = ctx->textureProvider();
    
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
    
    SkAutoTUnref<GrTexture> tex(provider->createTexture(desc, false, storage, w * 4));
    if (!tex) {
        REPORTER_ASSERT(reporter, false);
        return;
    }

    GrBackendObject srcTex = tex->getTextureHandle();
    ReleaseTextureContext releaseCtx(reporter);

    SkAutoTUnref<SkImage> refImg(make_desc_image(ctx, w, h, srcTex, &releaseCtx));
    SkAutoTUnref<SkImage> cpyImg(make_desc_image(ctx, w, h, srcTex, nullptr));

    test_image_color(reporter, refImg, expected0);
    test_image_color(reporter, cpyImg, expected0);

    // Now lets jam new colors into our "external" texture, and see if the images notice
    const SkPMColor expected1 = SkPreMultiplyColor(SK_ColorBLUE);
    sk_memset32(storage, expected1, w * h);
    tex->writePixels(0, 0, w, h, kSkia8888_GrPixelConfig, storage, GrContext::kFlushWrites_PixelOp);

    // The cpy'd one should still see the old color
#if 0
    // There is no guarantee that refImg sees the new color. We are free to have made a copy. Our
    // write pixels call violated the contract with refImg and refImg is now undefined.
    test_image_color(reporter, refImg, expected1);
#endif
    test_image_color(reporter, cpyImg, expected0);

    // Now exercise the release proc
    REPORTER_ASSERT(reporter, !releaseCtx.fIsReleased);
    refImg.reset(nullptr); // force a release of the image
    REPORTER_ASSERT(reporter, releaseCtx.fIsReleased);
}
#endif
DEF_GPUTEST(ImageTestsFromSurfaceTestsTODO, reporter, factory) {
    test_image(reporter);
    test_empty_image(reporter);
    test_imagepeek(reporter, factory);
}
