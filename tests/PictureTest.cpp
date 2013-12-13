/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkError.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureUtils.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkStream.h"


static void make_bm(SkBitmap* bm, int w, int h, SkColor color, bool immutable) {
    bm->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bm->allocPixels();
    bm->eraseColor(color);
    if (immutable) {
        bm->setImmutable();
    }
}

typedef void (*DrawBitmapProc)(SkCanvas*, const SkBitmap&, const SkPoint&);

static void drawbitmap_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkPoint& pos) {
    canvas->drawBitmap(bm, pos.fX, pos.fY, NULL);
}

static void drawbitmaprect_proc(SkCanvas* canvas, const SkBitmap& bm,
                                const SkPoint& pos) {
    SkRect r = {
        0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height())
    };
    r.offset(pos.fX, pos.fY);
    canvas->drawBitmapRectToRect(bm, NULL, r, NULL);
}

static void drawshader_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkPoint& pos) {
    SkRect r = {
        0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height())
    };
    r.offset(pos.fX, pos.fY);

    SkShader* s = SkShader::CreateBitmapShader(bm,
                                               SkShader::kClamp_TileMode,
                                               SkShader::kClamp_TileMode);
    SkPaint paint;
    paint.setShader(s)->unref();
    canvas->drawRect(r, paint);
    canvas->drawOval(r, paint);
    SkRRect rr;
    rr.setRectXY(r, 10, 10);
    canvas->drawRRect(rr, paint);
}

// Return a picture with the bitmaps drawn at the specified positions.
static SkPicture* record_bitmaps(const SkBitmap bm[], const SkPoint pos[],
                                 int count, DrawBitmapProc proc) {
    SkPicture* pic = new SkPicture;
    SkCanvas* canvas = pic->beginRecording(1000, 1000);
    for (int i = 0; i < count; ++i) {
        proc(canvas, bm[i], pos[i]);
    }
    pic->endRecording();
    return pic;
}

static void rand_rect(SkRect* rect, SkRandom& rand, SkScalar W, SkScalar H) {
    rect->fLeft   = rand.nextRangeScalar(-W, 2*W);
    rect->fTop    = rand.nextRangeScalar(-H, 2*H);
    rect->fRight  = rect->fLeft + rand.nextRangeScalar(0, W);
    rect->fBottom = rect->fTop + rand.nextRangeScalar(0, H);

    // we integralize rect to make our tests more predictable, since Gather is
    // a little sloppy.
    SkIRect ir;
    rect->round(&ir);
    rect->set(ir);
}

// Allocate result to be large enough to hold subset, and then draw the picture
// into it, offsetting by subset's top/left corner.
static void draw(SkPicture* pic, const SkRect& subset, SkBitmap* result) {
    SkIRect ir;
    subset.roundOut(&ir);
    int w = ir.width();
    int h = ir.height();
    make_bm(result, w, h, 0, false);

    SkCanvas canvas(*result);
    canvas.translate(-SkIntToScalar(ir.left()), -SkIntToScalar(ir.top()));
    canvas.drawPicture(*pic);
}

template <typename T> int find_index(const T* array, T elem, int count) {
    for (int i = 0; i < count; ++i) {
        if (array[i] == elem) {
            return i;
        }
    }
    return -1;
}

// Return true if 'ref' is found in array[]
static bool find(SkPixelRef const * const * array, SkPixelRef const * ref, int count) {
    return find_index<const SkPixelRef*>(array, ref, count) >= 0;
}

