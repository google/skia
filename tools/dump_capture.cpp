/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
#include "tools/flags/CommandLineFlags.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>

static DEFINE_string2(input, i, "", "Capture file(s) to dump.");
static DEFINE_bool2(assets, a, true, "Include assets in the dump.");
static DEFINE_string2(output, o, "", "Files to output text to");

static std::filesystem::path resolve_path(const char* rawPath) {
    std::filesystem::path p(rawPath);
    const char* bazel_wd = std::getenv("BUILD_WORKING_DIRECTORY");
    if (p.is_relative() && bazel_wd != nullptr) {
        p = std::filesystem::path(bazel_wd) / p;
    }
    return p;
}

// RecordDumper is a functor for inspecting canvas commands via SkRecord::visit()
// (see src/core/SkRecord.h).
//
// Specific overloads (e.g. operator()(const SkRecords::DrawRect&)) customize formatting
// or manage indentation state (Save/Restore). The generic template fallback handles all
// other SkRecords::* types by default via NameOf().
class RecordDumper {
public:
    // totalCommands is the number of operations in the record, used to calculate
    // the number of digits needed to format the command index column.
    RecordDumper(int totalCommands, FILE* output = stdout)
            : fDigits(0), fIndent(0), fIndex(0), fOutput(output) {
        while (totalCommands > 0) {
            totalCommands /= 10;
            fDigits++;
        }
        if (fDigits < 2) fDigits = 2;
    }

    template <typename T> void operator()(const T& command) { this->printName(NameOf(command)); }

    void operator()(const SkRecords::NoOp&) {}

    void operator()(const SkRecords::Restore& command) {
        if (fIndent > 0) --fIndent;
        this->printName(NameOf(command));
    }

    void operator()(const SkRecords::Save& command) {
        this->printName(NameOf(command));
        ++fIndent;
    }

    void operator()(const SkRecords::SaveLayer& command) {
        this->printName(NameOf(command));
        ++fIndent;
    }

    void operator()(const SkRecords::DrawRect& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawOval& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawOval [%g, %g, %g, %g]",
                 command.oval.left(),
                 command.oval.top(),
                 command.oval.right(),
                 command.oval.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawImage& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawImage [%g, %g]", command.left, command.top);
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawImageRect& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawImageRect dst=[%g, %g, %g, %g]",
                 command.dst.left(),
                 command.dst.top(),
                 command.dst.right(),
                 command.dst.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawPaint& command) {
        this->printName("DrawPaint (clear/paint)");
    }

    void operator()(const SkRecords::ClipRect& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "ClipRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        this->printName(buf);
    }

private:
    void printName(const char* name) {
        fprintf(fOutput, "      [%*d] ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; ++i) {
            fprintf(fOutput, "  ");
        }
        fprintf(fOutput, "%s\n", name);
        return;
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
        return "Unknown";
    }

    int fDigits;    // Number of digits for formatting the command index column.
    int fIndent;    // Current indentation level to represent Save/SaveLayer nesting depth.
    int fIndex;     // Zero-based index of the current command being dumped.
    FILE* fOutput;  // The file to write the output to. If null, print to std-out
};

void dump_assets(SkCapture::Metadata meta, sk_sp<SkCapture> capture, FILE* output = stdout) {
    fprintf(output,
            "--------------------------------------------------------------------------------\n");
    fprintf(output, "ASSETS SECTION (%u assets):\n", meta.numAssets);
    fprintf(output,
            "--------------------------------------------------------------------------------\n");

    for (uint32_t i = 0; i < meta.numAssets; ++i) {
        sk_sp<SkPicture> picture = capture->getAsset(i);
        if (!picture) {
            fprintf(output, "  Picture [%u]: <null>\n", i);
            continue;
        }

        SkRect cull = picture->cullRect();
        fprintf(output,
                "  Picture [%u]: Dimensions %dx%d (cull: [%g, %g, %g, %g])\n",
                i,
                SkScalarCeilToInt(cull.width()),
                SkScalarCeilToInt(cull.height()),
                cull.left(),
                cull.top(),
                cull.right(),
                cull.bottom());

        const int w = SkScalarCeilToInt(cull.width());
        const int h = SkScalarCeilToInt(cull.height());
        SkRecord record;
        SkRecordCanvas recCanvas(&record, w, h);
        picture->playback(&recCanvas);

        fprintf(output, "    Record count: %d operations\n", record.count());
        if (record.count() == 0) {
            fprintf(output, "    (Empty picture - snapped for idle surface)\n");
        } else {
            RecordDumper dumper(record.count(), output);
            for (int j = 0; j < record.count(); ++j) {
                record.visit(j, dumper);
            }
        }
        fprintf(output, "\n");
    }
}

