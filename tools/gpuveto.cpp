/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"

DEFINE_string2(readFile, r, "", "skp file to process.");
DEFINE_bool2(quiet, q, false, "quiet");

// This tool just loads a single skp, replays into a new SkPicture (to
// regenerate the GPU-specific tracking information) and reports
// the value of the suitableForGpuRasterization method.
// Return codes:
static const int kSuccess = 0;
static const int kError = 1;

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
#if SK_SUPPORT_GPU
    SkCommandLineFlags::SetUsage("Reports on an skp file's suitability for GPU rasterization");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_readFile.count() != 1) {
        if (!FLAGS_quiet) {
            SkDebugf("Missing input file\n");
        }
        return kError;
    }

    SkFILEStream inputStream(FLAGS_readFile[0]);
    if (!inputStream.isValid()) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't open file\n");
        }
        return kError;
    }

    SkPicture::InstallPixelRefProc proc = &sk_tools::LazyDecodeBitmap;

    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(&inputStream, proc));
    if (nullptr == picture.get()) {
        if (!FLAGS_quiet) {
            SkDebugf("Could not read the SkPicture\n");
        }
        return kError;
    }

    // The SkPicture tracking information is only generated during recording
    // an isn't serialized. Replay the picture to regenerated the tracking data.
    SkPictureRecorder recorder;
    picture->playback(recorder.beginRecording(picture->cullRect().width(), 
                                              picture->cullRect().height(), 
                                              nullptr, 0));
    SkAutoTUnref<SkPicture> recorded(recorder.endRecording());

    if (recorded->suitableForGpuRasterization(nullptr)) {
        SkDebugf("suitable\n");
    } else {
        SkDebugf("unsuitable\n");
    }

    return kSuccess;
#else
    SkDebugf("gpuveto is only useful when GPU rendering is enabled\n");
    return kError;
#endif
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
