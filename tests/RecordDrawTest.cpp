/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "RecordTestUtils.h"

#include "SkDebugCanvas.h"
#include "SkDrawPictureCallback.h"
#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkRecords.h"

static const int W = 1920, H = 1080;

static void draw_pos_text_h(SkCanvas* canvas, const char* text, SkScalar y) {
    const size_t len = strlen(text);
    SkAutoTMalloc<SkScalar> xpos(len);
    for (size_t i = 0; i < len; i++) {
        xpos[i] = (SkScalar)i;
    }
    canvas->drawPosTextH(text, len, xpos, y, SkPaint());
}

class JustOneDraw : public SkDrawPictureCallback {
public:
    JustOneDraw() : fCalls(0) {}

    virtual bool abortDrawing() SK_OVERRIDE { return fCalls++ > 0; }
private:
    int fCalls;
};

DEF_TEST(RecordDraw_Abort, r) {
    // Record two commands.
    SkRecord record;
    SkRecorder recorder(&record, W, H);
    recorder.drawRect(SkRect::MakeWH(200, 300), SkPaint());
    recorder.clipRect(SkRect::MakeWH(100, 200));

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, W, H);

    JustOneDraw callback;
    SkRecordDraw(record, &canvas, &callback);

    REPORTER_ASSERT(r, 3 == rerecord.count());
    assert_type<SkRecords::Save>    (r, rerecord, 0);
    assert_type<SkRecords::DrawRect>(r, rerecord, 1);
    assert_type<SkRecords::Restore> (r, rerecord, 2);
}

DEF_TEST(RecordDraw_Unbalanced, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);
    recorder.save();  // We won't balance this, but SkRecordDraw will for us.

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, W, H);
    SkRecordDraw(record, &canvas);

    REPORTER_ASSERT(r, 4 == rerecord.count());
    assert_type<SkRecords::Save>    (r, rerecord, 0);
    assert_type<SkRecords::Save>    (r, rerecord, 1);
    assert_type<SkRecords::Restore> (r, rerecord, 2);
    assert_type<SkRecords::Restore> (r, rerecord, 3);
}

// Rerecord into another SkRecord with a clip.
static void record_clipped(const SkRecord& record, SkRect clip, SkRecord* clipped) {
    SkRecorder recorder(clipped, W, H);
    recorder.clipRect(clip);
    SkRecordDraw(record, &recorder);
}

DEF_TEST(RecordDraw_PosTextHQuickReject, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    draw_pos_text_h(&recorder, "This will draw.", 20);
    draw_pos_text_h(&recorder, "This won't.", 5000);

    SkRecordBoundDrawPosTextH(&record);

    SkRecord clipped;
    record_clipped(record, SkRect::MakeLTRB(20, 20, 200, 200), &clipped);

    REPORTER_ASSERT(r, 4 == clipped.count());
    assert_type<SkRecords::ClipRect>    (r, clipped, 0);
    assert_type<SkRecords::Save>        (r, clipped, 1);
    assert_type<SkRecords::DrawPosTextH>(r, clipped, 2);
    assert_type<SkRecords::Restore>     (r, clipped, 3);
}

DEF_TEST(RecordDraw_Culling, r) {
    // Record these 7 drawing commands verbatim.
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());
        recorder.drawRect(SkRect::MakeWH(30, 30), SkPaint());
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    // Take a pass over to match up pushCulls and popCulls.
    SkRecordAnnotateCullingPairs(&record);

    // This clip intersects the outer cull, but allows us to quick reject the inner one.
    SkRecord clipped;
    record_clipped(record, SkRect::MakeLTRB(20, 20, 200, 200), &clipped);

    // If culling weren't working, we'd see 3 more commands recorded here.
    REPORTER_ASSERT(r, 7 == clipped.count());
    assert_type<SkRecords::ClipRect>(r, clipped, 0);
    assert_type<SkRecords::Save>    (r, clipped, 1);
    assert_type<SkRecords::PushCull>(r, clipped, 2);
    assert_type<SkRecords::DrawRect>(r, clipped, 3);
    assert_type<SkRecords::DrawRect>(r, clipped, 4);
    assert_type<SkRecords::PopCull> (r, clipped, 5);
    assert_type<SkRecords::Restore> (r, clipped, 6);
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

    SkRecordDraw(scaleRecord, &translateCanvas);
    REPORTER_ASSERT(r, 4 == translateRecord.count());
    assert_type<SkRecords::SetMatrix>(r, translateRecord, 0);
    assert_type<SkRecords::Save>     (r, translateRecord, 1);
    assert_type<SkRecords::SetMatrix>(r, translateRecord, 2);
    assert_type<SkRecords::Restore>  (r, translateRecord, 3);

    // When we look at translateRecord now, it should have its first +20,+20 translate,
    // then a 2x,3x scale that's been concatted with that +20,+20 translate.
    const SkRecords::SetMatrix* setMatrix;
    setMatrix = assert_type<SkRecords::SetMatrix>(r, translateRecord, 0);
    REPORTER_ASSERT(r, setMatrix->matrix == translate);

    setMatrix = assert_type<SkRecords::SetMatrix>(r, translateRecord, 2);
    SkMatrix expected = scale;
    expected.postConcat(translate);
    REPORTER_ASSERT(r, setMatrix->matrix == expected);
}
