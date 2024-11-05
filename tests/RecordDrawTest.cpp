/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecorder.h"
#include "src/core/SkRecords.h"
#include "tests/RecordTestUtils.h"
#include "tests/Test.h"

using namespace skia_private;

class SkImage;

static const int W = 1920, H = 1080;

class JustOneDraw : public SkPicture::AbortCallback {
public:
    JustOneDraw() : fCalls(0) {}

    bool abort() override { return fCalls++ > 0; }
private:
    int fCalls;
};

DEF_TEST(RecordDraw_LazySaves, r) {
    // Record two commands.
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    REPORTER_ASSERT(r, 0 == record.count());
    recorder.save();
    REPORTER_ASSERT(r, 0 == record.count());    // the save was not recorded (yet)
    recorder.drawColor(SK_ColorRED);
    REPORTER_ASSERT(r, 1 == record.count());
    recorder.scale(2, 2);
    REPORTER_ASSERT(r, 3 == record.count());    // now we see the save
    recorder.restore();
    REPORTER_ASSERT(r, 4 == record.count());

    assert_type<SkRecords::DrawPaint>(r, record, 0);
    assert_type<SkRecords::Save>     (r, record, 1);
    assert_type<SkRecords::Scale>    (r, record, 2);
    assert_type<SkRecords::Restore>  (r, record, 3);

    recorder.save();
    recorder.save();
    recorder.restore();
    recorder.restore();
    REPORTER_ASSERT(r, 4 == record.count());
}

DEF_TEST(RecordDraw_Abort, r) {
    // Record two commands.
    SkRecord record;
    SkRecorder recorder(&record, W, H);
    recorder.drawRect(SkRect::MakeWH(200, 300), SkPaint());
    recorder.clipRect(SkRect::MakeWH(100, 200));

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, W, H);

    JustOneDraw callback;
    SkRecordDraw(record, &canvas, nullptr, nullptr, 0, nullptr/*bbh*/, &callback);

    REPORTER_ASSERT(r, 1 == count_instances_of_type<SkRecords::DrawRect>(rerecord));
    REPORTER_ASSERT(r, 0 == count_instances_of_type<SkRecords::ClipRect>(rerecord));
}

DEF_TEST(RecordDraw_Unbalanced, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);
    recorder.save();  // We won't balance this, but SkRecordDraw will for us.
    recorder.scale(2, 2);

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, W, H);
    SkRecordDraw(record, &canvas, nullptr, nullptr, 0, nullptr/*bbh*/, nullptr/*callback*/);

    int save_count = count_instances_of_type<SkRecords::Save>(rerecord);
    int restore_count = count_instances_of_type<SkRecords::Save>(rerecord);
    REPORTER_ASSERT(r, save_count == restore_count);
}

DEF_TEST(RecordDraw_SetMatrixClobber, r) {
    // Set up an SkRecord that just scales by 2x,3x.
    SkRecord scaleRecord;
    SkRecorder scaleCanvas(&scaleRecord, W, H);
    SkMatrix scale;
    scale.setScale(2, 3);
    scaleCanvas.setMatrix(scale);

    // Set up an SkRecord with an initial +20, +20 translate.
    SkRecord translateRecord;
    SkRecorder translateCanvas(&translateRecord, W, H);
    SkMatrix translate;
    translate.setTranslate(20, 20);
    translateCanvas.setMatrix(translate);

    SkRecordDraw(scaleRecord, &translateCanvas, nullptr, nullptr, 0, nullptr/*bbh*/, nullptr/*callback*/);
    REPORTER_ASSERT(r, 4 == translateRecord.count());
    assert_type<SkRecords::SetM44>(r, translateRecord, 0);
    assert_type<SkRecords::Save>  (r, translateRecord, 1);
    assert_type<SkRecords::SetM44>(r, translateRecord, 2);
    assert_type<SkRecords::Restore>  (r, translateRecord, 3);

    // When we look at translateRecord now, it should have its first +20,+20 translate,
    // then a 2x,3x scale that's been concatted with that +20,+20 translate.
    const SkRecords::SetM44* setMatrix;
    setMatrix = assert_type<SkRecords::SetM44>(r, translateRecord, 0);
    REPORTER_ASSERT(r, setMatrix->matrix == SkM44(translate));

    setMatrix = assert_type<SkRecords::SetM44>(r, translateRecord, 2);
    SkMatrix expected = scale;
    expected.postConcat(translate);
    REPORTER_ASSERT(r, setMatrix->matrix == SkM44(expected));
}

