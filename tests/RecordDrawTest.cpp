/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkDebugCanvas.h"
#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkRecords.h"

static const int W = 1920, H = 1080;

static void draw_pos_text_h(SkCanvas* canvas, const char* text, SkScalar y) {
    const size_t len = strlen(text);
    SkAutoTMalloc<SkScalar> xpos(len);
    for (size_t i = 0; i < len; i++) {
        xpos[i] = (SkScalar)i;
    }
    canvas->drawPosTextH(text, len, xpos, y, SkPaint());
}

// Rerecord into another SkRecord using full SkCanvas semantics,
// tracking clips and allowing SkRecordDraw's quickReject() calls to work.
static void record_clipped(const SkRecord& record, SkRect clip, SkRecord* clipped) {
    SkRecorder recorder(SkRecorder::kReadWrite_Mode, clipped, W, H);
    recorder.clipRect(clip);
    SkRecordDraw(record, &recorder);
}

DEF_TEST(RecordDraw_PosTextHQuickReject, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    draw_pos_text_h(&recorder, "This will draw.", 20);
    draw_pos_text_h(&recorder, "This won't.", 5000);

    SkRecordBoundDrawPosTextH(&record);

    SkRecord clipped;
    record_clipped(record, SkRect::MakeLTRB(20, 20, 200, 200), &clipped);

    // clipRect and the first drawPosTextH.
    REPORTER_ASSERT(r, 2 == clipped.count());
}

DEF_TEST(RecordDraw_Culling, r) {
    // Record these 7 drawing commands verbatim.
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());
        recorder.drawRect(SkRect::MakeWH(30, 30), SkPaint());
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    // Take a pass over to match up pushCulls and popCulls.
    SkRecordAnnotateCullingPairs(&record);

    // This clip intersects the outer cull, but allows us to quick reject the inner one.
    SkRecord clipped;
    record_clipped(record, SkRect::MakeLTRB(20, 20, 200, 200), &clipped);

    // We'll keep the clipRect call from above, and the outer two drawRects, and the push/pop pair.
    // If culling weren't working, we'd see 8 commands recorded here.
    REPORTER_ASSERT(r, 5 == clipped.count());
}
