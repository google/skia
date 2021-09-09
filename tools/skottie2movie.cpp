/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skresources/include/SkResources.h"
#include "src/utils/SkOSPath.h"

#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

#include "include/gpu/GrContextOptions.h"

static DEFINE_string2(input, i, "", "skottie animation to render");
static DEFINE_string2(output, o, "", "mp4 file to create");
static DEFINE_string2(assetPath, a, "", "path to assets needed for json file");
static DEFINE_int_2(fps, f, 25, "fps");
static DEFINE_bool2(verbose, v, false, "verbose mode");
static DEFINE_bool2(loop, l, false, "loop mode for profiling");
static DEFINE_int(set_dst_width, 0, "set destination width (height will be computed)");
static DEFINE_bool2(gpu, g, false, "use GPU for rendering");

static void produce_frame(SkSurface* surf, skottie::Animation* anim, double frame) {
    anim->seekFrame(frame);
    surf->getCanvas()->clear(SK_ColorWHITE);
    anim->render(surf->getCanvas());
}

struct AsyncRec {
    SkImageInfo info;
    SkVideoEncoder* encoder;
};

int main(int argc, char** argv) {
    SkGraphics::Init();

    CommandLineFlags::SetUsage("Converts skottie to a mp4");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.count() == 0) {
        SkDebugf("-i input_file.json argument required\n");
        return -1;
    }

    auto contextType = sk_gpu_test::GrContextFactory::kGL_ContextType;
    GrContextOptions grCtxOptions;
    sk_gpu_test::GrContextFactory factory(grCtxOptions);

    SkString assetPath;
    if (FLAGS_assetPath.count() > 0) {
        assetPath.set(FLAGS_assetPath[0]);
    } else {
        assetPath = SkOSPath::Dirname(FLAGS_input[0]);
    }
    SkDebugf("assetPath %s\n", assetPath.c_str());

    auto animation = skottie::Animation::Builder()
        .setResourceProvider(skresources::FileResourceProvider::Make(assetPath))
        .makeFromFile(FLAGS_input[0]);
    if (!animation) {
        SkDebugf("failed to load %s\n", FLAGS_input[0]);
        return -1;
    }

    SkISize dim = animation->size().toRound();
    double duration = animation->duration();
    int fps = SkTPin(FLAGS_fps, 1, 240);
    double fps_scale = animation->fps() / fps;

    float scale = 1;
    if (FLAGS_set_dst_width > 0) {
        scale = FLAGS_set_dst_width / (float)dim.width();
        dim = { FLAGS_set_dst_width, SkScalarRoundToInt(scale * dim.height()) };
    }

    const int frames = SkScalarRoundToInt(duration * fps);
    const double frame_duration = 1.0 / fps;

    if (FLAGS_verbose) {
        SkDebugf("Size %dx%d duration %g, fps %d, frame_duration %g\n",
                 dim.width(), dim.height(), duration, fps, frame_duration);
    }

    SkVideoEncoder encoder;

    GrDirectContext* grctx = nullptr;
    sk_sp<SkSurface> surf;
    sk_sp<SkData> data;

    const auto info = SkImageInfo::MakeN32Premul(dim);
    do {
        double loop_start = SkTime::GetSecs();

        if (!encoder.beginRecording(dim, fps)) {
            SkDEBUGF("Invalid video stream configuration.\n");
            return -1;
        }

        // lazily allocate the surfaces
        if (!surf) {
            if (FLAGS_gpu) {
                grctx = factory.getContextInfo(contextType).directContext();
                surf = SkSurface::MakeRenderTarget(grctx,
                                                   SkBudgeted::kNo,
                                                   info,
                                                   0,
                                                   GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                                   nullptr);
                if (!surf) {
                    grctx = nullptr;
                }
            }
            if (!surf) {
                surf = SkSurface::MakeRaster(info);
            }
            surf->getCanvas()->scale(scale, scale);
        }

        for (int i = 0; i <= frames; ++i) {
            const double frame = i * fps_scale;
            if (FLAGS_verbose) {
                SkDebugf("rendering frame %g\n", frame);
            }

            produce_frame(surf.get(), animation.get(), frame);

            AsyncRec asyncRec = { info, &encoder };
            if (grctx) {
                auto read_pixels_cb = [](SkSurface::ReadPixelsContext ctx,
                                         std::unique_ptr<const SkSurface::AsyncReadResult> result) {
                    if (result && result->count() == 1) {
                        AsyncRec* rec = reinterpret_cast<AsyncRec*>(ctx);
                        rec->encoder->addFrame({rec->info, result->data(0), result->rowBytes(0)});
                    }
                };
                surf->asyncRescaleAndReadPixels(info, {0, 0, info.width(), info.height()},
                                                SkSurface::RescaleGamma::kSrc,
                                                SkImage::RescaleMode::kNearest,
                                                read_pixels_cb, &asyncRec);
                grctx->submit();
            } else {
                SkPixmap pm;
                SkAssertResult(surf->peekPixels(&pm));
                encoder.addFrame(pm);
            }
        }

        if (grctx) {
            // ensure all pending reads are completed
            grctx->flushAndSubmit(true);
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
