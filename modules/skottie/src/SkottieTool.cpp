/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skresources/include/SkResources.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/image/SkImage_Base.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <numeric>
#include <vector>

#if !defined(CPU_ONLY)
#include "include/gpu/GrContextOptions.h"
#include "tools/gpu/GrContextFactory.h"
#endif

#if defined(HAVE_VIDEO_ENCODER)
    #include "experimental/ffmpeg/SkVideoEncoder.h"
    const char* formats_help = "Output format (png, skp, mp4, or null)";
#else
    const char* formats_help = "Output format (png, skp, or null)";
#endif

static DEFINE_string2(input    , i, nullptr, "Input .json file.");
static DEFINE_string2(writePath, w, nullptr, "Output directory.  Frames are names [0-9]{6}.png.");
static DEFINE_string2(format   , f, "png"  , formats_help);

static DEFINE_double(t0,    0, "Timeline start [0..1].");
static DEFINE_double(t1,    1, "Timeline stop [0..1].");
static DEFINE_double(fps,   0, "Decode frames per second (default is animation native fps).");

static DEFINE_int(width , 800, "Render width.");
static DEFINE_int(height, 600, "Render height.");
static DEFINE_int(threads,  0, "Number of worker threads (0 -> cores count).");

static DEFINE_bool2(gpu, g, false, "Enable GPU rasterization.");

