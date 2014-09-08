/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "Test.h"
#include "RecordTestUtils.h"

#include "SkBBHFactory.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkUtils.h"
#include "GrRecordReplaceDraw.h"

static const int kWidth = 100;
static const int kHeight = 100;

class JustOneDraw : public SkDrawPictureCallback {
public:
    JustOneDraw() : fCalls(0) {}

    virtual bool abortDrawing() SK_OVERRIDE { return fCalls++ > 0; }
private:
    int fCalls;
};

// Make sure the abort callback works
DEF_TEST(RecordReplaceDraw_Abort, r) {
    // Record two commands.
    SkRecord record;
    SkRecorder recorder(&record, kWidth, kHeight);
    recorder.drawRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)), SkPaint());
    recorder.clipRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)));

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    GrReplacements replacements;
    JustOneDraw callback;
    GrRecordReplaceDraw(record, &canvas, NULL/*bbh*/, &replacements, &callback);

    REPORTER_ASSERT(r, 3 == rerecord.count());
    assert_type<SkRecords::Save>(r, rerecord, 0);
    assert_type<SkRecords::DrawRect>(r, rerecord, 1);
    assert_type<SkRecords::Restore>(r, rerecord, 2);
}

// Make sure GrRecordReplaceDraw balances unbalanced saves
DEF_TEST(RecordReplaceDraw_Unbalanced, r) {
    SkRecord record;
    SkRecorder recorder(&record, kWidth, kHeight);
    recorder.save();  // We won't balance this, but GrRecordReplaceDraw will for us.

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    GrReplacements replacements;
    GrRecordReplaceDraw(record, &canvas, NULL/*bbh*/, &replacements, NULL/*callback*/);

    REPORTER_ASSERT(r, 4 == rerecord.count());
    assert_type<SkRecords::Save>(r, rerecord, 0);
    assert_type<SkRecords::Save>(r, rerecord, 1);
    assert_type<SkRecords::Restore>(r, rerecord, 2);
    assert_type<SkRecords::Restore>(r, rerecord, 3);
}

static SkImage* make_image(SkColor color) {
    const SkPMColor pmcolor = SkPreMultiplyColor(color);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    const size_t rowBytes = info.minRowBytes();
    const size_t size = rowBytes * info.height();

    SkAutoMalloc addr(size);
    sk_memset32((SkPMColor*)addr.get(), pmcolor, SkToInt(size >> 2));

    return SkImage::NewRasterCopy(info, addr.get(), rowBytes);
}

// Test out the layer replacement functionality with and w/o a BBH
void test_replacements(skiatest::Reporter* r, bool useBBH) {
    SkRecord record;
    SkRecorder recorder(&record, kWidth, kHeight);
    SkAutoTDelete<SkPaint> paint(SkNEW(SkPaint));
    recorder.saveLayer(NULL, paint);
    recorder.clear(SK_ColorRED);
    recorder.restore();
    recorder.drawRect(SkRect::MakeWH(SkIntToScalar(kWidth/2), SkIntToScalar(kHeight/2)), 
                      SkPaint());

    GrReplacements replacements;
    GrReplacements::ReplacementInfo* ri = replacements.push();
    ri->fStart = 0;
    ri->fStop = 2;
    ri->fPos.set(0, 0);
    ri->fImage = make_image(SK_ColorRED);
    ri->fPaint = paint;
    ri->fSrcRect = SkIRect::MakeWH(kWidth, kHeight);

    SkAutoTUnref<SkBBoxHierarchy> bbh;

    if (useBBH) {
        SkRTreeFactory factory;
        bbh.reset((factory)(kWidth, kHeight));
        SkRecordFillBounds(record, bbh);
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);
    GrRecordReplaceDraw(record, &canvas, bbh, &replacements, NULL/*callback*/);

    REPORTER_ASSERT(r, 7 == rerecord.count());
    assert_type<SkRecords::Save>(r, rerecord, 0);
    assert_type<SkRecords::Save>(r, rerecord, 1);
    assert_type<SkRecords::SetMatrix>(r, rerecord, 2);
    assert_type<SkRecords::DrawBitmapRectToRect>(r, rerecord, 3);
    assert_type<SkRecords::Restore>(r, rerecord, 4);
    assert_type<SkRecords::DrawRect>(r, rerecord, 5);
    assert_type<SkRecords::Restore>(r, rerecord, 6);
}

DEF_TEST(RecordReplaceDraw_Replace, r)        { test_replacements(r, false); }
DEF_TEST(RecordReplaceDraw_ReplaceWithBBH, r) { test_replacements(r, true); }

#endif
