/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "RecordTestUtils.h"

#include "SkColorFilter.h"
#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkRecords.h"
#include "SkXfermode.h"
#include "SkPictureRecorder.h"
#include "SkPictureImageFilter.h"

static const int W = 1920, H = 1080;

DEF_TEST(RecordOpts_NoopDraw, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    recorder.drawRect(SkRect::MakeWH(200, 200), SkPaint());
    recorder.drawRect(SkRect::MakeWH(300, 300), SkPaint());
    recorder.drawRect(SkRect::MakeWH(100, 100), SkPaint());

    record.replace<SkRecords::NoOp>(1);  // NoOps should be allowed.

    SkRecordNoopSaveRestores(&record);

    REPORTER_ASSERT(r, 2 == count_instances_of_type<SkRecords::DrawRect>(record));
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
    for (unsigned index = 0; index < record.count(); index++) {
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
    switch (record.count()) {
        case 4:
            assert_type<SkRecords::Save>     (r, record, 0);
            assert_type<SkRecords::SaveLayer>(r, record, 1);
            assert_type<SkRecords::Restore>  (r, record, 2);
            assert_type<SkRecords::Restore>  (r, record, 3);
            break;
        case 2:
            assert_type<SkRecords::SaveLayer>(r, record, 0);
            assert_type<SkRecords::Restore>  (r, record, 1);
            break;
        case 0:
            break;
        default:
            REPORTER_ASSERT(r, false);
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

    SkPaint alphaOnlyLayerPaint, translucentLayerPaint, xfermodeLayerPaint;
    alphaOnlyLayerPaint.setColor(0x03000000);  // Only alpha.
    translucentLayerPaint.setColor(0x03040506);  // Not only alpha.
    xfermodeLayerPaint.setXfermodeMode(SkXfermode::kDstIn_Mode);  // Any effect will do.

    SkPaint opaqueDrawPaint, translucentDrawPaint;
    opaqueDrawPaint.setColor(0xFF020202);  // Opaque.
    translucentDrawPaint.setColor(0x0F020202);  // Not opaque.

    // SaveLayer/Restore removed: No paint = no point.
    recorder.saveLayer(NULL, NULL);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 0, true);

    // Bounds don't matter.
    recorder.saveLayer(&bounds, NULL);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 3, true);

    // TODO(mtklein): test case with null draw paint

    // No change: layer paint isn't alpha-only.
    recorder.saveLayer(NULL, &translucentLayerPaint);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 6, false);

    // No change: layer paint has an effect.
    recorder.saveLayer(NULL, &xfermodeLayerPaint);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 9, false);

    // SaveLayer/Restore removed: we can fold in the alpha!
    recorder.saveLayer(NULL, &alphaOnlyLayerPaint);
        recorder.drawRect(draw, translucentDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 12, true);

    // SaveLayer/Restore removed: we can fold in the alpha!
    recorder.saveLayer(NULL, &alphaOnlyLayerPaint);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_restore(r, &record, 15, true);

    const SkRecords::DrawRect* drawRect = assert_type<SkRecords::DrawRect>(r, record, 16);
    REPORTER_ASSERT(r, drawRect != NULL);
    REPORTER_ASSERT(r, drawRect->paint.getColor() == 0x03020202);
}

static void assert_merge_svg_opacity_and_filter_layers(skiatest::Reporter* r,
                                                       SkRecord* record,
                                                       unsigned i,
                                                       bool shouldBeNoOped) {
    SkRecordMergeSvgOpacityAndFilterLayers(record);
    if (shouldBeNoOped) {
        assert_type<SkRecords::NoOp>(r, *record, i);
        assert_type<SkRecords::NoOp>(r, *record, i + 6);
    } else {
        assert_type<SkRecords::SaveLayer>(r, *record, i);
        assert_type<SkRecords::Restore>(r, *record, i + 6);
    }
}

