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
#include "tools/dump_base.h"
#include "tools/flags/CommandLineFlags.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

static DEFINE_string2(input, i, "", "Capture file(s) to dump.");
static DEFINE_bool2(assets, a, true, "Include assets in the dump.");
static DEFINE_string2(output, o, "", "Files to output text to");

static std::string resolve_path(const char* rawPath) {
    std::filesystem::path p(rawPath);
    const char* bazel_wd = std::getenv("BUILD_WORKING_DIRECTORY");
    if (p.is_relative() && bazel_wd != nullptr) {
        p = std::filesystem::path(bazel_wd) / p;
    }
    return p.string();
}

void dump_assets(SkCapture::Metadata meta, sk_sp<SkCapture> capture, FILE* output = stdout) {
    if (!output) output = stdout;

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
            SkRecordDumper dumper(record.count(), output);
            for (int j = 0; j < record.count(); ++j) {
                record.visit(j, dumper);
            }
        }
        fprintf(output, "\n");
    }
}

void dump_timeline(SkCapture::Metadata meta, sk_sp<SkCapture> capture, FILE* output = stdout) {
    if (!output) output = stdout;

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
            std::string inPath = resolve_path(FLAGS_input[i]);
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
        std::string inPath = resolve_path(FLAGS_input[i]);
        std::string outPath = resolve_path(FLAGS_output[i]);

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