// Look at each pixel in bm, and if its color appears in colors[], find the
// corresponding value in refs[] and append that ref into array, skipping
// duplicates of the same value.
static void gather_from_colors(const SkBitmap& bm, SkPixelRef* const refs[],
                               int count, SkTDArray<SkPixelRef*>* array) {
    // Since we only want to return unique values in array, when we scan we just
    // set a bit for each index'd color found. In practice we only have a few
    // distinct colors, so we just use an int's bits as our array. Hence the
    // assert that count <= number-of-bits-in-our-int.
    SkASSERT((unsigned)count <= 32);
    uint32_t bitarray = 0;

    SkAutoLockPixels alp(bm);

    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            SkPMColor pmc = *bm.getAddr32(x, y);
            // the only good case where the color is not found would be if
            // the color is transparent, meaning no bitmap was drawn in that
            // pixel.
            if (pmc) {
                uint32_t index = SkGetPackedR32(pmc);
                SkASSERT(SkGetPackedG32(pmc) == index);
                SkASSERT(SkGetPackedB32(pmc) == index);
                SkASSERT(static_cast<int>(index) < count);
                bitarray |= 1 << index;
            }
        }
    }

    for (int i = 0; i < count; ++i) {
        if (bitarray & (1 << i)) {
            *array->append() = refs[i];
        }
    }
}

static void test_gatherpixelrefs(skiatest::Reporter* reporter) {
    const int IW = 8;
    const int IH = IW;
    const SkScalar W = SkIntToScalar(IW);
    const SkScalar H = W;

    static const int N = 4;
    SkBitmap bm[N];
    SkPixelRef* refs[N];

    const SkPoint pos[] = {
        { 0, 0 }, { W, 0 }, { 0, H }, { W, H }
    };

    // Our convention is that the color components contain the index of their
    // corresponding bitmap/pixelref
    for (int i = 0; i < N; ++i) {
        make_bm(&bm[i], IW, IH, SkColorSetARGB(0xFF, i, i, i), true);
        refs[i] = bm[i].pixelRef();
    }

    static const DrawBitmapProc procs[] = {
        drawbitmap_proc, drawbitmaprect_proc, drawshader_proc
    };

    SkRandom rand;
    for (size_t k = 0; k < SK_ARRAY_COUNT(procs); ++k) {
        SkAutoTUnref<SkPicture> pic(record_bitmaps(bm, pos, N, procs[k]));

        REPORTER_ASSERT(reporter, pic->willPlayBackBitmaps() || N == 0);
        // quick check for a small piece of each quadrant, which should just
        // contain 1 bitmap.
        for (size_t  i = 0; i < SK_ARRAY_COUNT(pos); ++i) {
            SkRect r;
            r.set(2, 2, W - 2, H - 2);
            r.offset(pos[i].fX, pos[i].fY);
            SkAutoDataUnref data(SkPictureUtils::GatherPixelRefs(pic, r));
            REPORTER_ASSERT(reporter, data);
            if (data) {
                int count = static_cast<int>(data->size() / sizeof(SkPixelRef*));
                REPORTER_ASSERT(reporter, 1 == count);
                REPORTER_ASSERT(reporter, *(SkPixelRef**)data->data() == refs[i]);
            }
        }

        // Test a bunch of random (mostly) rects, and compare the gather results
        // with a deduced list of refs by looking at the colors drawn.
        for (int j = 0; j < 100; ++j) {
            SkRect r;
            rand_rect(&r, rand, 2*W, 2*H);

            SkBitmap result;
            draw(pic, r, &result);
            SkTDArray<SkPixelRef*> array;

            SkData* data = SkPictureUtils::GatherPixelRefs(pic, r);
            size_t dataSize = data ? data->size() : 0;
            int gatherCount = static_cast<int>(dataSize / sizeof(SkPixelRef*));
            SkASSERT(gatherCount * sizeof(SkPixelRef*) == dataSize);
            SkPixelRef** gatherRefs = data ? (SkPixelRef**)(data->data()) : NULL;
            SkAutoDataUnref adu(data);

            gather_from_colors(result, refs, N, &array);

            /*
             *  GatherPixelRefs is conservative, so it can return more bitmaps
             *  that we actually can see (usually because of conservative bounds
             *  inflation for antialiasing). Thus our check here is only that
             *  Gather didn't miss any that we actually saw. Even that isn't
             *  a strict requirement on Gather, which is meant to be quick and
             *  only mostly-correct, but at the moment this test should work.
             */
            for (int i = 0; i < array.count(); ++i) {
                bool found = find(gatherRefs, array[i], gatherCount);
                REPORTER_ASSERT(reporter, found);
#if 0
                // enable this block of code to debug failures, as it will rerun
                // the case that failed.
                if (!found) {
                    SkData* data = SkPictureUtils::GatherPixelRefs(pic, r);
                    size_t dataSize = data ? data->size() : 0;
                }
#endif
            }
        }
    }
}

