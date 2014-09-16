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

DEF_TEST(RecordOpts_SaveSaveLayerRestoreRestore, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    // A previous bug NoOp'd away the first 3 commands.
    recorder.save();
        recorder.saveLayer(NULL, NULL);
        recorder.restore();
    recorder.restore();

    SkRecordNoopSaveRestores(&record);
    assert_type<SkRecords::Save>     (r, record, 0);
    assert_type<SkRecords::SaveLayer>(r, record, 1);
    assert_type<SkRecords::Restore>  (r, record, 2);
    assert_type<SkRecords::Restore>  (r, record, 3);
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
