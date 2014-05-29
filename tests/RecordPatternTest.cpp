#include "Test.h"

#include "SkRecord.h"
#include "SkRecordPattern.h"
#include "SkRecorder.h"
#include "SkRecords.h"

using namespace SkRecords;
typedef Pattern3<Is<Save>,
                 Is<ClipRect>,
                 Is<Restore> >
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
    REPORTER_ASSERT(r, pattern.first<Save>()      != NULL);
    REPORTER_ASSERT(r, pattern.second<ClipRect>() != NULL);
    REPORTER_ASSERT(r, pattern.third<Restore>()   != NULL);
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
    for (unsigned i = 0; i < 8; i++) {
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

DEF_TEST(RecordPattern_Star, r) {
    Pattern3<Is<Save>, Star<Is<ClipRect> >, Is<Restore> > pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);

    recorder.save();
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 0));

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 2));

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.clipRect(SkRect::MakeWH(100, 100));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 5));
}

DEF_TEST(RecordPattern_IsDraw, r) {
    Pattern3<Is<Save>, IsDraw, Is<Restore> > pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
    recorder.restore();

    REPORTER_ASSERT(r, !pattern.match(&record, 0));

    SkPaint paint;

    recorder.save();
        paint.setColor(0xEEAA8822);
        recorder.drawRect(SkRect::MakeWH(300, 200), paint);
    recorder.restore();

    recorder.save();
        paint.setColor(0xFACEFACE);
        recorder.drawPaint(paint);
    recorder.restore();

    REPORTER_ASSERT(r, pattern.match(&record, 3));
    REPORTER_ASSERT(r, pattern.first<Save>()    != NULL);
    REPORTER_ASSERT(r, pattern.second<SkPaint>()->getColor() == 0xEEAA8822);
    REPORTER_ASSERT(r, pattern.third<Restore>() != NULL);

    REPORTER_ASSERT(r, pattern.match(&record, 6));
    REPORTER_ASSERT(r, pattern.first<Save>()    != NULL);
    REPORTER_ASSERT(r, pattern.second<SkPaint>()->getColor() == 0xFACEFACE);
    REPORTER_ASSERT(r, pattern.third<Restore>() != NULL);
}

DEF_TEST(RecordPattern_Complex, r) {
    Pattern3<Is<Save>,
             Star<Not<Or3<Is<Save>,
                          Is<Restore>,
                          IsDraw> > >,
             Is<Restore> > pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);

    recorder.save();
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 0) == 2);

    recorder.save();
        recorder.save();
        recorder.restore();
    recorder.restore();
    REPORTER_ASSERT(r, !pattern.match(&record, 2));
    REPORTER_ASSERT(r, pattern.match(&record, 3) == 5);

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 6) == 9);

    recorder.save();
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.drawRect(SkRect::MakeWH(100, 3000), SkPaint());
    recorder.restore();
    REPORTER_ASSERT(r, !pattern.match(&record, 9));

    recorder.save();
        recorder.pushCull(SkRect::MakeWH(300, 200));
        recorder.clipRect(SkRect::MakeWH(300, 200));
        recorder.clipRect(SkRect::MakeWH(100, 400));
        recorder.popCull();
    recorder.restore();
    REPORTER_ASSERT(r, pattern.match(&record, 13) == 19);

    // Same as above, but using pattern.search to step through matches.
    unsigned begin, end = 0;
    REPORTER_ASSERT(r, pattern.search(&record, &begin, &end));
    REPORTER_ASSERT(r, begin == 0);
    REPORTER_ASSERT(r, end == 2);

    REPORTER_ASSERT(r, pattern.search(&record, &begin, &end));
    REPORTER_ASSERT(r, begin == 3);
    REPORTER_ASSERT(r, end == 5);

    REPORTER_ASSERT(r, pattern.search(&record, &begin, &end));
    REPORTER_ASSERT(r, begin == 6);
    REPORTER_ASSERT(r, end == 9);

    REPORTER_ASSERT(r, pattern.search(&record, &begin, &end));
    REPORTER_ASSERT(r, begin == 13);
    REPORTER_ASSERT(r, end == 19);

    REPORTER_ASSERT(r, !pattern.search(&record, &begin, &end));
}

DEF_TEST(RecordPattern_SaveLayerIsNotADraw, r) {
    Pattern1<IsDraw> pattern;

    SkRecord record;
    SkRecorder recorder(&record, 1920, 1200);
    recorder.saveLayer(NULL, NULL);

    REPORTER_ASSERT(r, !pattern.match(&record, 0));
}
