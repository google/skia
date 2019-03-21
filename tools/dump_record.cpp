/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommandLineFlags.h"
#include "DumpRecord.h"
#include "SkBitmap.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkStream.h"

#include <stdio.h>

static DEFINE_string2(skps, r, "", ".SKPs to dump.");
static DEFINE_string(match, "", "The usual filters on file names to dump.");
static DEFINE_bool2(optimize, O, false, "Run SkRecordOptimize before dumping.");
static DEFINE_bool(optimize2, false, "Run SkRecordOptimize2 before dumping.");
static DEFINE_int32(tile, 1000000000, "Simulated tile size.");
static DEFINE_bool(timeWithCommand, false,
                   "If true, print time next to command, else in first column.");
static DEFINE_string2(write, w, "", "Write the (optimized) picture to the named file.");

static void dump(const char* name, int w, int h, const SkRecord& record) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(w, h);
    SkCanvas canvas(bitmap);
    canvas.clipRect(SkRect::MakeWH(SkIntToScalar(FLAGS_tile),
                                   SkIntToScalar(FLAGS_tile)));

    printf("%s %s\n", FLAGS_optimize ? "optimized" : "not-optimized", name);

    DumpRecord(record, &canvas, FLAGS_timeWithCommand);
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (CommandLineFlags::ShouldSkip(FLAGS_match, FLAGS_skps[i])) {
            continue;
        }

        std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(FLAGS_skps[i]);
        if (!stream) {
            SkDebugf("Could not read %s.\n", FLAGS_skps[i]);
            return 1;
        }
        sk_sp<SkPicture> src(SkPicture::MakeFromStream(stream.get()));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", FLAGS_skps[i]);
            return 1;
        }
        const int w = SkScalarCeilToInt(src->cullRect().width());
        const int h = SkScalarCeilToInt(src->cullRect().height());

        SkRecord record;
        SkRecorder canvas(&record, w, h);
        src->playback(&canvas);

        if (FLAGS_optimize) {
            SkRecordOptimize(&record);
        }
        if (FLAGS_optimize2) {
            SkRecordOptimize2(&record);
        }

        dump(FLAGS_skps[i], w, h, record);

        if (FLAGS_write.count() > 0) {
            SkPictureRecorder r;
            SkRecordDraw(record,
                         r.beginRecording(SkRect::MakeIWH(w, h)),
                         nullptr,
                         nullptr,
                         0,
                         nullptr,
                         nullptr);
            sk_sp<SkPicture> dst(r.finishRecordingAsPicture());
            SkFILEWStream ostream(FLAGS_write[0]);
            dst->serialize(&ostream);
        }
    }

    return 0;
}
