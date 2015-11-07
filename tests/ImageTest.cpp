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
