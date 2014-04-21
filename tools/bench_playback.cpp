/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTime.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(skps, r, "skps", "Directory containing SKPs to read and re-record.");
DEFINE_int32(loops, 10, "Number of times to play back each SKP.");
DEFINE_bool(skr, false, "Play via SkRecord instead of SkPicture.");
DEFINE_int32(tile, 1000000000, "Simulated tile size.");
DEFINE_string(match, "", "The usual filters on file names of SKPs to bench.");

static void bench(SkPMColor* scratch, SkPicture& src, const char* name) {
    // We don't use the public SkRecording interface here because we need kWriteOnly_Mode.
    // (We don't want SkPicturePlayback to be able to optimize playing into our SkRecord.)
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, src.width(), src.height());
    src.draw(&recorder);

    SkRecordOptimize(&record);

    SkAutoTDelete<SkCanvas> canvas(SkCanvas::NewRasterDirectN32(src.width(),
                                                                src.height(),
                                                                scratch,
                                                                src.width() * sizeof(SkPMColor)));
    canvas->clipRect(SkRect::MakeWH(SkIntToScalar(FLAGS_tile), SkIntToScalar(FLAGS_tile)));

    const SkMSec start = SkTime::GetMSecs();
    for (int i = 0; i < FLAGS_loops; i++) {
        if (FLAGS_skr) {
            SkRecordDraw(record, canvas.get());
        } else {
            src.draw(canvas.get());
        }
    }

    const SkMSec elapsed = SkTime::GetMSecs() - start;
    const double msPerLoop = elapsed / (double)FLAGS_loops;
    printf("%6.2f\t%s\n", msPerLoop, name);
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics autoGraphics;

    // We share a single scratch bitmap among benches to reduce the profile noise from allocation.
    static const int kMaxArea = 209825221;  // tabl_mozilla is this big.
    SkAutoTMalloc<SkPMColor> scratch(kMaxArea);

    SkOSFile::Iter it(FLAGS_skps[0], ".skp");
    SkString filename;
    bool failed = false;
    while (it.next(&filename)) {
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, filename.c_str())) {
            continue;
        }

        const SkString path = SkOSPath::SkPathJoin(FLAGS_skps[0], filename.c_str());

        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path.c_str()));
        if (!stream) {
            SkDebugf("Could not read %s.\n", path.c_str());
            failed = true;
            continue;
        }
        SkAutoTUnref<SkPicture> src(SkPicture::CreateFromStream(stream));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", path.c_str());
            failed = true;
            continue;
        }

        if (src->width() * src->height() > kMaxArea) {
            SkDebugf("%s (%dx%d) is larger than hardcoded scratch bitmap (%dpx).\n",
                     path.c_str(), src->width(), src->height(), kMaxArea);
            failed = true;
            continue;
        }

        bench(scratch.get(), *src, filename.c_str());
    }
    return failed ? 1 : 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
