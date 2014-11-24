/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include "SkRecord.h"
#include "SkRecordDraw.h"

#include "DumpRecord.h"
#include "Timer.h"

namespace {

class Dumper {
public:
    explicit Dumper(SkCanvas* canvas, int count, bool timeWithCommand)
        : fDigits(0)
        , fIndent(0)
        , fIndex(0)
        , fDraw(canvas, NULL, NULL, 0, NULL)
        , fTimeWithCommand(timeWithCommand) {
        while (count > 0) {
            count /= 10;
            fDigits++;
        }
    }

    template <typename T>
    void operator()(const T& command) {
        Timer timer;
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
        if (!fTimeWithCommand) {
            printf("%6.1f ", time * 1000);
        }
        printf("%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            putchar('\t');
        }
        if (fTimeWithCommand) {
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
    int fIndex;
    SkRecords::Draw fDraw;
    const bool fTimeWithCommand;
};

}  // namespace

void DumpRecord(const SkRecord& record,
                  SkCanvas* canvas,
                  bool timeWithCommand) {
    Dumper dumper(canvas, record.count(), timeWithCommand);
    for (unsigned i = 0; i < record.count(); i++) {
        record.visit<void>(i, dumper);
    }
}
