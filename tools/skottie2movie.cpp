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
#include "include/private/SkMutex.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "src/core/SkTaskGroup.h"
#include "src/utils/SkOSPath.h"

#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(input, i, "", "skottie animation to render");
static DEFINE_string2(output, o, "", "mp4 file to create");
static DEFINE_string2(assetPath, a, "", "path to assets needed for json file");
static DEFINE_int_2(fps, f, 25, "fps");
static DEFINE_bool2(verbose, v, false, "verbose mode");
static DEFINE_bool2(loop, l, false, "loop mode for profiling");
static DEFINE_int(set_dst_width, 0, "set destination width (height will be computed)");
static DEFINE_int(threads,  0, "Number of worker threads (0 -> cores count).");
static DEFINE_bool2(skipEncoder, s, false, "Skip calling the encoder");

extern bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental;

struct RenderedFrame {
    SkImage*    fImage;
    int         fFrameIndex;

    bool operator<(const RenderedFrame& other) const {
        return fFrameIndex < other.fFrameIndex;
    }
};

struct SurfacePool {
    std::atomic<int>        fAtomicIndex;
    const int               fFrameCount;

    SurfacePool(int frameCount) : fAtomicIndex(1), fFrameCount(frameCount) {}

    int nextFrameToRender() {
        int index = fAtomicIndex.fetch_add(+1, std::memory_order_relaxed) + 1;
        if (index >= fFrameCount) {
            index = -1;
        }
        return index;
    }
};

template <typename T> void binsert(SkTDArray<T>* array, const T& value) {
    T* pos = std::lower_bound(array->begin(), array->end(), value);
    *array->insert(pos - array->begin()) = value;
}

struct RenderedFrameQue {
    SkMutex fMutex;
    SkTDArray<RenderedFrame> fFrames;
    SkVideoEncoder*          fEncoder;
    SurfacePool*             fSurfacePool;
    int                      fNextFrameIndexToEncode = 0;   // monotonic increasing

    RenderedFrameQue(SkVideoEncoder* encoder, SurfacePool* pool)
        : fEncoder(encoder), fSurfacePool(pool)
    {}

    void enque(SkImage* inputImage, int frameIndex) {
        bool need_insert = true;

        SkImage* image;
        do {
            image = nullptr;
            {
                SkAutoMutexExclusive ame(fMutex);
                if (need_insert) {
                    binsert(&fFrames, { inputImage, frameIndex });
                    need_insert = false;
                }
                if (fFrames.count() > 0 && fFrames[0].fFrameIndex == fNextFrameIndexToEncode) {
                    image = fFrames[0].fImage;
                    fFrames.remove(0);
                    // don't modify fNextFrameIndexToEncode yet
                }
            }

            if (image) {
                SkPixmap pm;
                image->peekPixels(&pm);
                if (!FLAGS_skipEncoder) {
                    fEncoder->addFrame(pm);
                }
                image->unref();

                {
                    SkAutoMutexExclusive ame(fMutex);
                    fNextFrameIndexToEncode += 1;
                }
            }
        } while (image);
    }
};

static sk_sp<SkData> make_the_movie(SkISize dim, int fps, sk_sp<skottie::ResourceProvider> rp,
                                    sk_sp<SkData> inputData, int frame_count, float scale) {
    const int worker_count = FLAGS_threads ? FLAGS_threads : 1;

    auto info = SkImageInfo::MakeN32Premul(dim);
    SkVideoEncoder encoder;
    encoder.beginRecording(dim, fps);

    SurfacePool provider(frame_count);

    RenderedFrameQue receiver(&encoder, &provider);

    // Fire up our workers, each of which loops until we're done

    SkTaskGroup::Enabler enabler(FLAGS_threads - 1);
    SkTaskGroup{}.batch(worker_count, [&](int worker_index) {
        auto surf = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(dim.width(), dim.height()));
        SkCanvas* canvas = surf->getCanvas();
        auto anim = skottie::Animation::Builder()
                            .setResourceProvider(rp)
                            .make(static_cast<const char*>(inputData->data()), inputData->size());

        int work_counter = 0;
        for (int frameIndex = worker_index; frameIndex < frame_count; frameIndex += worker_count) {
            double normal_time = frameIndex * 1.0 / (frame_count - 1);

            anim->seek(normal_time);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->scale(scale, scale);
            canvas->clear(SK_ColorWHITE);
            anim->render(canvas);
            receiver.enque(surf->makeImageSnapshot().release(), frameIndex);
            work_counter += 1;
        }
        if (FLAGS_verbose) {
            SkDebugf("[%d] work counter %d\n", worker_index, work_counter);
        }
    });

    return encoder.endRecording();
}

int main(int argc, char** argv) {
    SkGraphics::Init();
    gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = true;

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

    auto rp = skottie_utils::CachingResourceProvider::Make(
                  skottie_utils::FileResourceProvider::Make(SkOSPath::Dirname(FLAGS_input[0]),
                  /*predecode=*/true));
    auto inputData = SkData::MakeFromFileName(FLAGS_input[0]);

    if (!inputData) {
        SkDebugf("Could not load %s.\n", FLAGS_input[0]);
        return 1;
    }

    SkISize dim;
    double duration;
    {
        auto anim = skottie::Animation::Builder()
                            .setResourceProvider(rp)
                            .make(static_cast<const char*>(inputData->data()), inputData->size());
        dim = anim->size().toRound();
        duration = anim->duration();
    }

    int fps = FLAGS_fps;
    if (fps < 1) {
        fps = 1;
    } else if (fps > 240) {
        fps = 240;
    }

    float scale = 1;
    if (FLAGS_set_dst_width > 0) {
        scale = FLAGS_set_dst_width / (float)dim.width();
        dim = { FLAGS_set_dst_width, SkScalarRoundToInt(scale * dim.height()) };
    }

    const int frame_count = SkScalarRoundToInt(duration * fps);

    if (FLAGS_verbose) {
        SkDebugf("Size %dx%d duration %g, fps %d\n",
                 dim.width(), dim.height(), duration, fps);
    }

    sk_sp<SkData> outData;
    do {
        double loop_start = SkTime::GetSecs();

        outData = make_the_movie(dim, fps, rp, inputData, frame_count, scale);

        if (FLAGS_loop) {
            double loop_dur = SkTime::GetSecs() - loop_start;
            SkDebugf("recording secs %g, frames %d, recording fps %d\n",
                     loop_dur, frame_count, (int)(frame_count / loop_dur));
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
    ostream.write(outData->data(), outData->size());
    return 0;
}
