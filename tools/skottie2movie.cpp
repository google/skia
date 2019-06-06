/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(input, i, "", "skottie animation to render");
static DEFINE_string2(output, o, "", "mp4 file to create");
static DEFINE_string2(assetPath, a, "", "path to assets needed for json file");
static DEFINE_int_2(fps, f, 25, "fps");
static DEFINE_bool2(verbose, v, false, "verbose mode");
static DEFINE_bool2(loop, l, false, "loop mode for profiling");

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage("Converts skottie to a mp4");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.count() == 0) {
        SkDebugf("-i input_file.json argument required\n");
        return -1;
    }

    SkString assetPath;
    if (FLAGS_assetPath.count() > 0) {
        assetPath.set(FLAGS_assetPath[0]);
    } else {
        assetPath = SkOSPath::Dirname(FLAGS_input[0]);
    }
    SkDebugf("assetPath %s\n", assetPath.c_str());

    auto animation = skottie::Animation::Builder()
        .setResourceProvider(skottie_utils::FileResourceProvider::Make(assetPath))
        .makeFromFile(FLAGS_input[0]);
    if (!animation) {
        SkDebugf("failed to load %s\n", FLAGS_input[0]);
        return -1;
    }

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

    while (FLAGS_loop) {
        double start = SkTime::GetSecs();
        encoder.beginRecording(dim, fps);
        for (int i = 0; i <= frames; ++i) {
            animation->seek(i * 1.0 / frames);  // normalized time
            animation->render(encoder.beginFrame());
            encoder.endFrame();
        }
        (void)encoder.endRecording();
        if (FLAGS_verbose) {
            double dur = SkTime::GetSecs() - start;
            SkDebugf("%d frames, %g secs, %d fps\n",
                     frames, dur, (int)floor(frames / dur + 0.5));
        }
    }

    encoder.beginRecording(dim, fps);

    auto surf = SkSurface::MakeRaster(encoder.preferredInfo());

    for (int i = 0; i <= frames; ++i) {
        double ts = i * 1.0 / fps;
        if (FLAGS_verbose) {
            SkDebugf("rendering frame %d ts %g\n", i, ts);
        }
        animation->seek(i * 1.0 / frames);  // normalized time
        surf->getCanvas()->clear(SK_ColorWHITE);
        animation->render(surf->getCanvas());

        SkPixmap pm;
        SkAssertResult(surf->peekPixels(&pm));
        encoder.addFrame(pm);
    }

    if (FLAGS_output.count() == 0) {
        SkDebugf("missing -o output_file.mp4 argument\n");
        return 0;
    }

    auto data = encoder.endRecording();
    SkFILEWStream ostream(FLAGS_output[0]);
    if (!ostream.isValid()) {
        SkDebugf("Can't create output file %s\n", FLAGS_output[0]);
        return -1;
    }
    ostream.write(data->data(), data->size());
    return 0;
}