// Like a==b, with a little slop recognizing that float equality can be weird.
static bool sloppy_rect_eq(SkRect a, SkRect b) {
    SkRect inset(a), outset(a);
    inset.inset(1, 1);
    outset.outset(1, 1);
    return outset.contains(b) && !inset.contains(b);
}

// TODO This would be nice, but we can't get it right today.
#if 0
DEF_TEST(RecordDraw_BasicBounds, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);
    recorder.save();
        recorder.clipRect(SkRect::MakeWH(400, 500));
        recorder.scale(2, 2);
        recorder.drawRect(SkRect::MakeWH(320, 240), SkPaint());
    recorder.restore();

    AutoTArray<SkRect> bounds(record.count());
    SkRecordFillBounds(SkRect::MakeWH(SkIntToScalar(W), SkIntToScalar(H)), record, bounds.data());

    for (int i = 0; i < record.count(); i++) {
        REPORTER_ASSERT(r, sloppy_rect_eq(SkRect::MakeWH(400, 480), bounds[i]));
    }
}
#endif

// A regression test for crbug.com/415468 and https://bug.skia.org/2957 .
//
// This also now serves as a regression test for crbug.com/418417.  We used to adjust the
// bounds for the saveLayer, clip, and restore to be greater than the bounds of the picture.
// (We were applying the saveLayer paint to the bounds after restore, which makes no sense.)
DEF_TEST(RecordDraw_SaveLayerAffectsClipBounds, r) {
    SkRecord record;
    SkRecorder recorder(&record, 50, 50);

    // We draw a rectangle with a long drop shadow.  We used to not update the clip
    // bounds based on SaveLayer paints, so the drop shadow could be cut off.
    SkPaint paint;
    paint.setImageFilter(SkImageFilters::DropShadow(20, 0, 0, 0, SK_ColorBLACK,  nullptr));

    recorder.saveLayer(nullptr, &paint);
        recorder.clipRect(SkRect::MakeWH(20, 40));
        recorder.drawRect(SkRect::MakeWH(20, 40), SkPaint());
    recorder.restore();

    // Under the original bug, the right edge value of the drawRect would be 20 less than asserted
    // here because we intersected it with a clip that had not been adjusted for the drop shadow.
    //
    // The second bug showed up as adjusting the picture bounds (0,0,50,50) by the drop shadow too.
    // The saveLayer, clipRect, and restore bounds were incorrectly (0,0,70,50).
    //
    // Now, all recorded bounds should be (0,0,40,40), representing the union of the original
    // draw/clip (0,0,20,40) with the 20px offset drop shadow along the x-axis (20,0,40,40).
    // The saveLayer and restore match the output bounds of the drop shadow filter, instead of
    // expanding to fill the entire picture.
    AutoTArray<SkRect> bounds(record.count());
    AutoTMalloc<SkBBoxHierarchy::Metadata> meta(record.count());
    SkRecordFillBounds(SkRect::MakeWH(50, 50), record, bounds.data(), meta);
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[0], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[1], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[2], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[3], SkRect::MakeLTRB(0, 0, 40, 40)));
}