namespace {

static constexpr SkColor kClearColor = SK_ColorWHITE;

enum class OutputFormat {
    kPNG,
    kSKP,
    kNull,
    kMP4,
};


auto ms_since(std::chrono::steady_clock::time_point start) {
    const auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

std::unique_ptr<SkFILEWStream> make_file_stream(size_t frame_index, const char* extension) {
    const auto file = SkStringPrintf("0%06zu.%s", frame_index, extension);
    const auto path = SkOSPath::Join(FLAGS_writePath[0], file.c_str());

    auto stream = std::make_unique<SkFILEWStream>(path.c_str());

    return stream->isValid() ? std::move(stream) : nullptr;
}

class FrameSink {
public:
    virtual ~FrameSink() = default;

    static std::unique_ptr<FrameSink> Make(OutputFormat fmt, size_t frame_count);

    virtual void writeFrame(sk_sp<SkImage> frame, size_t frame_index) = 0;

    virtual void finalize(double fps) {}

protected:
    FrameSink() = default;

private:
    FrameSink(const FrameSink&)            = delete;
    FrameSink& operator=(const FrameSink&) = delete;
};

class PNGSink final : public FrameSink {
public:
    void writeFrame(sk_sp<SkImage> frame, size_t frame_index) override {
        auto stream = make_file_stream(frame_index, "png");

        if (!frame || !stream) {
            return;
        }

        // Set encoding options to favor speed over size.
        SkPngEncoder::Options options;
        options.fZLibLevel   = 1;
        options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;

        SkPixmap pixmap;
        SkAssertResult(frame->peekPixels(&pixmap));

        SkPngEncoder::Encode(stream.get(), pixmap, options);
    }
};

class NullSink final : public FrameSink {
public:
    void writeFrame(sk_sp<SkImage>, size_t) override {}
};

#if defined(HAVE_VIDEO_ENCODER)
class MP4Sink final : public FrameSink {
public:
    explicit MP4Sink(size_t frame_count) {
        fFrames.resize(frame_count);
    }

    void writeFrame(sk_sp<SkImage> frame, size_t frame_index) override {
        fFrames[frame_index].set_value(std::move(frame));
    }

    void finalize(double fps) override {
        SkVideoEncoder encoder;
        if (!encoder.beginRecording({FLAGS_width, FLAGS_height}, sk_double_round2int(fps))) {
            fprintf(stderr, "Invalid video stream configuration.\n");
        }

        std::vector<double> starved_ms;
        starved_ms.reserve(fFrames.size());

        for (auto& frame_promise : fFrames) {
            const auto start = std::chrono::steady_clock::now();
            auto frame = frame_promise.get_future().get();
            starved_ms.push_back(ms_since(start));

            if (!frame) continue;

            SkPixmap pixmap;
            SkAssertResult(frame->peekPixels(&pixmap));
            encoder.addFrame(pixmap);
        }

        auto mp4 = encoder.endRecording();

        SkFILEWStream{FLAGS_writePath[0]}
            .write(mp4->data(), mp4->size());

        // If everything's going well, the first frame should account for the most,
        // and ideally nearly all, starvation.
        double first = starved_ms[0];
        std::sort(starved_ms.begin(), starved_ms.end());
        double sum = std::accumulate(starved_ms.begin(), starved_ms.end(), 0);
        printf("Encoder starved stats: "
               "min %gms, med %gms, avg %gms, max %gms, sum %gms, first %gms (%s)\n",
               starved_ms[0], starved_ms[fFrames.size()/2], sum/fFrames.size(), starved_ms.back(),
               sum, first, first == starved_ms.back() ? "ok" : "BAD");

    }

    std::vector<std::promise<sk_sp<SkImage>>> fFrames;
};
#endif // HAVE_VIDEO_ENCODER

std::unique_ptr<FrameSink> FrameSink::Make(OutputFormat fmt, size_t frame_count) {
    switch (fmt) {
    case OutputFormat::kPNG:
        return std::make_unique<PNGSink>();
    case OutputFormat::kSKP:
        // The SKP generator does not use a sink.
        [[fallthrough]];
    case OutputFormat::kNull:
        return std::make_unique<NullSink>();
    case OutputFormat::kMP4:
#if defined(HAVE_VIDEO_ENCODER)
        return std::make_unique<MP4Sink>(frame_count);
#else
        return nullptr;
#endif
    }

    SkUNREACHABLE;
}

class FrameGenerator {
public:
    virtual ~FrameGenerator() = default;

    static std::unique_ptr<FrameGenerator> Make(FrameSink*, OutputFormat, const SkMatrix&);

    virtual void generateFrame(const skottie::Animation*, size_t frame_index) {}

protected:
    explicit FrameGenerator(FrameSink* sink) : fSink(sink) {}

    FrameSink* fSink;

private:
    FrameGenerator(const FrameGenerator&)            = delete;
    FrameGenerator& operator=(const FrameGenerator&) = delete;
};

class CPUGenerator final : public FrameGenerator {
public:
#if defined(GPU_ONLY)
    static std::unique_ptr<FrameGenerator> Make(FrameSink* sink, const SkMatrix& matrix) {
        return nullptr;
    }
#else
    static std::unique_ptr<FrameGenerator> Make(FrameSink* sink, const SkMatrix& matrix) {
        auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(FLAGS_width, FLAGS_height));
        if (!surface) {
            SkDebugf("Could not allocate a %d x %d surface.\n", FLAGS_width, FLAGS_height);
            return nullptr;
        }

        return std::unique_ptr<FrameGenerator>(new CPUGenerator(sink, std::move(surface), matrix));
    }

    void generateFrame(const skottie::Animation* anim, size_t frame_index) override {
        fSurface->getCanvas()->clear(kClearColor);
        anim->render(fSurface->getCanvas());

        fSink->writeFrame(fSurface->makeImageSnapshot(), frame_index);
    }

private:
    CPUGenerator(FrameSink* sink, sk_sp<SkSurface> surface, const SkMatrix& scale_matrix)
        : FrameGenerator(sink)
        , fSurface(std::move(surface))
    {
        fSurface->getCanvas()->concat(scale_matrix);
    }

    const sk_sp<SkSurface> fSurface;
#endif // !GPU_ONLY
};

class SKPGenerator final : public FrameGenerator {
public:
#if defined(CPU_ONLY) || defined(GPU_ONLY)
    static std::unique_ptr<FrameGenerator> Make(FrameSink* sink, const SkMatrix& matrix) {
        return nullptr;
    }
#else
    static std::unique_ptr<FrameGenerator> Make(FrameSink* sink, const SkMatrix& scale_matrix) {
        return std::unique_ptr<FrameGenerator>(new SKPGenerator(sink, scale_matrix));
    }

