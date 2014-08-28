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
    SkRecordDraw(record, &canvas, NULL/*bbh*/, &callback);

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
    SkRecordDraw(record, &canvas, NULL/*bbh*/, NULL/*callback*/);

    REPORTER_ASSERT(r, 4 == rerecord.count());
    assert_type<SkRecords::Save>    (r, rerecord, 0);
    assert_type<SkRecords::Save>    (r, rerecord, 1);
    assert_type<SkRecords::Restore> (r, rerecord, 2);
    assert_type<SkRecords::Restore> (r, rerecord, 3);
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

    SkRecordDraw(scaleRecord, &translateCanvas, NULL/*bbh*/, NULL/*callback*/);
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

struct TestBBH : public SkBBoxHierarchy {
    virtual void insert(void* data, const SkRect& bounds, bool defer) SK_OVERRIDE {
        Entry e = { (uintptr_t)data, bounds };
        entries.push(e);
    }
    virtual int getCount() const SK_OVERRIDE { return entries.count(); }

    virtual void flushDeferredInserts() SK_OVERRIDE {}

    virtual void search(const SkRect& query, SkTDArray<void*>* results) const SK_OVERRIDE {}
    virtual void clear() SK_OVERRIDE {}
    virtual void rewindInserts() SK_OVERRIDE {}
    virtual int getDepth() const SK_OVERRIDE { return -1; }

    struct Entry {
        uintptr_t data;
        SkRect bounds;
    };
    SkTDArray<Entry> entries;
};

// This test is not meant to make total sense yet.  It's testing the status quo
// of SkRecordFillBounds(), which itself doesn't make total sense yet.
DEF_TEST(RecordDraw_BBH, r) {
    TestBBH bbh;

    SkRecord record;

    SkRecorder recorder(&record, W, H);
    recorder.save();
        recorder.clipRect(SkRect::MakeWH(400, 500));
        recorder.scale(2, 2);
        recorder.drawRect(SkRect::MakeWH(320, 240), SkPaint());
    recorder.restore();

    SkRecordFillBounds(record, &bbh);

    REPORTER_ASSERT(r, bbh.entries.count() == 5);
    for (int i = 0; i < bbh.entries.count(); i++) {
        REPORTER_ASSERT(r, bbh.entries[i].data == (uintptr_t)i);

        // We'd like to assert bounds == SkRect::MakeWH(400, 480).
        // But we allow a little slop in recognition that float equality can be weird.
        REPORTER_ASSERT(r,  SkRect::MakeLTRB(-1, -1, 401, 481).contains(bbh.entries[i].bounds));
        REPORTER_ASSERT(r, !SkRect::MakeLTRB(+1, +1, 399, 479).contains(bbh.entries[i].bounds));
    }
}
