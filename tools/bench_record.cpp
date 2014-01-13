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
#include "SkStream.h"
#include "SkString.h"
#include "SkTime.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

// Just reading all the SKPs takes about 2 seconds for me, which is the same as about 100 loops of
// rerecording all the SKPs.  So we default to --loops=900, which makes ~90% of our time spent in
// recording, and this should take ~20 seconds to run.

DEFINE_string2(skps, r, "skps", "Directory containing SKPs to read and re-record.");
DEFINE_int32(loops, 900, "Number of times to re-record each SKP.");
DEFINE_int32(flags, SkPicture::kUsePathBoundsForClip_RecordingFlag, "RecordingFlags to use.");
DEFINE_bool(endRecording, true, "If false, don't time SkPicture::endRecording()");

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics autoGraphics;

    SkOSFile::Iter it(FLAGS_skps[0], ".skp");
    SkString filename;
    while (it.next(&filename)) {
        const SkString path = SkOSPath::SkPathJoin(FLAGS_skps[0], filename.c_str());

        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path.c_str()));
        if (!stream) {
            SkDebugf("Could not read %s.\n", path.c_str());
            continue;
        }
        SkAutoTUnref<SkPicture> src(SkPicture::CreateFromStream(stream));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", path.c_str());
            continue;
        }

        const SkMSec start = SkTime::GetMSecs();
        for (int i = 0; i < FLAGS_loops; i++) {
            SkPicture dst;
            src->draw(dst.beginRecording(src->width(), src->height(), FLAGS_flags));
            if (FLAGS_endRecording) dst.endRecording();
        }

        const SkMSec elapsed = SkTime::GetMSecs() - start;
        const double msPerLoop = elapsed / (double)FLAGS_loops;
        printf("%4.2f\t%s\n", msPerLoop, filename.c_str());
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