    void generateFrame(const skottie::Animation* anim, size_t frame_index) override {
        auto* canvas = fRecorder.beginRecording(FLAGS_width, FLAGS_height);
        canvas->concat(fScaleMatrix);
        anim->render(canvas);

        auto frame  = fRecorder.finishRecordingAsPicture();
        auto stream = make_file_stream(frame_index, "skp");

        if (frame && stream) {
            SkSerialProcs sProcs;
            sProcs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
                return SkPngEncoder::Encode(as_IB(img)->directContext(), img,
                                            SkPngEncoder::Options{});
            };
            frame->serialize(stream.get(), &sProcs);
        }
    }

private:
    SKPGenerator(FrameSink* sink, const SkMatrix& scale_matrix)
        : FrameGenerator(sink)
        , fScaleMatrix(scale_matrix)
    {}

    const SkMatrix    fScaleMatrix;
    SkPictureRecorder fRecorder;
#endif // !CPU_ONLY && !GPU_ONLY
};

class GPUGenerator final : public FrameGenerator {
public:
#if defined(CPU_ONLY)
    static std::unique_ptr<FrameGenerator> Make(FrameSink* sink, const SkMatrix& matrix) {
        return nullptr;
    }
#else
    static std::unique_ptr<FrameGenerator> Make(FrameSink* sink, const SkMatrix& matrix) {
        auto gpu_generator = std::unique_ptr<GPUGenerator>(new GPUGenerator(sink, matrix));

        return gpu_generator->isValid()
                ? std::unique_ptr<FrameGenerator>(gpu_generator.release())
                : nullptr;
    }

    ~GPUGenerator() override {
        // ensure all pending reads are completed
        fCtx->flushAndSubmit(GrSyncCpu::kYes);
    }

    void generateFrame(const skottie::Animation* anim, size_t frame_index) override {
        fSurface->getCanvas()->clear(kClearColor);
        anim->render(fSurface->getCanvas());

        auto rec = std::make_unique<AsyncRec>(fSink, frame_index);
        fSurface->asyncRescaleAndReadPixels(SkImageInfo::MakeN32Premul(FLAGS_width, FLAGS_height),
                                            {0, 0, FLAGS_width, FLAGS_height},
                                            SkSurface::RescaleGamma::kSrc,
                                            SkImage::RescaleMode::kNearest,
                                            AsyncCallback, rec.release());

        fCtx->submit();
    }

private:
    GPUGenerator(FrameSink* sink, const SkMatrix& matrix)
        : FrameGenerator(sink)
    {
        fCtx = fFactory.getContextInfo(skgpu::ContextType::kGL).directContext();
        fSurface = SkSurfaces::RenderTarget(fCtx,
                                            skgpu::Budgeted::kNo,
                                            SkImageInfo::MakeN32Premul(FLAGS_width, FLAGS_height),
                                            0,
                                            GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                            nullptr);
        if (fSurface) {
            fSurface->getCanvas()->concat(matrix);
        } else {
            fprintf(stderr, "Could not initialize GL context.\n");
        }
    }

    bool isValid() const { return !!fSurface; }

    struct AsyncRec {
        FrameSink* sink;
        size_t     index;

        AsyncRec(FrameSink* sink, size_t index) : sink(sink), index(index) {}
    };

