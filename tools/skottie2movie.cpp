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
static DEFINE_double(motion_angle, 180, "motion blur angle");
static DEFINE_double(motion_slope, 0, "motion blur slope");
static DEFINE_int(motion_samples, 1, "motion blur samples");

static void produce_frame(SkSurface* surf, skottie::Animation* anim, double frame_time) {
    anim->seekFrameTime(frame_time);
    surf->getCanvas()->clear(SK_ColorWHITE);
    anim->render(surf->getCanvas());
}

static void produce_frame(SkSurface* surf, SkSurface* tmp, skottie::Animation* anim,
                          double frame_time, double frame_duration, double motion_radius,
                          int motion_samples) {
    double samples_duration = frame_duration * motion_radius * 2;
    double dt = samples_duration / (motion_samples - 1);
    double t = frame_time - samples_duration / 2;

    SkPaint paint;
    paint.setAlphaf(1.0f / motion_samples);
    paint.setBlendMode(SkBlendMode::kPlus);
    surf->getCanvas()->clear(0);

    for (int i = 0; i < motion_samples; ++i) {
        if (FLAGS_verbose) {
            SkDebugf("time %g sample_time %g\n", frame_time, t);
        }
        produce_frame(tmp, anim, t);
        t += dt;
        tmp->draw(surf->getCanvas(), 0, 0, &paint);
    }
}

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage("Converts skottie to a mp4");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.count() == 0) {
        SkDebugf("-i input_file.json argument required\n");
        return -1;
    }

    if (FLAGS_motion_angle < 0 || FLAGS_motion_angle > 360) {
        SkDebugf("--motion_angle must be [0...360]\n");
        return -1;
    }
    if (FLAGS_motion_slope < -1 || FLAGS_motion_slope > 1) {
        SkDebugf("--motion_slope must be [-1...1]\n");
        return -1;
    }
    if (FLAGS_motion_samples < 1) {
        SkDebugf("--motion_samples must be >= 1\n");
        return -1;
    }

    // map angle=180 to radius=1/4 (of a frame duration)
    double motion_radius = FLAGS_motion_angle * 0.25 / 180.0;
    if (FLAGS_motion_samples == 1) {
        motion_radius = 0;  // no blur if we're only 1 sample
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

    const int frames = SkScalarRoundToInt(duration * fps);
    const double frame_duration = 1.0 / fps;

    if (FLAGS_verbose) {
        SkDebugf("size %dx%d duration %g, fps %d, frame_duration %g\n",
                 dim.width(), dim.height(), duration, fps, frame_duration);
    }

    SkVideoEncoder encoder;

    sk_sp<SkSurface> surf, tmp_surf;
    sk_sp<SkData> data;

    do {
        double loop_start = SkTime::GetSecs();

        encoder.beginRecording(dim, fps);
        // lazily allocate the surfaces
        if (!surf) {
            surf = SkSurface::MakeRaster(encoder.preferredInfo());
            tmp_surf = surf->makeSurface(surf->width(), surf->height());
        }

        for (int i = 0; i <= frames; ++i) {
            double ts = i * 1.0 / fps;
            if (FLAGS_verbose) {
                SkDebugf("rendering frame %d ts %g\n", i, ts);
            }

            double normal_time = i * 1.0 / frames;
            double frame_time = normal_time * duration;

            if (motion_radius > 0) {
                produce_frame(surf.get(), tmp_surf.get(), animation.get(), frame_time, frame_duration,
                              motion_radius, FLAGS_motion_samples);
            } else {
                produce_frame(surf.get(), animation.get(), frame_time);
            }

            SkPixmap pm;
            SkAssertResult(surf->peekPixels(&pm));
            encoder.addFrame(pm);
        }
        data = encoder.endRecording();

        if (FLAGS_loop) {
            double loop_dur = SkTime::GetSecs() - loop_start;
            SkDebugf("recording secs %g, frames %d, recording fps %d\n",
                     loop_dur, frames, (int)(frames / loop_dur));
        }
    } while (FLAGS_loop);

    if (FLAGS_output.count() == 0) {
        SkDebugf("missing -o output_file.mp4 argument\n");
        return 0;
    }

    SkFILEWStream ostream(FLAGS_output[0]);
    if (!ostream.isValid()) {
        SkDebugf("Can't create output file %s\n", FLAGS_output[0]);
        return -1;
    }
    ostream.write(data->data(), data->size());
    return 0;
}
