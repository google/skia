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
    }

    ~SurfacePool() {
        SkASSERT(fSurfaces.count() == fSurfaceCount);
        fSurfaces.unrefAll();
    }

    bool nextFrameToRender(RenderedFrame* frame) {
        SkAutoMutexExclusive ame(fMutex);

        SkASSERT(fNextFrameIndexToGenerate >= 0);
        SkASSERT(fNextFrameIndexToGenerate <= fFrameCount);
        if (fNextFrameIndexToGenerate >= fFrameCount) {
            return false;   // we're done
        }

        int count = fSurfaces.count();
        SkSurface* surf = fSurfaces[count - 1];
        fSurfaces.remove(count - 1);
        frame->fSurface = surf;
        frame->fFrameIndex = fNextFrameIndexToGenerate++;
        return true;
    }

    void addToPool(SkSurface* surface) {
        SkAutoMutexExclusive ame(fMutex);

        SkASSERT(fSurfaces.find(surface) < 0);
        *fSurfaces.append() = surface;
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
    const double frame_duration = 1.0 / fps;

    if (FLAGS_verbose) {
        SkDebugf("Size %dx%d duration %g, fps %d, frame_duration %g\n",
                 dim.width(), dim.height(), duration, fps, frame_duration);
    }

    // Setup our shared encoder, provider and receiver

    const int worker_count = FLAGS_threads ? FLAGS_threads : 1;

    SkVideoEncoder encoder;
    encoder.beginRecording(dim, fps);
    auto info = encoder.preferredInfo();

    const int surface_count = worker_count * 2; // need a way to predict this better
    SurfacePool provider(frame_count, surface_count, info);

    RenderedFrameQue receiver(&encoder, &provider);

    // Fire up our workers, each of which loops until we're done

    SkTaskGroup::Enabler enabler(FLAGS_threads - 1);
    SkTaskGroup{}.batch(worker_count, [&](int) {
        auto anim = skottie::Animation::Builder()
                            .setResourceProvider(rp)
                            .make(static_cast<const char*>(inputData->data()), inputData->size());

        RenderedFrame frame;
        while (provider.nextFrameToRender(&frame)) {
            double normal_time = frame.fFrameIndex * 1.0 / (frame_count - 1);

            anim->seek(normal_time);
            frame.fSurface->getCanvas()->clear(SK_ColorWHITE);
            anim->render(frame.fSurface->getCanvas());
            receiver.enque(frame);
        }
    });

    // All done, now stream out the video

    sk_sp<SkData> data = encoder.endRecording();

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
