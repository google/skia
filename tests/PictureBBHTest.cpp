/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "src/core/SkBBoxHierarchy.h"
#include "src/core/SkRectPriv.h"

#include "tests/Test.h"

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

DEF_TEST(PictureNegativeSpace, r) {
    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    SkRect cull = {-200,-200,+200,+200};

    {
        auto canvas = recorder.beginRecording(cull, &factory);
            canvas->save();
            canvas->clipRect(cull);
            canvas->drawRect({-20,-20,-10,-10}, SkPaint{});
            canvas->drawRect({-20,-20,-10,-10}, SkPaint{});
            canvas->restore();
        auto pic = recorder.finishRecordingAsPicture();
        REPORTER_ASSERT(r, pic->approximateOpCount() == 5);
        REPORTER_ASSERT(r, pic->cullRect() == (SkRect{-20,-20,-10,-10}));
    }

    {
        auto canvas = recorder.beginRecording(cull, &factory);
            canvas->clipRect(cull);
            canvas->drawRect({-20,-20,-10,-10}, SkPaint{});
            canvas->drawRect({-20,-20,-10,-10}, SkPaint{});
        auto pic = recorder.finishRecordingAsPicture();
        REPORTER_ASSERT(r, pic->approximateOpCount() == 3);
        REPORTER_ASSERT(r, pic->cullRect() == (SkRect{-20,-20,-10,-10}));
    }
}
