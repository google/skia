/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "RecordTestUtils.h"

#include "SkBlurImageFilter.h"
#include "SkColorFilter.h"
#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkRecords.h"
#include "SkPictureRecorder.h"
#include "SkPictureImageFilter.h"
#include "SkSurface.h"

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
    for (int i = 0; i < 3; i++) {
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
    for (int index = 0; index < record.count(); index++) {
        assert_type<SkRecords::NoOp>(r, record, index);
    }
}

DEF_TEST(RecordOpts_SaveSaveLayerRestoreRestore, r) {
    SkRecord record;
    SkRecorder recorder(&record, W, H);

    // A previous bug NoOp'd away the first 3 commands.
    recorder.save();
        recorder.saveLayer(nullptr, nullptr);
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

#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
static void assert_savelayer_restore(skiatest::Reporter* r,
                                     SkRecord* record,
                                     int i,
                                     bool shouldBeNoOped) {
    SkRecordNoopSaveLayerDrawRestores(record);
    if (shouldBeNoOped) {
        assert_type<SkRecords::NoOp>(r, *record, i);
        assert_type<SkRecords::NoOp>(r, *record, i+1);
    } else {
        assert_type<SkRecords::SaveLayer>(r, *record, i);
        assert_type<SkRecords::Restore>(r, *record, i+1);
    }
}

static void assert_savelayer_draw_restore(skiatest::Reporter* r,
                                          SkRecord* record,
                                          int i,
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
    xfermodeLayerPaint.setBlendMode(SkBlendMode::kDstIn);  // Any effect will do.

    SkPaint opaqueDrawPaint, translucentDrawPaint;
    opaqueDrawPaint.setColor(0xFF020202);  // Opaque.
    translucentDrawPaint.setColor(0x0F020202);  // Not opaque.

    // SaveLayer/Restore removed: No paint = no point.
    recorder.saveLayer(nullptr, nullptr);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 0, true);

    // Bounds don't matter.
    recorder.saveLayer(&bounds, nullptr);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 3, true);

    // TODO(mtklein): test case with null draw paint

    // No change: layer paint isn't alpha-only.
    recorder.saveLayer(nullptr, &translucentLayerPaint);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 6, false);

    // No change: layer paint has an effect.
    recorder.saveLayer(nullptr, &xfermodeLayerPaint);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 9, false);

    // SaveLayer/Restore removed: we can fold in the alpha!
    recorder.saveLayer(nullptr, &alphaOnlyLayerPaint);
        recorder.drawRect(draw, translucentDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 12, true);

    // SaveLayer/Restore removed: we can fold in the alpha!
    recorder.saveLayer(nullptr, &alphaOnlyLayerPaint);
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 15, true);

    const SkRecords::DrawRect* drawRect = assert_type<SkRecords::DrawRect>(r, record, 16);
    REPORTER_ASSERT(r, drawRect != nullptr);
    REPORTER_ASSERT(r, drawRect->paint.getColor() == 0x03020202);

    // saveLayer w/ backdrop should NOT go away
    sk_sp<SkImageFilter> filter(SkBlurImageFilter::Make(3, 3, nullptr));
    recorder.saveLayer({ nullptr, nullptr, filter.get(), nullptr, nullptr, 0});
        recorder.drawRect(draw, opaqueDrawPaint);
    recorder.restore();
    assert_savelayer_draw_restore(r, &record, 18, false);

    // saveLayer w/ clip mask should also NOT go away
    {
        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(10, 10));
        recorder.saveLayer({ nullptr, nullptr, nullptr, surface->makeImageSnapshot().get(),
                             nullptr, 0});
            recorder.drawRect(draw, opaqueDrawPaint);
        recorder.restore();
        assert_savelayer_draw_restore(r, &record, 21, false);
    }
}
#endif

