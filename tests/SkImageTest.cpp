/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkColor.h"
#include "SkDrawFilter.h"
#include "SkImagePriv.h"
#include "Test.h"

static const int gWidth = 20;
static const int gHeight = 20;

// Tests that SkNewImageFromBitmap obeys pixelref origin.
DEF_TEST(SkImageFromBitmap_extractSubset, reporter) {
    SkAutoTUnref<SkImage> image;
    {
        SkBitmap srcBitmap;
        srcBitmap.allocN32Pixels(gWidth, gHeight);
        srcBitmap.eraseColor(SK_ColorRED);
        SkBitmapDevice dev(srcBitmap);
        SkCanvas canvas(&dev);
        SkIRect r = SkIRect::MakeXYWH(5, 5, gWidth - 5, gWidth - 5);
        SkPaint p;
        p.setColor(SK_ColorGREEN);
        canvas.drawIRect(r, p);
        SkBitmap dstBitmap;
        srcBitmap.extractSubset(&dstBitmap, r);
        image.reset(SkNewImageFromBitmap(dstBitmap, true, NULL));
    }

    SkBitmap tgt;
    tgt.allocN32Pixels(gWidth, gHeight);
    SkBitmapDevice dev(tgt);
    SkCanvas canvas(&dev);
    canvas.clear(SK_ColorTRANSPARENT);
    canvas.drawImage(image, 0, 0, NULL);

    uint32_t pixel = 0;
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
    canvas.readPixels(info, &pixel, 4, gWidth - 6, gWidth - 6);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    canvas.readPixels(info, &pixel, 4, gWidth - 5, gWidth - 5);
    REPORTER_ASSERT(reporter, pixel == SK_ColorTRANSPARENT);
}

namespace {
class TestDrawFilterImage : public SkDrawFilter {
public:
    TestDrawFilterImage()
            : fFilteredImage(0)
            , fFilteredOthers(0)
            , fPreventImages(true)
            , fPreventOthers(true) {
    }

    bool filter(SkPaint*, Type type) SK_OVERRIDE {
        if (type == SkDrawFilter::kImage_Type) {
            if (fPreventImages) {
                fFilteredImage++;
                return false;
            }
            return true;
        }

        if (fPreventOthers) {
            fFilteredOthers++;
            return false;
        }
        return true;
    }
    int fFilteredImage;
    int fFilteredOthers;
    bool fPreventImages;
    bool fPreventOthers;
};
}

DEF_TEST(SkCanvas_test_draw_filter_image, reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    TestDrawFilterImage drawFilter;
    SkCanvas canvas(bitmap);

    SkBitmap imageBitmap;
    imageBitmap.allocN32Pixels(1, 1);
    imageBitmap.eraseColor(SK_ColorGREEN);
    SkAutoTUnref<SkImage> image(SkNewImageFromBitmap(imageBitmap, true, NULL));
    canvas.drawImage(image, 0, 0);

    uint32_t pixel;
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    canvas.setDrawFilter(&drawFilter);
    imageBitmap.eraseColor(SK_ColorRED);
    image.reset(SkNewImageFromBitmap(imageBitmap, true, NULL));
    canvas.drawImage(image, 0, 0);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
    REPORTER_ASSERT(reporter, drawFilter.fFilteredImage == 1);
    REPORTER_ASSERT(reporter, drawFilter.fFilteredOthers == 0);

    // Document a strange case: filtering everything but the images does not work as
    // expected. Images go through, but no pixels appear. (This due to SkCanvas::drawImage() using
    // SkCanvas::drawBitmap() instead of non-existing SkBaseDevice::drawImage()).
    drawFilter.fFilteredImage = 0;
    drawFilter.fPreventImages = false;

    canvas.drawImage(image, 0, 0);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
    REPORTER_ASSERT(reporter, drawFilter.fFilteredImage == 0);
    REPORTER_ASSERT(reporter, drawFilter.fFilteredOthers == 1);
}

namespace {
/*
 *  Subclass of looper that just draws once with one pixel offset.
 */
class OnceTestLooper : public SkDrawLooper {
public:
    OnceTestLooper() { }

    SkDrawLooper::Context* createContext(SkCanvas*, void* storage) const SK_OVERRIDE {
        return SkNEW_PLACEMENT(storage, OnceTestLooperContext());
    }

    size_t contextSize() const SK_OVERRIDE { return sizeof(OnceTestLooperContext); }

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const SK_OVERRIDE {
        str->append("OnceTestLooper:");
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(OnceTestLooper);

private:
    class OnceTestLooperContext : public SkDrawLooper::Context {
    public:
        OnceTestLooperContext() : fCount(0) {}
        virtual ~OnceTestLooperContext() {}

        bool next(SkCanvas* canvas, SkPaint*) SK_OVERRIDE {
            SkASSERT(fCount <= 1);
            canvas->translate(0, 1);
            return fCount++ < 1;
        }
    private:
        unsigned fCount;
    };
};

SkFlattenable* OnceTestLooper::CreateProc(SkReadBuffer&) { return SkNEW(OnceTestLooper); }
}

DEF_TEST(SkCanvas_test_draw_looper_image, reporter) {
    // Test that drawImage loops with the looper the correct way. At the time of writing, this was
    // tricky because drawImage was implemented with drawBitmap. The drawBitmap uses applies the
    // looper.
    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    bitmap.eraseColor(SK_ColorRED);
    OnceTestLooper drawLooper;
    SkCanvas canvas(bitmap);

    SkBitmap imageBitmap;
    imageBitmap.allocN32Pixels(1, 1);
    imageBitmap.eraseColor(SK_ColorGREEN);
    SkAutoTUnref<SkImage> image(SkNewImageFromBitmap(imageBitmap, true, NULL));
    SkPaint p;
    p.setLooper(&drawLooper);
    canvas.drawImage(image, 0, 0, &p);

    uint32_t pixel;
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    canvas.readPixels(info, &pixel, 4, 0, 1);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorRED);
    canvas.readPixels(info, &pixel, 4, 0, 2);
    REPORTER_ASSERT(reporter, pixel == SK_ColorRED);
}

