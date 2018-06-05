/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"
#include "SkSurface.h"

#if defined(SK_ENABLE_SKOTTIE)
#include "Skottie.h"
#endif

DEFINE_string2(input, i, nullptr, "Input .json file.");
DEFINE_string2(writePath, w, nullptr, "Output directory.  Frames are names [0-9]{6}.png.");

DEFINE_double(t0,   0, "Timeline start [0..1].");
DEFINE_double(t1,   1, "Timeline stop [0..1].");
DEFINE_double(fps, 30, "Decode frames per second.");

DEFINE_int32(width , 800, "Render width.");
DEFINE_int32(height, 600, "Render height.");

int main(int argc, char** argv) {
#if defined(SK_ENABLE_SKOTTIE)
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics ag;

    if (FLAGS_input.isEmpty() || FLAGS_writePath.isEmpty()) {
        SkDebugf("Missing required 'input' and 'writePath' args.\n");
        return 1;
    }

    if (FLAGS_fps <= 0) {
        SkDebugf("Invalid fps: %f.\n", FLAGS_fps);
        return 1;
    }

    if (!sk_mkdir(FLAGS_writePath[0])) {
        return 1;
    }

    auto anim = skottie::Animation::MakeFromFile(FLAGS_input[0]);
    if (!anim) {
        SkDebugf("Could not load animation: '%s'.\n", FLAGS_input[0]);
        return 1;
    }

    auto surface = SkSurface::MakeRasterN32Premul(FLAGS_width, FLAGS_height);
    if (!surface) {
        SkDebugf("Could not allocate a %d x %d buffer.\n", FLAGS_width, FLAGS_height);
        return 1;
    }

    auto* canvas = surface->getCanvas();
    canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(anim->size()),
                                            SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                            SkMatrix::kCenter_ScaleToFit));

    static constexpr double kMaxFrames = 10000;
    const auto t0 = SkTPin(FLAGS_t0, 0.0, 1.0),
               t1 = SkTPin(FLAGS_t1,  t0, 1.0),
               advance = 1 / std::min(anim->duration() * FLAGS_fps, kMaxFrames);

    size_t frame_index = 0;
    for (auto t = t0; t <= t1; t += advance) {
        canvas->clear(SK_ColorTRANSPARENT);

        anim->seek(t);
        anim->render(canvas);

        auto png_data = surface->makeImageSnapshot()->encodeToData();
        if (!png_data) {
            SkDebugf("Failed to encode frame #%lu\n", frame_index);
            return 1;
        }

        const auto frame_file = SkStringPrintf("0%06d.png", frame_index++);

        SkFILEWStream wstream(SkOSPath::Join(FLAGS_writePath[0], frame_file.c_str()).c_str());
        if (!wstream.isValid()) {
            SkDebugf("Could not open '%s/%s' for writing.\n",
                     FLAGS_writePath[0], frame_file.c_str());
            return 1;
        }

        wstream.write(png_data->data(), png_data->size());
    }
#else
    SkDebugf("This tool requires Skottie support.\n");
#endif

    return 0;
}