// A regression test for https://g-issues.skia.org/issues/362552959.
DEF_TEST(RecordDraw_EmptySaveLayerWithBackdropFilterAffectsCullRect, r) {
    SkRecord record;
    SkRecorder recorder(&record, 50, 50);

    auto blurFilter = SkImageFilters::Blur(5.0, 5.0, SkTileMode::kDecal, nullptr);

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(40, 40));
        recorder.saveLayer(SkCanvas::SaveLayerRec(nullptr, nullptr, blurFilter.get(),
                           SkTileMode::kDecal, nullptr, 0));
        recorder.restore();
    recorder.restore();

    // Even though there is nothing drawn inside the saveLayer, the bounds should still fill the
    // the clip region due to it affecting the backdrop.
    AutoTArray<SkRect> bounds(record.count());
    AutoTMalloc<SkBBoxHierarchy::Metadata> meta(record.count());
    SkRecordFillBounds(SkRect::MakeWH(50, 50), record, bounds.data(), meta);
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[0], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[1], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[2], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[3], SkRect::MakeLTRB(0, 0, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[4], SkRect::MakeLTRB(0, 0, 40, 40)));
}

DEF_TEST(RecordDraw_Metadata, r) {
    SkRecord record;
    SkRecorder recorder(&record, 50, 50);

    // Just doing some mildly interesting drawing, mostly grabbed from the unit test above.
    SkPaint paint;
    paint.setImageFilter(SkImageFilters::DropShadow(20, 0, 0, 0, SK_ColorBLACK,  nullptr));

    recorder.saveLayer(nullptr, &paint);
        recorder.clipRect(SkRect::MakeWH(20, 40));
        recorder.save();
            recorder.translate(10, 10);
            recorder.drawRect(SkRect::MakeWH(20, 40), SkPaint());
        recorder.restore();
    recorder.restore();

    AutoTArray<SkRect> bounds(record.count());
    AutoTMalloc<SkBBoxHierarchy::Metadata> meta(record.count());
    SkRecordFillBounds(SkRect::MakeWH(50, 50), record, bounds.data(), meta);

    REPORTER_ASSERT(r, !meta[0].isDraw);  // saveLayer (not a draw, but its restore will be)
    REPORTER_ASSERT(r, !meta[1].isDraw);  //   clip
    REPORTER_ASSERT(r, !meta[2].isDraw);  //   save
    REPORTER_ASSERT(r, !meta[3].isDraw);  //       translate
    REPORTER_ASSERT(r,  meta[4].isDraw);  //       drawRect
    REPORTER_ASSERT(r, !meta[5].isDraw);  //   restore  (paired with save, not a draw)
    REPORTER_ASSERT(r,  meta[6].isDraw);  // restore (paired with saveLayer, a draw)
}

// TODO This would be nice, but we can't get it right today.
#if 0
// When a saveLayer provides an explicit bound and has a complex paint (e.g., one that
// affects transparent black), that bound should serve to shrink the area of the required
// backing store.
DEF_TEST(RecordDraw_SaveLayerBoundsAffectsClipBounds, r) {
    SkRecord record;
    SkRecorder recorder(&record, 50, 50);

    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);

    SkRect layerBounds = SkRect::MakeLTRB(10, 10, 40, 40);
    recorder.saveLayer(&layerBounds, &p);
    recorder.drawRect(SkRect::MakeLTRB(20, 20, 30, 30), SkPaint());
    recorder.restore();

    AutoTArray<SkRect> bounds(record.count());
    SkRecordFillBounds(SkRect::MakeWH(50, 50), record, bounds.data());
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[0], SkRect::MakeLTRB(10, 10, 40, 40)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[1], SkRect::MakeLTRB(20, 20, 30, 30)));
    REPORTER_ASSERT(r, sloppy_rect_eq(bounds[2], SkRect::MakeLTRB(10, 10, 40, 40)));
}
#endif

DEF_TEST(RecordDraw_drawImage, r){
    class SkCanvasMock : public SkCanvas {
    public:
        SkCanvasMock(int width, int height) : SkCanvas(width, height) {
            this->resetTestValues();
        }

        void resetTestValues() {
            fDrawImageCalled = fDrawImageRectCalled = false;
        }

        bool fDrawImageCalled;
        bool fDrawImageRectCalled;
    };

    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10)));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());

    SkCanvasMock canvas(10, 10);
}
