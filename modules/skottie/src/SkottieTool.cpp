/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkMakeUnique.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkSurface.h"

#if defined(SK_ENABLE_SKOTTIE)
#include "Skottie.h"
#endif

DEFINE_string2(input    , i, nullptr, "Input .json file.");
DEFINE_string2(writePath, w, nullptr, "Output directory.  Frames are names [0-9]{6}.png.");
DEFINE_string2(format   , f, "png"  , "Output format (png or skp)");

DEFINE_double(t0,   0, "Timeline start [0..1].");
DEFINE_double(t1,   1, "Timeline stop [0..1].");
DEFINE_double(fps, 30, "Decode frames per second.");

DEFINE_int32(width , 800, "Render width.");
DEFINE_int32(height, 600, "Render height.");

namespace {

class Sink {
public:
    virtual ~Sink() = default;
    Sink(const Sink&) = delete;
    Sink& operator=(const Sink&) = delete;

    bool handleFrame(const sk_sp<skottie::Animation>& anim, size_t idx) const {
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

    virtual bool saveFrame(const sk_sp<skottie::Animation>& anim, SkFILEWStream*) const = 0;

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

    bool saveFrame(const sk_sp<skottie::Animation>& anim, SkFILEWStream* stream) const override {
        if (!fSurface) return false;

        auto* canvas = fSurface->getCanvas();
        SkAutoCanvasRestore acr(canvas, true);

        canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(anim->size()),
                                                SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                                SkMatrix::kCenter_ScaleToFit));

        canvas->clear(SK_ColorTRANSPARENT);
        anim->render(canvas);

        auto png_data = fSurface->makeImageSnapshot()->encodeToData();
        if (!png_data) {
            SkDebugf("Failed to encode frame!\n");
            return false;
        }

        return stream->write(png_data->data(), png_data->size());
    }

private:
    const sk_sp<SkSurface> fSurface;

    using INHERITED = Sink;
};

class SKPSink final : public Sink {
public:
    SKPSink() : INHERITED("skp") {}

    bool saveFrame(const sk_sp<skottie::Animation>& anim, SkFILEWStream* stream) const override {
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

} // namespace

int main(int argc, char** argv) {
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

    std::unique_ptr<Sink> sink;
    if (0 == strcmp(FLAGS_format[0], "png")) {
        sink = skstd::make_unique<PNGSink>();
    } else if (0 == strcmp(FLAGS_format[0], "skp")) {
        sink = skstd::make_unique<SKPSink>();
    } else {
        SkDebugf("Unknown format: %s\n", FLAGS_format[0]);
        return 1;
    }

    auto anim = skottie::Animation::MakeFromFile(FLAGS_input[0]);
    if (!anim) {
        SkDebugf("Could not load animation: '%s'.\n", FLAGS_input[0]);
        return 1;
    }

    static constexpr double kMaxFrames = 10000;
    const auto t0 = SkTPin(FLAGS_t0, 0.0, 1.0),
               t1 = SkTPin(FLAGS_t1,  t0, 1.0),
               advance = 1 / std::min(anim->duration() * FLAGS_fps, kMaxFrames);

    size_t frame_index = 0;
    for (auto t = t0; t <= t1; t += advance) {
        anim->seek(t);
        sink->handleFrame(anim, frame_index++);
    }

    return 0;
}
