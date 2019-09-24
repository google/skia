/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"

#include <vector>

static DEFINE_string2(input    , i, nullptr, "Input .json file.");
static DEFINE_string2(writePath, w, nullptr, "Output directory.  Frames are names [0-9]{6}.png.");
static DEFINE_string2(format   , f, "png"  , "Output format (png or skp)");

static DEFINE_double(t0,   0, "Timeline start [0..1].");
static DEFINE_double(t1,   1, "Timeline stop [0..1].");
static DEFINE_double(fps, 30, "Decode frames per second.");

static DEFINE_int(width , 800, "Render width.");
static DEFINE_int(height, 600, "Render height.");
static DEFINE_int(threads, -1, "Number of worker threads (-1 -> cores count).");

namespace {

class Sink {
public:
    virtual ~Sink() = default;
    Sink(const Sink&) = delete;
    Sink& operator=(const Sink&) = delete;

    bool handleFrame(const skottie::Animation* anim, size_t idx) const {
        const auto frame_file = SkStringPrintf("0%06d.%s", idx, fExtension.c_str());
        SkFILEWStream stream (SkOSPath::Join(FLAGS_writePath[0], frame_file.c_str()).c_str());

        if (!stream.isValid()) {
            SkDebugf("Could not open '%s/%s' for writing.\n",
                     FLAGS_writePath[0], frame_file.c_str());
            return false;
        }

        return this->saveFrame(anim, &stream);
    }

protected:
    Sink(const char* ext) : fExtension(ext) {}

    virtual bool saveFrame(const skottie::Animation* anim, SkFILEWStream*) const = 0;

private:
    const SkString fExtension;
};

class PNGSink final : public Sink {
public:
    PNGSink()
        : INHERITED("png")
        , fSurface(SkSurface::MakeRasterN32Premul(FLAGS_width, FLAGS_height)) {
        if (!fSurface) {
            SkDebugf("Could not allocate a %d x %d surface.\n", FLAGS_width, FLAGS_height);
        }
    }

    bool saveFrame(const skottie::Animation* anim, SkFILEWStream* stream) const override {
        if (!fSurface) return false;

        auto* canvas = fSurface->getCanvas();
        SkAutoCanvasRestore acr(canvas, true);

        canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(anim->size()),
                                                SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                                SkMatrix::kCenter_ScaleToFit));

        canvas->clear(SK_ColorTRANSPARENT);
        anim->render(canvas);


        // Set encoding options to favor speed over size.
        SkPngEncoder::Options options;
        options.fZLibLevel   = 1;
        options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;

        sk_sp<SkImage> img = fSurface->makeImageSnapshot();
        SkPixmap pixmap;
        return img->peekPixels(&pixmap)
            && SkPngEncoder::Encode(stream, pixmap, options);
    }

private:
    const sk_sp<SkSurface> fSurface;

    using INHERITED = Sink;
};

class SKPSink final : public Sink {
public:
    SKPSink() : INHERITED("skp") {}

    bool saveFrame(const skottie::Animation* anim, SkFILEWStream* stream) const override {
        SkPictureRecorder recorder;

        auto canvas = recorder.beginRecording(FLAGS_width, FLAGS_height);
        canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(anim->size()),
                                                SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                                SkMatrix::kCenter_ScaleToFit));
        anim->render(canvas);
        recorder.finishRecordingAsPicture()->serialize(stream);

        return true;
    }

private:
    const sk_sp<SkSurface> fSurface;

    using INHERITED = Sink;
};

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
        SkDebugf("Animation loaded with %lu error%s, %lu warning%s.\n",
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

std::unique_ptr<Sink> MakeSink(const char* fmt) {
    if (0 == strcmp(fmt, "png")) {
        return skstd::make_unique<PNGSink>();
    }
    if (0 == strcmp(fmt, "skp")) {
        return skstd::make_unique<SKPSink>();
    }

    SkDebugf("Unknown format: %s\n", FLAGS_format[0]);
    return nullptr;
}

} // namespace

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
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

    auto logger = sk_make_sp<Logger>();
    auto     rp = skottie_utils::CachingResourceProvider::Make(
                      skottie_utils::FileResourceProvider::Make(SkOSPath::Dirname(FLAGS_input[0]),
                                                                /*predecode=*/true));
    auto data   = SkData::MakeFromFileName(FLAGS_input[0]);

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

    logger->report();

    static constexpr double kMaxFrames = 10000;
    const auto t0 = SkTPin(FLAGS_t0, 0.0, 1.0),
               t1 = SkTPin(FLAGS_t1,  t0, 1.0),
               dt = 1 / std::min(anim->duration() * FLAGS_fps, kMaxFrames);

    const auto frame_count = static_cast<int>((t1 - t0) / dt);

    SkTaskGroup::Enabler enabler(FLAGS_threads);
    SkTaskGroup{}.batch(frame_count, [&](int i) {
        thread_local static auto* tl_sink = MakeSink(FLAGS_format[0]).release();
        thread_local static auto* tl_anim =
                skottie::Animation::Builder()
                    .setResourceProvider(rp)
                    .make(static_cast<const char*>(data->data()), data->size())
                    .release();

        if (tl_sink && tl_anim) {
            tl_anim->seek(t0 + dt * i);
            tl_sink->handleFrame(tl_anim, i);
        }
    });

    return 0;
}
