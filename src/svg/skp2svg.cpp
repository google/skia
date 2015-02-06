/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSVGCanvas.h"
#include "SkXMLWriter.h"

DEFINE_string2(input, i, "", "input skp file");
DEFINE_string2(output, o, "", "output svg file (optional)");

// return codes:
static const int kSuccess     = 0;
static const int kInvalidArgs = 1;
static const int kIOError     = 2;
static const int kNotAnSKP    = 3;

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Converts an SKP file to SVG.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.count() != 1) {
        SkDebugf("Missing input file\n");
        return kInvalidArgs;
    }

    SkFILEStream stream(FLAGS_input[0]);
    if (!stream.isValid()) {
        SkDebugf("Couldn't open file: %s\n", FLAGS_input[0]);
        return kIOError;
    }

    SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(&stream, &sk_tools::LazyDecodeBitmap));
    if (!SkToBool(pic.get())) {
        SkDebugf("Could not load SKP: %s\n", FLAGS_input[0]);
        return kNotAnSKP;
    }

    SkAutoTDelete<SkWStream> outStream;
    if (FLAGS_output.count() > 0) {
        SkFILEWStream* fileStream = SkNEW_ARGS(SkFILEWStream, (FLAGS_output[0]));
        if (!fileStream->isValid()) {
            SkDebugf("Couldn't open output file for writing: %s\n", FLAGS_output[0]);
            return kIOError;
        }
        outStream.reset(fileStream);
    } else {
        outStream.reset(SkNEW(SkDebugWStream));
    }

    SkAutoTDelete<SkXMLWriter> xmlWriter(SkNEW_ARGS(SkXMLStreamWriter, (outStream.get())));
    SkAutoTUnref<SkCanvas> svgCanvas(SkSVGCanvas::Create(pic->cullRect(), xmlWriter.get()));

    pic->playback(svgCanvas);

    return kSuccess;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
