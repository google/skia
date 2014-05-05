/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkRecords.h"

static const int W = 1920, H = 1080;

// If the command we're reading is a U, set ptr to it, otherwise set it to NULL.
template <typename U>
struct ReadAs {
    explicit ReadAs(const U** ptr) : ptr(ptr), type(SkRecords::Type(~0)) {}

    const U** ptr;
    SkRecords::Type type;

    void operator()(const U& r) { *ptr = &r; type = U::kType; }

    template <typename T>
    void operator()(const T&) { *ptr = NULL; type = U::kType; }
};

// Assert that the ith command in record is of type T, and return it.
template <typename T>
static const T* assert_type(skiatest::Reporter* r, const SkRecord& record, unsigned index) {
    const T* ptr = NULL;
    ReadAs<T> reader(&ptr);
    record.visit(index, reader);
    REPORTER_ASSERT(r, T::kType == reader.type);
    REPORTER_ASSERT(r, ptr != NULL);
    return ptr;
}

DEF_TEST(RecordOpts_Culling, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    recorder.drawRect(SkRect::MakeWH(1000, 10000), SkPaint());

    recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());
        recorder.drawRect(SkRect::MakeWH(30, 30), SkPaint());
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    SkRecordAnnotateCullingPairs(&record);

    REPORTER_ASSERT(r, 6 == assert_type<SkRecords::PairedPushCull>(r, record, 1)->skip);
    REPORTER_ASSERT(r, 2 == assert_type<SkRecords::PairedPushCull>(r, record, 4)->skip);
}

static void draw_pos_text(SkCanvas* canvas, const char* text, bool constantY) {
    const size_t len = strlen(text);
    SkAutoTMalloc<SkPoint> pos(len);
    for (size_t i = 0; i < len; i++) {
        pos[i].fX = (SkScalar)i;
        pos[i].fY = constantY ? SK_Scalar1 : (SkScalar)i;
    }
    canvas->drawPosText(text, len, pos, SkPaint());
}

DEF_TEST(RecordOpts_StrengthReduction, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    // We can convert a drawPosText into a drawPosTextH when all the Ys are the same.
    draw_pos_text(&recorder, "This will be reduced to drawPosTextH.", true);
    draw_pos_text(&recorder, "This cannot be reduced to drawPosTextH.", false);

    SkRecordReduceDrawPosTextStrength(&record);

    assert_type<SkRecords::DrawPosTextH>(r, record, 0);
    assert_type<SkRecords::DrawPosText>(r, record, 1);
}

DEF_TEST(RecordOpts_TextBounding, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    // First, get a drawPosTextH.  Here's a handy way.  Its text size will be the default (12).
    draw_pos_text(&recorder, "This will be reduced to drawPosTextH.", true);
    SkRecordReduceDrawPosTextStrength(&record);

    const SkRecords::DrawPosTextH* original =
        assert_type<SkRecords::DrawPosTextH>(r, record, 0);

    // This should wrap the original DrawPosTextH with minY and maxY.
    SkRecordBoundDrawPosTextH(&record);

    const SkRecords::BoundedDrawPosTextH* bounded =
        assert_type<SkRecords::BoundedDrawPosTextH>(r, record, 0);

    const SkPaint defaults;
    REPORTER_ASSERT(r, bounded->base == original);
    REPORTER_ASSERT(r, bounded->minY <= SK_Scalar1 - defaults.getTextSize());
    REPORTER_ASSERT(r, bounded->maxY >= SK_Scalar1 + defaults.getTextSize());
}

DEF_TEST(RecordOpts_SingleNoopSaveRestore, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(200, 200));
    recorder.restore();

    SkRecordNoopSaveRestores(&record);
    for (unsigned i = 0; i < 3; i++) {
        assert_type<SkRecords::NoOp>(r, record, i);
    }
}

DEF_TEST(RecordOpts_NoopSaveRestores, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    // The second pass will clean up this pair after the first pass noops all the innards.
    recorder.save();
        // A simple pointless pair of save/restore.
        recorder.save();
        recorder.restore();

        // As long as we don't draw in there, everything is a noop.
        recorder.save();
            recorder.clipRect(SkRect::MakeWH(200, 200));
            recorder.clipRect(SkRect::MakeWH(100, 100));
        recorder.restore();
    recorder.restore();

    // These will be kept (though some future optimization might noop the save and restore).
    recorder.save();
        recorder.drawRect(SkRect::MakeWH(200, 200), SkPaint());
    recorder.restore();

    SkRecordNoopSaveRestores(&record);

    for (unsigned index = 0; index < 8; index++) {
        assert_type<SkRecords::NoOp>(r, record, index);
    }
    assert_type<SkRecords::Save>(r, record, 8);
    assert_type<SkRecords::DrawRect>(r, record, 9);
    assert_type<SkRecords::Restore>(r, record, 10);
}
