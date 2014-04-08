#include "Test.h"

#include "SkRecord.h"
#include "SkRecordCulling.h"
#include "SkRecorder.h"
#include "SkRecords.h"

struct PushCullScanner {
    template <typename T> void operator()(const T&) {}

    SkTDArray<unsigned> fPopOffsets;
};

template <> void PushCullScanner::operator()(const SkRecords::PushCull& record) {
    *fPopOffsets.append() = record.popOffset;
}


DEF_TEST(RecordCulling, r) {
    SkRecord record;
    SkRecorder recorder(&record, 1920, 1080);

    recorder.drawRect(SkRect::MakeWH(1000, 10000), SkPaint());

    recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());
        recorder.drawRect(SkRect::MakeWH(30, 30), SkPaint());
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    SkRecordAnnotateCullingPairs(&record);

    PushCullScanner scan;
    record.visit(scan);

    REPORTER_ASSERT(r, 2 == scan.fPopOffsets.count());
    REPORTER_ASSERT(r, 6 == scan.fPopOffsets[0]);
    REPORTER_ASSERT(r, 2 == scan.fPopOffsets[1]);
}