#ifdef SK_DEBUG
// Ensure that deleting SkPicturePlayback does not assert. Asserts only fire in debug mode, so only
// run in debug mode.
static void test_deleting_empty_playback() {
    SkPicture picture;
    // Creates an SkPictureRecord
    picture.beginRecording(0, 0);
    // Turns that into an SkPicturePlayback
    picture.endRecording();
    // Deletes the old SkPicturePlayback, and creates a new SkPictureRecord
    picture.beginRecording(0, 0);
}

// Ensure that serializing an empty picture does not assert. Likewise only runs in debug mode.
static void test_serializing_empty_picture() {
    SkPicture picture;
    picture.beginRecording(0, 0);
    picture.endRecording();
    SkDynamicMemoryWStream stream;
    picture.serialize(&stream);
}
#endif

static void rand_op(SkCanvas* canvas, SkRandom& rand) {
    SkPaint paint;
    SkRect rect = SkRect::MakeWH(50, 50);

    SkScalar unit = rand.nextUScalar1();
    if (unit <= 0.3) {
//        SkDebugf("save\n");
        canvas->save();
    } else if (unit <= 0.6) {
//        SkDebugf("restore\n");
        canvas->restore();
    } else if (unit <= 0.9) {
//        SkDebugf("clip\n");
        canvas->clipRect(rect);
    } else {
//        SkDebugf("draw\n");
        canvas->drawPaint(paint);
    }
}

static void test_peephole() {
    SkRandom rand;

    for (int j = 0; j < 100; j++) {
        SkRandom rand2(rand); // remember the seed

        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(100, 100);

        for (int i = 0; i < 1000; ++i) {
            rand_op(canvas, rand);
        }
        picture.endRecording();

        rand = rand2;
    }

    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(100, 100);
        SkRect rect = SkRect::MakeWH(50, 50);

        for (int i = 0; i < 100; ++i) {
            canvas->save();
        }
        while (canvas->getSaveCount() > 1) {
            canvas->clipRect(rect);
            canvas->restore();
        }
        picture.endRecording();
    }
}

#ifndef SK_DEBUG
// Only test this is in release mode. We deliberately crash in debug mode, since a valid caller
// should never do this.
static void test_bad_bitmap() {
    // This bitmap has a width and height but no pixels. As a result, attempting to record it will
    // fail.
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    SkPicture picture;
    SkCanvas* recordingCanvas = picture.beginRecording(100, 100);
    recordingCanvas->drawBitmap(bm, 0, 0);
    picture.endRecording();

    SkCanvas canvas;
    canvas.drawPicture(picture);
}
#endif

#include "SkImageEncoder.h"

static SkData* encode_bitmap_to_data(size_t* offset, const SkBitmap& bm) {
    *offset = 0;
    return SkImageEncoder::EncodeData(bm, SkImageEncoder::kPNG_Type, 100);
}

static SkData* serialized_picture_from_bitmap(const SkBitmap& bitmap) {
    SkPicture picture;
    SkCanvas* canvas = picture.beginRecording(bitmap.width(), bitmap.height());
    canvas->drawBitmap(bitmap, 0, 0);
    SkDynamicMemoryWStream wStream;
    picture.serialize(&wStream, &encode_bitmap_to_data);
    return wStream.copyToData();
}

struct ErrorContext {
    int fErrors;
    skiatest::Reporter* fReporter;
};

static void assert_one_parse_error_cb(SkError error, void* context) {
    ErrorContext* errorContext = static_cast<ErrorContext*>(context);
    errorContext->fErrors++;
    // This test only expects one error, and that is a kParseError. If there are others,
    // there is some unknown problem.
    REPORTER_ASSERT_MESSAGE(errorContext->fReporter, 1 == errorContext->fErrors,
                            "This threw more errors than expected.");
    REPORTER_ASSERT_MESSAGE(errorContext->fReporter, kParseError_SkError == error,
                            SkGetLastErrorString());
}

