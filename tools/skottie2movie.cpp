/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/Skottie.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(input, i, "", "skottie animation to render");
static DEFINE_string2(output, o, "", "mp4 file to create");
static DEFINE_int_2(fps, f, 25, "fps");
static DEFINE_bool2(verbose, v, false, "verbose mode");
static DEFINE_bool2(forever, f, false, "forever mode for profiling");

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage("Converts skottie to a mp4");
    CommandLineFlags::Parse(argc, argv);

    if (argc <= 1) {
        SkDebugf("Need -i input_file.json\n");
        return -1;
    }
    SkFILEStream stream(FLAGS_input[0]);
    if (!stream.isValid()) {
        SkDebugf("Couldn't open skottie %s\n", FLAGS_input[0]);
        return -1;
    }

    auto animation = skottie::Animation::Builder().make(&stream);
    SkISize dim = animation->size().toRound();
    double duration = animation->duration();
    int fps = FLAGS_fps;
    if (fps < 1) {
        fps = 1;
    } else if (fps > 240) {
        fps = 240;
    }

    if (FLAGS_verbose) {
        SkDebugf("size %dx%d duration %g\n", dim.width(), dim.height(), duration, fps);
    }

    SkVideoEncoder encoder;
    const int frames = SkScalarRoundToInt(duration * fps);

    while (FLAGS_forever) {
        double now = SkTime::GetSecs();
        encoder.beginRecording(dim, fps);
        for (int i = 0; i <= frames; ++i) {
            animation->seek(i * 1.0 / frames);  // normalized time
            animation->render(encoder.beginFrame());
            encoder.endFrame();
        }
        (void)encoder.endRecording();
        SkDebugf("time in ms: %d\n", (int)((SkTime::GetSecs() - now) * 1000));
    }

    encoder.beginRecording(dim, fps);

    for (int i = 0; i <= frames; ++i) {
        double ts = i * 1.0 / fps;
        if (FLAGS_verbose) {
            SkDebugf("rendering frame %d ts %g\n", i, ts);
        }
        animation->seek(i * 1.0 / frames);  // normalized time
        animation->render(encoder.beginFrame());
        encoder.endFrame();
    }

    auto data = encoder.endRecording();
    SkFILEWStream ostream(FLAGS_output[0]);
    if (!ostream.isValid()) {
        SkDebugf("Can't create output file %s\n", FLAGS_output[0]);
        return -1;
    }
    ostream.write(data->data(), data->size());
}
