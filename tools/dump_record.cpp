/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include "BenchTimer.h"
#include "LazyDecodeBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkStream.h"

DEFINE_string2(skps, r, "", ".SKPs to dump.");
DEFINE_string(match, "", "The usual filters on file names to dump.");
DEFINE_bool2(optimize, O, false, "Run SkRecordOptimize before dumping.");
DEFINE_int32(tile, 1000000000, "Simulated tile size.");
DEFINE_bool(timeWithCommand, false, "If true, print time next to command, else in first column.");

class Dumper {
public:
    explicit Dumper(SkCanvas* canvas, int count) : fDigits(0), fIndent(0), fDraw(canvas) {
        while (count > 0) {
            count /= 10;
            fDigits++;
        }
    }

    unsigned index() const { return fDraw.index(); }
    void next() { fDraw.next(); }

    template <typename T>
    void operator()(const T& command) {
        BenchTimer timer;
        timer.start();
            fDraw(command);
        timer.end();

        this->print(command, timer.fCpu);
    }

    void operator()(const SkRecords::NoOp&) {
        // Move on without printing anything.
    }

    template <typename T>
    void print(const T& command, double time) {
        this->printNameAndTime(command, time);
    }

    void print(const SkRecords::Restore& command, double time) {
        --fIndent;
        this->printNameAndTime(command, time);
    }

    void print(const SkRecords::Save& command, double time) {
        this->printNameAndTime(command, time);
        ++fIndent;
    }

    void print(const SkRecords::SaveLayer& command, double time) {
        this->printNameAndTime(command, time);
        ++fIndent;
    }

private:
    template <typename T>
    void printNameAndTime(const T& command, double time) {
        if (!FLAGS_timeWithCommand) {
            printf("%6.1f ", time * 1000);
        }
        printf("%*d ", fDigits, fDraw.index());
        for (int i = 0; i < fIndent; i++) {
            putchar('\t');
        }
        if (FLAGS_timeWithCommand) {
            printf("%6.1f ", time * 1000);
        }
        puts(NameOf(command));
    }

    template <typename T>
    static const char* NameOf(const T&) {
    #define CASE(U) case SkRecords::U##_Type: return #U;
        switch(T::kType) { SK_RECORD_TYPES(CASE); }
    #undef CASE
        SkDEBUGFAIL("Unknown T");
        return "Unknown T";
    }

    static const char* NameOf(const SkRecords::SaveLayer&) {
        return "\x1b[31;1mSaveLayer\x1b[0m";  // Bold red.
    }

    int fDigits;
    int fIndent;
    SkRecords::Draw fDraw;
};


static void dump(const char* name, int w, int h, const SkRecord& record) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(w, h);
    SkCanvas canvas(bitmap);
    canvas.clipRect(SkRect::MakeWH(SkIntToScalar(FLAGS_tile), SkIntToScalar(FLAGS_tile)));

    printf("%s %s\n", FLAGS_optimize ? "optimized" : "not-optimized", name);

    for (Dumper dumper(&canvas, record.count()); dumper.index() < record.count(); dumper.next()) {
        record.visit<void>(dumper.index(), dumper);
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics ag;

    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, FLAGS_skps[i])) {
            continue;
        }

        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(FLAGS_skps[i]));
        if (!stream) {
            SkDebugf("Could not read %s.\n", FLAGS_skps[i]);
            exit(1);
        }
        SkAutoTUnref<SkPicture> src(
                SkPicture::CreateFromStream(stream, sk_tools::LazyDecodeBitmap));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", FLAGS_skps[i]);
            exit(1);
        }
        const int w = src->width(), h = src->height();

        SkRecord record;
        SkRecorder canvas(SkRecorder::kWriteOnly_Mode, &record, w, h);
        src->draw(&canvas);


        if (FLAGS_optimize) {
            SkRecordOptimize(&record);
        }

        dump(FLAGS_skps[i], w, h, record);
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
