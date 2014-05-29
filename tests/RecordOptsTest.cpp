/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "RecordTestUtils.h"

#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkRecords.h"
#include "SkXfermode.h"

static const int W = 1920, H = 1080;

DEF_TEST(RecordOpts_Culling, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

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

DEF_TEST(RecordOpts_NoopCulls, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    // All should be nooped.
    recorder.pushCull(SkRect::MakeWH(200, 200));
        recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.popCull();
    recorder.popCull();

    // Kept for now.  We could peel off a layer of culling.
    recorder.pushCull(SkRect::MakeWH(5, 5));
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    SkRecordNoopCulls(&record);

    for (unsigned i = 0; i < 4; i++) {
        assert_type<SkRecords::NoOp>(r, record, i);
    }
    assert_type<SkRecords::PushCull>(r, record, 4);
    assert_type<SkRecords::PushCull>(r, record, 5);
    assert_type<SkRecords::DrawRect>(r, record, 6);
    assert_type<SkRecords::PopCull>(r, record, 7);
    assert_type<SkRecords::PopCull>(r, record, 8);
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
    SkRecorder recorder(&record, W, H);

    // We can convert a drawPosText into a drawPosTextH when all the Ys are the same.
    draw_pos_text(&recorder, "This will be reduced to drawPosTextH.", true);
    draw_pos_text(&recorder, "This cannot be reduced to drawPosTextH.", false);

    SkRecordReduceDrawPosTextStrength(&record);

    assert_type<SkRecords::DrawPosTextH>(r, record, 0);
    assert_type<SkRecords::DrawPosText>(r, record, 1);
}

DEF_TEST(RecordOpts_TextBounding, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

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

DEF_TEST(RecordOpts_NoopDrawSaveRestore, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    // The save and restore are pointless if there's only draw commands in the middle.
    recorder.save();
        recorder.drawRect(SkRect::MakeWH(200, 200), SkPaint());
        recorder.drawRect(SkRect::MakeWH(300, 300), SkPaint());
        recorder.drawRect(SkRect::MakeWH(100, 100), SkPaint());
    recorder.restore();

    record.replace<SkRecords::NoOp>(2);  // NoOps should be allowed.

    SkRecordNoopSaveRestores(&record);

    assert_type<SkRecords::NoOp>(r, record, 0);
    assert_type<SkRecords::DrawRect>(r, record, 1);
    assert_type<SkRecords::NoOp>(r, record, 2);
    assert_type<SkRecords::DrawRect>(r, record, 3);
    assert_type<SkRecords::NoOp>(r, record, 4);
}

DEF_TEST(RecordOpts_SingleNoopSaveRestore, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

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
    SkRecorder recorder(&record, W, H);

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

    SkRecordNoopSaveRestores(&record);
    for (unsigned index = 0; index < 8; index++) {
        assert_type<SkRecords::NoOp>(r, record, index);
    }
}

static void assert_savelayer_restore(skiatest::Reporter* r,
                                     SkRecord* record,
                                     unsigned i,
                                     bool shouldBeNoOped) {
    SkRecordNoopSaveLayerDrawRestores(record);
    if (shouldBeNoOped) {
        assert_type<SkRecords::NoOp>(r, *record, i);
        assert_type<SkRecords::NoOp>(r, *record, i+2);
    } else {
        assert_type<SkRecords::SaveLayer>(r, *record, i);
        assert_type<SkRecords::Restore>(r, *record, i+2);
    }
}

DEF_TEST(RecordOpts_NoopSaveLayerDrawRestore, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    SkRect bounds = SkRect::MakeWH(100, 200);
    SkRect   draw = SkRect::MakeWH(50, 60);

    SkPaint goodLayerPaint, badLayerPaint, worseLayerPaint;
    goodLayerPaint.setColor(0x03000000);  // Only alpha.
    badLayerPaint.setColor( 0x03040506);  // Not only alpha.
    worseLayerPaint.setXfermodeMode(SkXfermode::kDstIn_Mode);  // Any effect will do.

    SkPaint goodDrawPaint, badDrawPaint;
    goodDrawPaint.setColor(0xFF020202);  // Opaque.
    badDrawPaint.setColor( 0x0F020202);  // Not opaque.

    // No change: optimization can't handle bounds.
    recorder.saveLayer(&bounds, NULL);
        recorder.drawRect(draw, goodDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 0, false);

    // SaveLayer/Restore removed: no bounds + no paint = no point.
    recorder.saveLayer(NULL, NULL);
        recorder.drawRect(draw, goodDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 3, true);

    // TODO(mtklein): test case with null draw paint

    // No change: layer paint isn't alpha-only.
    recorder.saveLayer(NULL, &badLayerPaint);
        recorder.drawRect(draw, goodDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 6, false);

    // No change: layer paint has an effect.
    recorder.saveLayer(NULL, &worseLayerPaint);
        recorder.drawRect(draw, goodDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 9, false);

    // No change: draw paint isn't opaque.
    recorder.saveLayer(NULL, &goodLayerPaint);
        recorder.drawRect(draw, badDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 12, false);

    // SaveLayer/Restore removed: we can fold in the alpha!
    recorder.saveLayer(NULL, &goodLayerPaint);
        recorder.drawRect(draw, goodDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 15, true);

    const SkRecords::DrawRect* drawRect = assert_type<SkRecords::DrawRect>(r, record, 16);
    REPORTER_ASSERT(r, drawRect != NULL);
    REPORTER_ASSERT(r, drawRect->paint.getColor() == 0x03020202);
}