DEF_TEST(RecordOpts_MergeSvgOpacityAndFilterLayers, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    SkRect bounds = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(200));
    SkRect clip = SkRect::MakeWH(SkIntToScalar(50), SkIntToScalar(60));

    SkPaint alphaOnlyLayerPaint;
    alphaOnlyLayerPaint.setColor(0x03000000);  // Only alpha.
    SkPaint translucentLayerPaint;
    translucentLayerPaint.setColor(0x03040506);  // Not only alpha.
    SkPaint xfermodePaint;
    xfermodePaint.setXfermodeMode(SkXfermode::kDstIn_Mode);
    SkPaint colorFilterPaint;
    colorFilterPaint.setColorFilter(
        SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::kSrcIn_Mode))->unref();

    SkPaint opaqueFilterLayerPaint;
    opaqueFilterLayerPaint.setColor(0xFF020202);  // Opaque.
    SkPaint translucentFilterLayerPaint;
    translucentFilterLayerPaint.setColor(0x0F020202);  // Not opaque.
    SkAutoTUnref<SkPicture> shape;
    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(100), SkIntToScalar(100));
        SkPaint shapePaint;
        shapePaint.setColor(SK_ColorWHITE);
        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(50), SkIntToScalar(50)), shapePaint);
        shape.reset(recorder.endRecordingAsPicture());
    }
    translucentFilterLayerPaint.setImageFilter(SkPictureImageFilter::Create(shape))->unref();

    int index = 0;

    {
        // Any combination of these should cause the pattern to be optimized.
        SkRect* firstBounds[] = { NULL, &bounds };
        SkPaint* firstPaints[] = { NULL, &alphaOnlyLayerPaint };
        SkRect* secondBounds[] = { NULL, &bounds };
        SkPaint* secondPaints[] = { &opaqueFilterLayerPaint, &translucentFilterLayerPaint };

        for (size_t i = 0; i < SK_ARRAY_COUNT(firstBounds); ++ i) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(firstPaints); ++j) {
                for (size_t k = 0; k < SK_ARRAY_COUNT(secondBounds); ++k) {
                    for (size_t m = 0; m < SK_ARRAY_COUNT(secondPaints); ++m) {
                        recorder.saveLayer(firstBounds[i], firstPaints[j]);
                        recorder.save();
                        recorder.clipRect(clip);
                        recorder.saveLayer(secondBounds[k], secondPaints[m]);
                        recorder.restore();
                        recorder.restore();
                        recorder.restore();
                        assert_merge_svg_opacity_and_filter_layers(r, &record, index, true);
                        index += 7;
                    }
                }
            }
        }
    }

    // These should cause the pattern to stay unoptimized:
    struct {
        SkPaint* firstPaint;
        SkPaint* secondPaint;
    } noChangeTests[] = {
        // No change: NULL filter layer paint not implemented.
        { &alphaOnlyLayerPaint, NULL },
        // No change: layer paint is not alpha-only.
        { &translucentLayerPaint, &opaqueFilterLayerPaint },
        // No change: layer paint has an xfereffect.
        { &xfermodePaint, &opaqueFilterLayerPaint },
        // No change: filter layer paint has an xfereffect.
        { &alphaOnlyLayerPaint, &xfermodePaint },
        // No change: layer paint has a color filter.
        { &colorFilterPaint, &opaqueFilterLayerPaint },
        // No change: filter layer paint has a color filter (until the optimization accounts for
        // constant color draws that can filter the color).
        { &alphaOnlyLayerPaint, &colorFilterPaint }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(noChangeTests); ++i) {
        recorder.saveLayer(NULL, noChangeTests[i].firstPaint);
        recorder.save();
        recorder.clipRect(clip);
        recorder.saveLayer(NULL, noChangeTests[i].secondPaint);
        recorder.restore();
        recorder.restore();
        recorder.restore();
        assert_merge_svg_opacity_and_filter_layers(r, &record, index, false);
        index += 7;
    }

    // Test the folded alpha value.
    recorder.saveLayer(NULL, &alphaOnlyLayerPaint);
    recorder.save();
    recorder.clipRect(clip);
    recorder.saveLayer(NULL, &opaqueFilterLayerPaint);
    recorder.restore();
    recorder.restore();
    recorder.restore();
    assert_merge_svg_opacity_and_filter_layers(r, &record, index, true);

    const SkRecords::SaveLayer* saveLayer = assert_type<SkRecords::SaveLayer>(r, record, index + 3);
    REPORTER_ASSERT(r, saveLayer != NULL);
    REPORTER_ASSERT(r, saveLayer->paint->getColor() == 0x03020202);

    index += 7;

    // Test that currently we do not fold alphas for patterns without the clip. This is just not
    // implemented.
    recorder.saveLayer(NULL, &alphaOnlyLayerPaint);
    recorder.saveLayer(NULL, &opaqueFilterLayerPaint);
    recorder.restore();
    recorder.restore();
    SkRecordMergeSvgOpacityAndFilterLayers(&record);
    assert_type<SkRecords::SaveLayer>(r, record, index);
    assert_type<SkRecords::SaveLayer>(r, record, index + 1);
    assert_type<SkRecords::Restore>(r, record, index + 2);
    assert_type<SkRecords::Restore>(r, record, index + 3);
    index += 4;
}
