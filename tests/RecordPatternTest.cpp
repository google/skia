/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkRecord.h"
#include "SkRecordPattern.h"
#include "SkRecorder.h"
#include "SkRecords.h"

using namespace SkRecords;
typedef Pattern<Is<Save>,
                Is<ClipRect>,
                Is<Restore>>
    SaveClipRectRestore;

DEF_TEST(RecordPattern_Simple, r) {
    SaveClipRectRestore pattern;

    SkRecord record;
    REPORTER_ASSERT(r, !pattern.match(&record, 0));

    SkRecorder recorder(&record, 1920, 1200);

    // Build up a save-clip-restore block.  The pattern will match only it's complete.
    recorder.save();
    REPORTER_ASSERT(r, !pattern.match(&record, 0));

    recorder.clipRect(SkRect::MakeWH(300, 200));
    REPORTER_ASSERT(r, !pattern.match(&record, 0));

    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 0));
    REPORTER_ASSERT(r, pattern.first<Save>()      != nullptr);
    REPORTER_ASSERT(r, pattern.second<ClipRect>() != nullptr);
    REPORTER_ASSERT(r, pattern.third<Restore>()   != nullptr);
}

DEF_TEST(RecordPattern_StartingIndex, r) {
    SaveClipRectRestore pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);

    // There will be two save-clipRect-restore blocks [0,3) and [3,6).
    for (int i = 0; i < 2; i++) {
        recorder.save();
            recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.restore();
    }

    // We should match only at 0 and 3.  Going over the length should fail gracefully.
    for (int i = 0; i < 8; i++) {
        if (i == 0 || i == 3) {
            REPORTER_ASSERT(r, pattern.match(&record, i) == i + 3);
        } else {
            REPORTER_ASSERT(r, !pattern.match(&record, i));
        }
    }
}

DEF_TEST(RecordPattern_DontMatchSubsequences, r) {
    SaveClipRectRestore pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.drawRect(SkRect::MakeWH(600, 300), SkPaint());
    recorder.restore();

    REPORTER_ASSERT(r, !pattern.match(&record, 0));
}

DEF_TEST(RecordPattern_Greedy, r) {
    Pattern<Is<Save>, Greedy<Is<ClipRect>>, Is<Restore>> pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);
    int index = 0;

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, index));
    index += 3;

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.clipRect(SkRect::MakeWH(100, 100));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, index));
}

DEF_TEST(RecordPattern_Complex, r) {
    Pattern<Is<Save>,
            Greedy<Not<Or<Is<Save>,
                          Is<Restore>,
                          IsDraw>>>,
            Is<Restore>> pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);
    int start, begin, end;

    start = record.count();
    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 0) == record.count());
    end = start;
    REPORTER_ASSERT(r, pattern.search(&record, &begin, &end));
    REPORTER_ASSERT(r, begin == start);
    REPORTER_ASSERT(r, end == record.count());

    start = record.count();
    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.drawRect(SkRect::MakeWH(100, 3000), SkPaint());
    recorder.restore();
    REPORTER_ASSERT(r, !pattern.match(&record, start));
    end = start;
    REPORTER_ASSERT(r, !pattern.search(&record, &begin, &end));

    start = record.count();
    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.clipRect(SkRect::MakeWH(100, 400));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, start) == record.count());
    end = start;
    REPORTER_ASSERT(r, pattern.search(&record, &begin, &end));
    REPORTER_ASSERT(r, begin == start);
    REPORTER_ASSERT(r, end == record.count());

    REPORTER_ASSERT(r, !pattern.search(&record, &begin, &end));
}

DEF_TEST(RecordPattern_SaveLayerIsNotADraw, r) {
    Pattern<IsDraw> pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);
    recorder.saveLayer(nullptr, nullptr);

    REPORTER_ASSERT(r, !pattern.match(&record, 0));
}