static void test_bitmap_with_encoded_data(skiatest::Reporter* reporter) {
    // Create a bitmap that will be encoded.
    SkBitmap original;
    make_bm(&original, 100, 100, SK_ColorBLUE, true);
    SkDynamicMemoryWStream wStream;
    if (!SkImageEncoder::EncodeStream(&wStream, original, SkImageEncoder::kPNG_Type, 100)) {
        return;
    }
    SkAutoDataUnref data(wStream.copyToData());

    SkBitmap bm;
    bool installSuccess = SkDecodingImageGenerator::Install(data, &bm);
    REPORTER_ASSERT(reporter, installSuccess);

    // Write both bitmaps to pictures, and ensure that the resulting data streams are the same.
    // Flattening original will follow the old path of performing an encode, while flattening bm
    // will use the already encoded data.
    SkAutoDataUnref picture1(serialized_picture_from_bitmap(original));
    SkAutoDataUnref picture2(serialized_picture_from_bitmap(bm));
    REPORTER_ASSERT(reporter, picture1->equals(picture2));
    // Now test that a parse error was generated when trying to create a new SkPicture without
    // providing a function to decode the bitmap.
    ErrorContext context;
    context.fErrors = 0;
    context.fReporter = reporter;
    SkSetErrorCallback(assert_one_parse_error_cb, &context);
    SkMemoryStream pictureStream(picture1);
    SkClearLastError();
    SkAutoUnref pictureFromStream(SkPicture::CreateFromStream(&pictureStream, NULL));
    REPORTER_ASSERT(reporter, pictureFromStream.get() != NULL);
    SkClearLastError();
    SkSetErrorCallback(NULL, NULL);
}

static void test_clone_empty(skiatest::Reporter* reporter) {
    // This is a regression test for crbug.com/172062
    // Before the fix, we used to crash accessing a null pointer when we
    // had a picture with no paints. This test passes by not crashing.
    {
        SkPicture picture;
        picture.beginRecording(1, 1);
        picture.endRecording();
        SkPicture* destPicture = picture.clone();
        REPORTER_ASSERT(reporter, NULL != destPicture);
        destPicture->unref();
    }
    {
        // Test without call to endRecording
        SkPicture picture;
        picture.beginRecording(1, 1);
        SkPicture* destPicture = picture.clone();
        REPORTER_ASSERT(reporter, NULL != destPicture);
        destPicture->unref();
    }
}

