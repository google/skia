/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "src/base/SkTime.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecordOpts.h"
#include "src/core/SkRecorder.h"
#include "src/image/SkImage_Base.h"
#include "tools/flags/CommandLineFlags.h"

#include <cstdio>

static DEFINE_string2(skps, r, "", ".SKPs to dump.");
static DEFINE_string(match, "", "The usual filters on file names to dump.");
static DEFINE_bool2(optimize, O, false, "Run SkRecordOptimize before dumping.");
static DEFINE_int(tile, 1000000000, "Simulated tile size.");
static DEFINE_bool(timeWithCommand, false,
                   "If true, print time next to command, else in first column.");
static DEFINE_string2(write, w, "", "Write the (optimized) picture to the named file.");

class Dumper {
public:
    explicit Dumper(SkCanvas* canvas, int count)
        : fDigits(0)
        , fIndent(0)
        , fIndex(0)
        , fDraw(canvas, nullptr, nullptr, 0, nullptr)
    {
        while (count > 0) {
            count /= 10;
            fDigits++;
        }
    }

    template <typename T>
    void operator()(const T& command) {
        auto start = SkTime::GetNSecs();
        fDraw(command);
        this->print(command, SkTime::GetNSecs() - start);
    }

    void operator()(const SkRecords::NoOp&) {
        // Move on without printing anything.
    }

    template <typename T>
    void print(const T& command, double ns) {
        this->printNameAndTime(command, ns);
    }

    void print(const SkRecords::Restore& command, double ns) {
        --fIndent;
        this->printNameAndTime(command, ns);
    }

    void print(const SkRecords::Save& command, double ns) {
        this->printNameAndTime(command, ns);
        ++fIndent;
    }

    void print(const SkRecords::SaveLayer& command, double ns) {
        this->printNameAndTime(command, ns);
        ++fIndent;
    }

    void print(const SkRecords::DrawPicture& command, double ns) {
        this->printNameAndTime(command, ns);

        if (auto bp = SkPicturePriv::AsSkBigPicture(command.picture)) {
            ++fIndent;

            const SkRecord& record = *bp->record();
            for (int i = 0; i < record.count(); i++) {
                record.visit(i, *this);
            }

            --fIndent;
        }
    }

    void print(const SkRecords::DrawAnnotation& command, double ns) {
        int us = (int)(ns * 1e-3);
        if (!FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            printf("    ");
        }
        if (FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("DrawAnnotation [%g %g %g %g] %s\n",
               command.rect.left(), command.rect.top(), command.rect.right(), command.rect.bottom(),
               command.key.c_str());
    }

private:
    template <typename T>
    void printNameAndTime(const T& command, double ns) {
        int us = (int)(ns * 1e-3);
        if (!FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            printf("    ");
        }
        if (FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        puts(NameOf(command));
    }

    template <typename T>
    static const char* NameOf(const T&) {
    #define CASE(U) case SkRecords::U##_Type: return #U;
        switch (T::kType) { SK_RECORD_TYPES(CASE) }
    #undef CASE
        SkDEBUGFAIL("Unknown T");
        return "Unknown T";
    }

    static const char* NameOf(const SkRecords::SaveLayer&) {
        return "\x1b[31;1mSaveLayer\x1b[0m";  // Bold red.
    }

    int fDigits;
    int fIndent;
    int fIndex;
    SkRecords::Draw fDraw;
};

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    for (int i = 0; i < FLAGS_skps.size(); i++) {
        if (CommandLineFlags::ShouldSkip(FLAGS_match, FLAGS_skps[i])) {
            continue;
        }

        std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(FLAGS_skps[i]);
        if (!stream) {
            SkDebugf("Could not read %s.\n", FLAGS_skps[i]);
            return 1;
        }
        sk_sp<SkPicture> src(SkPicture::MakeFromStream(stream.get()));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", FLAGS_skps[i]);
            return 1;
        }
        const int w = SkScalarCeilToInt(src->cullRect().width());
        const int h = SkScalarCeilToInt(src->cullRect().height());

        SkRecord record;
        SkRecorder rec(&record, w, h);
        src->playback(&rec);

        if (FLAGS_optimize) {
            SkRecordOptimize(&record);
        }

        SkBitmap bitmap;
        bitmap.allocN32Pixels(w, h);
        SkCanvas canvas(bitmap);
        canvas.clipRect(SkRect::MakeWH(SkIntToScalar(FLAGS_tile),
                                       SkIntToScalar(FLAGS_tile)));

        printf("%s %s\n", FLAGS_optimize ? "optimized" : "not-optimized", FLAGS_skps[i]);

        Dumper dumper(&canvas, record.count());
        for (int j = 0; j < record.count(); j++) {
            record.visit(j, dumper);
        }

        if (FLAGS_write.size() > 0) {
            SkPictureRecorder r;
            SkRecordDraw(record,
                         r.beginRecording(SkRect::MakeIWH(w, h)),
                         nullptr,
                         nullptr,
                         0,
                         nullptr,
                         nullptr);
            sk_sp<SkPicture> dst(r.finishRecordingAsPicture());
            SkFILEWStream ostream(FLAGS_write[0]);
            SkSerialProcs sProcs;
            sProcs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
                return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
            };
            dst->serialize(&ostream, &sProcs);
        }
    }

    return 0;
}