static void assert_merge_svg_opacity_and_filter_layers(skiatest::Reporter* r,
                                                       SkRecord* record,
                                                       int i,
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
    xfermodePaint.setBlendMode(SkBlendMode::kDstIn);
    SkPaint colorFilterPaint;
    colorFilterPaint.setColorFilter(
        SkColorFilter::MakeModeFilter(SK_ColorLTGRAY, SkBlendMode::kSrcIn));

    SkPaint opaqueFilterLayerPaint;
    opaqueFilterLayerPaint.setColor(0xFF020202);  // Opaque.
    SkPaint translucentFilterLayerPaint;
    translucentFilterLayerPaint.setColor(0x0F020202);  // Not opaque.
    sk_sp<SkPicture> shape;
    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(100), SkIntToScalar(100));
        SkPaint shapePaint;
        shapePaint.setColor(SK_ColorWHITE);
        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(50), SkIntToScalar(50)), shapePaint);
        shape = recorder.finishRecordingAsPicture();
    }
    translucentFilterLayerPaint.setImageFilter(SkPictureImageFilter::Make(shape));

    int index = 0;

    {
        sk_sp<SkImageFilter> filter(SkBlurImageFilter::Make(3, 3, nullptr));
        // first (null) should be optimized, 2nd should not
        SkImageFilter* filters[] = { nullptr, filter.get() };

        // Any combination of these should cause the pattern to be optimized.
        SkRect* firstBounds[] = { nullptr, &bounds };
        SkPaint* firstPaints[] = { nullptr, &alphaOnlyLayerPaint };
        SkRect* secondBounds[] = { nullptr, &bounds };
        SkPaint* secondPaints[] = { &opaqueFilterLayerPaint, &translucentFilterLayerPaint };

        for (auto outerF : filters) {
            bool outerNoOped = !outerF;
            for (auto innerF : filters) {
                for (size_t i = 0; i < SK_ARRAY_COUNT(firstBounds); ++ i) {
                    for (size_t j = 0; j < SK_ARRAY_COUNT(firstPaints); ++j) {
                        for (size_t k = 0; k < SK_ARRAY_COUNT(secondBounds); ++k) {
                            for (size_t m = 0; m < SK_ARRAY_COUNT(secondPaints); ++m) {
                                bool innerNoOped = !secondBounds[k] && !secondPaints[m] && !innerF;

                                recorder.saveLayer({firstBounds[i], firstPaints[j], outerF,
                                                    nullptr, nullptr, 0});
                                recorder.save();
                                recorder.clipRect(clip);
                                recorder.saveLayer({secondBounds[k], secondPaints[m], innerF,
                                                    nullptr, nullptr, 0});
                                recorder.restore();
                                recorder.restore();
                                recorder.restore();
                                assert_merge_svg_opacity_and_filter_layers(r, &record, index,
                                                                           outerNoOped);
                            #ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
                                assert_savelayer_restore(r, &record, index + 3, innerNoOped);
                            #endif
                                index += 7;
                            }
                        }
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
        // No change: nullptr filter layer paint not implemented.
        { &alphaOnlyLayerPaint, nullptr },
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
        recorder.saveLayer(nullptr, noChangeTests[i].firstPaint);
        recorder.save();
        recorder.clipRect(clip);
        recorder.saveLayer(nullptr, noChangeTests[i].secondPaint);
        recorder.restore();
        recorder.restore();
        recorder.restore();
        assert_merge_svg_opacity_and_filter_layers(r, &record, index, false);
        index += 7;
    }

    // Test the folded alpha value.
    recorder.saveLayer(nullptr, &alphaOnlyLayerPaint);
    recorder.save();
    recorder.clipRect(clip);
    recorder.saveLayer(nullptr, &opaqueFilterLayerPaint);
    recorder.restore();
    recorder.restore();
    recorder.restore();
    assert_merge_svg_opacity_and_filter_layers(r, &record, index, true);

    const SkRecords::SaveLayer* saveLayer = assert_type<SkRecords::SaveLayer>(r, record, index + 3);
    REPORTER_ASSERT(r, saveLayer != nullptr);
    REPORTER_ASSERT(r, saveLayer->paint->getColor() == 0x03020202);

    index += 7;

    // Test that currently we do not fold alphas for patterns without the clip. This is just not
    // implemented.
    recorder.saveLayer(nullptr, &alphaOnlyLayerPaint);
    recorder.saveLayer(nullptr, &opaqueFilterLayerPaint);
    recorder.restore();
    recorder.restore();
    SkRecordMergeSvgOpacityAndFilterLayers(&record);
    assert_type<SkRecords::SaveLayer>(r, record, index);
    assert_type<SkRecords::SaveLayer>(r, record, index + 1);
    assert_type<SkRecords::Restore>(r, record, index + 2);
    assert_type<SkRecords::Restore>(r, record, index + 3);
    index += 4;
}

static void do_draw(SkCanvas* canvas, SkColor color, bool doLayer) {
    canvas->drawColor(SK_ColorWHITE);

    SkPaint p;
    p.setColor(color);

    if (doLayer) {
        canvas->saveLayer(nullptr, nullptr);
        p.setBlendMode(SkBlendMode::kSrc);
        canvas->drawPaint(p);
        canvas->restore();
    } else {
        canvas->drawPaint(p);
    }
}

static bool is_equal(SkSurface* a, SkSurface* b) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    SkPMColor ca, cb;
    a->readPixels(info, &ca, sizeof(SkPMColor), 0, 0);
    b->readPixels(info, &cb, sizeof(SkPMColor), 0, 0);
    return ca == cb;
}

// Test drawing w/ and w/o a simple layer (no bounds or paint), so see that drawing ops
// that *should* draw the same in fact do.
//
// Perform this test twice : once directly, and once via a picture
//
static void do_savelayer_srcmode(skiatest::Reporter* r, SkColor color) {
    for (int doPicture = 0; doPicture <= 1; ++doPicture) {
        sk_sp<SkSurface> surf0 = SkSurface::MakeRasterN32Premul(10, 10);
        sk_sp<SkSurface> surf1 = SkSurface::MakeRasterN32Premul(10, 10);
        SkCanvas* c0 = surf0->getCanvas();
        SkCanvas* c1 = surf1->getCanvas();

        SkPictureRecorder rec0, rec1;
        if (doPicture) {
            c0 = rec0.beginRecording(10, 10);
            c1 = rec1.beginRecording(10, 10);
        }

        do_draw(c0, color, false);
        do_draw(c1, color, true);

        if (doPicture) {
            surf0->getCanvas()->drawPicture(rec0.finishRecordingAsPicture());
            surf1->getCanvas()->drawPicture(rec1.finishRecordingAsPicture());
        }

        // we replicate the assert so we can see which line is reported if there is a failure
        if (doPicture) {
            REPORTER_ASSERT(r, is_equal(surf0.get(), surf1.get()));
        } else {
            REPORTER_ASSERT(r, is_equal(surf0.get(), surf1.get()));
        }
    }
}

DEF_TEST(savelayer_srcmode_opaque, r) {
    do_savelayer_srcmode(r, SK_ColorRED);
}

DEF_TEST(savelayer_srcmode_alpha, r) {
    do_savelayer_srcmode(r, 0x80FF0000);
}