static void test_clip_bound_opt(skiatest::Reporter* reporter) {
    // Test for crbug.com/229011
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(4), SkIntToScalar(4),
                                    SkIntToScalar(2), SkIntToScalar(2));
    SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(7), SkIntToScalar(7),
                                    SkIntToScalar(1), SkIntToScalar(1));
    SkRect rect3 = SkRect::MakeXYWH(SkIntToScalar(6), SkIntToScalar(6),
                                    SkIntToScalar(1), SkIntToScalar(1));

    SkPath invPath;
    invPath.addOval(rect1);
    invPath.setFillType(SkPath::kInverseEvenOdd_FillType);
    SkPath path;
    path.addOval(rect2);
    SkPath path2;
    path2.addOval(rect3);
    SkIRect clipBounds;
    // Minimalist test set for 100% code coverage of
    // SkPictureRecord::updateClipConservativelyUsingBounds
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(invPath, SkRegion::kIntersect_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(invPath, SkRegion::kIntersect_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(invPath, SkRegion::kUnion_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kDifference_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kReverseDifference_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        // True clip is actually empty in this case, but the best
        // determination we can make using only bounds as input is that the
        // clip is included in the bounds of 'path'.
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(path2, SkRegion::kXOR_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
}

/**
 * A canvas that records the number of clip commands.
 */
class ClipCountingCanvas : public SkCanvas {
public:
    explicit ClipCountingCanvas(SkBaseDevice* device)
        : SkCanvas(device)
        , fClipCount(0){
    }

    virtual bool clipRect(const SkRect& r, SkRegion::Op op, bool doAA)
        SK_OVERRIDE {
        fClipCount += 1;
        return this->INHERITED::clipRect(r, op, doAA);
    }

    virtual bool clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA)
        SK_OVERRIDE {
        fClipCount += 1;
        return this->INHERITED::clipRRect(rrect, op, doAA);
    }

    virtual bool clipPath(const SkPath& path, SkRegion::Op op, bool doAA)
        SK_OVERRIDE {
        fClipCount += 1;
        return this->INHERITED::clipPath(path, op, doAA);
    }

    unsigned getClipCount() const { return fClipCount; }

private:
    unsigned fClipCount;

    typedef SkCanvas INHERITED;
};

static void test_clip_expansion(skiatest::Reporter* reporter) {
    SkPicture picture;
    SkCanvas* canvas = picture.beginRecording(10, 10, 0);

    canvas->clipRect(SkRect::MakeEmpty(), SkRegion::kReplace_Op);
    // The following expanding clip should not be skipped.
    canvas->clipRect(SkRect::MakeXYWH(4, 4, 3, 3), SkRegion::kUnion_Op);
    // Draw something so the optimizer doesn't just fold the world.
    SkPaint p;
    p.setColor(SK_ColorBLUE);
    canvas->drawPaint(p);

    SkBitmapDevice testDevice(SkBitmap::kNo_Config, 10, 10);
    ClipCountingCanvas testCanvas(&testDevice);
    picture.draw(&testCanvas);

    // Both clips should be present on playback.
    REPORTER_ASSERT(reporter, testCanvas.getClipCount() == 2);
}

static void test_hierarchical(skiatest::Reporter* reporter) {
    SkBitmap bm;
    make_bm(&bm, 10, 10, SK_ColorRED, true);

    SkCanvas* canvas;

    SkPicture childPlain;
    childPlain.beginRecording(10, 10);
    childPlain.endRecording();
    REPORTER_ASSERT(reporter, !childPlain.willPlayBackBitmaps()); // 0

    SkPicture childWithBitmap;
    childWithBitmap.beginRecording(10, 10)->drawBitmap(bm, 0, 0);
    childWithBitmap.endRecording();
    REPORTER_ASSERT(reporter, childWithBitmap.willPlayBackBitmaps()); // 1

    SkPicture parentPP;
    canvas = parentPP.beginRecording(10, 10);
    canvas->drawPicture(childPlain);
    parentPP.endRecording();
    REPORTER_ASSERT(reporter, !parentPP.willPlayBackBitmaps()); // 0

    SkPicture parentPWB;
    canvas = parentPWB.beginRecording(10, 10);
    canvas->drawPicture(childWithBitmap);
    parentPWB.endRecording();
    REPORTER_ASSERT(reporter, parentPWB.willPlayBackBitmaps()); // 1

    SkPicture parentWBP;
    canvas = parentWBP.beginRecording(10, 10);
    canvas->drawBitmap(bm, 0, 0);
    canvas->drawPicture(childPlain);
    parentWBP.endRecording();
    REPORTER_ASSERT(reporter, parentWBP.willPlayBackBitmaps()); // 1

    SkPicture parentWBWB;
    canvas = parentWBWB.beginRecording(10, 10);
    canvas->drawBitmap(bm, 0, 0);
    canvas->drawPicture(childWithBitmap);
    parentWBWB.endRecording();
    REPORTER_ASSERT(reporter, parentWBWB.willPlayBackBitmaps()); // 2
}

DEF_TEST(Picture, reporter) {
#ifdef SK_DEBUG
    test_deleting_empty_playback();
    test_serializing_empty_picture();
#else
    test_bad_bitmap();
#endif
    test_peephole();
    test_gatherpixelrefs(reporter);
    test_bitmap_with_encoded_data(reporter);
    test_clone_empty(reporter);
    test_clip_bound_opt(reporter);
    test_clip_expansion(reporter);
    test_hierarchical(reporter);
}
