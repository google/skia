/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkBBoxHierarchy.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"

#include "Test.h"

class PictureBBHTestBase {
public:
    PictureBBHTestBase(int playbackWidth, int playbackHeight,
        int recordWidth, int recordHeight) {

        fResultBitmap.allocN32Pixels(playbackWidth, playbackHeight);
        fPictureWidth = recordWidth;
        fPictureHeight = recordHeight;
    }

    virtual ~PictureBBHTestBase() { }

    virtual void doTest(SkCanvas& playbackCanvas, SkCanvas& recordingCanvas) = 0;

    void run(skiatest::Reporter* reporter) {
        // No BBH
        this->run(nullptr, reporter);

        // With an R-Tree
        SkRTreeFactory RTreeFactory;
        this->run(&RTreeFactory, reporter);
    }

private:
    void run(SkBBHFactory* factory, skiatest::Reporter* reporter) {
        SkCanvas playbackCanvas(fResultBitmap);
        playbackCanvas.clear(SK_ColorGREEN);
        SkPictureRecorder recorder;
        SkCanvas* recordCanvas = recorder.beginRecording(SkIntToScalar(fPictureWidth),
                                                         SkIntToScalar(fPictureHeight),
                                                         factory);
        this->doTest(playbackCanvas, *recordCanvas);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        playbackCanvas.drawPicture(picture);
        REPORTER_ASSERT(reporter, SK_ColorGREEN == fResultBitmap.getColor(0, 0));
    }

    SkBitmap fResultBitmap;
    int fPictureWidth, fPictureHeight;
};

// Test to verify the playback of an empty picture
//
class DrawEmptyPictureBBHTest : public PictureBBHTestBase {
public:
    DrawEmptyPictureBBHTest()
        : PictureBBHTestBase(2, 2, 1, 1) {}
    ~DrawEmptyPictureBBHTest() override {}

    void doTest(SkCanvas&, SkCanvas&) override {}
};

// Test to verify the playback of a picture into a canvas that has
// an empty clip.
//
class EmptyClipPictureBBHTest : public PictureBBHTestBase {
public:
    EmptyClipPictureBBHTest()
        : PictureBBHTestBase(2, 2, 3, 3) {}

    void doTest(SkCanvas& playbackCanvas, SkCanvas& recordingCanvas) override {
        // intersect with out of bounds rect -> empty clip.
        playbackCanvas.clipRect(SkRect::MakeXYWH(10, 10, 1, 1));
        SkPaint paint;
        recordingCanvas.drawRect(SkRect::MakeWH(3, 3), paint);
    }

    ~EmptyClipPictureBBHTest() override {}
};

DEF_TEST(PictureBBH, reporter) {

    DrawEmptyPictureBBHTest emptyPictureTest;
    emptyPictureTest.run(reporter);

    EmptyClipPictureBBHTest emptyClipPictureTest;
    emptyClipPictureTest.run(reporter);
}

DEF_TEST(RTreeMakeLargest, r) {
    // A call to insert() with 2 or more rects and a bounds of SkRect::MakeLargest()
    // used to fall into an infinite loop.

    SkRTreeFactory factory;
    std::unique_ptr<SkBBoxHierarchy> bbh{ factory(SkRect::MakeLargest()) };

    SkRect rects[] = { {0,0, 10,10}, {5,5,15,15} };
    bbh->insert(rects, SK_ARRAY_COUNT(rects));
    REPORTER_ASSERT(r, bbh->getRootBound() == SkRect::MakeWH(15,15));
}
