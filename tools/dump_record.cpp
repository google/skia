/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include "LazyDecodeBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkStream.h"

DEFINE_string2(skps, r, "", ".SKPs to dump.");
DEFINE_string(match, "", "The usual filters on file names to dump.");
DEFINE_bool2(optimize, O, false, "Run SkRecordOptimize before dumping.");

class Dumper {
public:
    Dumper() : fIndent(0) {}

    template <typename T>
    void operator()(const T& command) {
        this->printIndentedName(command);
    }

    void operator()(const SkRecords::Restore& command) {
        --fIndent;
        this->printIndentedName(command);
    }

    void operator()(const SkRecords::Save& command) {
        this->printIndentedName(command);
        ++fIndent;
    }

    void operator()(const SkRecords::SaveLayer& command) {
        this->printIndentedName(command);
        ++fIndent;
    }

private:
    template <typename T>
    void printIndentedName(const T& command) {
        for (int i = 0; i < fIndent; i++) {
            putchar('\t');
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

    int fIndent;
};


static void dump(const char* name, const SkRecord& record) {
    Dumper dumper;

    unsigned count = record.count();
    int digits = 0;
    while (count > 0) {
        count /= 10;
        digits++;
    }

    printf("%s %s\n", FLAGS_optimize ? "optimized" : "not-optimized", name);
    for (unsigned i = 0; i < record.count(); i++) {
        printf("%*d ", digits, i);
        record.visit<void>(i, dumper);
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

        SkRecord record;
        SkRecorder canvas(SkRecorder::kWriteOnly_Mode, &record, src->width(), src->height());
        src->draw(&canvas);

        if (FLAGS_optimize) {
            SkRecordOptimize(&record);
        }

        dump(FLAGS_skps[i], record);
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
