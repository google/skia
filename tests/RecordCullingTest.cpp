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

struct SkipScanner {
    template <typename T> void operator()(const T&) {}

    void apply(const SkRecord& record) {
        for (unsigned i = 0; i < record.count(); i++) {
            record.visit(i, *this);
        }
    }

    SkTDArray<unsigned> fSkips;
};

template <> void SkipScanner::operator()(const SkRecords::PairedPushCull& r) {
    *fSkips.append() = r.skip;
}


DEF_TEST(RecordCulling, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, 1920, 1080);

    recorder.drawRect(SkRect::MakeWH(1000, 10000), SkPaint());

    recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());
        recorder.drawRect(SkRect::MakeWH(30, 30), SkPaint());
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    SkRecordAnnotateCullingPairs(&record);

    SkipScanner scan;
    scan.apply(record);

    REPORTER_ASSERT(r, 2 == scan.fSkips.count());
    REPORTER_ASSERT(r, 6 == scan.fSkips[0]);
    REPORTER_ASSERT(r, 2 == scan.fSkips[1]);
}