    static void AsyncCallback(SkSurface::ReadPixelsContext ctx,
                              std::unique_ptr<const SkSurface::AsyncReadResult> result) {
        std::unique_ptr<const AsyncRec> rec(reinterpret_cast<const AsyncRec*>(ctx));
        if (result && result->count() == 1) {
            SkPixmap pm(SkImageInfo::MakeN32Premul(FLAGS_width, FLAGS_height),
                        result->data(0), result->rowBytes(0));

            auto release_proc = [](const void*, SkImages::ReleaseContext ctx) {
                std::unique_ptr<const SkSurface::AsyncReadResult>
                        adopted(reinterpret_cast<const SkSurface::AsyncReadResult*>(ctx));
            };

            auto frame_image =
                    SkImages::RasterFromPixmap(pm, release_proc, (void*)result.release());

            rec->sink->writeFrame(std::move(frame_image), rec->index);
        }
    }

    sk_gpu_test::GrContextFactory fFactory;
    GrDirectContext*              fCtx;
    sk_sp<SkSurface>              fSurface;
#endif // !CPU_ONLY
};

std::unique_ptr<FrameGenerator> FrameGenerator::Make(FrameSink* sink,
                                                     OutputFormat fmt,
                                                     const SkMatrix& matrix) {
    if (fmt == OutputFormat::kSKP) {
        return SKPGenerator::Make(sink, matrix);
    }

    return FLAGS_gpu
            ? GPUGenerator::Make(sink, matrix)
            : CPUGenerator::Make(sink, matrix);
}

class Logger final : public skottie::Logger {
public:
    struct LogEntry {
        SkString fMessage,
                 fJSON;
    };

    void log(skottie::Logger::Level lvl, const char message[], const char json[]) override {
        auto& log = lvl == skottie::Logger::Level::kError ? fErrors : fWarnings;
        log.push_back({ SkString(message), json ? SkString(json) : SkString() });
    }

    void report() const {
        SkDebugf("Animation loaded with %zu error%s, %zu warning%s.\n",
                 fErrors.size(), fErrors.size() == 1 ? "" : "s",
                 fWarnings.size(), fWarnings.size() == 1 ? "" : "s");

        const auto& show = [](const LogEntry& log, const char prefix[]) {
            SkDebugf("%s%s", prefix, log.fMessage.c_str());
            if (!log.fJSON.isEmpty())
                SkDebugf(" : %s", log.fJSON.c_str());
            SkDebugf("\n");
        };

        for (const auto& err : fErrors)   show(err, "  !! ");
        for (const auto& wrn : fWarnings) show(wrn, "  ?? ");
    }

private:
    std::vector<LogEntry> fErrors,
                          fWarnings;
};

} // namespace

extern bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental;

