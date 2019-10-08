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

struct RenderedFrame {
    SkSurface*  fSurface;
    int         fFrameIndex;

    bool operator<(const RenderedFrame& other) const {
        return fFrameIndex < other.fFrameIndex;
    }
};

struct SurfacePool {
    SkMutex fMutex;
    SkSemaphore fSema;
    SkTDArray<SkSurface*>   fSurfaces;
    int                     fNextFrameIndexToGenerate = 0;
    const int               fFrameCount;
    const int               fSurfaceCount;

    SurfacePool(int frameCount, int surfaceCount, const SkImageInfo& info)
        : fFrameCount(frameCount), fSurfaceCount(surfaceCount)
    {
        for (int i = 0; i < surfaceCount; ++i) {
            *fSurfaces.append() = SkSurface::MakeRaster(info).release();
        }
        fSema.signal(surfaceCount);
    }

    ~SurfacePool() {
        SkASSERT(fSurfaces.count() == fSurfaceCount);
        fSurfaces.unrefAll();
    }

    bool nextFrameToRender(RenderedFrame* frame) {
        if (this->allDone()) {
            return false;
        }

        do {
            fSema.wait();
        } while (!this->tryWork(frame));
        return true;
    }

    void addToPool(SkSurface* surface) {
        {
            SkAutoMutexExclusive ame(fMutex);
            SkASSERT(fSurfaces.find(surface) < 0);
            *fSurfaces.append() = surface;
        }
        fSema.signal();
    }

private:
    bool allDone() {
        SkAutoMutexExclusive ame(fMutex);

        SkASSERT(fNextFrameIndexToGenerate >= 0);
        SkASSERT(fNextFrameIndexToGenerate <= fFrameCount);
        return fNextFrameIndexToGenerate >= fFrameCount;
    }

    bool tryWork(RenderedFrame* frame) {
        SkAutoMutexExclusive ame(fMutex);

        int count = fSurfaces.count();
        if (count == 0) {
            return false;
        }

        SkSurface* surf = fSurfaces[count - 1];
        fSurfaces.remove(count - 1);
        frame->fSurface = surf;
        frame->fFrameIndex = fNextFrameIndexToGenerate++;

        if (0) {
            SkDebugf("surfacepool count %d, frameindex %d\n", count, frame->fFrameIndex);
        }
        return true;
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

    void enque(const RenderedFrame& frame) {
        SkAutoMutexExclusive ame(fMutex);

        binsert(&fFrames, frame);
        while (fFrames.count() > 0 && fFrames[0].fFrameIndex == fNextFrameIndexToEncode) {
            SkPixmap pm;
            fFrames[0].fSurface->peekPixels(&pm);
            fEncoder->addFrame(pm);
            fSurfacePool->addToPool(fFrames[0].fSurface);
            fFrames.remove(0);
            fNextFrameIndexToEncode += 1;
        }
    }
};

static sk_sp<SkData> make_the_movie(SkISize dim, int fps, sk_sp<skottie::ResourceProvider> rp,
                                    sk_sp<SkData> inputData, int frame_count) {
    const int worker_count = FLAGS_threads ? FLAGS_threads : 1;

    SkVideoEncoder encoder;
    encoder.beginRecording(dim, fps);
    auto info = encoder.preferredInfo();

    const int surface_count = worker_count * 2; // need a way to predict this better
    SurfacePool provider(frame_count, surface_count, info);

    RenderedFrameQue receiver(&encoder, &provider);

    // Fire up our workers, each of which loops until we're done

    SkTaskGroup::Enabler enabler(FLAGS_threads - 1);
    SkTaskGroup{}.batch(worker_count, [&](int worker_index) {
        auto anim = skottie::Animation::Builder()
                            .setResourceProvider(rp)
                            .make(static_cast<const char*>(inputData->data()), inputData->size());

        RenderedFrame frame;
        int work_counter = 0;
        while (provider.nextFrameToRender(&frame)) {
            double normal_time = frame.fFrameIndex * 1.0 / (frame_count - 1);

            anim->seek(normal_time);
            frame.fSurface->getCanvas()->clear(SK_ColorWHITE);
            anim->render(frame.fSurface->getCanvas());
            receiver.enque(frame);
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

        outData = make_the_movie(dim, fps, rp, inputData, frame_count);

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
