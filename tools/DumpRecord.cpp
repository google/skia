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
#include "SkTime.h"

namespace {

class Dumper {
public:
    explicit Dumper(SkCanvas* canvas, int count, bool timeWithCommand)
        : fDigits(0)
        , fIndent(0)
        , fIndex(0)
        , fDraw(canvas, nullptr, nullptr, 0, nullptr)
        , fTimeWithCommand(timeWithCommand) {
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

        if (auto bp = command.picture->asSkBigPicture()) {
            ++fIndent;

            const SkRecord& record = *bp->record();
            for (int i = 0; i < record.count(); i++) {
                record.visit(i, *this);
            }

            --fIndent;
        }
    }

private:
    template <typename T>
    void printNameAndTime(const T& command, double ns) {
        int us = (int)(ns * 1e-3);
        if (!fTimeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            printf("    ");
        }
        if (fTimeWithCommand) {
            printf("%6dus  ", us);
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
    for (int i = 0; i < record.count(); i++) {
        record.visit(i, dumper);
    }
}