void dump_timeline(SkCapture::Metadata meta, sk_sp<SkCapture> capture, FILE* output = stdout) {
    fprintf(output,
            "--------------------------------------------------------------------------------\n");
    fprintf(output, "TIMELINE SECTION (%u recordings):\n", meta.numRecordingCaptures);
    fprintf(output,
            "--------------------------------------------------------------------------------\n");
    for (uint32_t i = 0; i < meta.numRecordingCaptures; ++i) {
        const SkCapture::RecordingCapture* rec = capture->getRecordingCapture(i);
        if (!rec) {
            fprintf(output, "  Recording [%u]: <null>\n", i);
            continue;
        }
        fprintf(output, "  Recording [%u]: %d draw task(s)\n", i, rec->fDrawTasks.size());
        for (int j = 0; j < rec->fDrawTasks.size(); ++j) {
            uint32_t assetIdx = rec->fDrawTasks[j].fAssetIndex;
            sk_sp<SkPicture> pic = capture->getAsset(assetIdx);
            if (pic) {
                SkRect cull = pic->cullRect();
                fprintf(output,
                        "    [Task %d] -> Asset [%u] (Dimensions: %dx%d, cull: [%g, %g, %g, %g], "
                        "approxOps: %d)\n",
                        j,
                        assetIdx,
                        SkScalarCeilToInt(cull.width()),
                        SkScalarCeilToInt(cull.height()),
                        cull.left(),
                        cull.top(),
                        cull.right(),
                        cull.bottom(),
                        pic->approximateOpCount());
            } else {
                fprintf(output, "    [Task %d] -> Asset [%u] (<null>)\n", j, assetIdx);
            }
        }
        fprintf(output, "\n");
    }
}

void dump_capture_file(const char* path, bool include_assets = true, FILE* output = stdout) {
    if (!output) output = stdout;

    fprintf(output,
            "================================================================================\n");
    fprintf(output, "CAPTURE FILE: %s\n", path);
    fprintf(output,
            "================================================================================\n");

    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    if (!data) {
        fprintf(output, "Error: Could not read file '%s'\n\n", path);
        return;
    }

    fprintf(output, "File Size: %zu bytes\n", data->size());

    sk_sp<SkCapture> capture = SkCapture::MakeFromData(data);
    if (!capture) {
        fprintf(output, "Error: Failed to deserialize SkCapture from '%s'\n\n", path);
        return;
    }

    // Metadata Section
    SkCapture::Metadata meta = capture->getMetadata();
    fprintf(output, "Capture Format Version: %u\n", meta.version);
    fprintf(output, "Number of Recorded Assets: %u\n", meta.numAssets);
    fprintf(output, "Number of Recording Captures (Timeline): %u\n", meta.numRecordingCaptures);

    // Assets Section
    if (include_assets) dump_assets(meta, capture, output);

    // Timeline Section
    dump_timeline(meta, capture, output);

    fprintf(output,
            "================================================================================\n\n");
}

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage(
            R"(Dumps the contents and structure of SkCapture files (.capt). dump_capture
            Usage:
                dump_capture -i <file1.capt> [file2.capt ...]
                dump_capture -i <file.capt> --noassets
            )");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.isEmpty()) {
        CommandLineFlags::PrintUsage();
        return 1;
    }

    if (FLAGS_output.isEmpty()) {
        for (int i = 0; i < FLAGS_input.size(); ++i) {
            std::filesystem::path inPath = resolve_path(FLAGS_input[i]);
            dump_capture_file(inPath.c_str(), FLAGS_assets);
        }
        return 0;
    }

    if (FLAGS_output.size() != FLAGS_input.size()) {
        fprintf(stderr,
                "Error: Number of output files must match number of input files : output size "
                "'%d' != input size '%d'"
                "\n",
                FLAGS_output.size(),
                FLAGS_input.size());
        return 1;
    }

    for (int i = 0; i < FLAGS_input.size(); ++i) {
        std::filesystem::path inPath = resolve_path(FLAGS_input[i]);
        std::filesystem::path outPath = resolve_path(FLAGS_output[i]);

        FILE* out = fopen(outPath.c_str(), "w");
        if (!out) {
            fprintf(stderr, "Error: Failed to open output file '%s'\n", outPath.c_str());
            return 1;
        }

        dump_capture_file(inPath.c_str(), FLAGS_assets, out);

        if (out != stdout) {
            fclose(out);
        }
    }

    return 0;
}