int main(int argc, char** argv) {
    gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = true;
    CommandLineFlags::Parse(argc, argv);
    SkGraphics::Init();

    if (FLAGS_input.isEmpty() || FLAGS_writePath.isEmpty()) {
        SkDebugf("Missing required 'input' and 'writePath' args.\n");
        return 1;
    }

    OutputFormat fmt;
    if (0 == strcmp(FLAGS_format[0],  "png")) {
        fmt = OutputFormat::kPNG;
    } else if (0 == strcmp(FLAGS_format[0],  "skp")) {
        fmt = OutputFormat::kSKP;
    }  else if (0 == strcmp(FLAGS_format[0], "null")) {
        fmt = OutputFormat::kNull;
#if defined(HAVE_VIDEO_ENCODER)
    } else if (0 == strcmp(FLAGS_format[0],  "mp4")) {
        fmt = OutputFormat::kMP4;
#endif
    } else {
        fprintf(stderr, "Unknown format: %s\n", FLAGS_format[0]);
        return 1;
    }

    if (fmt != OutputFormat::kMP4 && !sk_mkdir(FLAGS_writePath[0])) {
        return 1;
    }

    auto logger = sk_make_sp<Logger>();
    auto     rp = skresources::CachingResourceProvider::Make(
                    skresources::DataURIResourceProviderProxy::Make(
                      skresources::FileResourceProvider::Make(SkOSPath::Dirname(FLAGS_input[0]),
                                                                /*predecode=*/true),
                      /*predecode=*/true));
    auto data   = SkData::MakeFromFileName(FLAGS_input[0]);
    auto precomp_interceptor =
            sk_make_sp<skottie_utils::ExternalAnimationPrecompInterceptor>(rp, "__");

    if (!data) {
        SkDebugf("Could not load %s.\n", FLAGS_input[0]);
        return 1;
    }

    // Instantiate an animation on the main thread for two reasons:
    //   - we need to know its duration upfront
    //   - we want to only report parsing errors once
    auto anim = skottie::Animation::Builder()
            .setLogger(logger)
            .setResourceProvider(rp)
            .make(static_cast<const char*>(data->data()), data->size());
    if (!anim) {
        SkDebugf("Could not parse animation: '%s'.\n", FLAGS_input[0]);
        return 1;
    }

    const auto scale_matrix = SkMatrix::RectToRect(SkRect::MakeSize(anim->size()),
                                                   SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                                   SkMatrix::kCenter_ScaleToFit);
    logger->report();

    const auto t0 = SkTPin(FLAGS_t0, 0.0, 1.0),
               t1 = SkTPin(FLAGS_t1,  t0, 1.0),
       native_fps = anim->fps(),
           frame0 = anim->duration() * t0 * native_fps,
         duration = anim->duration() * (t1 - t0);

    double fps = FLAGS_fps > 0 ? FLAGS_fps : native_fps;
    if (fps <= 0) {
        SkDebugf("Invalid fps: %f.\n", fps);
        return 1;
    }

    auto frame_count = static_cast<int>(duration * fps);
    static constexpr int kMaxFrames = 10000;
    if (frame_count > kMaxFrames) {
        frame_count = kMaxFrames;
        fps = frame_count / duration;
    }
    const auto fps_scale = native_fps / fps;

    printf("Rendering %f seconds (%d frames @%f fps).\n", duration, frame_count, fps);

    const auto sink = FrameSink::Make(fmt, frame_count);

    std::vector<double> frames_ms(frame_count);

    const auto thread_count = FLAGS_gpu ? 0 : FLAGS_threads - 1;
    SkTaskGroup::Enabler enabler(thread_count);

    SkTaskGroup tg;
    {
        // Depending on type (gpu vs. everything else), we use either a single generator
        // or one generator per worker thread, respectively.
        // Scoping is important for the single generator case because we want its destructor to
        // flush out any pending async operations.
        std::unique_ptr<FrameGenerator> singleton_generator;
        if (FLAGS_gpu) {
            singleton_generator = FrameGenerator::Make(sink.get(), fmt, scale_matrix);
        }

        tg.batch(frame_count, [&](int i) {
            // SkTaskGroup::Enabler creates a LIFO work pool,
            // but we want our early frames to start first.
            i = frame_count - 1 - i;

            const auto start = std::chrono::steady_clock::now();
            thread_local static auto* anim =
                    skottie::Animation::Builder()
                        .setResourceProvider(rp)
                        .setPrecompInterceptor(precomp_interceptor)
                        .make(static_cast<const char*>(data->data()), data->size())
                        .release();
            thread_local static auto* gen = singleton_generator
                    ? singleton_generator.get()
                    : FrameGenerator::Make(sink.get(), fmt, scale_matrix).release();

            if (gen && anim) {
                anim->seekFrame(frame0 + i * fps_scale);
                gen->generateFrame(anim, SkToSizeT(i));
            } else {
                sink->writeFrame(nullptr, SkToSizeT(i));
            }

            frames_ms[i] = ms_since(start);
        });
    }

    sink->finalize(fps);
    tg.wait();


    std::sort(frames_ms.begin(), frames_ms.end());
    double sum = std::accumulate(frames_ms.begin(), frames_ms.end(), 0);
    printf("Frame time stats: min %gms, med %gms, avg %gms, max %gms, sum %gms\n",
           frames_ms[0], frames_ms[frame_count/2], sum/frame_count, frames_ms.back(), sum);

    return 0;
}
