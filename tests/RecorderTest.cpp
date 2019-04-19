/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkPictureRecorder.h"
#include "SkRecord.h"
#include "SkRecorder.h"
#include "SkRecords.h"
#include "SkShader.h"
#include "SkSurface.h"

#define COUNT(T) + 1
static const int kRecordTypes = SK_RECORD_TYPES(COUNT);
#undef COUNT

// Tallies the types of commands it sees into a histogram.
class Tally {
public:
    Tally() { sk_bzero(&fHistogram, sizeof(fHistogram)); }

    template <typename T>
    void operator()(const T&) { ++fHistogram[T::kType]; }

    template <typename T>
    int count() const { return fHistogram[T::kType]; }

    void apply(const SkRecord& record) {
        for (int i = 0; i < record.count(); i++) {
            record.visit(i, *this);
        }
    }

private:
    int fHistogram[kRecordTypes];
};

DEF_TEST(Recorder, r) {
    SkRecord record;
    SkRecorder recorder(&record, 1920, 1080);

    recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());

    Tally tally;
    tally.apply(record);
    REPORTER_ASSERT(r, 1 == tally.count<SkRecords::DrawRect>());
}

// Regression test for leaking refs held by optional arguments.
DEF_TEST(Recorder_RefLeaking, r) {
    // We use SaveLayer to test:
    //   - its SkRect argument is optional and SkRect is POD.  Just testing that that works.
    //   - its SkPaint argument is optional and SkPaint is not POD.  The bug was here.

    SkRect bounds = SkRect::MakeWH(320, 240);
    SkPaint paint;
    paint.setShader(SkShaders::Empty());

    REPORTER_ASSERT(r, paint.getShader()->unique());
    {
        SkRecord record;
        SkRecorder recorder(&record, 1920, 1080);
        recorder.saveLayer(&bounds, &paint);
        REPORTER_ASSERT(r, !paint.getShader()->unique());
    }
    REPORTER_ASSERT(r, paint.getShader()->unique());
}

DEF_TEST(Recorder_drawImage_takeReference, reporter) {

    sk_sp<SkImage> image;
    {
        auto surface(SkSurface::MakeRasterN32Premul(100, 100));
        surface->getCanvas()->clear(SK_ColorGREEN);
        image = surface->makeImageSnapshot();
    }
    {
        SkRecord record;
        SkRecorder recorder(&record, 100, 100);

        // DrawImage is supposed to take a reference
        recorder.drawImage(image, 0, 0);
        REPORTER_ASSERT(reporter, !image->unique());

        Tally tally;
        tally.apply(record);

        REPORTER_ASSERT(reporter, 1 == tally.count<SkRecords::DrawImage>());
    }
    REPORTER_ASSERT(reporter, image->unique());

    {
        SkRecord record;
        SkRecorder recorder(&record, 100, 100);

        // DrawImageRect is supposed to take a reference
        recorder.drawImageRect(image, SkRect::MakeWH(100, 100), nullptr);
        REPORTER_ASSERT(reporter, !image->unique());

        Tally tally;
        tally.apply(record);

        REPORTER_ASSERT(reporter, 1 == tally.count<SkRecords::DrawImageRect>());
    }
    REPORTER_ASSERT(reporter, image->unique());
}
