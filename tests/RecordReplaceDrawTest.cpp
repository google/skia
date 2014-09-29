/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "Test.h"

#include "GrRecordReplaceDraw.h"
#include "RecordTestUtils.h"
#include "SkBBHFactory.h"
#include "SkPictureRecorder.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkUtils.h"

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
    SkAutoTUnref<const SkPicture> pic;

    {
        // Record two commands.
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)), SkPaint());
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)));

        pic.reset(recorder.endRecording());
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    GrReplacements replacements;
    JustOneDraw callback;
    GrRecordReplaceDraw(pic, &canvas, &replacements, SkMatrix::I(), &callback);

    REPORTER_ASSERT(r, 3 == rerecord.count());
    assert_type<SkRecords::Save>(r, rerecord, 0);
    assert_type<SkRecords::DrawRect>(r, rerecord, 1);
    assert_type<SkRecords::Restore>(r, rerecord, 2);
}

// Make sure GrRecordReplaceDraw balances unbalanced saves
DEF_TEST(RecordReplaceDraw_Unbalanced, r) {
    SkAutoTUnref<const SkPicture> pic;

    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        // We won't balance this, but GrRecordReplaceDraw will for us.
        canvas->save();

        pic.reset(recorder.endRecording());
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    GrReplacements replacements;
    GrRecordReplaceDraw(pic, &canvas, &replacements, SkMatrix::I(), NULL/*callback*/);

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
    SkAutoTUnref<const SkPicture> pic;

    {
        SkRTreeFactory bbhFactory;
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight), 
                                                   useBBH ? &bbhFactory : NULL);

        SkAutoTDelete<SkPaint> paint(SkNEW(SkPaint));
        canvas->saveLayer(NULL, paint);
        canvas->clear(SK_ColorRED);
        canvas->restore();
        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(kWidth / 2), SkIntToScalar(kHeight / 2)),
                         SkPaint());

        pic.reset(recorder.endRecording());
    }

    GrReplacements replacements;
    GrReplacements::ReplacementInfo* ri = replacements.push();
    ri->fStart = 0;
    ri->fStop = 2;
    ri->fPos.set(0, 0);
    ri->fImage = make_image(SK_ColorRED);
    ri->fPaint = SkNEW(SkPaint);
    ri->fSrcRect = SkIRect::MakeWH(kWidth, kHeight);

    SkAutoTUnref<SkBBoxHierarchy> bbh;

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);
    GrRecordReplaceDraw(pic, &canvas, &replacements, SkMatrix::I(), NULL/*callback*/);

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
