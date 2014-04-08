#include "Test.h"

#include "SkRecord.h"
#include "SkRecorder.h"
#include "SkRecords.h"

#define COUNT(T) + 1
static const int kRecordTypes = SK_RECORD_TYPES(COUNT);
#undef COUNT

// Tallies the types of commands it sees into histogram.
class Tally {
public:
    explicit Tally(int histogram[kRecordTypes]) : fHistogram(histogram) {}

    template <typename T> void operator()(const T&) {
        ++fHistogram[T::kType];
    }

private:
    int* fHistogram;
};

DEF_TEST(Recorder, r) {
    SkRecord record;
    SkRecorder recorder(&record, 1920, 1080);

    recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());

    int histogram[kRecordTypes];
    bzero(&histogram, sizeof(histogram));

    record.visit(Tally(histogram));

    REPORTER_ASSERT(r, 1 == histogram[SkRecords::DrawRect::kType]);
}
