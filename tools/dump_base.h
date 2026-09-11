/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef dump_base_DEFINED
#define dump_base_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordCanvas.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkTime.h"
#include "tools/flags/CommandLineFlags.h"

#include <cstdio>
#include <optional>

// SkRecordDumper is a base visitor functor for inspecting and formatting canvas commands
// via SkRecord::visit() (see src/core/SkRecord.h).
//
// Specific overloads (DrawRect, ClipRect, DrawImage, etc.) format command parameters
// into text strings and pass them to visitingPrinter(name, ns).
// Derived classes implement visitingPrinter to handle indentation, indexing, timing,
// and writing to the desired output stream (fOutput).
class SkRecordDumper {
public:
    // Constructor for benchmarking / canvas playback (e.g. dump_record)
    SkRecordDumper(SkCanvas* canvas,
                   int totalCommands,
                   bool printTime = false,
                   FILE* output = stdout)
            : fDigits(0)
            , fIndent(0)
            , fIndex(0)
            , fOutput(output ? output : stdout)
            , fPrintTime(printTime) {
        while (totalCommands > 0) {
            totalCommands /= 10;
            fDigits++;
        }
        if (fDigits < 2) fDigits = 2;
        if (canvas) {
            fDraw.emplace(canvas, nullptr, nullptr, 0, nullptr);
        }
    }

    // Convenience constructor for offline dumping without canvas playback (e.g. dump_capture)
    SkRecordDumper(int totalCommands, FILE* output = stdout)
            : SkRecordDumper(nullptr, totalCommands, false, output) {}

    virtual ~SkRecordDumper() = default;

    template <typename T> void operator()(const T& command) {
        auto start = SkTime::GetNSecs();
        if (fDraw) {
            (*fDraw)(command);
        }
        print(command, SkTime::GetNSecs() - start);
    }

    void operator()(const SkRecords::NoOp&) {}

    template <typename T> void print(const T& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);
    }

    void print(const SkRecords::Restore& command, double ns) {
        if (fIndent > 0) --fIndent;
        this->visitingPrinter(NameOf(command), ns);
    }

    void print(const SkRecords::Save& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);
        ++fIndent;
    }

    void print(const SkRecords::SaveLayer& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);
        ++fIndent;
    }

    void print(const SkRecords::DrawPicture& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);

        ++fIndent;

        const SkRecord* record = SkPicturePriv::GetRecord(command.picture.get());
        for (int i = 0; i < record->count(); i++) {
            record->visit(i, *this);
        }

        --fIndent;
    }

    void print(const SkRecords::DrawAnnotation& command, double ns) {
        char buf[256];
        snprintf(buf,
                 sizeof(buf),
                 "DrawAnnotation [%g %g %g %g] %s",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom(),
                 command.key.c_str());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawRect& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawOval& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawOval [%g, %g, %g, %g]",
                 command.oval.left(),
                 command.oval.top(),
                 command.oval.right(),
                 command.oval.bottom());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawImage& command, double ns) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawImage [%g, %g]", command.left, command.top);
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawImageRect& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawImageRect dst=[%g, %g, %g, %g]",
                 command.dst.left(),
                 command.dst.top(),
                 command.dst.right(),
                 command.dst.bottom());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawPaint& command, double ns) {
        visitingPrinter("DrawPaint (clear/paint)", ns);
    }

    void print(const SkRecords::ClipRect& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "ClipRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        visitingPrinter(buf, ns);
    }

protected:
    void visitingPrinter(const char* name, double ns) {
        int us = (int)(ns * 1e-3);
        if (!fPrintTime) {
            fprintf(fOutput, "%6dus  ", us);
        }
        fprintf(fOutput, "%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            fprintf(fOutput, "    ");
        }
        if (fPrintTime) {
            fprintf(fOutput, "%6dus  ", us);
        }
        fprintf(fOutput, "%s\n", name);
    }

    // NameOf returns the string name of an SkRecord command type.
    // It uses the X-macro pattern via SK_RECORD_TYPES (defined in src/core/SkRecord.h):
    // SK_RECORD_TYPES(M) expands macro M over all SkRecord command types.
    // CASE(U) generates a case label for SkRecords::U##_Type returning its stringified name "#U".
    template <typename T> static const char* NameOf(const T&) {
#define CASE(U)               \
    case SkRecords::U##_Type: \
        return #U;
        switch (T::kType) { SK_RECORD_TYPES(CASE) }
#undef CASE
        SkDEBUGFAIL("Unknown T");
        return "Unknown T";
    }

    static const char* NameOf(const SkRecords::SaveLayer&) {
        return "\x1b[31;1mSaveLayer\x1b[0m";  // Bold red.
    }

    int fDigits;    // Number of digits for formatting the command index column.
    int fIndent;    // Current indentation level to represent Save/SaveLayer nesting depth.
    int fIndex;     // Zero-based index of the current command being dumped.
    FILE* fOutput;  // The file to write the output to. If null, print to stdout.
    bool fPrintTime;
    std::optional<SkRecords::Draw> fDraw;
};

#endif  // dump_base_DEFINED
